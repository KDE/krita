/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kdebug.h"

#include "KoShapeContainer.h"
#include "kis_shape_selection_model.h"
#include "kis_paint_device.h"
#include "kis_shape_selection.h"

KisShapeSelectionModel::KisShapeSelectionModel(KisImageSP image, KisPaintDeviceSP dev, KisShapeSelection* shapeSelection)
 : m_image(image)
 , m_parentPaintDevice(dev)
 , m_shapeSelection(shapeSelection)
{
}

KisShapeSelectionModel::~KisShapeSelectionModel()
{
}

void KisShapeSelectionModel::add(KoShape *child) {
    if(m_shapes.contains(child))
        return;
    m_shapes.append(child);
}

void KisShapeSelectionModel::setClipping(const KoShape *child, bool clipping)
{
}

bool KisShapeSelectionModel::childClipped(const KoShape *child) const
{
    return false;
}

void KisShapeSelectionModel::remove(KoShape *child)
{
    m_shapes.removeAll(child);
}

int KisShapeSelectionModel::count() const
{
    return m_shapes.count();
}

QList<KoShape*> KisShapeSelectionModel::iterator() const
{
    return QList<KoShape*>(m_shapes);
}
void KisShapeSelectionModel::containerChanged(KoShapeContainer *)
{
}

void KisShapeSelectionModel::childChanged(KoShape *, KoShape::ChangeType )
{
    m_shapeSelection->setDirty();
    QRectF bound = m_shapeSelection->selectionOutline().boundingRect();

    QMatrix matrix;
    matrix.scale(m_image->xRes(), m_image->yRes());
    bound = matrix.mapRect(bound);

    QRect updateRect = bound.toAlignedRect();
    updateRect = updateRect.unite(m_parentPaintDevice->selection()->selectedRect());
    m_parentPaintDevice->setDirty(updateRect);
    m_parentPaintDevice->emitSelectionChanged();
}

bool KisShapeSelectionModel::isChildLocked(const KoShape *child) const {
    return child->isLocked() || child->parent()->isLocked();
}
