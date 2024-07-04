#ifndef DREAM_H
#define DREAM_H

#include "QtPositioning/qgeoaddress.h"
#include <QString>
#include <QTime>
#include <QDate>
#include <QJsonObject>
#include <QUuid>
#include <QColor>

struct Dream {
    QString id;
    QString parentID;
    QList<QString> childIDs;

    // original properties
    QString originalTranscript;
    QDateTime recordingDateTime;
    QGeoAddress recordingLocation;
    int recordingLength = 0; // ms

    // generated properties
    QString revisedTranscript;
    QString title;
    QString oneWordDescription;
    bool isNightmare = false;
    bool isLucid = false;
    QList<QString> similarDreams;

    // meta
    bool isGenerated = false; // the parent dream keeps the original transcript, child dreams remove it

    // toggleable
    bool isFavorited = false;

    Dream();
    static Dream forkDream(Dream parentDream);

    QJsonObject toJson() const;
    void updateFromJson(const QJsonObject &obj);

    bool isValid() const {
        return recordingLength > 0 && recordingDateTime.isValid();
    }

    bool operator==(const Dream &other) const {
        return this->id == other.id;
    }

    bool operator<(const Dream& other) const {
        return recordingDateTime < other.recordingDateTime;
    }
};

#endif // DREAM_H
