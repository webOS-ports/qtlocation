/****************************************************************************
**
** Copyright (C) 2013 Jolla Ltd.
** Contact: Aaron McCarthy <aaron.mccarthy@jollamobile.com>
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtPositioning module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgeopositioninfosource_geoclue2_p.h"

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QSaveFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QVariantMap>
#include <QtCore/QtNumeric>

#ifdef Q_LOCATION_GEOCLUE2_DEBUG
#include <QDebug>
#endif

#include <dbus/dbus-glib.h>

QT_BEGIN_NAMESPACE

#define MINIMUM_UPDATE_INTERVAL 1000
#define UPDATE_TIMEOUT_COLD_START 120000

namespace
{

void position_changed(GeocluePosition *position, GeocluePositionFields fields, int timestamp,
                      double latitude, double longitude, double altitude,
                      GeoclueAccuracy *accuracy, QGeoPositionInfoSourceGeoclue2 *master)
{
    Q_UNUSED(position)

    if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE && fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
        master->updatePosition(fields, timestamp, latitude, longitude, altitude, accuracy);
    else
        master->regularUpdateFailed();
}

void position_callback(GeocluePosition *pos, GeocluePositionFields fields, int timestamp,
                       double latitude, double longitude, double altitude,
                       GeoclueAccuracy *accuracy, GError *error, gpointer userdata)
{
    Q_UNUSED(pos)

    if (error)
        g_error_free(error);

    QGeoPositionInfoSourceGeoclue2 *master =
        static_cast<QGeoPositionInfoSourceGeoclue2 *>(userdata);

    if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE && fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
        master->updatePosition(fields, timestamp, latitude, longitude, altitude, accuracy);
}

}

// Copied from http://www.freedesktop.org/software/geoclue/docs/geoclue-gclue-enums.html#GClueAccuracyLevel
enum GClueAccuracyLevel {
    GCLUE_ACCURACY_LEVEL_NONE,
    GCLUE_ACCURACY_LEVEL_COUNTRY,
    GCLUE_ACCURACY_LEVEL_CITY,
    GCLUE_ACCURACY_LEVEL_NEIGHBORHOOD,
    GCLUE_ACCURACY_LEVEL_STREET,
    GCLUE_ACCURACY_LEVEL_EXACT,
}

QGeoPositionInfoSourceGeoclue2::QGeoPositionInfoSourceGeoclue2(QObject *parent)
    : QGeoPositionInfoSource(parent),
      m_manager(0),
      m_client(0),
      m_lastLocation(0),
      m_error(NoError),
      m_regularUpdateTimedOut(false)
{
#ifndef QT_NO_DATASTREAM
    // Load the last known position
    QFile file(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
               QStringLiteral("/qtposition-geoclue2"));
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream out(&file);
        out >> m_lastPosition;
        out >> m_lastSatellitePosition;
    }
#endif

    // Set up the timer
    m_requestTimer.setSingleShot(true);
    QObject::connect(&m_requestTimer, SIGNAL(timeout()), this, SLOT(requestUpdateTimeout()));

    // Set up the GeoClue Manager and Client
    m_manager = new GeoClue2Manager(QStringLiteral("org.freedesktop.GeoClue2"),
                                    QStringLiteral("/org/freedesktop/GeoClue2"),
                                    QDBusConnection::systemBus(),
                                    this);
    if (m_manager && m_manager.isValid() {
        QDBusPendingReply<QDBusObjectPath> reply = m_manger.GetClient()
        reply.waitForFinished();
        if (reply.isValid()) {
            QDBusObjectPath clientPath = reply.value();
            m_client = new GeoClue2Client(QStringLiteral("org.freedesktop.GeoClue2"),
                                          clientPath.path(),
                                          QDBusConnection::systemBus(),
                                          this);
        }
    }

    if (m_client && m_client.isValid()) {
        m_client->setRequestedAccuracyLevel(GCLUE_ACCURACY_LEVEL_EXACT);
        m_client->setDistanceThreshold(0);
        connect(m_client,
                SIGNAL(LocationUpdated(const QDBusObjectPath&, const QDBusObjectPath&)),
                SLOT(locationUpdated(const QDBusObjectPath&, const QDBusObjectPath&)));
    }
}

QGeoPositionInfoSourceGeoclue2::~QGeoPositionInfoSourceGeoclue2()
{
#ifndef QT_NO_DATASTREAM
    // Save the last known position
    if (m_lastPosition.isValid()) {
        QSaveFile file(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                       QStringLiteral("/qtposition-geoclue2"));
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QDataStream out(&file);
            // Only save position and timestamp.
            out << QGeoPositionInfo(m_lastPosition.coordinate(), m_lastPosition.timestamp());
            out << QGeoPositionInfo(m_lastSatellitePosition.coordinate(), m_lastSatellitePosition.timestamp());
            file.commit();
        }
    }
#endif
}

// Reimp methods

void QGeoPositionInfoSourceGeoclue2::setUpdateInterval(int msec)
{
    if (msec == updateInterval() || msec < 0)
        return;

    // If interval is 0 then only update when change is detected
    // TODO Do we still need the minimum interval?
    if (msec != 0)
        msec = qMax(minimumUpdateInterval(), msec);
    QGeoPositionInfoSource::setUpdateInterval(msec);

    // If the Client is active, then change its update interval.
    if (m_client && m_client.active())
        geoclueChangeUpdateInterval();
}

void QGeoPositionInfoSourceGeoclue2::setPreferredPositioningMethods(PositioningMethods methods)
{
#ifdef Q_LOCATION_GEOCLUE2_DEBUG
    qDebug() << "QGeoPositionInfoSourceGeoclue2 requested to set methods to" << methods
             << ", currently set to:" << preferredPositioningMethods();
#endif

    if (methods == preferredPositioningMethods())
        return;

    QGeoPositionInfoSource::setPreferredPositioningMethods(methods);

    if (m_client == 0 || !m_client.active() || methods != preferredPositioningMethods())
        return;

    // If the GeoClue client is active then changing method won't have any effect, so stop it first
    if (!geoclueClientStop())
        return;
    geoclueChangeAccuracyLevel();
    geoclueClientStart();
    
#ifdef Q_LOCATION_GEOCLUE2_DEBUG
    qDebug() << "QGeoPositionInfoSourceGeoclue2 Requested GeoClue2 to set accuracy to " << accuracyLevel
             << ", was actually set to " << m_client->requestedAccuracyLevel();
#endif
}

QGeoPositionInfo QGeoPositionInfoSourceGeoclue2::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    if (fromSatellitePositioningMethodsOnly)
        return m_lastSatellitePosition;
    return m_lastPosition;
}

QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceGeoclue2::supportedPositioningMethods() const
{
    GClueAccuracyLevel accuracyLevel = m_manager->availableAccuracyLevel();
    if (accuracyLevel == GCLUE_ACCURACY_LEVEL_EXACT)
        return AllPositioningMethods;
    if (accuracyLevel == GCLUE_ACCURACY_LEVEL_NONE)
        return NoPositioningMethods;
    return NonSatellitePositioningMethods;
}

int QGeoPositionInfoSourceGeoclue2::minimumUpdateInterval() const
{
    return MINIMUM_UPDATE_INTERVAL;
}

QGeoPositionInfoSource::Error QGeoPositionInfoSourceGeoclue2::error() const
{
    return m_error;
}

void QGeoPositionInfoSourceGeoclue2::startUpdates()
{
    if (m_client == 0 || !m_client.isValid() || m_client.active())
        return;

    geoclueChangeUpdateInterval();
    geoclueChangeAccuracyLevel();
    geoclueClientStart();

    // Emit last known position on start.
    if (m_lastPosition.isValid()) {
        QMetaObject::invokeMethod(this, "positionUpdated", Qt::QueuedConnection,
                                  Q_ARG(QGeoPositionInfo, m_lastPosition));
    }
}

void QGeoPositionInfoSourceGeoclue2::stopUpdates()
{
    if (m_client == 0 || !m_client.active())
        return;

    // TODO Stop timer
    geoclueClientStop();
}

void QGeoPositionInfoSourceGeoclue2::requestUpdate(int timeout)
{
    // if active
    // - if updateinterval = 0 wait for timeout to see if new position has been updated, if yes do nothing, if no timeout
    // - if updateInterval > 0 set distanceThreshold to 0, wait for gc signal or timeout, set distanceThreshold back to max
    // if inactive
    // - start with updateInterval = 0
    // - if NonSatellitePositioningMethods then wait for first signal or timeout
    // - if SatellitePositioningMethods then wait for right accuracy or timeout

    // Or, simply wait out the timeout, check if m_lastPosition was in timeout, otherwise signal
    // - may mean extra positions signalled?  Don't need to wait the whole time?

    if (timeout < minimumUpdateInterval() && timeout != 0) {
        emit updateTimeout();
        return;
    }
    if (m_requestTimer.isActive()) {
#ifdef Q_LOCATION_GEOCLUE2_DEBUG
      qDebug() << "QGeoPositionInfoSourceGeoclue2 request timer was active, ignoring startUpdates.";
#endif
        return;
    }

    if (!hasMasterClient()) {
        configurePositionSource();
        setOptions();
    }

    // Create better logic for timeout value (specs leave it impl dependant).
    // Especially if there are active updates ongoing, there is no point of waiting
    // for whole cold start time.
    m_requestTimer.start(timeout ? timeout : UPDATE_TIMEOUT_COLD_START);

    if (m_pos)
        geoclue_position_get_position_async(m_pos, position_callback, this);
}

// Internal code
void locationUpdated(const QDBusObjectPath &oldLocation, const QDBusObjectPath &newLocation)
{
    delete m_lastLocation;
    m_lastLocation = new GeoClue2Location(QStringLiteral("org.freedesktop.GeoClue2"),
                                          newLocation.path(),
                                          QDBusConnection::systemBus(),
                                          this);
    if (m_lastLocation == 0)
        return;
    
}

bool QGeoPositionInfoSourceGeoclue2::geoclueClientStart()
{
    if (m_client == 0)
        return false;
    QDBusPendingReply<> reply = m_client->Start();
    reply.waitForFinished();
#ifdef Q_LOCATION_GEOCLUE2_DEBUG
    if (!reply.isValid())
        qDebug() << "QGeoPositionInfoSourceGeoclue2 GeoClue2::Client::Start failed!";
#endif
    return reply.isValid();
}

bool QGeoPositionInfoSourceGeoclue2::geoclueClientStop()
{
    if (m_client == 0)
        return false;
    QDBusPendingReply<> reply = m_client->Start();
    reply.waitForFinished();
#ifdef Q_LOCATION_GEOCLUE2_DEBUG
    if (!reply.isValid())
        qDebug() << "QGeoPositionInfoSourceGeoclue2 GeoClue2::Client::Stop failed!";
#endif
    return reply.isValid();
}

void QGeoPositionInfoSourceGeoclue2::geoclueChangeUpdateInterval()
{
    if (m_client == 0)
        return;
    if (updateInterval() == 0) {
        // Only update when changed
        m_client->setDistanceThreshold(0);
        //TODO disable timer
    } else {
        // GeoClue2 currently only supports auto-updates every x meters not every x seconds
        // Instead we need to set the update distance to max and manually ask for update when needed
        m_client->setDistanceThreshold(std::numeric_limits<uint>::max());
        //TODO set up timer
    }
}

// Note this assumes Client is not active
void QGeoPositionInfoSourceGeoclue2::geoclueChangeAccuracyLevel()
{
    if (m_client == 0)
        return;
    GClueAccuracyLevel accuracyLevel = GCLUE_ACCURACY_LEVEL_EXACT;
    if (preferredPositioningMethods() == NonSatellitePositioningMethods)
        accuracyLevel = GCLUE_ACCURACY_LEVEL_STREET;
    else if (preferredPositioningMethods() == NoPositioningMethods)
        accuracyLevel = GCLUE_ACCURACY_LEVEL_NONE;
    m_client->setRequestedAccuracyLevel(accuracyLevel);
}

void QGeoPositionInfoSourceGeoclue2::regularUpdateFailed()
{
#ifdef Q_LOCATION_GEOCLUE2_DEBUG
    qDebug() << "QGeoPositionInfoSourceGeoclue2 regular update failed.";
#endif

    if (m_running && !m_regularUpdateTimedOut) {
        m_regularUpdateTimedOut = true;
        emit updateTimeout();
    }
}

void QGeoPositionInfoSourceGeoclue2::updatePosition(GeocluePositionFields fields,
                                                    int timestamp, double latitude,
                                                    double longitude, double altitude,
                                                    GeoclueAccuracy *accuracy)
{
    if (m_requestTimer.isActive())
        m_requestTimer.stop();

    QGeoCoordinate coordinate(latitude, longitude);
    if (fields & GEOCLUE_POSITION_FIELDS_ALTITUDE)
        coordinate.setAltitude(altitude);

    m_lastPosition = QGeoPositionInfo(coordinate, QDateTime::fromTime_t(timestamp));

    if (accuracy) {
        double horizontalAccuracy = qQNaN();
        double verticalAccuracy = qQNaN();
        GeoclueAccuracyLevel accuracyLevel = GEOCLUE_ACCURACY_LEVEL_NONE;
        geoclue_accuracy_get_details(accuracy, &accuracyLevel, &horizontalAccuracy, &verticalAccuracy);

        m_lastPositionFromSatellite = accuracyLevel & GEOCLUE_ACCURACY_LEVEL_DETAILED;

        if (!qIsNaN(horizontalAccuracy))
            m_lastPosition.setAttribute(QGeoPositionInfo::HorizontalAccuracy, horizontalAccuracy);
        if (!qIsNaN(verticalAccuracy))
            m_lastPosition.setAttribute(QGeoPositionInfo::VerticalAccuracy, verticalAccuracy);
    }

    m_regularUpdateTimedOut = false;

    emit positionUpdated(m_lastPosition);

#ifdef Q_LOCATION_GEOCLUE2_DEBUG
    qDebug() << "Lat, lon, alt, speed:"
             << m_lastPosition.coordinate().latitude()
             << m_lastPosition.coordinate().longitude()
             << m_lastPosition.coordinate().altitude()
             << m_lastPosition.attribute(QGeoPositionInfo::GroundSpeed);
#endif

    // Only stop positioning if regular updates not active.
    if (!m_running) {
        cleanupPositionSource();
        releaseMasterClient();
    }
}

void QGeoPositionInfoSourceGeoclue2::cleanupPositionSource()
{
    if (m_pos) {
        g_object_unref(m_pos);
        m_pos = 0;
    }
}

void QGeoPositionInfoSourceGeoclue2::setOptions()
{
    if (!m_pos)
        return;

    QVariantMap options;
    options.insert(QStringLiteral("UpdateInterval"), updateInterval());

    GHashTable *gOptions = g_hash_table_new(g_str_hash, g_str_equal);

    for (QVariantMap::ConstIterator i = options.constBegin(); i != options.constEnd(); ++i) {
        char *key = qstrdup(i.key().toUtf8().constData());

        const QVariant v = i.value();

        GValue *value = new GValue;
        *value = G_VALUE_INIT;

        switch (v.userType()) {
        case QMetaType::QString:
            g_value_init(value, G_TYPE_STRING);
            g_value_set_string(value, v.toString().toUtf8().constData());
            break;
        case QMetaType::Int:
            g_value_init(value, G_TYPE_INT);
            g_value_set_int(value, v.toInt());
            break;
        default:
            qWarning("Unexpected type %d %s", v.userType(), v.typeName());
        }

        g_hash_table_insert(gOptions, key, value);
    }

    geoclue_provider_set_options(GEOCLUE_PROVIDER(m_pos), gOptions, 0);

    GHashTableIter iter;
    char *key;
    GValue *value;

    g_hash_table_iter_init(&iter, gOptions);
    while (g_hash_table_iter_next(&iter, reinterpret_cast<void **>(&key), reinterpret_cast<void **>(&value))) {
        delete[] key;
        delete value;
    }

    g_hash_table_destroy(gOptions);
}

void QGeoPositionInfoSourceGeoclue2::requestUpdateTimeout()
{
#ifdef Q_LOCATION_GEOCLUE2_DEBUG
    qDebug() << "QGeoPositionInfoSourceGeoclue2 requestUpdate timeout occurred.";
#endif
    // If we end up here, there has not been valid position update.
    emit updateTimeout();

    // Only stop positioning if regular updates not active.
    if (!m_running) {
        cleanupPositionSource();
        releaseMasterClient();
    }
}

void QGeoPositionInfoSourceGeoclue2::positionProviderChanged(const QByteArray &service, const QByteArray &path)
{
    if (m_pos)
        cleanupPositionSource();

    if (service.isEmpty() || path.isEmpty()) {
        if (!m_regularUpdateTimedOut) {
            m_regularUpdateTimedOut = true;
            emit updateTimeout();
        }
        return;
    }

    m_pos = geoclue_position_new(service.constData(), path.constData());
    if (m_pos) {
        if (m_running) {
            g_signal_connect(G_OBJECT(m_pos), "position-changed",
                             G_CALLBACK(position_changed), this);
        }

        // Get the current position immediately.
        geoclue_position_get_position_async(m_pos, position_callback, this);
        setOptions();
    }
}

bool QGeoPositionInfoSourceGeoclue2::configurePositionSource()
{
    switch (preferredPositioningMethods()) {
    case SatellitePositioningMethods:
        return createMasterClient(GEOCLUE_ACCURACY_LEVEL_DETAILED, GEOCLUE_RESOURCE_GPS);
    case NonSatellitePositioningMethods:
        return createMasterClient(GEOCLUE_ACCURACY_LEVEL_NONE, GeoclueResourceFlags(GEOCLUE_RESOURCE_CELL | GEOCLUE_RESOURCE_NETWORK));
    case AllPositioningMethods:
        return createMasterClient(GEOCLUE_ACCURACY_LEVEL_NONE, GEOCLUE_RESOURCE_ALL);
    default:
        qWarning("GeoPositionInfoSourceGeoClueMaster unknown preferred method.");
    }

    return false;
}

#include "moc_qgeopositioninfosource_geoclue2_p.cpp"

QT_END_NAMESPACE
