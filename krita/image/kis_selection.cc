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

#include "kundo2command.h"

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
    KisNodeSP parentNode;

    bool isVisible; //false is the selection decoration should not be displayed
    KisDefaultBoundsBaseSP defaultBounds;
    KisPixelSelectionSP pixelSelection;
    KisSelectionComponent *shapeSelection;
};

KisSelection::KisSelection(KisDefaultBoundsBaseSP defaultBounds)
    : m_d(new Private)
{
    if (!defaultBounds) {
        defaultBounds = new KisSelectionDefaultBounds();
    }
    m_d->defaultBounds = defaultBounds;

    m_d->pixelSelection = new KisPixelSelection(m_d->defaultBounds, this);
    m_d->pixelSelection->setParentNode(m_d->parentNode);
}

KisSelection::KisSelection(const KisSelection& rhs)
    : KisShared(),
      m_d(new Private)
{
    copyFrom(rhs);
}

KisSelection &KisSelection::operator=(const KisSelection &rhs)
{
    if (&rhs != this) {
        copyFrom(rhs);
    }
    return *this;
}

void KisSelection::copyFrom(const KisSelection &rhs)
{
    m_d->isVisible = rhs.m_d->isVisible;
    m_d->defaultBounds = rhs.m_d->defaultBounds;
    m_d->parentNode = 0; // not supposed to be shared

    Q_ASSERT(rhs.m_d->pixelSelection);
    m_d->pixelSelection = new KisPixelSelection(*rhs.m_d->pixelSelection);
    m_d->pixelSelection->setParentSelection(this);


    if (rhs.m_d->shapeSelection) {
        m_d->shapeSelection = rhs.m_d->shapeSelection->clone(this);
        Q_ASSERT(m_d->shapeSelection);
        Q_ASSERT(m_d->shapeSelection != rhs.m_d->shapeSelection);
    }
    else {
        m_d->shapeSelection = 0;
    }
}

KisSelection::~KisSelection()
{
    delete m_d->shapeSelection;
    delete m_d;
}

void KisSelection::setParentNode(KisNodeSP node)
{
    m_d->parentNode = node;
    m_d->pixelSelection->setParentNode(node);
}

// for testing purposes only
KisNodeSP KisSelection::parentNode() const
{
    return m_d->parentNode;
}

bool KisSelection::outlineCacheValid() const
{
    return hasShapeSelection() ||
        m_d->pixelSelection->outlineCacheValid();
}

QPainterPath KisSelection::outlineCache() const
{
    QPainterPath outline;

    if (hasShapeSelection()) {
        outline += m_d->shapeSelection->outlineCache();
    } else if (m_d->pixelSelection->outlineCacheValid()) {
        outline += m_d->pixelSelection->outlineCache();
    }

    return outline;
}

void KisSelection::recalculateOutlineCache()
{
    Q_ASSERT(m_d->pixelSelection);

    if (hasShapeSelection()) {
        m_d->shapeSelection->recalculateOutlineCache();
    } else if (!m_d->pixelSelection->outlineCacheValid()) {
        m_d->pixelSelection->recalculateOutlineCache();
    }
}

bool KisSelection::hasPixelSelection() const
{
    return m_d->pixelSelection && !m_d->pixelSelection->isEmpty();
}

bool KisSelection::hasShapeSelection() const
{
    return m_d->shapeSelection && !m_d->shapeSelection->isEmpty();
}

KisPixelSelectionSP KisSelection::pixelSelection() const
{
    return m_d->pixelSelection;
}

KisSelectionComponent* KisSelection::shapeSelection() const
{
    return m_d->shapeSelection;
}

void KisSelection::setShapeSelection(KisSelectionComponent* shapeSelection)
{
    m_d->shapeSelection = shapeSelection;
}

KisPixelSelectionSP KisSelection::getOrCreatePixelSelection()
{
    return m_d->pixelSelection;
}

KisPaintDeviceSP KisSelection::projection() const
{
    return m_d->pixelSelection;
}

void KisSelection::updateProjection(const QRect &rc)
{
    if(hasShapeSelection()) {
        m_d->shapeSelection->renderToProjection(m_d->pixelSelection, rc);
        m_d->pixelSelection->setOutlineCache(m_d->shapeSelection->outlineCache());
    }
}

void KisSelection::updateProjection()
{
    if(hasShapeSelection()) {
        m_d->pixelSelection->clear();
        m_d->shapeSelection->renderToProjection(m_d->pixelSelection);
        m_d->pixelSelection->setOutlineCache(m_d->shapeSelection->outlineCache());
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
    return m_d->pixelSelection->isTotallyUnselected(r);
}

QRect KisSelection::selectedRect() const
{
    return m_d->pixelSelection->selectedRect();
}

QRect KisSelection::selectedExactRect() const
{
    return m_d->pixelSelection->selectedExactRect();
}

qint32 KisSelection::x() const
{
    return m_d->pixelSelection->x();
}

qint32 KisSelection::y() const
{
    return m_d->pixelSelection->y();
}

void KisSelection::setX(qint32 x)
{
    Q_ASSERT(m_d->pixelSelection);

    qint32 delta = x - m_d->pixelSelection->x();
    m_d->pixelSelection->setX(x);
    if (m_d->shapeSelection) {
        m_d->shapeSelection->moveX(delta);
    }
}

void KisSelection::setY(qint32 y)
{
    Q_ASSERT(m_d->pixelSelection);

    qint32 delta = y - m_d->pixelSelection->y();
    m_d->pixelSelection->setY(y);
    if (m_d->shapeSelection) {
        m_d->shapeSelection->moveY(delta);
    }
}

void KisSelection::setDefaultBounds(KisDefaultBoundsBaseSP bounds)
{
    m_d->defaultBounds = bounds;
    m_d->pixelSelection->setDefaultBounds(bounds);
}

void KisSelection::clear()
{
    // FIXME: check whether this is safe
    delete m_d->shapeSelection;
    m_d->shapeSelection = 0;

    m_d->pixelSelection->clear();
}

KUndo2Command* KisSelection::flatten()
{
    KUndo2Command *command = 0;

    if (hasShapeSelection()) {
        command = m_d->shapeSelection->resetToEmpty();
    }

    return command;
}

void KisSelection::notifySelectionChanged()
{
    KisNodeSP parentNode;
    if (!(parentNode = this->parentNode())) return;

    KisNodeGraphListener *listener;
    if (!(listener = parentNode->graphListener())) return;

    listener->notifySelectionChanged();
}

quint8 KisSelection::selected(qint32 x, qint32 y) const
{
    KisHLineConstIteratorSP iter = m_d->pixelSelection->createHLineConstIteratorNG(x, y, 1);

    const quint8 *pix = iter->oldRawData();

    return *pix;
}
