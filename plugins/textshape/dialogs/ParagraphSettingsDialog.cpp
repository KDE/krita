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

#include "ParagraphSettingsDialog.h"

#include "ParagraphGeneral.h"
#include "../commands/ChangeListCommand.h"
#include "../TextTool.h"

#include <KoParagraphStyle.h>
#include <KoTextDocument.h>

#include <QTextBlock>
#include <QTimer>

ParagraphSettingsDialog::ParagraphSettingsDialog(TextTool *tool, QTextCursor *cursor, QWidget* parent)
        : KDialog(parent),
        m_tool(tool),
        m_cursor(cursor)
{
    setCaption(i18n("Paragraph Format"));
    setModal(true);
    setButtons(Ok | Cancel | Apply);
    setDefaultButton(Ok);

    m_paragraphGeneral = new ParagraphGeneral;
    m_paragraphGeneral->hideStyleName(true);
    setMainWidget(m_paragraphGeneral);

    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    initTabs();
}

ParagraphSettingsDialog::~ParagraphSettingsDialog()
{
}

void ParagraphSettingsDialog::initTabs()
{
    KoParagraphStyle *style = KoParagraphStyle::fromBlock(m_cursor->block());
    m_paragraphGeneral->setStyle(style, KoList::level(m_cursor->block()));
}

void ParagraphSettingsDialog::slotOk()
{
    slotApply();
    KDialog::accept();
}

void ParagraphSettingsDialog::slotApply()
{
    emit startMacro(i18n("Paragraph Settings\n"));
    KoParagraphStyle chosenStyle;
    m_paragraphGeneral->save(&chosenStyle);
    QTextBlockFormat format;
    chosenStyle.applyStyle(format);
    m_cursor->mergeBlockFormat(format);
    if (chosenStyle.listStyle()) {
        ChangeListCommand *cmd = new ChangeListCommand(*m_cursor, chosenStyle.listStyle(), 0, ChangeListCommand::MergeWithAdjacentList);
        m_tool->addCommand(cmd);
    } else {
        QTextList *list = m_cursor->block().textList();
        if (list) { // then remove it.
            list->remove(m_cursor->block());
        }
    }
    emit stopMacro();
}

void ParagraphSettingsDialog::setUnit(const KoUnit &unit)
{
    m_paragraphGeneral->setUnit(unit);
}

#include <ParagraphSettingsDialog.moc>
