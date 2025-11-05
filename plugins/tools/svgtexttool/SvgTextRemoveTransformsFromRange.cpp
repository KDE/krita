/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgTextRemoveTransformsFromRange.h"

SvgTextRemoveTransformsFromRange::SvgTextRemoveTransformsFromRange(KoSvgTextShape *shape, int pos, int anchor, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_pos(pos)
    , m_anchor(anchor)
    , m_textData(shape->getMemento())
{
    setText(kundo2_i18n("Remove Character Transforms"));
}

void SvgTextRemoveTransformsFromRange::undo()
{
    QRectF updateRect = m_shape->boundingRect();

    m_shape->setMemento(m_textData);

    updateRect |= m_shape->boundingRect();
    m_shape->notifyCursorPosChanged(m_pos, m_anchor);
    m_shape->updateAbsolute(updateRect);
}

void SvgTextRemoveTransformsFromRange::redo()
{
    QRectF updateRect = m_shape->boundingRect();

    const int indexPos = m_shape->indexForPos(m_pos);
    const int indexAnchor = m_shape->indexForPos(m_anchor);

    m_shape->removeTransformsFromRange(m_pos, m_anchor);

    updateRect |= m_shape->boundingRect();
    m_shape->notifyCursorPosChanged(m_shape->posForIndex(indexPos), m_shape->posForIndex(indexAnchor));
    m_shape->updateAbsolute(updateRect);
}
