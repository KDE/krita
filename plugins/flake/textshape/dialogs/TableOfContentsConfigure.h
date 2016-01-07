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

#ifndef TABLEOFCONTENTSCONFIGURE_H
#define TABLEOFCONTENTSCONFIGURE_H

#include "ui_TableOfContentsConfigure.h"

#include <KoZoomHandler.h>

#include <QDialog>
#include <QTextBlock>

namespace Ui
{
class TableOfContentsConfigure;
}

class QTextBlock;
class TableOfContentsStyleConfigure;
class TableOfContentsEntryModel;
class TableOfContentsEntryDelegate;
class KoTableOfContentsGeneratorInfo;
class KoTextEditor;

class TableOfContentsConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit TableOfContentsConfigure(KoTextEditor *editor, QTextBlock block, QWidget *parent = 0);
    TableOfContentsConfigure(KoTextEditor *editor, KoTableOfContentsGeneratorInfo *info, QWidget *parent = 0);
    ~TableOfContentsConfigure();
    KoTableOfContentsGeneratorInfo *currentToCData();

public Q_SLOTS:
    void setDisplay();
    void save();
    void cleanUp();
    void updatePreview();

private Q_SLOTS:
    void showStyleConfiguration();
    void titleTextChanged(const QString &text);
    void useOutline(int state);
    void useIndexSourceStyles(int state);

private:

    Ui::TableOfContentsConfigure ui;
    KoTextEditor *m_textEditor;
    TableOfContentsStyleConfigure *m_tocStyleConfigure;
    KoTableOfContentsGeneratorInfo *m_tocInfo;
    QTextBlock m_block;
    QTextDocument *m_document;
    TableOfContentsEntryModel *m_tocEntryStyleModel;
    TableOfContentsEntryDelegate *m_tocEntryConfigureDelegate;

    void init();
};

Q_DECLARE_METATYPE(QTextBlock)
#endif // TABLEOFCONTENTSCONFIGURE_H
