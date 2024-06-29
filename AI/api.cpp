#include "api.h"
#include "openai_message.h"
#include "openai_request.h"
#include "QDate"
#include "QJsonObject"
#include "QtCore/qjsonarray.h"
#include "QtCore/qjsondocument.h"
#include "../mainwindow.h"
#include "../dreammanager.h"

QList<APITool> API::toolList = {};

API::API()
{

}

QList<APITool> API::tools()
{
    if (toolList.isEmpty()) {
        generateTools();
    }
    return toolList;
}

void API::generateTools()
{
    QJsonObject parametersPropertiesObject {
        {"revisedTranscript", QJsonObject{{"type", "string"}, {"description",
            "The section of the original transcript relevant to this dream with all grammar fixed."}}},
        {"title", QJsonObject{{"type", "string"}, {"description", "The title of the dream."}}},
        {"oneWordDescription", QJsonObject{{"type", "string"}, {"description", "A one-word description of the dream."}}},
        {"isNightmare", QJsonObject{{"type", "boolean"}, {"description", "Indicates if the dream is a nightmare."}}},
        {"isLucid", QJsonObject{{"type", "boolean"}, {"description", "Indicates if the dream is lucid."}}},
        {"similarDreams", QJsonObject{
          {"type", "array"},
          {"items", QJsonObject{
                        {"type", "string"},
                        {"description", "The unique identifier of a dream."}
                    }},
          {"description", "List of IDs of similar dreams. Can be empty."}
        }}
    };

    QJsonObject parametersObject {
        {"type", "object"},
        {"properties", parametersPropertiesObject},
        {"required", QJsonArray{"revisedTranscript", "title", "oneWordDescription", "isNightmare", "isLucid"}}
    };

    QJsonObject functionObject {
        {"name", "generateDreamData"},
        {"description", "Generate data for an individual dream found within a transcript of the dreams the user had last night."},
        {"parameters", parametersObject}
    };

    QJsonObject generateDreamDataDescription {
        {"type", "function"},
        {"function", functionObject}
    };

    toolList.append(APITool{&API::generateDreamData, generateDreamDataDescription});


    QJsonObject getDreamsInRangeParametersPropertiesObject {
        {"startDate", QJsonObject{{"type", "string"}, {"description", "The start date of the range, in yyyy-MM-dd format"}}},
        {"endDate", QJsonObject{{"type", "string"}, {"description", "The end date of the range, in yyyy-MM-dd format"}}}
    };

    QJsonObject getDreamsInRangeParametersObject {
        {"type", "object"},
        {"properties", getDreamsInRangeParametersPropertiesObject},
        {"required", QJsonArray{"startDate", "endDate"}}
    };

    QJsonObject getDreamsInRangeFunctionObject {
        {"name", "getDreamsInRange"},
        {"description", "Retrieve dreams within a specified date range"},
        {"parameters", getDreamsInRangeParametersObject}
    };

    QJsonObject getDreamsInRangeDescription {
        {"type", "function"},
        {"function", getDreamsInRangeFunctionObject}
    };

    toolList.append(APITool{&API::getDreamsInRange, getDreamsInRangeDescription});
}


QJsonArray API::getToolsJsonArray()
{
    QJsonArray toolArray;
    foreach (APITool tool, tools()) {
        toolArray.append(tool.description);
    }
    return toolArray;
}

APITool API::getToolByName(const QString &name)
{
    foreach (APITool tool, tools()) {
        if (tool.getName() == name) {
            return tool;
        }
    }
    return APITool();
}

void API::processToolCalls(const QJsonArray &toolCalls, OpenAIRequest *chatRequest)
{
    if (toolCalls.isEmpty()) return;

    for (int i = 0; i < toolCalls.size(); i++) {
        QJsonObject toolCall = toolCalls.at(i).toObject();
        QJsonObject function = toolCall["function"].toObject();

        // call the function
        QString functionName = function["name"].toString();
        APITool functionToCall = API::getToolByName(functionName);

        QString argumentsStr = function["arguments"].toString();
        QJsonDocument doc = QJsonDocument::fromJson(argumentsStr.toUtf8());
        QJsonObject functionArgs = doc.object();

        printToolCall(functionName, functionArgs);

        QString functionResponse;
        if (functionToCall.isValid()) {
            functionResponse = functionToCall.execute(functionArgs);
        } else {
            functionResponse = functionName + " is not a valid function.";
        }
        qDebug() << functionResponse;

        // append the function response to conversation
        OpenAIMessage *toolMessage = new OpenAIMessage(functionResponse, OpenAIMessage::Role::Tool);
        toolMessage->setTool_call_id(toolCall["id"].toString());
        chatRequest->addMessage(toolMessage);
    }

    // request that the responses be summarized or that more function calls be made
    chatRequest->execute();

    MainWindow::self()->saveSettings();
}

void API::printToolCall(const QString &name, const QJsonObject &args)
{
    QStringList debugStringList{name, "("};
    for (auto it = args.begin(); it != args.end(); ++it) {
    debugStringList << it.value().toVariant().typeName() << " " << it.value().toString() << ", ";
    }
    debugStringList << ")";
    qDebug() << debugStringList.join("");
}

// single dream object is created initially when recording is stopped
// this can be morphed into multiple dream objects forked from the original
// will maintain the original object until llm is done generating then remove it
QString API::generateDreamData(const QJsonObject &jsonObject)
{
    Dream parentDream = DreamManager::self()->getNewestOriginalDream();

    Dream childDream = Dream::forkDream(parentDream);
    childDream.updateFromJson(jsonObject);
    DreamManager::self()->insertDream(childDream);

    MainWindow::self()->updateWidgets();

    return "Dream data successfully generated.";
}

// only show certain information
// must alredy be generated

//    {"id", id},
//    {"title", title},
//    {"isNightmare", isNightmare},
//    {"isLucid", isLucid},
QString API::getDreamsInRange(const QJsonObject &jsonObject)
{
    QDate startDate = QDate::fromString(jsonObject["startDate"].toString(), "yyyy-MM-dd");
    QDate endDate = QDate::fromString(jsonObject["endDate"].toString(), "yyyy-MM-dd");

    QJsonArray dreamArray = DreamManager::self()->getDreamsForDateRangeJson(startDate, endDate);

    QJsonDocument doc(dreamArray);
    return doc.toJson(QJsonDocument::Compact);
}












