/* This file is part of the KDE project

   Copyright 2017 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "SvgTextChangeCommand.h"

#include <math.h>
#include <klocalizedstring.h>
#include <KoImageData.h>

#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"

SvgTextChangeCommand::SvgTextChangeCommand(KoSvgTextShape *shape,
                                           const QString &svg,
                                           const QString &defs,
                                           bool richTextPreferred,
                                           KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_svg(svg)
    , m_defs(defs)
    , m_richTextPreferred(richTextPreferred)
{
    Q_ASSERT(shape);
    setText(kundo2_i18n("Change SvgTextTool"));
    m_oldRichTextPreferred = m_shape->isRichTextPreferred();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    converter.convertToSvg(&m_oldSvg, &m_oldDefs);
}

SvgTextChangeCommand::~SvgTextChangeCommand()
{
}

void SvgTextChangeCommand::redo()
{
    m_shape->update();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    // Hardcoded resolution?
    converter.convertFromSvg(m_svg, m_defs, m_shape->boundingRect(), 72.0);
    m_shape->setRichTextPreferred(m_richTextPreferred);
    m_shape->update();
}

void SvgTextChangeCommand::undo()
{
    m_shape->update();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    // Hardcoded resolution?
    converter.convertFromSvg(m_oldSvg, m_oldDefs, m_shape->boundingRect(), 72.0);
    m_shape->setRichTextPreferred(m_oldRichTextPreferred);
    m_shape->update();
}
