#pragma once
#include <QAbstractListModel>
#include <QMetaEnum>
#include <QString>
#include <QVector>
#include <qqmlintegration.h>

class RegionModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
QML_SINGLETON // Tells Qt to manage this class as a single global instance

    public : enum Region {
        Europe = 0,
        Americas,
        Asia
    };
    Q_ENUM(Region)

    enum Roles {
        NameRole = Qt::UserRole + 1,
        ValueRole,
        ServerRole
    };

    struct RegionItem {
        Region value;
        QString displayName;
        QString server;

        RegionItem(Region v, const QString &d, const QString &s)
            : value(v)
            , displayName(d)
            , server(s)
        {
        }
    };

    explicit RegionModel(QObject *parent = nullptr);

    static QString server(Region region);

    // QAbstractListModel interface overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Invokable utility functions
    Q_INVOKABLE int serverToEnum(const QString &server) const;
    Q_INVOKABLE int indexOfServer(const QString &server) const;
    Q_INVOKABLE QString getDisplayName(int region) const;

private:
    QVector<RegionItem> m_items;
};
