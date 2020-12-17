/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_OPENGL_CANVAS_DEBUGGER_H
#define __KIS_OPENGL_CANVAS_DEBUGGER_H

#include <QScopedPointer>
#include <QObject>


class KisOpenglCanvasDebugger : public QObject
{
    Q_OBJECT
public:
    KisOpenglCanvasDebugger();
    ~KisOpenglCanvasDebugger();

    static KisOpenglCanvasDebugger* instance();

    bool showFpsOnCanvas() const;

    void nofityPaintRequested();
    void nofitySyncStatus(bool value);
    qreal accumulatedFps();

private Q_SLOTS:
    void slotConfigChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_OPENGL_CANVAS_DEBUGGER_H */
