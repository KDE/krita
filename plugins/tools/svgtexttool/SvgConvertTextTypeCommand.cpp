/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgConvertTextTypeCommand.h"

SvgConvertTextTypeCommand::SvgConvertTextTypeCommand(KoSvgTextShape *shape, ConversionType type, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_conversionType(type)
{
    setText(kundo2_i18n("Convert Text Type"));
}

void SvgConvertTextTypeCommand::redo()
{
    QRectF updateRect = m_shape->boundingRect();
    m_textData = m_shape->getMemento();

    if (m_conversionType == ToPreFormatted) {
        m_shape->convertCharTransformsToPreformatted(false);
    } else if (m_conversionType == ToInlineSize) {
        m_shape->convertCharTransformsToPreformatted(true);
    }

    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());
}

void SvgConvertTextTypeCommand::undo()
{
    QRectF updateRect = m_shape->boundingRect();
    m_shape->setMemento(m_textData, 0, 0);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());
}
