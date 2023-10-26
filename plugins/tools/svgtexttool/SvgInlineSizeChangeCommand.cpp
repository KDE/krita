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

bool changeTextShapeInlineSize(KoSvgTextShape *const shape, const double newInlineSize)
{
    KoSvgTextShapeMarkupConverter converter(shape);
    QString svg;
    QString defs;
    converter.convertToSvg(&svg, &defs);

    // Wow, phasing XML with regex?
    // Trust me, this isn't as bad as it sounds, because the svg output has a
    // strict, deterministic format that we have control of, and we don't need
    // to parse anything beyond the attribute list of the root element.

    bool success = false;

    static QRegularExpression s_regexFindStyle(
        QStringLiteral(R"==(^<text(?:\s+[^=>\s]+="[^"]*")*\s+style="([^"]*)")=="));
    QRegularExpressionMatch matchStyle = s_regexFindStyle.match(svg);
    if (matchStyle.hasMatch()) {
        static QRegularExpression s_regexFindInlineSize(
            QStringLiteral(R"==((?:^|;)inline-size:\s*(\d+(?:\.\d+)?)\s*(?:$|;))=="));
        QRegularExpressionMatch matchInlineSize = s_regexFindInlineSize.match(matchStyle.capturedRef(1));
        if (matchInlineSize.hasMatch()) {
            const int start = matchStyle.capturedStart(1) + matchInlineSize.capturedStart(1);
            const int length = matchInlineSize.capturedLength(1);
            svg.replace(start, length, QString::number(newInlineSize));
            success = true;
        } else {
            qWarning() << "SvgInlineSizeChangeCommand: Cannot find inline-size from text";
        }
    } else {
        qWarning() << "SvgInlineSizeChangeCommand: Cannot find style attribute from text";
    }

    if (Q_UNLIKELY(!success)) {
        // Currently, Krita does not write inline-size as an attribute, but
        // also try this just in case.
        static QRegularExpression s_regexFindInlineSizeAttr(
            QStringLiteral(R"==(^<text(?:\s+[^=>\s]+="[^"]*")*\s+inline-size="(\d+(?:\.\d+)?)")=="));
        QRegularExpressionMatch matchInlineSizeAttr = s_regexFindInlineSizeAttr.match(svg);

        if (matchInlineSizeAttr.hasMatch()) {
            const int start = matchInlineSizeAttr.capturedStart(1);
            const int length = matchInlineSizeAttr.capturedLength(1);
            svg.replace(start, length, QString::number(newInlineSize));
            success = true;
        } else {
            qWarning() << "SvgInlineSizeChangeCommand: Cannot find inline-size attribute from text";
        }
    }

    if (Q_UNLIKELY(!success)) {
        qWarning() << "SvgInlineSizeChangeCommand: Failed to set inline-size";
        return false;
    }

    converter.convertFromSvg(svg, defs, shape->boundingRect(), 72);
    return true;
}

void SvgInlineSizeChangeCommand::applyInlineSize(double inlineSize)
{
    QRectF updateRect = m_shape->boundingRect();
    if (changeTextShapeInlineSize(m_shape, inlineSize)) {
        updateRect |= m_shape->boundingRect();
        m_shape->updateAbsolute(updateRect);
    }
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
