/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "ArtisticTextShapeOnPathWidget.h"
#include "ui_ArtisticTextShapeOnPathWidget.h"
#include "ArtisticTextTool.h"
#include <QAction>

ArtisticTextShapeOnPathWidget::ArtisticTextShapeOnPathWidget(ArtisticTextTool *tool, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ArtisticTextShapeOnPathWidget)
    , m_textTool(tool)
{
    ui->setupUi(this);
    ui->detachFromPath->setDefaultAction(tool->action("artistictext_detach_from_path"));
    ui->convertToPath->setDefaultAction(tool->action("artistictext_convert_to_path"));

    connect(ui->startOffset, SIGNAL(valueChanged(int)), this, SIGNAL(offsetChanged(int)));
}

ArtisticTextShapeOnPathWidget::~ArtisticTextShapeOnPathWidget()
{
    delete ui;
}

void ArtisticTextShapeOnPathWidget::updateWidget()
{
    ArtisticTextToolSelection *selection = dynamic_cast<ArtisticTextToolSelection *>(m_textTool->selection());
    if (!selection) {
        return;
    }

    ArtisticTextShape *currentText = selection->selectedShape();
    if (!currentText) {
        return;
    }

    ui->startOffset->blockSignals(true);
    ui->startOffset->setValue(static_cast<int>(currentText->startOffset() * 100.0));
    ui->startOffset->setEnabled(currentText->isOnPath());
    ui->startOffset->blockSignals(false);
}
