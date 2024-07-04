#include "locationmanager.h"
#include <QGeoCoordinate>
#include "QGeoLocation"
#include "QGeoAddress"
#include "QtCore/qcoreapplication.h"
#include "QtCore/qpermissions.h"
#include "QMessageBox"
#include "QJsonObject"

LocationManager* LocationManager::singleton = nullptr;

LocationManager* LocationManager::self()
{
    if (!singleton) {
        singleton = new LocationManager;
    }
    return singleton;
}

LocationManager::LocationManager(QObject *parent)
    : QObject(parent)
{
    QStringList providerList = QGeoServiceProvider::availableServiceProviders();

    foreach (auto entry, providerList) {
        serviceProvider = new QGeoServiceProvider(entry);
        if (serviceProvider) {
            geoCoder = serviceProvider->geocodingManager();
            if (geoCoder) {
                break;
            }
        }
    }
}

LocationManager::~LocationManager()
{
    if (source) {
        delete source;
    }
}

void LocationManager::requestUserLocation()
{
#if QT_CONFIG(permissions)
    QLocationPermission locationPermission;
    permissionStatus = qApp->checkPermission(locationPermission);

    switch (permissionStatus) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(locationPermission, this, &LocationManager::requestUserLocation);
        return;
    case Qt::PermissionStatus::Denied:
        QMessageBox::warning(NULL, "Permission Error", "Location permission is not granted!");
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }

    if (!source) {
        source = QGeoPositionInfoSource::createDefaultSource(this);
        connect(source, &QGeoPositionInfoSource::positionUpdated, this, &LocationManager::positionUpdated);
    }

    source->startUpdates();
#endif
}

void LocationManager::positionUpdated(const QGeoPositionInfo &info)
{
    if (info.isValid()) {
        QGeoCoordinate coord = info.coordinate();
        qDebug() << coord;
        if (geoCoder) {
            QGeoCodeReply *reply = geoCoder->reverseGeocode(coord);
            connect(reply, &QGeoCodeReply::finished, this, &LocationManager::reverseGeocodeFinished);
        }
    }
}

void LocationManager::reverseGeocodeFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply->error() == QGeoCodeReply::NoError) {
        auto locations = reply->locations();
        if (!locations.isEmpty()) {
            emit locationObtained(locations.first().address());
        }
    }
    reply->deleteLater();

    source->stopUpdates();
}

QJsonObject LocationManager::geoAddressToJson(const QGeoAddress &address)
{
    QJsonObject json;
    json["text"] = address.text();
    json["country"] = address.country();
    json["countryCode"] = address.countryCode();
    json["state"] = address.state();
    json["county"] = address.county();
    json["city"] = address.city();
    json["district"] = address.district();
    json["postalCode"] = address.postalCode();
    json["street"] = address.street();
    json["streetNumber"] = address.streetNumber();
    return json;
}

QGeoAddress LocationManager::jsonToGeoAddress(const QJsonObject &json)
{
    QGeoAddress address;
    if (json.contains("text") && json["text"].isString())
        address.setText(json["text"].toString());
    if (json.contains("country") && json["country"].isString())
        address.setCountry(json["country"].toString());
    if (json.contains("countryCode") && json["countryCode"].isString())
        address.setCountryCode(json["countryCode"].toString());
    if (json.contains("state") && json["state"].isString())
        address.setState(json["state"].toString());
    if (json.contains("county") && json["county"].isString())
        address.setCounty(json["county"].toString());
    if (json.contains("city") && json["city"].isString())
        address.setCity(json["city"].toString());
    if (json.contains("district") && json["district"].isString())
        address.setDistrict(json["district"].toString());
    if (json.contains("postalCode") && json["postalCode"].isString())
        address.setPostalCode(json["postalCode"].toString());
    if (json.contains("street") && json["street"].isString())
        address.setStreet(json["street"].toString());
    if (json.contains("streetNumber") && json["streetNumber"].isString())
        address.setStreetNumber(json["streetNumber"].toString());
    return address;
}

QString LocationManager::formattedAddress(const QGeoAddress &address)
{
    if (address.isEmpty()) return "";

    // Remove zip code, county, and district
//    QStringList parts = address.text().split(", ");

    QString result;
//    result += parts.at(0);
//    result += " ";
//    result += parts.at(1);
//    result += ", ";
//    result += address.city();
//    result += ", ";
//    result += address.state().isEmpty() ? address.country() : address.state();

    result = address.city() + ", " + (address.state().isEmpty() ? address.country() : address.state());

    return result;
}












