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

#ifndef TABLEOFCONTENTSSTYLECONFIGURE_H
#define TABLEOFCONTENTSSTYLECONFIGURE_H

#include "TableOfContentsStyleModel.h"
#include "TableOfContentsStyleDelegate.h"

#include <QDialog>

namespace Ui
{
class TableOfContentsStyleConfigure;
}

class QStandardItemModel;
class KoStyleManager;
class TableOfContentsStyleModel;
class KoTableOfContentsGeneratorInfo;

class TableOfContentsStyleConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit TableOfContentsStyleConfigure(KoStyleManager *manager, QWidget *parent = 0);
    ~TableOfContentsStyleConfigure();
    void initializeUi(KoTableOfContentsGeneratorInfo *info);

public Q_SLOTS:
    void save();
    void discardChanges();

private:
    Ui::TableOfContentsStyleConfigure *ui;
    QStandardItemModel *m_stylesTree;
    KoStyleManager *m_styleManager;
    KoTableOfContentsGeneratorInfo *m_tocInfo;
    TableOfContentsStyleModel *m_stylesModel;
    TableOfContentsStyleDelegate m_delegate;
};

#endif // TABLEOFCONTENTSSTYLECONFIGURE_H
