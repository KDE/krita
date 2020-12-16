/*
 *  SPDX-FileCopyrightText: 2003-2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
