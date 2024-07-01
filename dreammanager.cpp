#include "dreammanager.h"
#include "QSettings"
#include "QJsonDocument"
#include "QJsonArray"


DreamManager *DreamManager::singleton = NULL;

DreamManager::DreamManager(QObject *parent)
    : QObject{parent}
{
    if (!singleton) {
        singleton = this;
    }
}

DreamManager *DreamManager::self()
{
    if (!singleton) {
        singleton = new DreamManager();
    }
    return singleton;
}

void DreamManager::insertDream(const Dream &dream) {
    if (!dream.isValid()) return;
    idToDreamMap.insert(dream.id, dream);
}

void DreamManager::insertDreams(const QList<Dream> &dreams)
{
    foreach (auto dream, dreams) {
        insertDream(dream);
    }
}

void DreamManager::removeDream(const Dream &dream)
{
    idToDreamMap.remove(dream.id);
}

void DreamManager::removeDream(const QString &dreamId) {
    idToDreamMap.remove(dreamId);
}

QList<Dream> DreamManager::getAllDreams()
{
    return idToDreamMap.values();
}

Dream DreamManager::getDream(const QString &dreamId) const {
    return idToDreamMap.value(dreamId);
}

bool DreamManager::containsDream(const QString &dreamId) const {
    return idToDreamMap.contains(dreamId);
}

void DreamManager::clearDreams() {
    idToDreamMap.clear();
}

// currently sent entirely to the LLM
QJsonObject DreamManager::getJsonObject()
{
    QJsonObject jObj;

    jObj["dreamArray"] = dreamListToJsonArray(getAllDreams());

    return jObj;
}

void DreamManager::loadJsonObject(const QJsonObject &jObj)
{
    clearDreams();

    QJsonArray dreamArray = jObj["dreamArray"].toArray();
    insertDreams(jsonArrayToDreamList(dreamArray));

}

QList<Dream> DreamManager::getDreamsForDateRange(const QDate &startDate, const QDate &endDate)
{
    auto dreams = getAllDreams();

    for (int i = dreams.size(); i >= 0; i--) {
        Dream dream = dreams.at(i);
        if (dream.recordingDateTime.date() < startDate || dream.recordingDateTime.date() > endDate) {
            dreams.removeAt(i);
        }
    }

    return dreams;
}

void DreamManager::jsonArrayRemoveIf(QJsonArray &jsonArray, QString key, QVariant value)
{
    for (int i = jsonArray.size(); i >= 0; i--) {
        QJsonObject jObj = jsonArray.at(i).toObject();
        if (jObj.value(key) == value) {
            jsonArray.removeAt(i);
        }
    }
}

// if keys is empty use all keys
QJsonArray DreamManager::dreamListToJsonArray(QList<Dream> dreams, QList<QString> keys)
{
    std::sort(dreams.begin(), dreams.end());

    QJsonArray dreamArray;
    foreach (auto dream, dreams) {
        QJsonObject dreamObj = dream.toJson();

        if (keys.isEmpty()) {
            dreamArray.append(dreamObj);
        } else {
            QJsonObject culledObj;
            foreach (auto key, keys) {
                if (dreamObj.contains(key)) {
                    culledObj[key] = dreamObj[key];
                }
            }
            dreamArray.append(culledObj);
        }
    }
    return dreamArray;
}

QList<Dream> DreamManager::jsonArrayToDreamList(const QJsonArray &jsonArray)
{
    QList<Dream> dreamList;
    foreach (auto val, jsonArray) {
        QJsonObject dreamObj = val.toObject();
        Dream dream;
        dream.updateFromJson(dreamObj);
        if (dream.isValid()) {
            dreamList.append(dream);
        }
    }
    return dreamList;
}












