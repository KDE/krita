/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "NewStyleWidget.h"

#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

NewStyleWidget::NewStyleWidget(QWidget *parent)
        : QWidget(parent)
{
    widget.setupUi(this);

    QButtonGroup *bg = new QButtonGroup(this);
    bg->addButton(widget.paragraph);
    bg->addButton(widget.character);

    connect(widget.create, SIGNAL(pressed()), this, SLOT(createButtonPressed()));
}

void NewStyleWidget::createButtonPressed()
{
    if (widget.character->isChecked()) {
        KoCharacterStyle *style = new KoCharacterStyle();
        style->setName(widget.name->text());
        emit newCharacterStyle(style);
    } else {
        KoParagraphStyle *style = new KoParagraphStyle();
        style->setName(widget.name->text());
        emit newParagraphStyle(style);
    }
}

#include <NewStyleWidget.moc>
