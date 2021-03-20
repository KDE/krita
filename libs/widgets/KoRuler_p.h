/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KORULER_P_H
#define KORULER_P_H

#include <KoUnit.h>

class RulerTabChooser : public QWidget
{
public:
    RulerTabChooser(QWidget *parent) : QWidget(parent), m_type(QTextOption::LeftTab), m_showTabs(false) {}
    ~RulerTabChooser() override {}

    inline QTextOption::TabType type() {return m_type;}
    void setShowTabs(bool showTabs) { if (m_showTabs == showTabs) return; m_showTabs = showTabs; update(); }
    void mousePressEvent(QMouseEvent *) override;

    void paintEvent(QPaintEvent *) override;

private:
    QTextOption::TabType m_type { QTextOption::LeftTab };
    bool m_showTabs :1;
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

    QRectF drawBackground(const KoRulerPrivate *ruler, QPainter &painter) override;
    void drawTabs(const KoRulerPrivate *ruler, QPainter &painter) override;
    void drawMeasurements(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle) override;
    void drawIndents(const KoRulerPrivate *ruler, QPainter &painter) override;
    QSize sizeHint() override;

private:
    qreal lengthInPixel {0.0};
};

class VerticalPaintingStrategy : public PaintingStrategy
{
public:
    VerticalPaintingStrategy() : lengthInPixel(1) {}

    QRectF drawBackground(const KoRulerPrivate *ruler, QPainter &painter) override;
    void drawTabs(const KoRulerPrivate *, QPainter &) override {}
    void drawMeasurements(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle) override;
    void drawIndents(const KoRulerPrivate *, QPainter &) override { }
    QSize sizeHint() override;

private:
    qreal lengthInPixel {0.0};
};

class HorizontalDistancesPaintingStrategy : public HorizontalPaintingStrategy
{
public:
    HorizontalDistancesPaintingStrategy() {}

    void drawMeasurements(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle) override;

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

    int offset { 0 };
    qreal rulerLength {0.0};
    qreal activeRangeStart {0.0};
    qreal activeRangeEnd {0.0};
    qreal activeOverrideRangeStart {0.0};
    qreal activeOverrideRangeEnd {0.0};

    int mouseCoordinate {0};
    int showMousePosition {0};

    bool showSelectionBorders {false};
    qreal firstSelectionBorder {0.0};
    qreal secondSelectionBorder {0.0};

    bool showIndents {false};
    qreal firstLineIndent {0.0};
    qreal paragraphIndent {0.0};
    qreal endIndent {0.0};

    bool showTabs {false};
    bool relativeTabs {false};
    bool tabMoved {false}; // set to true on first move of a selected tab
    QList<KoRuler::Tab> tabs;
    int originalIndex {0}; //index of selected tab before we started dragging it.
    int currentIndex {0}; //index of selected tab or selected HotSpot - only valid when selected indicates tab or hotspot
    KoRuler::Tab deletedTab;
    qreal tabDistance {0.0};

    struct HotSpotData {
        qreal position;
        int id;
    };
    QList<HotSpotData> hotspots;

    bool rightToLeft {false};
    enum Selection {
        None,
        Tab,
        FirstLineIndent,
        ParagraphIndent,
        EndIndent,
        HotSpot
    };
    Selection selected {None};
    int selectOffset {0};

    QList<QAction*> popupActions;

    RulerTabChooser *tabChooser {0};

    // Cached painting strategies
    PaintingStrategy * normalPaintingStrategy {0};
    PaintingStrategy * distancesPaintingStrategy {0};

    // Current painting strategy
    PaintingStrategy * paintingStrategy {0};

    KoRuler *ruler {0};

    bool guideCreationStarted {false};

    qreal pixelStep {0.0};

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
