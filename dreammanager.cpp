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

    jObj["dreamArray"] = getAllDreamsJson();

    return jObj;
}

void DreamManager::loadJsonObject(const QJsonObject &jObj)
{
    clearDreams();

    QJsonArray dreamArray = jObj["dreamArray"].toArray();

    for (int i = 0; i < dreamArray.size(); i++) {
        QJsonObject dreamObject = dreamArray.at(i).toObject();
        Dream dream;
        dream.updateFromJson(dreamObject);
        if (!dream.isValid()) continue;
        insertDream(dream);
    }
}

QJsonArray DreamManager::getAllDreamsJson()
{
    auto dreams = getAllDreams();
    return dreamListToJson(dreams);
}

QJsonArray DreamManager::getDreamsForDateJson(const QDate &date)
{
    return getDreamsForDateRangeJson(date, date);
}

QJsonArray DreamManager::getDreamsForDateRangeJson(const QDate &startDate, const QDate &endDate)
{
    auto dreams = getAllDreams();

    // cull events
    dreams.removeIf([&](const Dream &dream){
        if (dream.recordingDateTime.date() < startDate || dream.recordingDateTime.date() > endDate) {
            return true;
        }
        return false;
    });

    return dreamListToJson(dreams);
}

QJsonArray DreamManager::dreamListToJson(QList<Dream> dreams)
{
    std::sort(dreams.begin(), dreams.end());

    QJsonArray dreamArray;
    foreach (auto dream, dreams) {
        if (!dream.isGenerated) continue;

        QJsonObject dreamObj = dream.toJson();

        QJsonObject culledObj;
        culledObj["id"] = dreamObj["id"];
        culledObj["title"] = dreamObj["title"];
        culledObj["isNightmare"] = dreamObj["isNightmare"];
        culledObj["isLucid"] = dreamObj["isLucid"];

        dreamArray.append(culledObj);
    }
    return dreamArray;
}

void DreamManager::saveSettings()
{
    QSettings settings;
    QJsonDocument doc(getJsonObject());
    settings.setValue("dreamManager", QString::fromUtf8(doc.toJson()));
}

void DreamManager::loadSettings()
{
    QSettings settings;
    QString eventsString = settings.value("dreamManager").toString();
    if (eventsString != "") {
        QJsonDocument doc = QJsonDocument::fromJson(eventsString.toUtf8());
        loadJsonObject(doc.object());
    }
}












