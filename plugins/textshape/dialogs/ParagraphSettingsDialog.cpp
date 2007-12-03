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

#include "ParagraphIndentSpacing.h"
#include "ParagraphLayout.h"
#include "ParagraphBulletsNumbers.h"
#include "../commands/ChangeListCommand.h"
#include "../TextTool.h"

#include <KoParagraphStyle.h>
#include <KoLayoutVisitor.h>

#include <QTextBlock>
#include <QTimer>


ParagraphSettingsDialog::ParagraphSettingsDialog(QWidget *parent, TextTool *tool)
    : KPageDialog(parent),
    m_tool(tool),
    m_style(0),
    m_ownStyle(false),
    m_visited(false)
{
    setFaceType(KPageDialog::Tabbed);
    m_paragraphIndentSpacing = new ParagraphIndentSpacing (this);
    m_paragraphLayout = new ParagraphLayout (this);
    m_paragraphBulletsNumbers = new ParagraphBulletsNumbers (this);

    addPage(m_paragraphIndentSpacing, "Indent/Spacing");
    addPage(m_paragraphLayout, "General Layout");
    addPage(m_paragraphBulletsNumbers, "Bullets/Numbers");
}

ParagraphSettingsDialog::~ParagraphSettingsDialog() {
    if(m_ownStyle)
        delete m_style;
}

void ParagraphSettingsDialog::accept() {
    if(m_style) {
        emit startMacro(i18n("Paragraph Settings\n"));
        m_paragraphIndentSpacing->save();
        m_paragraphLayout->save();
        m_paragraphBulletsNumbers->save();

        QTextBlockFormat format;
        m_style->applyStyle(format);
        m_cursor.mergeBlockFormat(format);
        if(m_style->listStyle().isValid()) {
            ChangeListCommand *cmd = new ChangeListCommand(m_cursor.block(), m_style->listStyle());
            m_tool->addCommand(cmd);
        }
        else {
            QTextList *list = m_cursor.block().textList();
            if(list) { // then remove it.
                list->remove(m_cursor.block());
            }
        }

        emit stopMacro();
    }

    KDialog::accept();
    deleteLater();
}

void ParagraphSettingsDialog::reject() {
    KDialog::reject();
    deleteLater();
}

void ParagraphSettingsDialog::open(const QTextCursor &cursor) {
    m_cursor = cursor;
    m_ownStyle = true;
    open( KoParagraphStyle::fromBlock(m_cursor.block()) );
}

void ParagraphSettingsDialog::open(KoParagraphStyle *style) {
    m_style = style;
    m_paragraphIndentSpacing->open(style);
    m_paragraphLayout->open(style);
    m_paragraphBulletsNumbers->open(style);
}

void ParagraphSettingsDialog::setUnit(const KoUnit &unit) {
    m_paragraphIndentSpacing->setUnit(unit);
}

void ParagraphSettingsDialog::showEvent (QShowEvent *e) {
    KDialog::showEvent(e);
    if(m_visited) return;
    m_visited = true;
    QTimer::singleShot(0, this, SLOT(visit()));
}

void ParagraphSettingsDialog::visit() {
    KoLayoutVisitor visitor;
    visitor.visit(m_paragraphBulletsNumbers);
    visitor.visit(m_paragraphLayout);
    visitor.visit(m_paragraphIndentSpacing);
    visitor.relayout();
}

#include <ParagraphSettingsDialog.moc>
