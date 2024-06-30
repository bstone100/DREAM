#include "locationmanager.h"
#include <QGeoCoordinate>
#include "QGeoLocation"
#include "QGeoAddress"
#include "QtCore/qcoreapplication.h"
#include "QtCore/qpermissions.h"
#include "QMessageBox"

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
            emit locationObtained(locations.first().address().text());
        }
    }
    reply->deleteLater();

    source->stopUpdates();
}








