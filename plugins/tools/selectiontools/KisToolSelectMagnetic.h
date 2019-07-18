/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
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

#ifndef KIS_TOOL_SELECT_MAGNETIC_H_
#define KIS_TOOL_SELECT_MAGNETIC_H_

#include <QPoint>
#include "KisSelectionToolFactoryBase.h"
#include <kis_tool_select_base.h>
#include <kis_icon.h>
#include "KisMagneticWorker.h"

class QPainterPath;

class KisToolSelectMagnetic : public KisToolSelect
{
    Q_OBJECT

public:
    KisToolSelectMagnetic(KoCanvasBase *canvas);
    ~KisToolSelectMagnetic() override = default;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void paint(QPainter& gc, const KoViewConverter &converter) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(KoPointerEvent *event) override;

    void resetCursorStyle();

public Q_SLOTS:
    void deactivate() override;
    void activate(KoToolBase::ToolActivation activation, const QSet<KoShape*> &shapes) override;
    void requestUndoDuringStroke() override;

protected:
    using KisToolSelectBase::m_widgetHelper;

private:
    void finishSelectionAction();
    void updateFeedback();
    void updateContinuedMode();
    void updateCanvas();

    QPainterPath m_paintPath;
    QVector<QPointF> m_points;
    QVector<QPoint> m_anchorPoints;
    bool m_continuedMode;
    QPointF m_lastCursorPos;
    QPoint m_lastAnchor;
    bool m_complete;
    KisMagneticWorker m_worker;
    uint m_radius, m_threshold;
    int m_checkPoint;
    QRectF m_snapBound;
};

class KisToolSelectMagneticFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectMagneticFactory()
        : KisSelectionToolFactoryBase("KisToolSelectMagnetic")
    {
        setToolTip(i18n("Magnetic Selection Tool"));
        setSection(TOOL_TYPE_SELECTION);
        setIconName(koIconNameCStr("tool_magnetic_selection"));
        setPriority(8);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolSelectMagneticFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolSelectMagnetic(canvas);
    }
};


#endif //__selecttoolmagnetic_h__

