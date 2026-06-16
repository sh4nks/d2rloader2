
#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QDebug>
#include <QDir>
#include <QObject>
#include <QString>

#include "models/profile.h"

using namespace std;

class ProfileManager : public QObject
{
    Q_OBJECT

public:
    // Constructor
    ProfileManager();

    // Destuctor
    ~ProfileManager();

    // QML Invokable standalone functions
    Q_INVOKABLE void clone(int id);
    Q_INVOKABLE void add(int id, QString name, bool checked, double score, QString filepath);
    Q_INVOKABLE void update(int id, QString name, bool checked, double score, QString filepath);
    Q_INVOKABLE void remove(int id, QString name, bool checked, double score, QString filepath);
    Q_INVOKABLE void save(QString file_path);
    Q_INVOKABLE bool load(QString file_path);

Q_SIGNALS:

    // QML properties signals
    void itemChanged();

private:
    // Variables
    QList<Profile> m_profileList;
    void updateQmlItemList();

    // Aux functions
    QJsonDocument toJson();
    void loadFromJson(QJsonDocument doc);
};

#endif // PROFILEMANAGER_H
