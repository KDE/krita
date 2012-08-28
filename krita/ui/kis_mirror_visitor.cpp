/*
 *  Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_mirror_visitor.h"

#include <kis_transform_visitor.h>

KisMirrorVisitor::KisMirrorVisitor(KisImageWSP image, Qt::Orientation orientation) :
    m_image(image),
    m_orientation(orientation)
{

}

bool KisMirrorVisitor::visit(KisSelectionMask* mask)
{
    mirrorMask(mask);
    return true;
}

bool KisMirrorVisitor::visit(KisTransparencyMask* mask)
{
    mirrorMask(mask);
    return true;
}

bool KisMirrorVisitor::visit(KisFilterMask* mask)
{
    mirrorMask(mask);
    return true;
}

bool KisMirrorVisitor::visit(KisCloneLayer* layer)
{
    return true;

}

bool KisMirrorVisitor::visit(KisGeneratorLayer* layer)
{
    return true;

}

bool KisMirrorVisitor::visit(KisExternalLayer* layer)
{
    if (m_orientation == Qt::Horizontal) {
        KisTransformVisitor visitor(m_image, -1.0, 1.0, 0.0, 0.0, 0.0, m_image->width(), 0, 0, 0);
        layer->accept(visitor);
    } else {
        KisTransformVisitor visitor(m_image, 1.0, -1.0, 0.0, 0.0, 0.0, 0, m_image->height(), 0, 0);
        layer->accept(visitor);
    }
    visitAll(layer);
    return true;

}

bool KisMirrorVisitor::visit(KisAdjustmentLayer* layer)
{
    return true;

}

bool KisMirrorVisitor::visit(KisGroupLayer* layer)
{
    visitAll(layer);
    return true;

}

bool KisMirrorVisitor::visit(KisPaintLayer* layer)
{
    KisPaintDeviceSP dev = layer->paintDevice();

    QString name;
    if (m_orientation == Qt::Horizontal) {
        name = i18n("Mirror Layer X");
    } else {
        name = i18n("Mirror Layer Y");
    }
    KisTransaction transaction(name, dev);

    QRect dirty;
    if (m_orientation == Qt::Horizontal) {
        dirty = KisTransformWorker::mirrorX(dev, m_image->width()/2.0f);
    } else {
        dirty = KisTransformWorker::mirrorY(dev, m_image->height()/2.0f);
    }
    layer->setDirty(dirty);

    transaction.commit(m_image->undoAdapter());
    visitAll(layer);
    return true;
}

bool KisMirrorVisitor::visit(KisNode* node)
{
    return true;

}

bool KisMirrorVisitor::mirrorMask(KisMask* mask)
{
    KoProperties properties;
    KisSelectionSP selection = mask->selection();
    if (selection->hasPixelSelection()) {
        KisPaintDeviceSP dev = selection->getOrCreatePixelSelection();

        QString name;
        if (m_orientation == Qt::Horizontal) {
            name = i18n("Mirror Mask X");
        } else {
            name = i18n("Mirror Mask Y");
        }
        KisSelectionTransaction transaction(name, m_image, selection);

        QRect dirty;
        if (m_orientation == Qt::Horizontal) {
            dirty = KisTransformWorker::mirrorX(dev, m_image->width()/2.0f);
        } else {
            dirty = KisTransformWorker::mirrorY(dev, m_image->height()/2.0f);
        }
        mask->setDirty(dirty);
        transaction.commit(m_image->undoAdapter());   
        dev->setDirty(dirty);
    }
/* TODO: Doesn't work correctly even thoush it's the same code as the shape layer
    if (selection->hasShapeSelection()) {
        if (m_orientation == Qt::Horizontal) {
            KisTransformVisitor visitor(m_image, -1.0, 1.0, 0.0, 0.0, 0.0, m_image->width(), 0, 0, 0, true);
            mask->accept(visitor);
        } else {
            KisTransformVisitor visitor(m_image, 1.0, -1.0, 0.0, 0.0, 0.0, 0, m_image->height(), 0, 0, true);
            mask->accept(visitor);
        }
    } */
    selection->updateProjection();
    return true;
}
