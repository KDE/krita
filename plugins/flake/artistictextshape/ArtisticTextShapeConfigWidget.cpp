/* This file is part of the KDE project
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ArtisticTextShapeConfigWidget.h"
#include "ArtisticTextTool.h"
#include "ArtisticTextToolSelection.h"
#include "ArtisticTextShape.h"

#include <QAction>

ArtisticTextShapeConfigWidget::ArtisticTextShapeConfigWidget(ArtisticTextTool *textTool)
    : m_textTool(textTool)
{
    Q_ASSERT(m_textTool);

    widget.setupUi(this);

    widget.bold->setDefaultAction(textTool->action("artistictext_font_bold"));
    widget.italic->setDefaultAction(textTool->action("artistictext_font_italic"));
    widget.superScript->setDefaultAction(textTool->action("artistictext_superscript"));
    widget.subScript->setDefaultAction(textTool->action("artistictext_subscript"));
    widget.anchorStart->setDefaultAction(textTool->action("artistictext_anchor_start"));
    widget.anchorMiddle->setDefaultAction(textTool->action("artistictext_anchor_middle"));
    widget.anchorEnd->setDefaultAction(textTool->action("artistictext_anchor_end"));
    widget.fontSize->setRange(2, 1000);

    connect(widget.fontFamily, SIGNAL(currentFontChanged(QFont)), this, SIGNAL(fontFamilyChanged(QFont)));
    connect(widget.fontSize, SIGNAL(valueChanged(int)), this, SIGNAL(fontSizeChanged(int)));
}

void ArtisticTextShapeConfigWidget::blockChildSignals(bool block)
{
    widget.fontFamily->blockSignals(block);
    widget.fontSize->blockSignals(block);
}

void ArtisticTextShapeConfigWidget::updateWidget()
{
    ArtisticTextToolSelection *selection = dynamic_cast<ArtisticTextToolSelection *>(m_textTool->selection());
    if (!selection) {
        return;
    }

    ArtisticTextShape *currentText = selection->selectedShape();
    if (!currentText) {
        return;
    }

    blockChildSignals(true);

    QFont font = currentText->fontAt(m_textTool->textCursor());

    widget.fontSize->setValue(font.pointSize());
    font.setPointSize(8);
    widget.fontFamily->setCurrentFont(font);

    blockChildSignals(false);
}
