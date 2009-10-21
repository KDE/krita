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

#include "kis_shape_selection_model.h"
#include "kis_debug.h"

#include "KoShapeContainer.h"

#include "kis_paint_device.h"
#include "kis_shape_selection.h"
#include "kis_selection.h"
#include "kis_image.h"
#include "kis_undo_adapter.h"

KisShapeSelectionModel::KisShapeSelectionModel(KisImageWSP image, KisSelectionSP selection, KisShapeSelection* shapeSelection)
        : m_image(image)
        , m_parentSelection(selection)
        , m_shapeSelection(shapeSelection)
{
}

KisShapeSelectionModel::~KisShapeSelectionModel()
{
}

void KisShapeSelectionModel::add(KoShape *child)
{
    if (m_shapeMap.contains(child))
        return;

    child->setBorder(0);
    child->setBackground(0);
    m_shapeMap.insert(child, child->boundingRect());
    m_shapeSelection->setDirty();

    QRect updateRect = child->boundingRect().toAlignedRect();
    QMatrix matrix;
    matrix.scale(m_image->xRes(), m_image->yRes());
    updateRect = matrix.mapRect(updateRect);

    if (m_shapeMap.count() == 1) {
        // The shape is the first one, so the shape selection just got created
        // Pixel selection provides no longer the datamanager of the selection
        // so update the whole selection
        m_parentSelection->updateProjection();
    } else {
        m_parentSelection->updateProjection(updateRect);
    }

    m_image->undoAdapter()->emitSelectionChanged();
}

void KisShapeSelectionModel::remove(KoShape *child)
{
    QRect updateRect = child->boundingRect().toAlignedRect();
    m_shapeMap.remove(child);

    m_shapeSelection->setDirty();

    QMatrix matrix;
    matrix.scale(m_image->xRes(), m_image->yRes());
    updateRect = matrix.mapRect(updateRect);
    m_parentSelection->updateProjection(updateRect);

    m_image->undoAdapter()->emitSelectionChanged();
}

void KisShapeSelectionModel::setClipping(const KoShape *child, bool clipping)
{
    Q_UNUSED(child);
    Q_UNUSED(clipping);
}

bool KisShapeSelectionModel::childClipped(const KoShape *child) const
{
    Q_UNUSED(child);
    return false;
}

int KisShapeSelectionModel::count() const
{
    return m_shapeMap.count();
}

QList<KoShape*> KisShapeSelectionModel::childShapes() const
{
    return QList<KoShape*>(m_shapeMap.keys());
}
void KisShapeSelectionModel::containerChanged(KoShapeContainer *)
{
}

void KisShapeSelectionModel::childChanged(KoShape * child, KoShape::ChangeType type)
{
    if (type == KoShape::ParentChanged)
        return;

    m_shapeSelection->setDirty();
    QRectF changedRect = m_shapeMap[child];
    changedRect = changedRect.unite(child->boundingRect());

    QMatrix matrix;
    matrix.scale(m_image->xRes(), m_image->yRes());
    changedRect = matrix.mapRect(changedRect);

    m_shapeMap[child] = child->boundingRect();

    m_parentSelection->updateProjection(changedRect.toAlignedRect());
    m_parentSelection->updateProjection();
    m_image->undoAdapter()->emitSelectionChanged();
}

bool KisShapeSelectionModel::isChildLocked(const KoShape *child) const
{
    return child->isGeometryProtected() || child->parent()->isGeometryProtected();
}
