/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef VIEWMODESWITCHEVENT_H
#define VIEWMODESWITCHEVENT_H

#include <QEvent>
#include <QPointF>
#include <KoColor.h>
#include <resources/KoAbstractGradient.h>
#include <kis_node.h>
#include <resources/KoPattern.h>
#include <brushengine/kis_paintop_preset.h>
#include <kis_smoothing_options.h>
#include "kis_grid_config.h"

struct ViewModeSynchronisationObject {
    ViewModeSynchronisationObject() : initialized(false) { }

    bool initialized;

    QPoint documentOffset;
    float zoomLevel;
    float rotationAngle;

    KoColor backgroundColor;
    KoColor foregroundColor;
    float exposure;
    float gamma;
    QString compositeOp;
    KoPattern* pattern;
    KoAbstractGradient* gradient;
    KisNodeSP node;
    KisPaintOpPresetSP paintOp;
    float opacity;
    bool globalAlphaLock;
    QString activeToolId;

    KisGridConfig gridConfig;

    // Mirror-axes
    QPointF mirrorAxesCenter;
    bool mirrorHorizontal;
    bool mirrorVertical;

    KisSmoothingOptionsSP smoothingOptions;
};

class ViewModeSwitchEvent : public QEvent
{
public:
    enum ViewModeEventType {
        AboutToSwitchViewModeEvent = QEvent::User + 10,
        SwitchedToDesktopModeEvent,
        SwitchedToSketchModeEvent,
    };

    inline ViewModeSwitchEvent(ViewModeEventType type, QObject* fromView, QObject* toView, ViewModeSynchronisationObject* syncObject)
            : QEvent(static_cast<QEvent::Type>(type))
            , m_fromView(fromView)
            , m_toView(toView)
            , m_syncObject(syncObject) {

    }

    inline QObject* fromView() const {
        return m_fromView;
    }
    inline QObject* toView() const {
        return m_toView;
    }

    inline ViewModeSynchronisationObject* synchronisationObject() const {
        return m_syncObject;
    }

private:
    QObject* m_fromView;
    QObject* m_toView;
    ViewModeSynchronisationObject* m_syncObject;
};

#endif // VIEWMODESWITCHEVENT_H
