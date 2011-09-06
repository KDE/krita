/* This file is part of the KDE project
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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


#include "TableOfContentsConfigure.h"
#include "TableOfContentsStyleConfigure.h"
#include "KoTableOfContentsGeneratorInfo.h"
#include "KoTextDocument.h"

#include <KoTextEditor.h>
#include <KoParagraphStyle.h>



TableOfContentsConfigure::TableOfContentsConfigure(KoTextEditor *editor, QTextBlock block, QWidget *parent) :
    QDialog(parent),
    m_textEditor(editor),
    m_document(0),
    m_tocStyleConfigure(0),
    m_tocInfo(0),
    m_block(block)
{
    ui.setupUi(this);

    ui.lineEditTitle->setText(i18n("Table Title"));
    ui.useOutline->setText(i18n("Use outline"));
    ui.useStyles->setText(i18n("Use styles"));
    ui.configureStyles->setText(i18n("Configure"));

    ui.tocPreview->setStyleManager(KoTextDocument(m_textEditor->document()).styleManager());

    connect(this, SIGNAL(accepted()), this, SLOT(save()));
    connect(this, SIGNAL(rejected()), this, SLOT(cleanUp()));
    connect(ui.configureStyles, SIGNAL(clicked(bool)), this, SLOT(showStyleConfiguration(bool)));
    connect(ui.lineEditTitle, SIGNAL(returnPressed()), this, SLOT(updatePreview()));

    KoTableOfContentsGeneratorInfo *info = block.blockFormat().property(KoParagraphStyle::TableOfContentsData).value<KoTableOfContentsGeneratorInfo*>();
    m_tocInfo = info->clone();

    setVisible(true);
    setDisplay();
}

TableOfContentsConfigure::~TableOfContentsConfigure()
{
}

void TableOfContentsConfigure::setDisplay()
{
    ui.lineEditTitle->setText(m_tocInfo->m_indexTitleTemplate.text);
    ui.useOutline->setCheckState(m_tocInfo->m_useOutlineLevel ? Qt::Checked : Qt::Unchecked);
    ui.useStyles->setCheckState(m_tocInfo->m_useIndexSourceStyles ? Qt::Checked : Qt::Unchecked);

    connect(ui.lineEditTitle, SIGNAL(textChanged(const QString &)), this, SLOT(titleTextChanged(const QString&)));
    connect(ui.useOutline, SIGNAL(stateChanged(int )), this, SLOT(useOutline(int)));
    connect(ui.useStyles, SIGNAL(stateChanged(int )), this, SLOT(useIndexSourceStyles(int)));

    connect(this, SIGNAL(accepted()), this, SLOT(save()));
    connect(this, SIGNAL(rejected()), this, SLOT(cleanUp()));

    updatePreview();
}

void TableOfContentsConfigure::save()
{
    m_tocInfo->m_name = ui.lineEditTitle->text();
    m_tocInfo->m_indexTitleTemplate.text = ui.lineEditTitle->text();
    m_tocInfo->m_useOutlineLevel = ui.useOutline->checkState() == Qt::Checked ? true : false;
    m_tocInfo->m_useIndexSourceStyles = ui.useStyles->checkState() == Qt::Checked ? true : false;

    m_textEditor->updateTableOfContents(m_tocInfo, m_block);
    cleanUp();
}

void TableOfContentsConfigure::showStyleConfiguration(bool show)
{
    if (!m_tocStyleConfigure) {
        m_tocStyleConfigure = new TableOfContentsStyleConfigure(KoTextDocument(m_textEditor->document()).styleManager(), this);
    }
    m_tocStyleConfigure->initializeUi(m_tocInfo);

}

void TableOfContentsConfigure::titleTextChanged(const QString &text)
{
    m_tocInfo->m_indexTitleTemplate.text = text;
    updatePreview();
}

void TableOfContentsConfigure::useOutline(int state)
{
    m_tocInfo->m_useOutlineLevel = ui.useOutline->checkState() == Qt::Checked ? true : false;
    updatePreview();
}

void TableOfContentsConfigure::useIndexSourceStyles(int state)
{
    m_tocInfo->m_useIndexSourceStyles = ui.useStyles->checkState() == Qt::Checked ? true : false;
    updatePreview();
}

void TableOfContentsConfigure::updatePreview()
{
    ui.tocPreview->updatePreview(m_tocInfo);
}

void TableOfContentsConfigure::cleanUp()
{
    disconnect(ui.lineEditTitle, SIGNAL(textChanged (const QString &)), this, SLOT(titleTextChanged(const QString &)));
    disconnect(ui.useOutline, SIGNAL(stateChanged(int )), this, SLOT(useOutline(int)));
    disconnect(ui.useStyles, SIGNAL(stateChanged(int )), this, SLOT(useIndexSourceStyles(int)));

    disconnect(this, SIGNAL(accepted()), this, SLOT(save()));
    disconnect(this, SIGNAL(rejected()), this, SLOT(cleanUp()));

    delete m_tocInfo;
    m_tocInfo=0;
}

