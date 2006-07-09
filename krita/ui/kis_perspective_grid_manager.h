/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PERSPECTIVE_GRID_MANAGER_H
#define KIS_PERSPECTIVE_GRID_MANAGER_H

#include <qobject.h>
#include <qpoint.h>

class KActionCollection;
class KToggleAction;
class KisView;

struct KisPerspectiveGrid {
    QPoint topLeft, topRight, bottomLeft, bottomRight;
};

class KisPerspectiveGridManager : public QObject
{
    Q_OBJECT
    public:
        KisPerspectiveGridManager(KisView * parent);
        ~KisPerspectiveGridManager();
        void setup(KActionCollection * collection);
        void drawGrid(QRect wr, QPainter *p, bool openGL = false);
    public slots:
        void updateGUI();
    private slots:
        void toggleGrid();
    private:
        KisView* m_view;
        KToggleAction* m_toggleGrid;
        KisPerspectiveGrid m_grid;
};

#endif
