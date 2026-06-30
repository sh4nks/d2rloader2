#pragma once
#include <QAbstractListModel>
#include <QMetaEnum>
#include <QString>
#include <QVector>
#include <qhashfunctions.h>
#include <qqmlintegration.h>

class AuthMethodModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
QML_SINGLETON // Tells Qt to manage this class as a single global instance

    public : enum AuthMethod {
        Password = 0,
        Token,
        Steam
    };
    Q_ENUM(AuthMethod)

    enum Roles {
        NameRole = Qt::UserRole + 1,
        ValueRole,
        ShortcodeRole
    };

    struct AuthMethodItem {
        AuthMethod value;
        QString displayName;
        QString shortcode;

        AuthMethodItem(AuthMethod v, const QString &d, const QString &s)
            : value(v)
            , displayName(d)
            , shortcode(s)
        {
        }
    };

    explicit AuthMethodModel(QObject *parent = nullptr);

    // QAbstractListModel interface overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Invokable utility functions
    Q_INVOKABLE int shortcodeToEnum(const QString &shortcode) const;
    Q_INVOKABLE int indexOfShortcode(const QString &shortcode) const;
    Q_INVOKABLE QString getDisplayName(int authMethod) const;

private:
    QVector<AuthMethodItem> m_items;
};
