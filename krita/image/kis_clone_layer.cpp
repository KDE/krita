/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_clone_layer.h"

#include <kis_debug.h>
#include <klocale.h>

#include <KoIcon.h>

#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_clone_info.h"
#include "kis_paint_layer.h"


struct KisCloneLayer::Private
{
public:
    KisLayerSP copyFrom;
    KisCloneInfo copyFromInfo;
    CopyLayerType type;
    qint32 x;
    qint32 y;
};

KisCloneLayer::KisCloneLayer(KisLayerSP from, KisImageWSP image, const QString &name, quint8 opacity)
        : KisLayer(image, name, opacity)
        , m_d(new Private())
{
    m_d->copyFrom = from;
    m_d->type = COPY_PROJECTION;
    m_d->x = 0;
    m_d->y = 0;

    // When loading the layer we copy from might not exist yet
    if (m_d->copyFrom) {
        m_d->copyFrom->registerClone(this);
    }
}

KisCloneLayer::KisCloneLayer(const KisCloneLayer& rhs)
        : KisLayer(rhs)
        , m_d(new Private())
{
    m_d->copyFrom = rhs.copyFrom();
    m_d->type = rhs.copyType();
    m_d->x = rhs.x();
    m_d->y = rhs.y();
    if (m_d->copyFrom) {
        m_d->copyFrom->registerClone(this);
    }
}

KisCloneLayer::~KisCloneLayer()
{
    if (m_d->copyFrom) {
        m_d->copyFrom->unregisterClone(this);
    }
    delete m_d;
}

KisLayerSP KisCloneLayer::reincarnateAsPaintLayer() const
{
    KisPaintDeviceSP newOriginal = new KisPaintDevice(*original());
    KisPaintLayerSP newLayer = new KisPaintLayer(image(), name(), opacity(), newOriginal);
    newLayer->setX(x());
    newLayer->setY(y());
    newLayer->setCompositeOp(compositeOpId());
    newLayer->mergeNodeProperties(nodeProperties());

    return newLayer;
}

bool KisCloneLayer::allowAsChild(KisNodeSP node) const
{
    return node->inherits("KisMask");
}

KisPaintDeviceSP KisCloneLayer::paintDevice() const
{
    return 0;
}

KisPaintDeviceSP KisCloneLayer::original() const
{
    Q_ASSERT(m_d->copyFrom);

    KisPaintDeviceSP retval;
    switch (m_d->type) {
    case COPY_PROJECTION:
        retval = m_d->copyFrom->projection();
        break;

    case COPY_ORIGINAL:
    default:
        retval = m_d->copyFrom->original();
    }

    return retval;
}

bool KisCloneLayer::needProjection() const
{
    return m_d->x || m_d->y;
}

void KisCloneLayer::copyOriginalToProjection(const KisPaintDeviceSP original,
        KisPaintDeviceSP projection,
        const QRect& rect) const
{
    QRect copyRect = rect;
    copyRect.translate(-m_d->x, -m_d->y);

    KisPainter gc(projection);
    gc.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(rect.topLeft(), original, copyRect);
}

void KisCloneLayer::setDirtyOriginal(const QRect &rect)
{
    QRect localRect = rect;
    localRect.translate(m_d->x, m_d->y);
    KisLayer::setDirty(localRect);
}

qint32 KisCloneLayer::x() const
{
    return m_d->x;
}
qint32 KisCloneLayer::y() const
{
    return m_d->y;
}
void KisCloneLayer::setX(qint32 x)
{
    m_d->x = x;
}
void KisCloneLayer::setY(qint32 y)
{
    m_d->y = y;
}

QRect KisCloneLayer::extent() const
{
    QRect rect = original()->extent();
    if(m_d->x || m_d->y) {
        rect.translate(m_d->x, m_d->y);
    }
    return rect | projection()->extent();
}

QRect KisCloneLayer::exactBounds() const
{
    QRect rect = original()->exactBounds();
    if(m_d->x || m_d->y) {
        rect.translate(m_d->x, m_d->y);
    }
    return rect | projection()->exactBounds();
}

QRect KisCloneLayer::accessRect(const QRect &rect, PositionToFilthy pos) const
{
    QRect resultRect = rect;

    if(pos & (N_FILTHY_PROJECTION | N_FILTHY) && (m_d->x || m_d->y)) {
        resultRect |= rect.translated(-m_d->x, -m_d->y);
    }

    return resultRect;
}

bool KisCloneLayer::accept(KisNodeVisitor & v)
{
    return v.visit(this);
}

void KisCloneLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

void KisCloneLayer::setCopyFrom(KisLayerSP fromLayer)
{
    if (m_d->copyFrom) {
        m_d->copyFrom->unregisterClone(this);
    }

    m_d->copyFrom = fromLayer;

    if (m_d->copyFrom) {
        m_d->copyFrom->registerClone(this);
    }
}

KisLayerSP KisCloneLayer::copyFrom() const
{
    return m_d->copyFrom;
}

void KisCloneLayer::setCopyType(CopyLayerType type)
{
    m_d->type = type;
}

CopyLayerType KisCloneLayer::copyType() const
{
    return m_d->type;
}

KisCloneInfo KisCloneLayer::copyFromInfo() const
{
    return m_d->copyFrom ? KisCloneInfo(m_d->copyFrom) : m_d->copyFromInfo;
}

void KisCloneLayer::setCopyFromInfo(KisCloneInfo info)
{
    Q_ASSERT(!m_d->copyFrom);
    m_d->copyFromInfo = info;
}

QIcon KisCloneLayer::icon() const
{
    return koIcon("edit-copy");
}

KoDocumentSectionModel::PropertyList KisCloneLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    if (m_d->copyFrom)
        l << KoDocumentSectionModel::Property(i18n("Copy From"), m_d->copyFrom->name());
    return l;
}


#include "kis_clone_layer.moc"
