/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QPLACELOCATION_H
#define QPLACELOCATION_H

#include <QSharedDataPointer>
#include <QString>
#include <QVariant>
#include <QList>
#include "qgeocoordinate.h"
#include "qmobilityglobal.h"
#include "qplacealternativevalue.h"
#include "qplaceaddress.h"
#include "qplacegeoboundingbox.h"

QTM_BEGIN_NAMESPACE

class QPlaceLocationPrivate;

class Q_LOCATION_EXPORT QPlaceLocation
{
public:
    QPlaceLocation();
    QPlaceLocation(const QPlaceLocation &other);

    virtual ~QPlaceLocation();

    QPlaceLocation &operator=(const QPlaceLocation &other);

    bool operator==(const QPlaceLocation &other) const;
    bool operator!=(const QPlaceLocation &other) const {
        return !(other == *this);
    }

    QVariantHash additionalData() const;
    void setAdditionalData(const QVariantHash &data);
    QPlaceAddress address() const;
    void setAddress(const QPlaceAddress &address);
    QList<QPlaceAlternativeValue> alternativeLabels() const;
    void setAlternativeLabels(const QList<QPlaceAlternativeValue> &labels);
    QGeoCoordinate displayPosition() const;
    void setDisplayPosition(const QGeoCoordinate &position);
    QList<QGeoCoordinate> navigationPositions() const;
    void setNavigationPositions(const QList<QGeoCoordinate> &positions);
    QString label() const;
    void setLabel(const QString &label);
    QString locationId() const;
    void setLocationId(const QString &locationId);
    int locationScore() const;
    void setLocationScore(const int &score);
    QPlaceGeoBoundingBox mapView() const;
    void setMapView(const QPlaceGeoBoundingBox &coordinate);

private:
    QSharedDataPointer<QPlaceLocationPrivate> d;
};

QTM_END_NAMESPACE

#endif // QPLACELOCATION_H