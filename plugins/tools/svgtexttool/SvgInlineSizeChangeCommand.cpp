/*
 * SPDX-FileCopyrightText: 2023 Alvin <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgInlineSizeChangeCommand.h"
#include "SvgInlineSizeHelper.h"

#include <QRegularExpression>

#include <klocalizedstring.h>

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"

#include "kis_assert.h"
#include "kis_command_ids.h"

SvgInlineSizeChangeCommand::SvgInlineSizeChangeCommand(KoSvgTextShape *shape, double inlineSize, KUndo2Command *parent)
    : SvgInlineSizeChangeCommand(shape, inlineSize, SvgInlineSizeHelper::getInlineSizePt(shape), parent)
{
}

SvgInlineSizeChangeCommand::SvgInlineSizeChangeCommand(KoSvgTextShape *shape,
                                                       double inlineSize,
                                                       double oldInlineSize,
                                                       KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_inlineSize(inlineSize)
    , m_oldInlineSize(oldInlineSize)
{
    setText(kundo2_i18n("Adjust text auto wrap"));
}

void SvgInlineSizeChangeCommand::applyInlineSize(double inlineSize)
{
    QRectF updateRect = m_shape->boundingRect();

    KoSvgTextProperties properties = m_shape->textProperties();
    KoSvgText::AutoValue inlineSizeProp = properties.propertyOrDefault(KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();
    inlineSizeProp.customValue = inlineSize;
    inlineSizeProp.isAuto = false;
    properties.setProperty(KoSvgTextProperties::InlineSizeId, KoSvgText::fromAutoValue(inlineSizeProp));
    m_shape->setPropertiesAtPos(-1, properties);
    m_shape->updateAbsolute(updateRect);
}

void SvgInlineSizeChangeCommand::redo()
{
    applyInlineSize(m_inlineSize);
}

void SvgInlineSizeChangeCommand::undo()
{
    applyInlineSize(m_oldInlineSize);
}

int SvgInlineSizeChangeCommand::id() const
{
    return KisCommandUtils::SvgInlineSizeChangeCommand;
}

bool SvgInlineSizeChangeCommand::mergeWith(const KUndo2Command *otherCommand)
{
    const SvgInlineSizeChangeCommand *other = dynamic_cast<const SvgInlineSizeChangeCommand *>(otherCommand);

    if (!other || other->m_shape != m_shape) {
        return false;
    }

    m_inlineSize = other->m_inlineSize;

    return true;
}
