#include "gamelauncher.h"
#include "../accounts/gamesettings.h"
#include "../accounts/profile.h"
#include "../core/logging.h"
#include "d2rloaderconfig.h"

#include <KLocalizedString>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

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

void GameLauncher::addProfileEnvironment(QProcessEnvironment &environment, const Profile *profile)
{
    const QStringList lines = profile->environmentVariables().split(u'\n');
    for (const QString &line : lines) {
        const QString variable = line.trimmed();
        const qsizetype separator = variable.indexOf(u'=');
        if (separator > 0) {
            environment.insert(variable.left(separator).trimmed(), variable.mid(separator + 1).trimmed());
        } else if (!variable.isEmpty()) {
            qCWarning(LOG_GAME) << "ignoring environment line without NAME=value:" << variable;
        }
    }
}

void GameLauncher::launchSteamUrl(Profile *profile, const QStringList &arguments)
{
    const QUrl url(QStringLiteral("steam://run/%1//%2/").arg(GameSettings::SteamAppId, arguments.join(u' ')));
    qCInfo(LOG_GAME) << "launching" << profile->profileName() << "through" << url;
    if (!QDesktopServices::openUrl(url)) {
        Q_EMIT failed(profile, i18nc("@info", "Could not start Steam."));
        return;
    }
    Q_EMIT launched(profile, 0);
}
