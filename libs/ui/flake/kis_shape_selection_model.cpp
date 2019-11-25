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

#include <KoShapeContainer.h>
#include <KoShapeBackground.h>
#include <KoShapeManager.h>

#include "kis_shape_selection.h"
#include "kis_selection.h"
#include "kis_image.h"
#include "kis_update_selection_job.h"


KisShapeSelectionModel::KisShapeSelectionModel(KisImageWSP image, KisSelectionWSP selection, KisShapeSelection* shapeSelection)
    : m_image(image)
    , m_parentSelection(selection)
    , m_shapeSelection(shapeSelection)
    , m_updatesEnabled(true)
{
}

KisShapeSelectionModel::~KisShapeSelectionModel()
{
    m_image = 0;
    m_parentSelection = 0;
}

void KisShapeSelectionModel::requestUpdate(const QRect &updateRect)
{
    m_shapeSelection->recalculateOutlineCache();

    if (m_updatesEnabled) {
        m_parentSelection->requestCompressedProjectionUpdate(updateRect);
    }
}

void KisShapeSelectionModel::add(KoShape *child)
{
    if (!m_shapeSelection) return;

    if (m_shapeMap.contains(child))
        return;

    child->setStroke(KoShapeStrokeModelSP());
    child->setBackground( QSharedPointer<KoShapeBackground>(0));
    m_shapeMap.insert(child, child->boundingRect());
    m_shapeSelection->shapeManager()->addShape(child);

    QRect updateRect = child->boundingRect().toAlignedRect();
    if (m_image.isValid()) {
        QTransform matrix;
        matrix.scale(m_image->xRes(), m_image->yRes());
        updateRect = matrix.mapRect(updateRect);
    }

    if (m_shapeMap.count() == 1) {
        // The shape is the first one, so the shape selection just got created
        // Pixel selection provides no longer the datamanager of the selection
        // so update the whole selection
        requestUpdate(QRect());
    } else {
        requestUpdate(updateRect);
    }
}

void KisShapeSelectionModel::remove(KoShape *child)
{
    if (!m_shapeMap.contains(child)) return;

    QRect updateRect = child->boundingRect().toAlignedRect();
    m_shapeMap.remove(child);

    if (m_shapeSelection) {
        m_shapeSelection->shapeManager()->remove(child);
    }
    if (m_image.isValid()) {
        QTransform matrix;
        matrix.scale(m_image->xRes(), m_image->yRes());
        updateRect = matrix.mapRect(updateRect);
        if (m_shapeSelection) { // No m_shapeSelection indicates the selection is being deleted
            requestUpdate(updateRect);

            if (m_updatesEnabled && m_shapeMap.isEmpty()) {
                m_parentSelection->notifyShapeSelectionBecameEmpty();
            }
        }
    }
}

void KisShapeSelectionModel::setUpdatesEnabled(bool enabled)
{
    m_updatesEnabled = enabled;
}

bool KisShapeSelectionModel::updatesEnabled() const
{
    return m_updatesEnabled;
}

void KisShapeSelectionModel::setClipped(const KoShape *child, bool clipping)
{
    Q_UNUSED(child);
    Q_UNUSED(clipping);
}

bool KisShapeSelectionModel::isClipped(const KoShape *child) const
{
    Q_UNUSED(child);
    return false;
}

void KisShapeSelectionModel::setInheritsTransform(const KoShape *shape, bool inherit)
{
    Q_UNUSED(shape);
    Q_UNUSED(inherit);
}

bool KisShapeSelectionModel::inheritsTransform(const KoShape *shape) const
{
    Q_UNUSED(shape);
    return false;
}

int KisShapeSelectionModel::count() const
{
    return m_shapeMap.count();
}

QList<KoShape*> KisShapeSelectionModel::shapes() const
{
    return QList<KoShape*>(m_shapeMap.keys());
}
void KisShapeSelectionModel::containerChanged(KoShapeContainer *, KoShape::ChangeType)
{
}

void KisShapeSelectionModel::childChanged(KoShape * child, KoShape::ChangeType type)
{
    if (!m_shapeSelection) return;

    // TODO: check if still needed
    if (type == KoShape::ParentChanged) return;

    QRectF changedRect = m_shapeMap[child];
    changedRect = changedRect.united(child->boundingRect());
    m_shapeMap[child] = child->boundingRect();

    if (m_image.isValid()) {
        QTransform matrix;
        matrix.scale(m_image->xRes(), m_image->yRes());
        changedRect = matrix.mapRect(changedRect);
    }

    requestUpdate(changedRect.toAlignedRect());
}

void KisShapeSelectionModel::setShapeSelection(KisShapeSelection* selection)
{
    m_shapeSelection = selection;
}
