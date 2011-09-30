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

#ifndef QGEOTILEDMAPREPLY_H
#define QGEOTILEDMAPREPLY_H

#include "qglobal.h"

#include <QObject>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Location)

class TileSpec;
class QGeoTiledMapReplyPrivate;

class Q_LOCATION_EXPORT QGeoTiledMapReply : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError,
        CommunicationError,
        ParseError,
        UnknownError
    };

    QGeoTiledMapReply(const TileSpec &spec, QObject *parent = 0);
    QGeoTiledMapReply(Error error, const QString &errorString, QObject *parent = 0);
    virtual ~QGeoTiledMapReply();

    bool isFinished() const;
    Error error() const;
    QString errorString() const;

    bool isCached() const;

    TileSpec tileSpec() const;

    QByteArray mapImageData() const;
    QString mapImageFormat() const;

    virtual void abort();

Q_SIGNALS:
    void finished();
    void error(QGeoTiledMapReply::Error error, const QString &errorString = QString());

protected:
    void setError(Error error, const QString &errorString);
    void setFinished(bool finished);

    void setCached(bool cached);

    void setMapImageData(const QByteArray &data);
    void setMapImageFormat(const QString &format);

private:
    QGeoTiledMapReplyPrivate *d_ptr;
    Q_DISABLE_COPY(QGeoTiledMapReply)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif