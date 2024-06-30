#ifndef LOCATIONMANAGER_H
#define LOCATIONMANAGER_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <QGeoCodingManager>
#include <QGeoServiceProvider>

class LocationManager : public QObject
{
    Q_OBJECT

public:
    static LocationManager* self();
    void requestUserLocation();

signals:
    void locationObtained(const QString &location);

private:
    explicit LocationManager(QObject *parent = nullptr);
    ~LocationManager();

    static LocationManager *singleton;
    QGeoPositionInfoSource *source = NULL;
    QGeoServiceProvider *serviceProvider = NULL;
    QGeoCodingManager *geoCoder = NULL;

    Qt::PermissionStatus permissionStatus = Qt::PermissionStatus::Undetermined;
private slots:
    void positionUpdated(const QGeoPositionInfo &info);
    void reverseGeocodeFinished();
};

#endif // LOCATIONMANAGER_H
