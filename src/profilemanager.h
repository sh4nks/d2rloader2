
#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QDir>

#include "profile.h"

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

signals:

    // QML properties signals
    void itemChanged();

private:

    // Variables
    std::list<Account> m_accountList;
    void updateQmlItemList();

    // Aux functions
    QJsonDocument toJson();
    void loadFromJson(QJsonDocument doc);
};

#endif // PROFILEMANAGER_H
