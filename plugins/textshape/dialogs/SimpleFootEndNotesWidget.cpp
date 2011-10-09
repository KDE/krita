/* This file is part of the KDE project
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
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
#include "SimpleFootEndNotesWidget.h"
#include "TextTool.h"
#include "FormattingButton.h"

#include <KAction>
#include <KDebug>
#include <KoInlineNote.h>

#include <QWidget>


SimpleFootEndNotesWidget::SimpleFootEndNotesWidget(TextTool *tool ,QWidget *parent)
        : QWidget(parent)
{
    widget.setupUi(this);
    widget.addFootnote->addAction(tool->action("insert_autofootnote"));
    widget.addFootnote->addAction(tool->action("insert_labeledfootnote"));
    widget.addEndnote->addAction(tool->action("insert_autoendnote"));
    widget.addEndnote->addAction(tool->action("insert_labeledendnote"));
    widget.settings->setDefaultAction(tool->action("format_notes"));

    connect(widget.addFootnote, SIGNAL(doneWithFocus()), this, SIGNAL(doneWithFocus()));
    connect(widget.addEndnote, SIGNAL(doneWithFocus()), this, SIGNAL(doneWithFocus()));
    connect(widget.settings, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
}

#include <SimpleFootEndNotesWidget.moc>
