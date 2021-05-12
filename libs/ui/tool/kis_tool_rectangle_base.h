/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_TOOL_RECTANGLE_BASE_H
#define KIS_TOOL_RECTANGLE_BASE_H

#include <kis_tool_shape.h>
#include <kis_cursor.h>

class KRITAUI_EXPORT KisToolRectangleBase : public KisToolShape
{
Q_OBJECT

Q_SIGNALS:
    void rectangleChanged(const QRectF &newRect);
    void sigRequestReloadConfig();

public Q_SLOTS:
    void constraintsChanged(bool forceRatio, bool forceWidth, bool forceHeight, float ratio, float width, float height);
    void roundCornersChanged(int rx, int ry);
public:
    enum ToolType {
        PAINT,
        SELECT
    };

    explicit KisToolRectangleBase(KoCanvasBase * canvas, KisToolRectangleBase::ToolType type, const QCursor & cursor=KisCursor::load("tool_rectangle_cursor.png", 6, 6));

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void paint(QPainter& gc, const KoViewConverter &converter) override;
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void listenToModifiers(bool listen) override;
    bool listeningToModifiers() override;

    QList<QPointer<QWidget> > createOptionWidgets() override;
    void showSize();

protected:
    virtual void finishRect(const QRectF &rect, qreal roundCornersX, qreal roundCornersY) = 0;

    QPointF m_dragCenter;
    QPointF m_dragStart;
    QPointF m_dragEnd;
    ToolType m_type;

    bool m_isRatioForced;
    bool m_isWidthForced;
    bool m_isHeightForced;
    bool m_rotateActive;
    bool m_listenToModifiers;
    float m_forcedRatio;
    float m_forcedWidth;
    float m_forcedHeight;
    int m_roundCornersX;
    int m_roundCornersY;
    qreal m_referenceAngle;
    qreal m_angle;
    qreal m_angleBuffer;

    bool isFixedSize();
    qreal getRotationAngle();
    QPainterPath drawX(const QPointF &pt);
    void applyConstraints(QSizeF& area, bool overrideRatio);
    void getRotatedPath(QPainterPath &path, const QPointF &center, const qreal &angle);

    void updateArea();
    virtual void paintRectangle(QPainter &gc, const QRectF &imageRect);
    virtual QRectF createRect(const QPointF &start, const QPointF &end);
    virtual bool showRoundCornersGUI() const;
};

#endif // KIS_TOOL_RECTANGLE_BASE_H
