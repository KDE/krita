/* This file is part of the KDE project
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
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
#include "../TextTool.h"

#include <KoParagraphStyle.h>
#include <KoTextDocument.h>
#include <KoList.h>
#include <KoTextEditor.h>
#include <KoListLevelProperties.h>
#include <commands/ParagraphFormattingCommand.h>

#include <QTextBlock>
#include <QTimer>

ParagraphSettingsDialog::ParagraphSettingsDialog(TextTool *tool, KoTextEditor *editor, QWidget *parent)
    : KoDialog(parent)
    , m_tool(tool)
    , m_editor(editor)
    , m_styleChanged(false)
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

    // Do this after initTabs so it doesn't cause signals prematurely
    connect(m_paragraphGeneral, SIGNAL(styleChanged()), this, SLOT(styleChanged()));
}

ParagraphSettingsDialog::~ParagraphSettingsDialog()
{
}

void ParagraphSettingsDialog::initTabs()
{
    KoParagraphStyle *style = KoParagraphStyle::fromBlock(m_editor->block());
    m_paragraphGeneral->setStyle(style, KoList::level(m_editor->block()));
}

void ParagraphSettingsDialog::styleChanged(bool state)
{
    m_styleChanged = state;
}

void ParagraphSettingsDialog::slotOk()
{
    slotApply();
    KoDialog::accept();
}

void ParagraphSettingsDialog::slotApply()
{
    if (!m_styleChanged) {
        return;
    }

    KoParagraphStyle chosenStyle;
    m_paragraphGeneral->save(&chosenStyle);

    QTextCharFormat cformat;
    QTextBlockFormat format;
    chosenStyle.KoCharacterStyle::applyStyle(cformat);
    chosenStyle.applyStyle(format);

    KoListLevelProperties llp;
    if (chosenStyle.listStyle()) {
        llp = chosenStyle.listStyle()->levelProperties(chosenStyle.listStyle()->listLevels().first());
    } else {
        llp.setStyle(KoListStyle::None);
    }

    m_editor->applyDirectFormatting(cformat, format, llp);

    m_styleChanged = false;
}

void ParagraphSettingsDialog::setUnit(const KoUnit &unit)
{
    m_paragraphGeneral->setUnit(unit);
}

void ParagraphSettingsDialog::setImageCollection(KoImageCollection *imageCollection)
{
    m_paragraphGeneral->setImageCollection(imageCollection);
}
