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

class KoCanvasBase;
class KisCommonColors;
class KisColSelNgBar;
class KisColorSelector;
class KisMyPaintShadeSelector;
class KisMinimalShadeSelector;

class QVBoxLayout;
class QHBoxLayout;

class KisColorSelectorNgDockerWidget : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorNgDockerWidget(QWidget *parent = 0);
    void setCanvas(KoCanvasBase* canvas);

public slots:
    void openSettings();
protected:
    void resizeEvent(QResizeEvent* e=0);

private:
    void updateLayout();

    KisColSelNgBar* m_barWidget;
    KisColorSelector* m_colorSelectorWidget;
    KisMyPaintShadeSelector* m_myPaintShadeWidget;
    KisMinimalShadeSelector* m_minimalShadeWidget;
    KisColorPatches* m_lastColorsWidget;
    KisCommonColors* m_commonColorsWidget;
    QWidget* m_bigWidgetsParent;

    QHBoxLayout* m_verticalColorPatchesLayout; // vertical color patches should be added here
    QVBoxLayout* m_horizontalColorPatchesLayout;//horizontal ----------"----------------------
    QVBoxLayout* m_standardBarLayout;

    //shade selector options
    QWidget* m_shadeWidget;
    bool m_shadeSelectorHideable;

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
