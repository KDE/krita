/*
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOOLOUTLINEBASE_H
#define KISTOOLOUTLINEBASE_H

#include <kis_tool_shape.h>
#include <kis_cursor.h>

class KisInputActionGroupsMaskGuard;

class KRITAUI_EXPORT KisToolOutlineBase : public KisToolShape
{
    Q_OBJECT

public:
    enum ToolType {
        PAINT,
        SELECT
    };

    KisToolOutlineBase(KoCanvasBase *canvas,
                       ToolType type,
                       const QCursor & cursor=KisCursor::load("tool_outline_selection_cursor.png", 6, 6));
    ~KisToolOutlineBase() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void paint(QPainter& gc, const KoViewConverter &converter) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    KisPopupWidgetInterface* popupWidget() override;

    bool hasUserInteractionRunning() const;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;

    void undoLastPoint();

protected:
    virtual void finishOutline(const QVector<QPointF>& points) = 0;

private:
    static constexpr int FEEDBACK_LINE_WIDTH{2};

    QPainterPath m_paintPath;
    QVector<QPointF> m_points;
    bool m_continuedMode;
    QPointF m_lastCursorPos;
    ToolType m_type;
    int m_numberOfContinuedModePoints;
    bool m_hasUserInteractionRunning;
    QScopedPointer<KisInputActionGroupsMaskGuard> m_blockModifyingActionsGuard;

    void updateFeedback();
    void updateContinuedMode();
    void updateCanvas();
    void endStroke();
    void cancelStroke();
    QRectF dragBoundingRect();
    void installBlockActionGuard();
    void uninstallBlockActionGuard();

};

#endif
