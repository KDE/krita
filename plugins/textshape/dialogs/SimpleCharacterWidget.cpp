/* This file is part of the KDE project
 * Copyright (C) 2007, 2008, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009-2010 Casper Boemann <cbo@boemann.dk>
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
#include "SimpleCharacterWidget.h"
#include "TextTool.h"
#include "../ListItemsHelper.h"
#include "../commands/ChangeListCommand.h"

#include <KAction>
#include <KoTextBlockData.h>
#include <KoParagraphStyle.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocumentLayout.h>
#include <KoZoomHandler.h>

#include <KDebug>

#include <QTextLayout>
#include <QComboBox>

SimpleCharacterWidget::SimpleCharacterWidget(TextTool *tool, QWidget *parent)
        : QWidget(parent),
        m_blockSignals(false),
        m_comboboxHasBidiItems(false),
        m_tool(tool)
{
    widget.setupUi(this);
    widget.bold->setDefaultAction(tool->action("format_bold"));
    widget.italic->setDefaultAction(tool->action("format_italic"));
    widget.strikeOut->setDefaultAction(tool->action("format_strike"));
    widget.underline->setDefaultAction(tool->action("format_underline"));
    widget.textColor->setDefaultAction(tool->action("format_textcolor"));
    widget.backgroundColor->setDefaultAction(tool->action("format_backgroundcolor"));
    widget.superscript->setDefaultAction(tool->action("format_super"));
    widget.subscript->setDefaultAction(tool->action("format_sub"));

    QComboBox *family = qobject_cast<QComboBox*> (tool->action("format_fontfamily")->requestWidget(this));
    if (family) { // kdelibs 4.1 didn't return anything here.
        widget.fontsFrame->addWidget(family,0,0);
        connect(family, SIGNAL(activated(int)), this, SIGNAL(doneWithFocus()));
    }
    QComboBox *size = qobject_cast<QComboBox*> (tool->action("format_fontsize")->requestWidget(this));
    if (size) { // kdelibs 4.1 didn't return anything here.
        widget.fontsFrame->addWidget(size,0,1);
        connect(size, SIGNAL(activated(int)), this, SIGNAL(doneWithFocus()));
    }

    widget.fontsFrame->setColumnStretch(0,1);
}

void SimpleCharacterWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

void SimpleCharacterWidget::setCurrentFormat(const QTextCharFormat& format)
{
    Q_UNUSED(format);
}

#include <SimpleCharacterWidget.moc>
