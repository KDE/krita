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

#ifndef KIS_GRID_MANAGER_H
#define KIS_GRID_MANAGER_H

#include "kis_canvas_decoration.h"
#include <QPainter>

#include "kis_types.h"
#include <krita_export.h>
#include "kis_action_manager.h"
#include "kis_action.h"

class KisGridDecoration;
class KisViewManager;


class KRITAUI_EXPORT KisGridManager : public QObject
{
    Q_OBJECT
public:
    KisGridManager(KisViewManager * parent);
    virtual ~KisGridManager();
public:

    void setup(KisActionManager * actionManager);

    void setView(QPointer<KisView>imageView);

public slots:

    void updateGUI();
    void checkVisibilityAction(bool check);

private slots:

    void toggleSnapToGrid();
    void fastConfig1x1();
    void fastConfig2x2();
    void fastConfig5x5();
    void fastConfig10x10();
    void fastConfig20x20();
    void fastConfig40x40();
    void fastConfig4x4();
    void fastConfig8x8();
    void fastConfig16x16();
    void fastConfig32x32();
    void fastConfig64x64();

private:
    void setFastConfig(int size);

    KisAction *m_toggleGrid;
    KisAction* m_toggleSnapToGrid;
    KisAction* m_gridFastConfig1x1;
    KisAction* m_gridFastConfig2x2;
    KisAction* m_gridFastConfig5x5;
    KisAction* m_gridFastConfig10x10;
    KisAction* m_gridFastConfig20x20;
    KisAction* m_gridFastConfig40x40;
    KisAction* m_gridFastConfig4x4;
    KisAction* m_gridFastConfig8x8;
    KisAction* m_gridFastConfig16x16;
    KisAction* m_gridFastConfig32x32;
    KisAction* m_gridFastConfig64x64;

    QPointer<KisView> m_imageView;
    KisGridDecoration* m_gridDecoration;
};

#endif
