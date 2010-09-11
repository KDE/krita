/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KORULER_P_H
#define KORULER_P_H

class RulerTabChooser : public QWidget
{
public:
    RulerTabChooser(QWidget *parent) : QWidget(parent), m_type(QTextOption::LeftTab) {}
    virtual ~RulerTabChooser() {}

    inline QTextOption::TabType type() {return m_type;}
    void mousePressEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);

private:
    QTextOption::TabType m_type;
};

class PaintingStrategy
{
public:
    /// constructor
    PaintingStrategy() {}
    /// destructor
    virtual ~PaintingStrategy() {}

    /**
     * Draw the background of the ruler.
     * @param ruler the ruler to draw on.
     * @param painter the painter we can paint with.
     */
    virtual QRectF drawBackground(const KoRulerPrivate *ruler, QPainter &painter) = 0;

    /**
     * Draw the indicators for text-tabs.
     * @param ruler the ruler to draw on.
     * @param painter the painter we can paint with.
     */
    virtual void drawTabs(const KoRulerPrivate *ruler, QPainter &painter) = 0;

    /**
     * Draw the indicators for the measurements which typically are drawn every [unit].
     * @param ruler the ruler to draw on.
     * @param painter the painter we can paint with.
     * @param rectangle
     */
    virtual void drawMeasurements(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle) = 0;

    /**
     * Draw the indicators for the indents of a text paragraph
     * @param ruler the ruler to draw on.
     * @param painter the painter we can paint with.
     */
    virtual void drawIndents(const KoRulerPrivate *ruler, QPainter &painter) = 0;

    /**
     *returns the size suggestion for a ruler with this strategy.
     */
    virtual QSize sizeHint() = 0;
};

class HorizontalPaintingStrategy : public PaintingStrategy
{
public:
    HorizontalPaintingStrategy() : lengthInPixel(1) {}

    virtual QRectF drawBackground(const KoRulerPrivate *ruler, QPainter &painter);
    virtual void drawTabs(const KoRulerPrivate *ruler, QPainter &painter);
    virtual void drawMeasurements(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle);
    virtual void drawIndents(const KoRulerPrivate *ruler, QPainter &painter);
    virtual QSize sizeHint();

private:
    qreal lengthInPixel;
};

class VerticalPaintingStrategy : public PaintingStrategy
{
public:
    VerticalPaintingStrategy() : lengthInPixel(1) {}

    virtual QRectF drawBackground(const KoRulerPrivate *ruler, QPainter &painter);
    virtual void drawTabs(const KoRulerPrivate *, QPainter &) {}
    virtual void drawMeasurements(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle);
    virtual void drawIndents(const KoRulerPrivate *, QPainter &) { }
    virtual QSize sizeHint();

private:
    qreal lengthInPixel;
};

class HorizontalDistancesPaintingStrategy : public HorizontalPaintingStrategy
{
public:
    HorizontalDistancesPaintingStrategy() {}

    virtual void drawMeasurements(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle);

private:
    void drawDistanceLine(const KoRulerPrivate *d, QPainter &painter, const qreal start, const qreal end);
};

class KoRulerPrivate
{
public:
    KoRulerPrivate(KoRuler *parent, const KoViewConverter *vc, Qt::Orientation orientation);
    ~KoRulerPrivate();

    void emitTabChanged();

    KoUnit unit;
    const Qt::Orientation orientation;
    const KoViewConverter * const viewConverter;

    int offset;
    qreal rulerLength;
    qreal activeRangeStart;
    qreal activeRangeEnd;
    qreal activeOverrideRangeStart;
    qreal activeOverrideRangeEnd;

    int mouseCoordinate;
    int showMousePosition;

    bool showSelectionBorders;
    qreal firstSelectionBorder;
    qreal secondSelectionBorder;

    bool showIndents;
    qreal firstLineIndent;
    qreal paragraphIndent;
    qreal endIndent;

    bool showTabs;
    bool tabMoved; // set to true on first move of a selected tab
    QList<KoRuler::Tab> tabs;
    int originalIndex; //index of selected tab before we started dragging it.
    int currentIndex; //index of selected tab or selected HotSpot - only valid when selected indicates tab or hotspot
    KoRuler::Tab deletedTab;

    struct HotSpotData {
        qreal position;
        int id;
    };
    QList<HotSpotData> hotspots;

    bool rightToLeft;
    enum Selection {
        None,
        Tab,
        FirstLineIndent,
        ParagraphIndent,
        EndIndent,
        HotSpot
    };
    Selection selected;
    int selectOffset;

    QList<QAction*> popupActions;

    RulerTabChooser *tabChooser;

    // Cached painting strategies
    PaintingStrategy * normalPaintingStrategy;
    PaintingStrategy * distancesPaintingStrategy;

    // Current painting strategy
    PaintingStrategy * paintingStrategy;

    KoRuler *ruler;

    qreal numberStepForUnit() const;
    /// @return The rounding of value to the nearest multiple of stepValue
    qreal doSnapping(const qreal value) const;
    Selection selectionAtPosition(const QPoint & pos, int *selectOffset = 0);
    int hotSpotIndex(const QPoint & pos);
    qreal effectiveActiveRangeStart() const;
    qreal effectiveActiveRangeEnd() const;

    friend class VerticalPaintingStrategy;
    friend class HorizontalPaintingStrategy;
};

#endif
