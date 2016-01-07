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
#ifndef INSERTBIBLIOGRAPHYDIALOG_H
#define INSERTBIBLIOGRAPHYDIALOG_H

#include "ui_InsertBibliographyDialog.h"

#include <QDialog>
#include <QTextBlock>

#include <KoTextEditor.h>

class KoBibliographyInfo;
class QListWidgetItem;

class InsertBibliographyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InsertBibliographyDialog(KoTextEditor *editor, QWidget *parent = 0);
    QString bibliographyType();

public Q_SLOTS:
    void insert();
    void updateFields();
    void addField();
    void removeField();
    void addSpan();
    void insertTabStop();
    void removeTabStop();
    void spanChanged(QListWidgetItem *);

private:
    Ui::InsertBibliographyDialog dialog;
    bool m_blockSignals;
    KoTextEditor *m_editor;
    KoBibliographyInfo *m_bibInfo;
};

#endif // INSERTBIBLIOGRAPHYDIALOG_H
