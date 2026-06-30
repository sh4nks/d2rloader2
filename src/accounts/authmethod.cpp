#include "authmethod.h"
#include <QAbstractListModel>
#include <QMetaEnum>
#include <qhashfunctions.h>
#include <qtmetamacros.h>

AuthMethodModel::AuthMethodModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_items = {{Password, tr("Password"), QStringLiteral("password")},
               {Token, tr("Token"), QStringLiteral("token")},
               {Steam, tr("Steam"), QStringLiteral("steam")}};
};

int AuthMethodModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.count();
}

QVariant AuthMethodModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.count())
        return QVariant();

    const auto &item = m_items.at(index.row());

    if (role == NameRole)
        return item.displayName;
    else if (role == ValueRole)
        return static_cast<int>(item.value);
    else if (role == ShortcodeRole)
        return item.shortcode;

    return QVariant();
}

QHash<int, QByteArray> AuthMethodModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[ValueRole] = "value";
    roles[ShortcodeRole] = "shortcode";
    return roles;
}

// New Invokable lookup engine function to safely find the Enum via its server string
Q_INVOKABLE int AuthMethodModel::shortcodeToEnum(const QString &shortcode) const
{
    for (const auto &item : m_items) {
        if (item.shortcode == shortcode) {
            return static_cast<int>(item.value);
        }
    }
    return static_cast<int>(Password);
}

// Optional utility function to find index in model via server (useful for ComboBox initialization)
Q_INVOKABLE int AuthMethodModel::indexOfShortcode(const QString &shortcode) const
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items.at(i).shortcode == shortcode) {
            return i;
        }
    }
    return 0;
}

Q_INVOKABLE QString AuthMethodModel::getDisplayName(int authMethod) const
{
    for (const auto &item : m_items) {
        if (static_cast<int>(item.value) == authMethod) {
            return item.displayName;
        }
    }
    return tr("Unknown");
}

#include "moc_authmethod.cpp"
