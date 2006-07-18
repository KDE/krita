/*
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ICON_ITEM_H_
#define KIS_ICON_ITEM_H_

#include <QTableWidgetItem>
#include <QPixmap>

class KisResource;

class KisIconItem : public QTableWidgetItem {

public:
    KisIconItem(KisResource *resource);
    virtual ~KisIconItem();

    KisResource *resource() const;

    virtual int compare(const QTableWidgetItem *other) const;

    void updatePixmaps();

private:
    KisResource *m_resource;
};

#endif // KIS_ICON_ITEM_H_

