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
#include "kis_color_patches.h"

class KisCanvas2;
class KisCommonColors;
class KisColorSelectorContainer;

class QVBoxLayout;
class QHBoxLayout;

class KisColorSelectorNgDockerWidget : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorNgDockerWidget(QWidget *parent = 0);
    void setCanvas(KisCanvas2* canvas);

public slots:
    void openSettings();
protected:

private:
    void updateLayout();

    KisColorSelectorContainer* m_colorSelectorContainer;
    KisColorPatches* m_lastColorsWidget;
    KisCommonColors* m_commonColorsWidget;

    QHBoxLayout* m_verticalColorPatchesLayout; // vertical color patches should be added here
    QVBoxLayout* m_horizontalColorPatchesLayout;//horizontal ----------"----------------------

    KisCanvas2* m_canvas;

    //color patches options
    bool m_lastColorsShow;
    KisColorPatches::Direction m_lastColorsDirection;
    bool m_lastColorsScrolling;
    int m_lastColorsColCount;
    int m_lastColorsRowCount;

    bool m_commonColorsShow;
    KisColorPatches::Direction m_commonColorsDirection;
    bool m_commonColorsScrolling;
    int m_commonColorsColCount;
    int m_commonColorsRowCount;
};

#endif // COLORSELECTORNGWIDGET_H
