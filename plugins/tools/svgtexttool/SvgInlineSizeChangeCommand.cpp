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
#include <KoShapeBulkActionLock.h>

#include "kis_assert.h"
#include "kis_command_ids.h"

SvgInlineSizeChangeCommand::SvgInlineSizeChangeCommand(KoSvgTextShape *shape, double inlineSize, KUndo2Command *parent)
    : SvgInlineSizeChangeCommand(shape, inlineSize, SvgInlineSizeHelper::getInlineSizePt(shape), 0, 0, QPointF(), QPointF(), parent)
{
}

SvgInlineSizeChangeCommand::SvgInlineSizeChangeCommand(KoSvgTextShape *shape,
                                                       double inlineSize,
                                                       double oldInlineSize,
                                                       int anchor,
                                                       int oldAnchor,
                                                       QPointF movePos,
                                                       QPointF oldPos,
                                                       KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_inlineSize(inlineSize)
    , m_oldInlineSize(oldInlineSize)
    , m_anchor(anchor)
    , m_oldAnchor(oldAnchor)
    , m_originalPos(oldPos)
    , m_movePos(movePos)
{
    setText(kundo2_i18n("Adjust text auto wrap"));
}

void SvgInlineSizeChangeCommand::applyInlineSize(double inlineSize, int anchor, QPointF pos, bool undo)
{
    KoShapeBulkActionLock lock(m_shape);

    KoSvgTextProperties properties = m_shape->propertiesForPos(-1);
    KoSvgText::AutoValue inlineSizeProp = properties.propertyOrDefault(KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();
    inlineSizeProp.customValue = inlineSize;
    inlineSizeProp.isAuto = false;
    properties.setProperty(KoSvgTextProperties::InlineSizeId, KoSvgText::fromAutoValue(inlineSizeProp));
    properties.setProperty(KoSvgTextProperties::TextAnchorId, QVariant(anchor));

    if (undo) {
        m_shape->setPropertiesAtPos(-1, properties);
        m_shape->setAbsolutePosition(pos, KoFlake::TopLeft);
    } else {
        m_shape->setAbsolutePosition(pos, KoFlake::TopLeft);
        m_shape->setPropertiesAtPos(-1, properties);
    }

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

void SvgInlineSizeChangeCommand::redo()
{
    applyInlineSize(m_inlineSize, m_anchor, m_movePos, false);
}

void SvgInlineSizeChangeCommand::undo()
{
    applyInlineSize(m_oldInlineSize, m_oldAnchor, m_originalPos, true);
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
    m_anchor = other->m_anchor;
    m_movePos = other->m_movePos;

    return true;
}
