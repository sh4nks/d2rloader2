#include "gamelauncher.h"
#include "../accounts/profile.h"
#include "d2rloaderconfig.h"

#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#if defined(Q_OS_LINUX)
#include "gamelauncher_linux.h"
#endif

GameLauncher *GameLauncher::create(QObject *parent)
{
#if defined(Q_OS_LINUX)
    return new LinuxGameLauncher(parent);
#else
    Q_UNUSED(parent)
    return nullptr;
#endif
}

std::expected<QString, QString> GameLauncher::gameExecutable(const Profile *profile)
{
    const QString gamePath = profile->gamePath().isEmpty() ? D2RLoaderConfig::self()->gamePath() : profile->gamePath();
    if (gamePath.isEmpty()) {
        return std::unexpected(i18nc("@info", "No game directory is configured. Set it in the application settings."));
    }

    const QString executable = QDir(gamePath).filePath(QStringLiteral("D2R.exe"));
    if (!QFileInfo::exists(executable)) {
        return std::unexpected(i18nc("@info", "Could not find D2R.exe in \"%1\".", gamePath));
    }
    return executable;
}

std::expected<QStringList, QString> GameLauncher::gameArguments(const Profile *profile)
{
    QStringList arguments{QStringLiteral("-address"), RegionModel::server(profile->region())};

    switch (profile->authMethod()) {
    case AuthMethodModel::Password:
        if (profile->email().isEmpty() || profile->password().isEmpty()) {
            return std::unexpected(i18nc("@info", "Password authentication needs an email address and a password."));
        }
        arguments << QStringLiteral("-username") << profile->email() << QStringLiteral("-password") << profile->password();
        break;
    case AuthMethodModel::Token:
        if (profile->token().isEmpty()) {
            return std::unexpected(i18nc("@info", "Token authentication needs a token."));
        }
        arguments << QStringLiteral("-uid") << QStringLiteral("osi");
        break;
    case AuthMethodModel::Steam:
        break;
    }

    arguments << QProcess::splitCommand(profile->gameParameters());
    return arguments;
}

QStringList GameLauncher::redacted(QStringList arguments)
{
    for (const auto &secret : {QStringLiteral("-username"), QStringLiteral("-password")}) {
        const qsizetype index = arguments.indexOf(secret);
        if (index >= 0 && index + 1 < arguments.size()) {
            arguments[index + 1] = QStringLiteral("*****");
        }
    }
    return arguments;
}
