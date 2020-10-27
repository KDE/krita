/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_take_all_shapes_command.h"

#include <klocalizedstring.h>
#include "kis_shape_selection.h"
#include <kis_image.h>


KisTakeAllShapesCommand::KisTakeAllShapesCommand(KisShapeSelection *shapeSelection, bool takeSilently, bool restoreSilently)
    : KUndo2Command(kundo2_i18n("Clear Vector Selection")),
      m_shapeSelection(shapeSelection),
      m_takeSilently(takeSilently),
      m_restoreSilently(restoreSilently)
{
}

KisTakeAllShapesCommand::~KisTakeAllShapesCommand()
{
    Q_FOREACH (KoShape *shape, m_shapes) {
        delete shape;
    }
}

void KisTakeAllShapesCommand::redo()
{
    if (m_takeSilently) {
        m_shapeSelection->setUpdatesEnabled(false);
    }

    m_shapes = m_shapeSelection->shapes();

    Q_FOREACH (KoShape *shape, m_shapes) {
        m_shapeSelection->removeShape(shape);
    }

    if (m_takeSilently) {
        m_shapeSelection->setUpdatesEnabled(true);
    }
}

void KisTakeAllShapesCommand::undo()
{
    if (m_restoreSilently) {
        m_shapeSelection->setUpdatesEnabled(false);
    }

    Q_FOREACH (KoShape *shape, m_shapes) {
        m_shapeSelection->addShape(shape);
    }

    m_shapes.clear();

    if (m_restoreSilently) {
        m_shapeSelection->setUpdatesEnabled(true);
    }
}

