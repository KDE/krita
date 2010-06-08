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


#ifndef COLORSELECTORNGWIDGET_H
#define COLORSELECTORNGWIDGET_H

#include <QWidget>

class KoCanvasBase;
class KisColSelNgCommonColors;

class KisColSelNgWidget : public QWidget
{
Q_OBJECT
public:
    explicit KisColSelNgWidget(QWidget *parent = 0);
    void setCanvas(KoCanvasBase* canvas);

private:
    KisColSelNgCommonColors* m_commonColorsWidget;
};

#endif // COLORSELECTORNGWIDGET_H
