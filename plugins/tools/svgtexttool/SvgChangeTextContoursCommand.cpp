/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <KoShape.h>
#include <KoSvgTextShape.h>

#include "SvgChangeTextContoursCommand.h"


SvgChangeTextContoursCommand::SvgChangeTextContoursCommand(KoSvgTextShape *textShape, QList<KoShape*> shapes, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_textShape(textShape)
    , m_newShapes(shapes)
{
    setText(kundo2_i18n("Change Text Contours"));
    m_oldShapes = m_textShape->shapesInside();
}

void SvgChangeTextContoursCommand::redo()
{
    QRectF updateRect = m_textShape->boundingRect();
    m_textShape->setShapesInside(m_newShapes);
    updateRect |= m_textShape->boundingRect();
    m_textShape->updateAbsolute(updateRect);
}

void SvgChangeTextContoursCommand::undo()
{
    QRectF updateRect = m_textShape->boundingRect();
    m_textShape->setShapesInside(m_oldShapes);
    updateRect |= m_textShape->boundingRect();
    m_textShape->updateAbsolute(updateRect);
}
