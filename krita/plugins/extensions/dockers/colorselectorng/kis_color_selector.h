/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#ifndef KIS_COLSELNG_COLOR_SELECTOR_H
#define KIS_COLSELNG_COLOR_SELECTOR_H

#include "kis_color_selector_base.h"

class QColor;
class KisColorSelectorTriangle;
class KisColorSelectorRing;

class KisColorSelector : public KisColorSelectorBase
{
public:
    KisColorSelector(QWidget* parent = 0);
    QColor pickColorAt(int x, int y);
    KisColorSelectorBase* createPopup() const;
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent* e);
private:
    KisColorSelectorRing* m_ring;
    KisColorSelectorTriangle* m_triangle;
};

#endif // KIS_COLSELNG_COLOR_SELECTOR_H
