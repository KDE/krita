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

#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_node_visitor.h"


class KisCloneLayer::Private
{
public:
    KisLayerSP copyFrom;
    QString copyFromName; // Used during loading only
    CopyLayerType type;
    qint32 x;
    qint32 y;
};

KisCloneLayer::KisCloneLayer(KisLayerSP from, KisImageWSP img, const QString &name, quint8 opacity)
        : KisLayer(img, name, opacity)
        , m_d(new Private())
{
    m_d->copyFrom = from;
    m_d->type = COPY_PROJECTION;
    m_d->x = 0;
    m_d->y = 0;
}

KisCloneLayer::KisCloneLayer(const KisCloneLayer& rhs)
        : KisLayer(rhs)
        , KisIndirectPaintingSupport(rhs)
        , m_d(new Private())
{
    m_d->copyFrom = rhs.copyFrom();
    m_d->type = rhs.copyType();
    m_d->x = rhs.x();
    m_d->y = rhs.y();
}

KisCloneLayer::~KisCloneLayer()
{
    delete m_d;
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

QRect KisCloneLayer::repaintOriginal(KisPaintDeviceSP original,
                                     const QRect& rect)
{
    Q_UNUSED(original);
    return rect;
}

void KisCloneLayer::copyOriginalToProjection(const KisPaintDeviceSP original,
        KisPaintDeviceSP projection,
        const QRect& rect) const
{
    QRect copyRect = rect;
    copyRect.moveTo(m_d->x, m_d->y);

    KisPainter gc(projection);
    gc.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(rect.topLeft(), original, copyRect);
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
    QRect rect = KisLayer::extent();
    rect.moveTo(m_d->x, m_d->y);
    return rect;
}

QRect KisCloneLayer::exactBounds() const
{
    QRect rect = KisLayer::exactBounds();
    rect.moveTo(m_d->x, m_d->y);
    return rect;
}

bool KisCloneLayer::accept(KisNodeVisitor & v)
{
    return v.visit(this);
}

void KisCloneLayer::setCopyFrom(KisLayerSP fromLayer, CopyLayerType type)
{
    m_d->type = type;
    m_d->copyFrom = fromLayer;
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

void KisCloneLayer::setCopyFromName(const QString& layerName)
{
    Q_ASSERT(!m_d->copyFrom);
    m_d->copyFromName = layerName;
}

QString KisCloneLayer::copyFromName() const
{
    return m_d->copyFrom ? m_d->copyFrom->name() : m_d->copyFromName;
}

QIcon KisCloneLayer::icon() const
{
    return KIcon("edit-copy");
}

KoDocumentSectionModel::PropertyList KisCloneLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    if (m_d->copyFrom)
        l << KoDocumentSectionModel::Property(i18n("Copy From"), m_d->copyFrom->name());
    return l;
}


#include "kis_clone_layer.moc"
