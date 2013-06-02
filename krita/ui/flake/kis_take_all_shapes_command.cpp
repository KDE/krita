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

#include <klocale.h>
#include "kis_shape_selection.h"


KisTakeAllShapesCommand::KisTakeAllShapesCommand(KisShapeSelection *shapeSelection)
    : KUndo2Command(i18nc("(qtundo-format)", "Clear Vector Selection")),
      m_shapeSelection(shapeSelection)
{
}

KisTakeAllShapesCommand::~KisTakeAllShapesCommand()
{
    foreach (KoShape *shape, m_shapes) {
        delete shape;
    }
}

void KisTakeAllShapesCommand::redo()
{
    m_shapes = m_shapeSelection->shapes();

    foreach (KoShape *shape, m_shapes) {
        m_shapeSelection->removeShape(shape);
    }
}

void KisTakeAllShapesCommand::undo()
{
    foreach (KoShape *shape, m_shapes) {
        m_shapeSelection->addShape(shape);
    }

    m_shapes.clear();
}

