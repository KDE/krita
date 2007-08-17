/*
 * Copyright (C) 2007, Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_prescaled_projection.h"
#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QRect>
#include <QPoint>

#include "kis_config.h"
#include "kis_config_notifier.h"

struct KisPrescaledProjection::Private
{
    Private()
        : useDeferredSmoothing( false )
        , useNearestNeighbour( false )
        , useQtScaling( false )
        , useSampling( false )
        , useSmoothScaling( true ) // Default
        , drawCheckers( false )
        , scrollCheckers( false )
        , checkSize( 32 )
        {
        }

    bool useDeferredSmoothing;
    bool useNearestNeighbour;
    bool useQtScaling;
    bool useSampling;
    bool useSmoothScaling;
    bool drawCheckers;
    bool scrollCheckers;
    QColor checkersColor;
    qint32 checkSize;
    QImage unscaledCache;
    QImage prescaledQImage;
    QPixmap prescaledPixmap;
};

KisPrescaledProjection::KisPrescaledProjection()
    : QObject( 0 )
    , m_d( new Private() )
{
    updateSettings();
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(updateSettings()));
}

KisPrescaledProjection::~KisPrescaledProjection()
{
    delete m_d;
}


bool KisPrescaledProjection::drawCheckers() const
{
    return m_d->drawCheckers;
}


void KisPrescaledProjection::setDrawCheckers( bool drawCheckers )
{
    m_d->drawCheckers = drawCheckers;
}

QPixmap KisPrescaledProjection::prescaledPixmap() const
{
    return m_d->prescaledPixmap;
}

QImage KisPrescaledProjection::prescaledQImage() const
{
    return m_d->prescaledQImage;
}

void KisPrescaledProjection::updateSettings()
{
    KisConfig cfg;
    m_d->useDeferredSmoothing = cfg.useDeferredSmoothing();
    m_d->useNearestNeighbour = cfg.fastZoom();
    m_d->useQtScaling = cfg.useQtSmoothScaling();
    m_d->useSampling = cfg.useSampling();
    // If any of the above are true, we don't use our own smooth scaling
    m_d->useSmoothScaling = !( m_d->useNearestNeighbour ||
                               m_d->useSampling ||
                               m_d->useQtScaling ||
                               m_d->useDeferredSmoothing );
    m_d->scrollCheckers = cfg.scrollCheckers();
    m_d->checkSize = cfg.checkSize();
    m_d->checkersColor = cfg.checkersColor();
}

void KisPrescaledProjection::documentOffsetMoved( const QPoint &documentOffset )
{
}

void KisPrescaledProjection::updateCanvasProjection( const QRect & rc )
{
}

void KisPrescaledProjection::setImageSize(qint32 w, qint32 h)
{
}

#include "kis_prescaled_projection.moc"
