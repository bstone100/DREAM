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
    void insertDreams(const QList<Dream> &dreams);

    void removeDream(const Dream &dream);
    void removeDream(const QString &dreamId);

    QList<Dream> getAllDreams();

    Dream getDream(const QString &dreamId) const;
    Dream getParentDream(const QString &dreamId);
    bool containsDream(const QString &dreamId) const;
    void clearDreams();

    QJsonArray dreamListToJsonArray(QList<Dream> dreams, QList<QString> keys = {});
    QList<Dream> jsonArrayToDreamList(const QJsonArray &jsonArray);

    QJsonObject getJsonObject();
    void loadJsonObject(const QJsonObject &jObj);

    QList<Dream> getDreamsForDateRange(const QDate &startDate, const QDate &endDate);
    void jsonArrayRemoveIf(QJsonArray &jsonArray, QString key, QVariant value);

private:
    QMap<QString, Dream> idToDreamMap;

    static DreamManager *singleton;

signals:

};

#endif // DREAMMANAGER_H
