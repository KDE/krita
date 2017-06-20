/*
 *  Copyright (c) 2003-2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_CANVAS_CONTROLS_MANAGER_H
#define KIS_CANVAS_CONTROLS_MANAGER_H

#include <QObject>
#include <QPointer>

#include <kritaui_export.h>

class KisViewManager;
class KisActionManager;
class KisView;

class KRITAUI_EXPORT KisCanvasControlsManager: public QObject
{
    Q_OBJECT

public:
    KisCanvasControlsManager(KisViewManager * view);
    ~KisCanvasControlsManager() override;

    void setup(KisActionManager *actionManager);
    void setView(QPointer<KisView>imageView);

private Q_SLOTS:
    void makeColorLighter();
    void makeColorDarker();
    void makeColorDesaturated();
    void makeColorSaturated();
    void shiftHueClockWise();
    void shiftHueCounterClockWise();
    void makeColorRed();
    void makeColorGreen();
    void makeColorBlue();
    void makeColorYellow();

    void increaseOpacity();
    void decreaseOpacity();
private:
    void transformColor(int step);
    void transformSaturation(int step);
    void transformHue(int step);
    void transformRed(int step);
    void transformBlue(int step);
    void stepAlpha(float step);

private:
    KisViewManager * m_view;
};

#endif // KIS_CANVAS_CONTROLS_MANAGER_H
