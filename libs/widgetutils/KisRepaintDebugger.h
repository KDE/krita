/*
 * SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_REPAINT_DEBUGGER_H
#define KIS_REPAINT_DEBUGGER_H

#include <QtGlobal>

#include <kritawidgetutils_export.h>

class QPaintDevice;
class QPaintEvent;
class QRect;

/**
 * A utility class to aid debugging widget or surface redraws. It lets you
 * paint out the update rects and makes it obvious by cycling between colors
 * with sharp contrast for each paint. This class is controlled globally by
 * the environment variable KRITA_DEBUG_REPAINT. KisRepaintDebugger will only
 * work when this environment variable is set to `1`.
 *
 * For optimal effect, one of the `paint` methods shall be called at the end
 * of the `paintEvent`, after all the custom painting. If a QPainter has been
 * instantiated at the topmost function scope, you must explicitly call `end()`
 * on it to release the paint device so that KisRepaintDebugger can use it.
 *
 * This class is stateful, so it shall be kept as a class member object and
 * reused.
 *
 * Seizure Warning: Enabling repaint debug will produce heavy flashing visuals.
 * This may potentially trigger seizures for people with photosensitive epilepsy.
 */
class KRITAWIDGETUTILS_EXPORT KisRepaintDebugger
{
public:
    /**
     * Whether KisRepaintDebugger is enabled globally. This is controlled
     * by the environment variable KRITA_DEBUG_REPAINT.
     */
    static bool enabled();

    KisRepaintDebugger() = default;
    ~KisRepaintDebugger() = default;

    void paint(QPaintDevice *paintDevice, const QRect &widgetRect);
    void paint(QPaintDevice *paintDevice, const QVector<QRect> &widgetRects);
    void paint(QPaintDevice *paintDevice, const QPaintEvent *event);
    void paintFull(QPaintDevice *paintDevice);

private:
    void paint(QPaintDevice *paintDevice, const QRect *widgetRects, size_t count);

private:
    unsigned int m_colorIndex {0};
};

#endif // KIS_REPAINT_DEBUGGER_H
