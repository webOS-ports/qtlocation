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
#include "qgeotilerequestmanager_p.h"

#include <QSharedPointer>
#include <QDebug>
#include "qgeotilespec_p.h"
#include "qgeotiledmap_p.h"
#include "qgeotiledmappingmanagerengine_p.h"
#include "qgeotilecache_p.h"

QT_BEGIN_NAMESPACE

class RetryFuture;

class QGeoTileRequestManagerPrivate
{
public:
    explicit QGeoTileRequestManagerPrivate(QGeoTiledMap *map);
    ~QGeoTileRequestManagerPrivate();

    QGeoTiledMap *map_;

    QList<QSharedPointer<QGeoTileTexture> > requestTiles(const QSet<QGeoTileSpec> &tiles);
    void tileError(const QGeoTileSpec &tile, const QString &errorString);

    QHash<QGeoTileSpec, int> retries_;
    QHash<QGeoTileSpec, QSharedPointer<RetryFuture> > futures_;
    QSet<QGeoTileSpec> requested_;

    void tileFetched(const QGeoTileSpec &spec);
};

QGeoTileRequestManager::QGeoTileRequestManager(QGeoTiledMap *map)
    : d_ptr(new QGeoTileRequestManagerPrivate(map))
{
}

QGeoTileRequestManager::~QGeoTileRequestManager()
{
    delete d_ptr;
}

QList<QSharedPointer<QGeoTileTexture> > QGeoTileRequestManager::requestTiles(const QSet<QGeoTileSpec> &tiles)
{
    Q_D(QGeoTileRequestManager);
    return d->requestTiles(tiles);
}

void QGeoTileRequestManager::tileFetched(const QGeoTileSpec &spec)
{
    Q_D(QGeoTileRequestManager);
    d->tileFetched(spec);
}

void QGeoTileRequestManager::tileError(const QGeoTileSpec &tile, const QString &errorString)
{
    Q_D(QGeoTileRequestManager);
    d->tileError(tile, errorString);
}

QGeoTileRequestManagerPrivate::QGeoTileRequestManagerPrivate(QGeoTiledMap *map)
    : map_(map)
{
}

QGeoTileRequestManagerPrivate::~QGeoTileRequestManagerPrivate()
{
}

QList<QSharedPointer<QGeoTileTexture> > QGeoTileRequestManagerPrivate::requestTiles(const QSet<QGeoTileSpec> &tiles)
{
    QSet<QGeoTileSpec> cancelTiles = requested_ - tiles;
    QSet<QGeoTileSpec> requestTiles = tiles - requested_;
    QSet<QGeoTileSpec> cached;
//    int tileSize = tiles.size();
//    int newTiles = requestTiles.size();

    typedef QSet<QGeoTileSpec>::const_iterator iter;

    QList<QSharedPointer<QGeoTileTexture> > cachedTex;

    QGeoTiledMappingManagerEngine *engine = map_ ?
                static_cast<QGeoTiledMappingManagerEngine *>(map_->engine()) : 0;

    // remove tiles in cache from request tiles
    if (engine) {
        iter i = requestTiles.constBegin();
        iter end = requestTiles.constEnd();
        for (; i != end; ++i) {
            QGeoTileSpec tile = *i;
            QSharedPointer<QGeoTileTexture> tex = engine->getTileTexture(tile);
            if (tex) {
                cachedTex << tex;
                cached.insert(tile);
            }
        }
    }

    requestTiles -= cached;

    requested_ -= cancelTiles;
    requested_ += requestTiles;

//    qDebug() << "required # tiles: " << tileSize << ", new tiles: " << newTiles << ", total server requests: " << requested_.size();

    if (!requestTiles.isEmpty() || !cancelTiles.isEmpty()) {
        if (engine) {
//            qDebug() << "new server requests: " << requestTiles.size() << ", server cancels: " << cancelTiles.size();
            engine->updateTileRequests(map_, requestTiles, cancelTiles);

            // Remove any cancelled tiles from the error retry hash to avoid
            // re-using the numbers for a totally different request cycle.
            iter i = cancelTiles.constBegin();
            iter end = cancelTiles.constEnd();
            for (; i != end; ++i) {
                retries_.remove(*i);
                futures_.remove(*i);
            }
        }
    }

    return cachedTex;
}

void QGeoTileRequestManagerPrivate::tileFetched(const QGeoTileSpec &spec)
{
    map_->newTileFetched(spec);
    requested_.remove(spec);
    retries_.remove(spec);
    futures_.remove(spec);
}

// Represents a tile that needs to be retried after a certain period of time
class RetryFuture : public QObject
{
    Q_OBJECT
public:
    RetryFuture(const QGeoTileSpec &tile, QGeoTiledMap *map, QObject *parent = 0);

public Q_SLOTS:
    void retry();

private:
    QGeoTileSpec tile_;
    QGeoTiledMap *map_;
};

RetryFuture::RetryFuture(const QGeoTileSpec &tile, QGeoTiledMap *map, QObject *parent)
    : QObject(parent), tile_(tile), map_(map)
{}

void RetryFuture::retry()
{
    QSet<QGeoTileSpec> requestTiles;
    QSet<QGeoTileSpec> cancelTiles;
    requestTiles.insert(tile_);
    if (map_) {
        QGeoTiledMappingManagerEngine *engine =
                static_cast<QGeoTiledMappingManagerEngine *>(map_->engine());
        engine->updateTileRequests(map_, requestTiles, cancelTiles);
    }
}

void QGeoTileRequestManagerPrivate::tileError(const QGeoTileSpec &tile, const QString &errorString)
{
    if (requested_.contains(tile)) {
        int count = retries_.value(tile, 0);
        retries_.insert(tile, count + 1);

        if (count >= 5) {
            qWarning("QGeoTileRequestManager: Failed to fetch tile (%d,%d,%d) 5 times, giving up. "
                     "Last error message was: '%s'",
                     tile.x(), tile.y(), tile.zoom(), qPrintable(errorString));
            requested_.remove(tile);
            retries_.remove(tile);
            futures_.remove(tile);

        } else {
            // Exponential time backoff when retrying
            int delay = (1 << count) * 500;

            QSharedPointer<RetryFuture> future(new RetryFuture(tile, map_));
            futures_.insert(tile, future);

            QTimer::singleShot(delay, future.data(), SLOT(retry()));
            // Passing .data() to singleShot is ok -- Qt will clean up the
            // connection if the target qobject is deleted
        }
    }
}

#include "qgeotilerequestmanager.moc"

QT_END_NAMESPACE
