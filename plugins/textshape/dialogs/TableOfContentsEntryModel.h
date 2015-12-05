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

#ifndef TABLEOFCONTENTSENTYMODEL_H
#define TABLEOFCONTENTSENTYMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QPair>

class KoStyleManager;
class KoTableOfContentsGeneratorInfo;

class TableOfContentsEntryModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ModelColumns { Levels = 0, Styles = 1 };
    TableOfContentsEntryModel(KoStyleManager *manager, KoTableOfContentsGeneratorInfo *info);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void saveData();

Q_SIGNALS:
    void tocEntryDataChanged();

private:

    QList <QPair <QString, int> > m_tocEntries; //first contains the text that will appear in table view, and second one is the styleId
    KoStyleManager *m_styleManager;
    KoTableOfContentsGeneratorInfo *m_tocInfo;
};

#endif // TABLEOFCONTENTSENTYMODEL_H
