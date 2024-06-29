#ifndef DREAMMANAGER_H
#define DREAMMANAGER_H

#include <QObject>
#include "QtCore/qjsonarray.h"
#include "dream.h"

class DreamManager : public QObject
{
    Q_OBJECT
public:
    explicit DreamManager(QObject *parent = nullptr);

    static DreamManager *self();

    void insertDream(const Dream &dream);

    void removeDream(const Dream &dream);
    void removeDream(const QString &dreamId);

    QList<Dream> getAllDreams();
    Dream getNewestOriginalDream();

    Dream getDream(const QString &dreamId) const;
    bool containsDream(const QString &dreamId) const;
    void clearDreams();

    QJsonObject getJsonObject();
    void loadJsonObject(const QJsonObject &jObj);

    QJsonArray getAllDreamsJson();
    QJsonArray getDreamsForDateJson(const QDate &date);
    QJsonArray getDreamsForDateRangeJson(const QDate &startDate, const QDate &endDate);

    void saveSettings();
    void loadSettings();

private:
    QMap<QString, Dream> idToDreamMap;

    QJsonArray dreamListToJson(QList<Dream> dreams);

    static DreamManager *singleton;

signals:

};

#endif // DREAMMANAGER_H
