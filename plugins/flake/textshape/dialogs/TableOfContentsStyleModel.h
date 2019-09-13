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

#ifndef TABLEOFCONTENTSSTYLEMODEL_H
#define TABLEOFCONTENTSSTYLEMODEL_H

#include <QAbstractTableModel>

class KoStyleManager;
class KoStyleThumbnailer;
class KoTableOfContentsGeneratorInfo;

class TableOfContentsStyleModel : public QAbstractTableModel
{
public:
    TableOfContentsStyleModel(const KoStyleManager *manager, KoTableOfContentsGeneratorInfo *info);
    ~TableOfContentsStyleModel() override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void saveData();

protected:
    QList<int> m_styleList; // list of style IDs
    QList<int> m_outlineLevel;

private:
    const KoStyleManager *m_styleManager;
    KoStyleThumbnailer *m_styleThumbnailer;
    KoTableOfContentsGeneratorInfo *m_tocInfo;

    int getOutlineLevel(int styleId);
    void setOutlineLevel(int styleId, int outLineLevel);
};

#endif // TABLEOFCONTENTSSTYLEMODEL_H

