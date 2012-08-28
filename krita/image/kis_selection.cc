/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
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

#include "kis_selection.h"

#include "kis_selection_component.h"
#include "kis_pixel_selection.h"

#include "kis_default_bounds.h"
#include "kis_iterator_ng.h"

struct KisSelection::Private {
    Private()
        : isVisible(true),
          shapeSelection(0)
    {
    }

    // used for forwarding setDirty signals only
    KisNodeWSP parentNode;

    bool isVisible; //false is the selection decoration should not be displayed
    KisDefaultBoundsBaseSP defaultBounds;
    KisPixelSelectionSP projection;
    KisPixelSelectionSP pixelSelection;
    KisSelectionComponent *shapeSelection;

    KisPixelSelectionSP getProjection()
    {
        if (pixelSelection && !shapeSelection) {
            return pixelSelection;
        }
        else {
            if(!projection) {
                projection = new KisPixelSelection(defaultBounds);
            }
            return projection;
        }

    }

};

KisSelection::KisSelection(KisDefaultBoundsBaseSP defaultBounds)
    : m_d(new Private)
{
    if (!defaultBounds) {
        defaultBounds = new KisSelectionDefaultBounds();
    }
    m_d->defaultBounds = defaultBounds;
}

KisSelection::KisSelection(const KisSelection& rhs)
    : KisShared(),
      m_d(new Private)
{
    m_d->isVisible = rhs.m_d->isVisible;
    m_d->defaultBounds = rhs.m_d->defaultBounds;
    m_d->parentNode = 0; // not supposed to be shared

    if(rhs.m_d->projection) {
        m_d->projection = new KisPixelSelection(*rhs.m_d->projection);
        Q_ASSERT(m_d->projection);
    }

    if(rhs.m_d->pixelSelection) {
        m_d->pixelSelection = new KisPixelSelection(*rhs.m_d->pixelSelection);
        Q_ASSERT(m_d->pixelSelection);
    }

    if (rhs.m_d->shapeSelection) {
        m_d->shapeSelection = rhs.m_d->shapeSelection->clone(this);
        Q_ASSERT(m_d->shapeSelection);
        Q_ASSERT(m_d->shapeSelection != rhs.m_d->shapeSelection);
    }
    else {
        m_d->shapeSelection = 0;
    }
}

KisSelection &KisSelection::operator=(const KisSelection &rhs)
{
    if (&rhs != this) {
        m_d->isVisible = rhs.m_d->isVisible;
        m_d->defaultBounds = rhs.m_d->defaultBounds;
        m_d->parentNode = 0; // not supposed to be shared

        if(rhs.m_d->projection) {
            m_d->projection = new KisPixelSelection(*rhs.m_d->projection);
            Q_ASSERT(m_d->projection);
        }

        if(rhs.m_d->pixelSelection) {
            m_d->pixelSelection = new KisPixelSelection(*rhs.m_d->pixelSelection);
            Q_ASSERT(m_d->pixelSelection);
        }

        if (rhs.m_d->shapeSelection) {
            m_d->shapeSelection = rhs.m_d->shapeSelection->clone(this);
            Q_ASSERT(m_d->shapeSelection);
            Q_ASSERT(m_d->shapeSelection != rhs.m_d->shapeSelection);
        }
        else {
            m_d->shapeSelection = 0;
        }
    }
    return *this;
}

KisSelection::~KisSelection()
{
    delete m_d->shapeSelection;
    delete m_d;
}

void KisSelection::setParentNode(KisNodeWSP node)
{
    m_d->parentNode = node;

    if(m_d->pixelSelection) {
        m_d->pixelSelection->setParentNode(node);
    }

    /**
     * NOTE: We shouldn't set the parent node for the projection
     * device because noone is considered to be painting on the
     * projection
     */
}

// for testing purposes only
KisNodeWSP KisSelection::parentNode() const
{
    return m_d->parentNode;
}

bool KisSelection::hasPixelSelection() const
{
    return m_d->pixelSelection;
}

bool KisSelection::hasShapeSelection() const
{
    return m_d->shapeSelection;
}

QVector<QPolygon> KisSelection::outline() const
{
    return m_d->getProjection()->outline();
}

KisPixelSelectionSP KisSelection::pixelSelection() const
{
    return m_d->pixelSelection;
}

KisSelectionComponent* KisSelection::shapeSelection() const
{
    return m_d->shapeSelection;
}

void KisSelection::setPixelSelection(KisPixelSelectionSP pixelSelection)
{
    m_d->pixelSelection = pixelSelection;
    if(m_d->pixelSelection) {
        m_d->pixelSelection->setParentNode(m_d->parentNode);
    }
}

void KisSelection::setShapeSelection(KisSelectionComponent* shapeSelection)
{
    m_d->shapeSelection = shapeSelection;
}

KisPixelSelectionSP KisSelection::getOrCreatePixelSelection()
{
    if (!m_d->pixelSelection) {
        m_d->pixelSelection = new KisPixelSelection(m_d->defaultBounds);
        m_d->pixelSelection->setParentNode(m_d->parentNode);
    }

    return m_d->pixelSelection;
}

KisPaintDeviceSP KisSelection::projection() const
{
    return m_d->getProjection();
}

void KisSelection::updateProjection(const QRect &rc)
{
    KisPixelSelectionSP currentProjection = m_d->getProjection();
    if(currentProjection == m_d->pixelSelection) return;

    QRect updateRect = rc;

    if(m_d->pixelSelection) {
        if(*(m_d->pixelSelection->defaultPixel()) !=
           *(currentProjection->defaultPixel())) {

            quint8 defPixel = *(m_d->pixelSelection->defaultPixel());
            currentProjection->setDefaultPixel(&defPixel);
            updateRect |= selectedRect();
        }
        currentProjection->clear(updateRect);
        m_d->pixelSelection->renderToProjection(currentProjection.data(), updateRect);
    }

    if(m_d->shapeSelection) {
        m_d->shapeSelection->renderToProjection(currentProjection.data(), updateRect);
    }
}

void KisSelection::updateProjection()
{
    /**
     * This method resembles updateProjection(rect), because
     * we cannot get an extent of KisSelectionComponent.
     */

    KisPixelSelectionSP currentProjection = m_d->getProjection();
    if(currentProjection == m_d->pixelSelection) return;

    currentProjection->clear();

    if(m_d->pixelSelection) {
        if(*(m_d->pixelSelection->defaultPixel()) !=
           *(currentProjection->defaultPixel())) {

            quint8 defPixel = *(m_d->pixelSelection->defaultPixel());
            currentProjection->setDefaultPixel(&defPixel);
        }
        m_d->pixelSelection->renderToProjection(currentProjection.data());
    }

    if(m_d->shapeSelection) {
        m_d->shapeSelection->renderToProjection(currentProjection.data());
    }
}

void KisSelection::setVisible(bool visible)
{
    m_d->isVisible = visible;
}

bool KisSelection::isVisible()
{
    return m_d->isVisible;
}

bool KisSelection::isTotallyUnselected(const QRect & r) const
{
    return m_d->getProjection()->isTotallyUnselected(r);
}

QRect KisSelection::selectedRect() const
{
    return m_d->getProjection()->selectedRect();
}

QRect KisSelection::selectedExactRect() const
{
    return m_d->getProjection()->selectedExactRect();
}

qint32 KisSelection::x() const
{
    return m_d->getProjection()->x();
}

qint32 KisSelection::y() const
{
    return m_d->getProjection()->y();
}

void KisSelection::setX(qint32 x)
{
    KisPixelSelectionSP currentProjection = m_d->getProjection();
    qint32 delta = x - currentProjection->x();
    currentProjection->setX(x);
    if (m_d->pixelSelection) {
        m_d->pixelSelection->setX(x);
    }
    if (m_d->shapeSelection) {
        m_d->shapeSelection->moveX(delta);
    }
}

void KisSelection::setY(qint32 y)
{
    KisPixelSelectionSP currentProjection = m_d->getProjection();
    qint32 delta = y - currentProjection->y();
    currentProjection->setY(y);
    if (m_d->pixelSelection) {
        m_d->pixelSelection->setY(y);
    }
    if (m_d->shapeSelection) {
        m_d->shapeSelection->moveY(delta);
    }
}


void KisSelection::setDefaultBounds(KisDefaultBoundsBaseSP bounds)
{
    m_d->defaultBounds = bounds;

    m_d->getProjection()->setDefaultBounds(bounds);
    if(m_d->pixelSelection) {
        m_d->pixelSelection->setDefaultBounds(bounds);
    }
}

void KisSelection::clear()
{
    if(m_d->pixelSelection) {
        m_d->pixelSelection->clear();
    }

    // FIXME: check whether this is safe
    delete m_d->shapeSelection;

    KisPixelSelectionSP currentProjection = m_d->getProjection();
    if(currentProjection != m_d->pixelSelection) {
        currentProjection->clear();
    }
}

void KisSelection::flatten()
{
    if (hasShapeSelection()) {
        updateProjection();
        m_d->pixelSelection = m_d->projection;
        delete m_d->shapeSelection;
        m_d->shapeSelection = 0;
    }
}

quint8 KisSelection::selected(qint32 x, qint32 y) const
{
    KisHLineConstIteratorSP iter = m_d->getProjection()->createHLineConstIteratorNG(x, y, 1);

    const quint8 *pix = iter->oldRawData();

    return *pix;
}

void KisSelection::setDirty(const QRect &rc)
{
    Q_UNUSED(rc);
    //FIXME: do nothing
}
