/****************************************************************************
 **
 ** Copyright (C) 2015 The Qt Company Ltd.
 ** Contact: http://www.qt.io/licensing/
 **
 ** This file is part of the QtLocation module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL3$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see http://www.qt.io/terms-conditions. For further
 ** information use the contact form at http://www.qt.io/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 3 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPLv3 included in the
 ** packaging of this file. Please review the following information to
 ** ensure the GNU Lesser General Public License version 3 requirements
 ** will be met: https://www.gnu.org/licenses/lgpl.html.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 2.0 or later as published by the Free
 ** Software Foundation and appearing in the file LICENSE.GPL included in
 ** the packaging of this file. Please review the following information to
 ** ensure the GNU General Public License version 2.0 requirements will be
 ** met: http://www.gnu.org/licenses/gpl-2.0.html.
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "qgeotiledmap_nokia.h"
#include "qgeotiledmappingmanagerengine_nokia.h"
#include "qgeomapcontroller_p.h"

#include <QDebug>
#include <QObject>
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QImage>
#include <QRect>

#include <QStaticText>

QT_BEGIN_NAMESPACE

/*!
 Constructs a new tiled map data object, which stores the map data required by
 \a geoMap and makes use of the functionality provided by \a engine.
 */
QGeoTiledMapNokia::QGeoTiledMapNokia(QGeoTiledMappingManagerEngineNokia *engine, QObject *parent /*= 0*/) :
    QGeoTiledMap(engine, parent),
    logo(":/images/logo.png") // HERE logo image
{}

QGeoTiledMapNokia::~QGeoTiledMapNokia() {}

void QGeoTiledMapNokia::evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles)
{
    const int spaceToLogo = 4;
    const int blurRate = 1;
    const int fontSize = 10;

    QGeoTiledMappingManagerEngineNokia *engineNokia = static_cast<QGeoTiledMappingManagerEngineNokia *>(engine());
    const QString copyrightsString = engineNokia->evaluateCopyrightsText(activeMapType(), mapController()->zoom(), visibleTiles);

    if (width() > 0 && height() > 0 && ((copyrightsString.isNull() && copyrightsSlab.isNull()) || copyrightsString != lastCopyrightsString)) {
        QFont font("Sans Serif");
        font.setPixelSize(fontSize);
        font.setStyleHint(QFont::SansSerif);
        font.setWeight(QFont::Bold);

        QRect textBounds = QFontMetrics(font).boundingRect(0, 0, width(), height(), Qt::AlignBottom | Qt::AlignLeft | Qt::TextWordWrap, copyrightsString);

        copyrightsSlab = QImage(logo.width() + textBounds.width() + spaceToLogo + blurRate * 2,
                                qMax(logo.height(), textBounds.height() + blurRate * 2),
                                QImage::Format_ARGB32_Premultiplied);
        copyrightsSlab.fill(Qt::transparent);

        QPainter painter(&copyrightsSlab);
        painter.drawImage(QPoint(0, copyrightsSlab.height() - logo.height()), logo);
        painter.setFont(font);
        painter.setPen(QColor(0, 0, 0, 64));
        painter.translate(spaceToLogo + logo.width(), -blurRate);
        for (int x=-blurRate; x<=blurRate; ++x) {
            for (int y=-blurRate; y<=blurRate; ++y) {
                painter.drawText(x, y, textBounds.width(), copyrightsSlab.height(),
                                 Qt::AlignBottom | Qt::AlignLeft | Qt::TextWordWrap,
                                 copyrightsString);
            }
        }
        painter.setPen(Qt::white);
        painter.drawText(0, 0, textBounds.width(), copyrightsSlab.height(),
                         Qt::AlignBottom | Qt::AlignLeft | Qt::TextWordWrap,
                         copyrightsString);
        painter.end();

        lastCopyrightsString = copyrightsString;
    }

    emit copyrightsChanged(copyrightsSlab);
}

int QGeoTiledMapNokia::mapVersion()
{
    QGeoTiledMappingManagerEngineNokia *engineNokia = static_cast<QGeoTiledMappingManagerEngineNokia *>(engine());
    return engineNokia->mapVersion();
}

QT_END_NAMESPACE
