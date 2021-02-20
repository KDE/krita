/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

