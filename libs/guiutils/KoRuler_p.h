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

class RulerTabChooser : public QWidget {
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
public:
    RulerTabChooser(QWidget *parent) : QWidget(parent), m_type(QTextOption::LeftTab) {}
    virtual ~RulerTabChooser() {}

    inline QTextOption::TabType type() {return m_type;}
    void mousePressEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);

private:
    QTextOption::TabType m_type;
#endif
};

class PaintingStrategy {
public:
    PaintingStrategy() {}
    virtual ~PaintingStrategy() {}

    virtual QRectF drawBackground(const KoRulerPrivate *ruler, QPainter &painter) = 0;
    virtual void drawTabs(const KoRulerPrivate *ruler, QPainter &painter) = 0;
    virtual void drawRulerStripes(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle) = 0;
    virtual void drawIndents(const KoRulerPrivate *ruler, QPainter &painter) = 0;
    virtual QSize sizeHint() = 0;

};

class HorizontalPaintingStrategy : public PaintingStrategy {
public:
    HorizontalPaintingStrategy() : lengthInPixel(1) {}

    virtual QRectF drawBackground(const KoRulerPrivate *ruler, QPainter &painter);
    virtual void drawTabs(const KoRulerPrivate *ruler, QPainter &painter);
    virtual void drawRulerStripes(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle);
    virtual void drawIndents(const KoRulerPrivate *ruler, QPainter &painter);
    virtual QSize sizeHint();

private:
    double lengthInPixel;
};

class VerticalPaintingStrategy : public PaintingStrategy {
public:
    VerticalPaintingStrategy() : lengthInPixel(1) {}

    virtual QRectF drawBackground(const KoRulerPrivate *ruler, QPainter &painter);
    virtual void drawTabs(const KoRulerPrivate *, QPainter &) {}
    virtual void drawRulerStripes(const KoRulerPrivate *ruler, QPainter &painter, const QRectF &rectangle);
    virtual void drawIndents(const KoRulerPrivate *, QPainter &) { }
    virtual QSize sizeHint();

private:
    double lengthInPixel;
};

class KoRulerPrivate {
public:
    KoRulerPrivate(KoRuler *parent, const KoViewConverter *vc, Qt::Orientation orientation);
    KoUnit unit;
    const Qt::Orientation orientation;
    const KoViewConverter * const viewConverter;

    int offset;
    double rulerLength;
    double activeRangeStart;
    double activeRangeEnd;

    int mouseCoordinate;
    int showMousePosition;

    bool showSelectionBorders;
    double firstSelectionBorder;
    double secondSelectionBorder;

    bool showIndents;
    double firstLineIndent;
    double paragraphIndent;
    double endIndent;

    bool showTabs;
    QList<KoRuler::Tab> tabs;
    int currentTab; //indext of selected tab - only valid when selected indicates tab
    KoRuler::Tab deletedTab;

    bool rightToLeft;
    enum Selection {
        None,
        Tab,
        FirstLineIndent,
        ParagraphIndent,
        EndIndent
    };
    Selection selected;
    int selectOffset;

    RulerTabChooser *tabChooser;
    PaintingStrategy * const paintingStrategy;
    KoRuler *ruler;

    double numberStepForUnit() const;
    /// @return The rounding of value to the nearest multiple of stepValue
    double doSnapping(const double value) const;

    friend class VerticalPaintingStrategy;
    friend class HorizontalPaintingStrategy;
};

#endif
