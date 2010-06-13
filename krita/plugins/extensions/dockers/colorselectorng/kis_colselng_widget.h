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
#include "kis_colselng_color_patches.h"

class KoCanvasBase;
class KisColSelNgCommonColors;
class KisColSelNgBar;
class KisColSelNgColorSelector;
class KisColSelNgMyPaintShadeSelector;
class KisColSelNgShadeSelector;

class QVBoxLayout;
class QHBoxLayout;

class KisColSelNgWidget : public QWidget
{
Q_OBJECT
public:
    explicit KisColSelNgWidget(QWidget *parent = 0);
    void setCanvas(KoCanvasBase* canvas);

public slots:
    void openSettings();

private:
    void updateLayout();

    KisColSelNgBar* m_barWidget;
    KisColSelNgColorSelector* m_colorSelectorWidget;
    KisColSelNgMyPaintShadeSelector* m_myPaintShadeWidget;
    KisColSelNgShadeSelector* m_shadeSelectionWidget;
    KisColSelNgColorPatches* m_lastColorsWidget;
    KisColSelNgCommonColors* m_commonColorsWidget;

    QVBoxLayout* m_bigWidgetsLayout;
    QHBoxLayout* m_horizontalColorPatchesLayout;
    QVBoxLayout* m_verticalColorPathcesLayout;

    //color patches options
    bool m_lastColorsShow;
    KisColSelNgColorPatches::Direction m_lastColorsDirection;
    bool m_lastColorsScrolling;
    int m_lastColorsColCount;
    int m_lastColorsRowCount;

    bool m_commonColorsShow;
    KisColSelNgColorPatches::Direction m_commonColorsDirection;
    bool m_commonColorsScrolling;
    int m_commonColorsColCount;
    int m_commonColorsRowCount;
};

#endif // COLORSELECTORNGWIDGET_H
