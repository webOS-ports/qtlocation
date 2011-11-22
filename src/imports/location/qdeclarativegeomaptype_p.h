/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtLocation module of the Qt Toolkit.
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

#ifndef QDECLARATIVEGEOMAPTYPE_H
#define QDECLARATIVEGEOMAPTYPE_H

#include <QtDeclarative/qdeclarative.h>
#include "maptype.h"
#include <QObject>

QT_BEGIN_NAMESPACE

class QDeclarativeGeoMapType : public QObject
{
    Q_OBJECT
    Q_ENUMS(MapStyle)

    Q_PROPERTY(MapStyle style READ style CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(bool mobile READ mobile CONSTANT)

public:
    enum MapStyle {
        NoMap = MapType::NoMap,
        StreetMap = MapType::StreetMap,
        SatelliteMapDay = MapType::SatelliteMapDay,
        SatelliteMapNight = MapType::SatelliteMapNight,
        TerrainMap = MapType::TerrainMap,
        HybridMap = MapType::HybridMap,
        TransitMap = MapType::TransitMap,
        GrayStreetMap = MapType::GrayStreetMap,
        CustomMap = 100
    };

    QDeclarativeGeoMapType(const MapType mapType, QObject* parent = 0);
    ~QDeclarativeGeoMapType();

    MapStyle style() const;
    QString name() const;
    QString description() const;
    bool mobile() const;

    const MapType mapType(){return mapType_;}

private:
    MapType mapType_;

};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE(QDeclarativeGeoMapType))

#endif