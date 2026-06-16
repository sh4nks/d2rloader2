
#include "profilemanager.h"
#include "../accounts/models/profile.h"
#include <QJsonObject>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qobject.h>

ProfileManager::ProfileManager()
{
    qDebug() << "(ProfileManager) Initialization ...";

    //  // Add some elements to the list as example
    //  clearList();
    //  addItem(0, "Batman", false, 75.0, "no_file");
    //  addItem(1, "Robin", false, 50.0, "no_file");
    //  addItem(2, "Joker", true, 100.0, "no_file");
}

ProfileManager::~ProfileManager()
{
}

void ProfileManager::clone(int id)
{
}

void ProfileManager::add(int id, QString name, bool checked, double score, QString filepath)
{
    // Profile* new_item = new Profile();
    // new_item->update(id, name, checked, score, filepath);
    // _item_list.insert(new_item);

    updateQmlItemList();
}

void ProfileManager::update(int id, QString name, bool checked, double score, QString filepath)
{
}

void ProfileManager::remove(int id, QString name, bool checked, double score, QString filepath)
{
}

void ProfileManager::updateQmlItemList()
{
}

void ProfileManager::save(QString file_path)
{
// Prepare file name and path
#ifdef __linux__
    file_path.remove(QString::fromUtf8("file://"));
#elif _WIN32
    file_path.remove(QString::fromUtf8("file:///"));
#endif
    QString full_file_path = file_path.contains(QString::fromUtf8(".json")) ? file_path : file_path.append(QString::fromUtf8(".json"));
    qDebug() << "(ProfileManager) Requested save list as " << full_file_path;

    // Get the current list as JSON
    QJsonDocument doc = toJson();

    // Save JSON
    QFile jsonFile(full_file_path);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
    jsonFile.close();
}

bool ProfileManager::load(QString file_path)
{
    if (!file_path.contains(QString::fromUtf8(".json"))) {
        qCritical() << "(ProfileManager) Error, the file should be a JSON file";
        return false;
    }

// Prepare file
#ifdef __linux__
    file_path.remove(QString::fromUtf8("file://"));
#elif _WIN32
    file_path.remove(QString::fromUtf8("file:///"));
#endif
    qDebug() << "(ProfileManager) Requested load the list " << file_path;

    // Load JSON file
    QFile jsonFile(file_path);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument doc = QJsonDocument().fromJson(jsonFile.readAll());

    // Parse it to the app list
    loadFromJson(doc);

    return true;
}

/** *********************************
 *  Auxiliar functions
 ** ********************************/
QJsonDocument ProfileManager::toJson()
{
    QJsonDocument doc;
    // QJsonArray objs_array;
    // for(const auto item : _item_list)
    // {
    //     QJsonObject json_obj;
    //     json_obj.insert("id", item->id());
    //     json_obj.insert("name", item->name());
    //     json_obj.insert("score", item->score());
    //     json_obj.insert("checked", item->checked());
    //     json_obj.insert("filepath", item->filepath());

    //     objs_array.push_back(json_obj);
    // }

    // QJsonDocument doc(objs_array);
    // //qDebug() << doc.toJson();
    return doc;
}

void ProfileManager::loadFromJson(QJsonDocument doc)
{
    // qDebug() << doc.toJson();
    QJsonArray objs_array = doc.array();
    qDebug() << "Loading " << objs_array.size() << " elements";

    for (const auto value : objs_array) {
        QJsonObject obj = value.toObject();
        auto profile = Profile::fromJson(obj);
        // add(obj["id"].toInt(), obj["name"].toString(), obj["checked"].toBool(), obj["score"].toDouble(), obj["filepath"].toString());
    }
}
