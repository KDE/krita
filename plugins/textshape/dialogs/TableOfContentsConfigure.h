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

#include <QDialog>
#include "ui_TableOfContentsConfigure.h"

namespace Ui {
    class TableOfContentsConfigure;
}

class KoTextEditor;
class QTextBlock;
class TableOfContentsStyleConfigure;

class TableOfContentsConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit TableOfContentsConfigure(KoTextEditor *editor, QWidget *parent = 0);
    ~TableOfContentsConfigure();

public slots:
    void setDisplay();
    void save();

private slots:
    void tocListIndexChanged(int index);
    void showStyleConfiguration(bool show);

private:

    Ui::TableOfContentsConfigure ui;
    KoTextEditor *m_textEditor;
    TableOfContentsStyleConfigure *m_tocStyleConfigure;
    QTextDocument *document;
};

Q_DECLARE_METATYPE(QTextBlock)
#endif // TABLEOFCONTENTSCONFIGURE_H
