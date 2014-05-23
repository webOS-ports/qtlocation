/****************************************************************************
**
** Copyright (C) 2014 John Layt <jlayt@kde.org>
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

#ifndef QGEOPOSITIONINFOSOURCE_GEOCLUE2_P_H
#define QGEOPOSITIONINFOSOURCE_GEOCLUE2_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qgeopositioninfosource.h>
#include <QTimer>

//#define Q_LOCATION_GEOCLUE2_DEBUG

QT_BEGIN_NAMESPACE

class QGeoPositionInfoSourceGeoclue2 : public QGeoPositionInfoSource
{
    Q_OBJECT

public:
    QGeoPositionInfoSourceGeoclue2(QObject *parent = 0);
    ~QGeoPositionInfoSourceGeoclue2();

    // Reimp from QGeoPositionInfoSource
    void setUpdateInterval(int msec) Q_DECL_OVERRIDE;
    void setPreferredPositioningMethods(PositioningMethods methods) Q_DECL_OVERRIDE;
    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const Q_DECL_OVERRIDE;
    PositioningMethods supportedPositioningMethods() const Q_DECL_OVERRIDE;
    int minimumUpdateInterval() const Q_DECL_OVERRIDE;
    Error error() const Q_DECL_OVERRIDE;

    // Local methods
    void updatePosition(GeocluePositionFields fields, int timestamp, double latitude,
                        double longitude, double altitude, GeoclueAccuracy *accuracy);
    void regularUpdateFailed();

public Q_SLOTS:
    // Reimp from QGeoPositionInfoSource
    void startUpdates() Q_DECL_OVERRIDE;
    void stopUpdates() Q_DECL_OVERRIDE;
    void requestUpdate(int timeout = 5000) Q_DECL_OVERRIDE;

private Q_SLOTS:
    // Local slots
    void locationUpdated(const QDBusObjectPath &oldLocation, const QDBusObjectPath &newLocation);
    void requestUpdateTimeout();
    void positionProviderChanged(const QByteArray &service, const QByteArray &path);

private:
    // Local methods
    bool configurePositionSource();
    void cleanupPositionSource();
    void setOptions();

    // GeoClue2 proxy objects
    org::freedesktop::GeoClue2Manager *m_manager;
    org::freedesktop::GeoClue2Client *m_client;
    org::freedesktop::GeoClue2Client *m_lastLocation;

    QGeoPositionInfo m_lastPosition;
    QGeoPositionInfo m_lastSatellitePosition;
    QTimer m_requestTimer;
    Error m_error;
    bool m_regularUpdateTimedOut;
    bool m_running;
};

QT_END_NAMESPACE

#endif // QGEOPOSITIONINFOSOURCE_GEOCLUE2_P_H
