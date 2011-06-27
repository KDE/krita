/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
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
#ifndef CITATIONINSERTIONDIALOG_H
#define CITATIONINSERTIONDIALOG_H

#include "ui_CitationInsertionDialog.h"
#include <kdialog.h>
#include <KoListStyle.h>

#include <QTextBlock>


class TextTool;
class KoStyleManager;

class CitationInsertionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CitationInsertionDialog(QTextDocument* document,QWidget *parent = 0);

public slots:
    void setStyleManager(KoStyleManager *sm);
    void insertCitation();
    void selectionChangedFromExistingCites();

signals:
    void doneWithFocus();

private:
    Ui::CitationInsertionDialog widget;
    KoStyleManager *m_styleManager;
    bool m_blockSignals;
    bool m_comboboxHasBidiItems;
    QTextBlock m_currentBlock;
    QTextDocument *document;
};

#endif // CITATIONBIBLIOGRAPHYWIDGET_H
