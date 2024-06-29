#include "dream.h"
#include "QJsonArray"
#include "dreammanager.h"

Dream::Dream()
{
    id = QString::number(QUuid::createUuid().data1);
}

// make child and update parent and child
// return child
Dream Dream::forkDream(Dream parentDream)
{
    Dream childDream;
    QString childID = childDream.id;

    childDream = parentDream;
    childDream.originalTranscript = "";
    childDream.isGenerated = true;
    childDream.id = childID;
    childDream.parentID = parentDream.id;

    DreamManager::self()->insertDream(childDream);

    parentDream.childIDs.append(childID);

    DreamManager::self()->insertDream(parentDream);

    return childDream;
}

QJsonObject Dream::toJson() const {
    return QJsonObject{
        {"id", id},
        {"parentID", parentID},
        {"originalTranscript", originalTranscript},
        {"recordingDateTime", recordingDateTime.toString("yyyy-MM-dd HH:mm")},
        {"recordingLocation", recordingLocation},
        {"recordingLength", recordingLength},
        {"revisedTranscript", revisedTranscript},
        {"title", title},
        {"oneWordDescription", oneWordDescription},
        {"isNightmare", isNightmare},
        {"isLucid", isLucid},
        {"similarDreams", QJsonArray::fromStringList(similarDreams)},
        {"isGenerated", isGenerated},
        {"isFavorited", isFavorited}
    };
}

void Dream::updateFromJson(const QJsonObject &obj)
{
    if (obj.contains("originalTranscript")) {
        originalTranscript = obj["originalTranscript"].toString();
    }
    if (obj.contains("recordingDateTime")) {
        recordingDateTime = QDateTime::fromString(obj["recordingDateTime"].toString(), "yyyy-MM-dd HH:mm");
    }
    if (obj.contains("recordingLocation")) {
        recordingLocation = obj["recordingLocation"].toString();
    }
    if (obj.contains("recordingLength")) {
        recordingLength = obj["recordingLength"].toInt();
    }
    if (obj.contains("revisedTranscript")) {
        revisedTranscript = obj["revisedTranscript"].toString();
    }
    if (obj.contains("title")) {
        title = obj["title"].toString();
    }
    if (obj.contains("oneWordDescription")) {
        oneWordDescription = obj["oneWordDescription"].toString();
    }
    if (obj.contains("isNightmare")) {
        isNightmare = obj["isNightmare"].toBool();
    }
    if (obj.contains("isLucid")) {
        isLucid = obj["isLucid"].toBool();
    }
    if (obj.contains("similarDreams")) {
        similarDreams = QVector<QString>::fromList(obj["similarDreams"].toVariant().toStringList());
    }
    if (obj.contains("isGenerated")) {
        isGenerated = obj["isGenerated"].toBool();
    }
    if (obj.contains("isFavorited")) {
        isFavorited = obj["isFavorited"].toBool();
    }
}








