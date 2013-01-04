/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef FLIPBOOKVIEW_H
#define FLIPBOOKVIEW_H

#include <QListView>

class FlipbookView : public QListView
{
    Q_OBJECT
public:
    explicit FlipbookView(QWidget *parent = 0);
    
    void goPrevious();
    void goNext();


signals:
    void currentItemChanged(const QModelIndex &);

protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    
};

#endif // FLIPBOOKVIEW_H
