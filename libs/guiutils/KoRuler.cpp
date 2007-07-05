/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
   Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoRuler.h"

#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include <QPainter>
#include <QResizeEvent>
#include <QMouseEvent>

#include <KoViewConverter.h>

class KoTabChooser : public QWidget {
    public:
        KoTabChooser() {}
        virtual ~KoTabChooser() {}
        KoRuler::TabType type() {return m_type;}
        void mousePressEvent( QMouseEvent *e ) {}

    private:
        KoRuler::TabType m_type;
};

class KoRulerPrivate {
    public:
    KoRulerPrivate(const KoViewConverter *vc) : m_viewConverter(vc), m_mouseCoordinate(-1) {}
        KoUnit m_unit;
        Qt::Orientation m_orientation;
        const KoViewConverter* m_viewConverter;

        int m_offset;
        double m_rulerLength;
        double m_activeRangeStart;
        double m_activeRangeEnd;

        int m_mouseCoordinate;
        int m_showMousePosition;

        bool m_showSelectionBorders;
        double m_firstSelectionBorder;
        double m_secondSelectionBorder;

        bool m_showIndents;
        double m_firstLineIndent;
        double m_paragraphIndent;
        double m_endIndent;

        bool m_showTabs;
        QList<KoRuler::Tab> m_tabs;
        int m_tabIndex; //indext of selected tab - only valid when m_selected indicates tab

        bool m_rightToLeft;
        int m_selected;
        int m_selectOffset;

        KoTabChooser *m_tabChooser;
};


KoRuler::KoRuler(QWidget* parent, Qt::Orientation orientation, const KoViewConverter* viewConverter)
  : QWidget(parent)
  , d( new KoRulerPrivate( viewConverter) )
{
    d->m_orientation = orientation;

    setUnit(KoUnit(KoUnit::Point));
    setRulerLength(0);
    setOffset(0);
    setActiveRange(0, 0);
    setShowMousePosition(false);
    setShowSelectionBorders(false);
    setShowIndents(false); 
    setShowTabs(false);

    setRightToLeft(false);
    d->m_firstLineIndent = d->m_paragraphIndent = 0;
    d->m_endIndent = 0;
    updateMouseCoordinate(-1);
    d->m_selected = 0;

    if(orientation == Qt::Horizontal) {
        d->m_tabChooser = new KoTabChooser;
    } else {
        d->m_tabChooser = 0;
    }
}

KoRuler::~KoRuler()
{
    delete d;
}

KoUnit KoRuler::unit() const
{
    return d->m_unit;
}

void KoRuler::setUnit(KoUnit unit)
{
    d->m_unit = unit;
    update();
}

double KoRuler::rulerLength() const
{
    return d->m_rulerLength;
}

Qt::Orientation KoRuler::orientation() const
{
    return d->m_orientation;
}

void KoRuler::setOffset(int offset)
{
    d->m_offset = offset;
    update();
}

void KoRuler::setRulerLength(double length)
{
    d->m_rulerLength = length;
    update();
}

void KoRuler::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    double numberStep = numberStepForUnit(); // number step in unit

    QRectF rectangle;
    QRectF activeRangeRectangle;
    double lengthPixel = 0;
    int numberStepPixel = 0;

    if(orientation() == Qt::Horizontal) {
        lengthPixel = d->m_viewConverter->documentToViewX(rulerLength());
        numberStepPixel = qRound(d->m_viewConverter->documentToViewX(
            d->m_unit.fromUserValue(numberStep)));
        rectangle.setX(qMax(0, d->m_offset));
        rectangle.setY(5);
        rectangle.setWidth(qMin((double)width() - 1.0 - rectangle.x(), (d->m_offset >= 0 ) ? lengthPixel : lengthPixel + d->m_offset ));
        rectangle.setHeight(height() - 10.0);
        activeRangeRectangle.setX(qMax(rectangle.x() + 1,
              d->m_viewConverter->documentToViewX(d->m_activeRangeStart) + d->m_offset));
        activeRangeRectangle.setY(rectangle.y() + 1);
        activeRangeRectangle.setRight(qMin(rectangle.right() - 1,
              d->m_viewConverter->documentToViewX(d->m_activeRangeEnd) + d->m_offset));
        activeRangeRectangle.setHeight(rectangle.height() - 2);
    } else {
        lengthPixel = d->m_viewConverter->documentToViewY(rulerLength());
        numberStepPixel = qRound(d->m_viewConverter->documentToViewY(
            d->m_unit.fromUserValue(numberStep)));
        rectangle.setX(0);
        rectangle.setY(qMax(0, d->m_offset));
        rectangle.setWidth(width() - 1.0);
        rectangle.setHeight(qMin((double)height() - 1.0 - rectangle.y(), (d->m_offset >= 0 ) ? lengthPixel : lengthPixel + d->m_offset ));
        activeRangeRectangle.setX(rectangle.x() + 1);
        activeRangeRectangle.setY(qMax(rectangle.y() + 1,
            d->m_viewConverter->documentToViewY(d->m_activeRangeStart) + d->m_offset));
        activeRangeRectangle.setWidth(rectangle.width() - 2);
        activeRangeRectangle.setBottom(qMin(rectangle.bottom() - 1,
            d->m_viewConverter->documentToViewY(d->m_activeRangeEnd) + d->m_offset));
    }

    painter.setPen(palette().color(QPalette::Mid));
    painter.drawRect(rectangle);

    if(d->m_activeRangeStart != d->m_activeRangeEnd) {
        painter.fillRect(activeRangeRectangle, palette().brush(QPalette::Base));
    }

    QFont font = KGlobalSettings::toolBarFont();
    QFontMetrics fontMetrics(font);
    int textLength = 0;

    // Calc the longest text length
    for(int i = 0; i < lengthPixel; i += numberStepPixel) {
        textLength = qMax(textLength, fontMetrics.width(
            QString::number((i / numberStepPixel) * numberStep)));
    }

    textLength += 4;  // Add some padding

    // Change number step so all digits fits
    while(textLength > numberStepPixel) {
        numberStepPixel += numberStepPixel;
        numberStep += numberStep;
    }

    int start=0;

    // Calc the first number step
    if(d->m_offset < 0) {
        start = qAbs(d->m_offset);
    }

    int mouseCoord = d->m_mouseCoordinate - start;

    // make a little hack so rulers shows correctly inversed number aligned
    double lengthInUnit = d->m_unit.toUserValue(rulerLength());
    double hackyLength = lengthInUnit - fmod(lengthInUnit, numberStep);
    if(d->m_rightToLeft && orientation() == Qt::Horizontal) {
        start -= int(d->m_viewConverter->documentToViewX(fmod(rulerLength(),
                    d->m_unit.fromUserValue(numberStep))));
    }

    int stepCount = (start / numberStepPixel) + 1;
    int halfStepCount = (start / qRound(numberStepPixel * 0.5)) + 1;
    int quarterStepCount = (start / qRound(numberStepPixel * 0.25)) + 1;

    int pos = 0;
    painter.setPen(palette().color(QPalette::Text));

    if(orientation() == Qt::Horizontal) {
        if(d->m_offset > 0) {
            painter.translate(d->m_offset, 0);
        }

        int len = qRound(rectangle.width()) + start;
        int nextStep = qRound(d->m_viewConverter->documentToViewX(
            d->m_unit.fromUserValue(numberStep * stepCount)));
        int nextHalfStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
            numberStep * 0.5 * halfStepCount)));
        int nextQuarterStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
            numberStep * 0.25 * quarterStepCount)));

        for(int i = start; i < len; ++i) {
            pos = i - start;

            if(i == nextStep) {
                if(pos != 0) {
                    painter.drawLine(QPointF(pos, rectangle.bottom()-1), QPointF(pos, rectangle.bottom() -10));
                }

                QString numberText = QString::number(stepCount * numberStep);
                if (d->m_rightToLeft) // this is done in a hacky way with the fine tuning done above 
                    numberText = QString::number(hackyLength - stepCount * numberStep);
                painter.drawText(QPointF(pos - 0.5*fontMetrics.width(numberText), rectangle.bottom() -12), numberText);

                ++stepCount;
                nextStep = qRound(d->m_viewConverter->documentToViewX(
                    d->m_unit.fromUserValue(numberStep * stepCount)));
                ++halfStepCount;
                nextHalfStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
                    numberStep * 0.5 * halfStepCount)));
                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            } else if(i == nextHalfStep) {
                if(pos != 0) {
                    painter.drawLine(QPointF(pos, rectangle.bottom()-1), QPointF(pos, rectangle.bottom() - 8));
                }

                ++halfStepCount;
                nextHalfStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
                    numberStep * 0.5 * halfStepCount)));
                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            } else if(i == nextQuarterStep) {
                if(pos != 0) {
                    painter.drawLine(QPointF(pos, rectangle.bottom()-1), QPointF(pos, rectangle.bottom() - 4));
                }

                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            }
        }

        // Draw the mouse indicator
        if(d->m_showMousePosition && (mouseCoord > 0) && (mouseCoord < rectangle.width()) ) {
            painter.drawLine(QPointF(mouseCoord, rectangle.y() + 1),
                              QPointF(mouseCoord, rectangle.bottom() - 1));
        }

        if(d->m_showSelectionBorders) {
            // Draw first selection border
            if(d->m_firstSelectionBorder > 0) {
                double border = d->m_viewConverter->documentToViewX(d->m_firstSelectionBorder);
                painter.drawLine(QPointF(border, rectangle.y() + 1), QPointF(border, rectangle.bottom() - 1));
            }
            // Draw second selection border
            if(d->m_secondSelectionBorder > 0) {
                double border = d->m_viewConverter->documentToViewX(d->m_secondSelectionBorder);
                painter.drawLine(QPointF(border, rectangle.y() + 1), QPointF(border, rectangle.bottom() - 1));
            }
        }

        if (d->m_showIndents) {
            QPolygonF polygon;

            painter.setBrush(palette().brush(QPalette::Base));
            painter.setRenderHint( QPainter::Antialiasing );

            double x;
            // Draw first line start indent
            if (d->m_rightToLeft) {
                x = d->m_viewConverter->documentToViewX(d->m_activeRangeEnd
                        - d->m_firstLineIndent - d->m_paragraphIndent) + qMin(0, d->m_offset);
                x = int(x+0.5); //go to nearest integer so that the 0.5 added below ensures sharp lines
                polygon << QPointF(x+0.5, 0.5)
                        << QPointF(x-9.5, 5.5)
                        << QPointF(x+0.5, 10.5)
                        << QPointF(x+0.5, 0.5);
                painter.drawPolygon(polygon);
            } else {
                x = d->m_viewConverter->documentToViewX(d->m_activeRangeStart
                        + d->m_firstLineIndent + d->m_paragraphIndent) + qMin(0, d->m_offset);
                x = int(x+0.5); //go to nearest integer so that the 0.5 added below ensures sharp lines
                polygon << QPointF(x+0.5, 0.5)
                        << QPointF(x+10.5, 5.5)
                        << QPointF(x+0.5, 10.5)
                        << QPointF(x+0.5, 0.5);
                painter.drawPolygon(polygon);
            }
            // Draw rest of the lines start indent or end indent if mode is rightToLeft
            polygon.clear();
            if (d->m_rightToLeft)
                x = d->m_viewConverter->documentToViewX(d->m_activeRangeStart + d->m_endIndent)
                        + qMin(0, d->m_offset);
            else
                x = d->m_viewConverter->documentToViewX(d->m_activeRangeStart
                             + d->m_paragraphIndent) + qMin(0, d->m_offset);
            x = int(x+0.5); //go to nearest integer so that the 0.5 added below ensures sharp lines
            polygon << QPointF(x+0.5, height() - 10.5)
                        << QPointF(x+10.5, height() - 5.5)
                        << QPointF(x+0.5, height() - 0.5)
                        << QPointF(x+0.5, height() - 10.5);
            painter.drawPolygon(polygon);

            // Draw end indent or paragraph indent if mode is rightToLeft
            polygon.clear();
            if (d->m_rightToLeft)
                x = d->m_viewConverter->documentToViewX(d->m_activeRangeEnd
                             - d->m_paragraphIndent) + qMin(0, d->m_offset);
            else
                x = d->m_viewConverter->documentToViewX(d->m_activeRangeEnd - d->m_endIndent)
                        + qMin(0, d->m_offset);
            x = int(x+0.5); //go to nearest integer so that the 0.5 added below ensures sharp lines
            polygon << QPointF(x+0.5, height() - 10.5)
                        << QPointF(x-9.5, height() - 5.5)
                        << QPointF(x+0.5, height() - 0.5)
                        << QPointF(x+0.5, height() - 10.5);
            painter.drawPolygon(polygon);

            painter.setRenderHint( QPainter::Antialiasing, false );
        }

        if (d->m_showTabs) {
            QPolygonF polygon;

            painter.setBrush(palette().color(QPalette::Text));
            painter.setRenderHint( QPainter::Antialiasing );

            foreach (Tab t, d->m_tabs) {
                double x;
                if (d->m_rightToLeft)
                    x = d->m_viewConverter->documentToViewX(d->m_activeRangeEnd - t.position)
                            + qMin(0, d->m_offset);
                else
                    x = d->m_viewConverter->documentToViewX(d->m_activeRangeStart + t.position)
                            + qMin(0, d->m_offset);

                polygon.clear();
                switch (t.type) {
                case LeftTab:
                    polygon << QPointF(x+0.5, height() - 8.5)
                        << QPointF(x-5.5, height() - 2.5)
                        << QPointF(x+0.5, height() - 2.5);
                    painter.drawPolygon(polygon);
                    break;
                case RightTab:
                    polygon << QPointF(x+0.5, height() - 8.5)
                        << QPointF(x+6.5, height() - 2.5)
                        << QPointF(x+0.5, height() - 2.5);
                    painter.drawPolygon(polygon);
                    break;
                case CenterTab:
                    polygon << QPointF(x+0.5, height() - 8.5)
                        << QPointF(x-5.5, height() - 2.5)
                        << QPointF(x+6.5, height() - 2.5);
                    painter.drawPolygon(polygon);
                    break;
                case DelimiterTab:
                    polygon << QPointF(x-5.5, height() - 2.5)
                        << QPointF(x+0.5, height() - 8.5)
                        << QPointF(x+6.5, height() - 2.5);
                    painter.drawPolyline(polygon);
                    break;
                default:
                    break;
                }
            }
            painter.setRenderHint( QPainter::Antialiasing, false );
        }
    } else {
        if(d->m_offset > 0) {
            painter.translate(0, d->m_offset);
        }

        int len = qRound(rectangle.height()) + start;
        int nextStep = qRound(d->m_viewConverter->documentToViewY(
            d->m_unit.fromUserValue(numberStep * stepCount)));
        int nextHalfStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
            numberStep * 0.5 * halfStepCount)));
        int nextQuarterStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
            numberStep * 0.25 * quarterStepCount)));

        for(int i = start; i < len; ++i) {
            pos = i - start;

            if(i == nextStep) {
                if(pos != 0) {
                    painter.drawLine(QPointF(rectangle.right() - 10, pos), QPointF(rectangle.right() - 1, pos));
                }

                QString numberText = QString::number(stepCount * numberStep);
                painter.drawText(QPointF(rectangle.right() - 12 -fontMetrics.width(numberText),
                         pos + 4), numberText);

                ++stepCount;
                nextStep = qRound(d->m_viewConverter->documentToViewY(
                    d->m_unit.fromUserValue(numberStep * stepCount)));
                ++halfStepCount;
                nextHalfStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
                    numberStep * 0.5 * halfStepCount)));
                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            } else if(i == nextHalfStep) {
                if(pos != 0) {
                    painter.drawLine(QPointF(rectangle.right() - 8, pos), QPointF(rectangle.right() - 1, pos));
                }

                ++halfStepCount;
                nextHalfStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
                    numberStep * 0.5 * halfStepCount)));
                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            } else if(i == nextQuarterStep) {
                if(pos != 0) {
                    painter.drawLine(QPointF(rectangle.right() - 4, pos), QPointF(rectangle.right() - 1, pos));
                }

                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            }
        }

        // Draw the mouse indicator
        if(d->m_showMousePosition && (mouseCoord > 0) && (mouseCoord < rectangle.height()) ) {
            painter.drawLine(QPointF(rectangle.x() + 1, mouseCoord),
                              QPointF(rectangle.right() - 1, mouseCoord));
        }

        if(d->m_showSelectionBorders) {
            // Draw first selection border
            if(d->m_firstSelectionBorder > 0) {
                double border = d->m_viewConverter->documentToViewY(d->m_firstSelectionBorder);
                painter.drawLine(QPointF(rectangle.x() + 1, border), QPointF(rectangle.right() - 1, border));
            }
            // Draw second selection border
            if(d->m_secondSelectionBorder > 0) {
                double border = d->m_viewConverter->documentToViewY(d->m_secondSelectionBorder);
                painter.drawLine(QPointF(rectangle.x() + 1, border), QPointF(rectangle.right() - 1, border));
            }
        }
    }
}

QSize KoRuler::minimumSizeHint() const
{
    QSize size;
    QFont font = KGlobalSettings::toolBarFont();
    QFontMetrics fm(font);

    int minimum = fm.height() + 14;

    size.setWidth( minimum );
    size.setHeight( minimum );

    return size;
}

QSize KoRuler::sizeHint() const
{
    return minimumSizeHint();
}

void KoRuler::setActiveRange(double start, double end)
{
    d->m_activeRangeStart = start;
    d->m_activeRangeEnd = end;
    update();
}

void KoRuler::updateMouseCoordinate(int coordinate)
{
    if(d->m_mouseCoordinate == coordinate)
        return;
    d->m_mouseCoordinate = coordinate;
    update();
}

void KoRuler::setShowMousePosition(bool show)
{
    d->m_showMousePosition = show;
    update();
}

void KoRuler::setRightToLeft(bool isRightToLeft)
{
    d->m_rightToLeft = isRightToLeft;
    update();
}

void KoRuler::setShowIndents(bool show)
{
    d->m_showIndents = show;
    update();
}

void KoRuler::setFirstLineIndent(double indent)
{
    d->m_firstLineIndent = indent;
    update();
}

void KoRuler::setParagraphIndent(double indent)
{
    d->m_paragraphIndent = indent;
    update();
}

void KoRuler::setEndIndent(double indent)
{
    d->m_endIndent = indent;
    update();
}

double KoRuler::firstLineIndent() const
{
    return d->m_firstLineIndent;
}

double KoRuler::paragraphIndent() const
{
    return d->m_paragraphIndent;
}

double KoRuler::endIndent() const
{
    return d->m_endIndent;
}

QWidget *KoRuler::tabChooser()
{
    return d->m_tabChooser;
}

double KoRuler::numberStepForUnit() const
{
    double numberStep;

    switch(d->m_unit.indexInList()) {
        case KoUnit::Didot:
        case KoUnit::Inch:
        case KoUnit::Centimeter:
        case KoUnit::Decimeter:
            numberStep = 1.0;
            break;
        case KoUnit::Millimeter:
        case KoUnit::Pica:
        case KoUnit::Cicero:
            numberStep = 10.0;
            break;
        case KoUnit::Point:
        default:
            numberStep = 100.0;
    }

    return numberStep;
}

void KoRuler::setShowSelectionBorders(bool show)
{
    d->m_showSelectionBorders = show;
    update();
}

void KoRuler::updateSelectionBorders(double first, double second)
{
    d->m_firstSelectionBorder = first;
    d->m_secondSelectionBorder = second;
    update();
}

void KoRuler::setShowTabs(bool show)
{
    d->m_showTabs = show;
}

void KoRuler::updateTabs(const QList<KoRuler::Tab> &tabs)
{
    d->m_tabs = tabs;
}

QList<KoRuler::Tab> KoRuler::tabs() const {
    return d->m_tabs;
}

void KoRuler::mousePressEvent ( QMouseEvent* ev )
{
    QPoint pos = ev->pos();
    d->m_selected = 0;

    int x;

    if (d->m_showTabs && pos.y() > height() - 9) {
        int i = 0;
        foreach (Tab t, d->m_tabs) {
            if (d->m_rightToLeft)
                x = int(d->m_viewConverter->documentToViewX(d->m_activeRangeEnd - t.position)
                        + d->m_offset);
            else
                x = int(d->m_viewConverter->documentToViewX(d->m_activeRangeStart + t.position)
                        + d->m_offset);

            switch (t.type) {
            case LeftTab:
                if (pos.x() >= x-6 && pos.x() <= x) {
                    d->m_selected = 4;
                    d->m_selectOffset = x - pos.x();
                    d->m_tabIndex = i;
                }
                break;
            case RightTab:
                if (pos.x() >= x && pos.x() <= x+6) {
                    d->m_selected = 4;
                    d->m_selectOffset = x - pos.x();
                    d->m_tabIndex = i;
                }
                break;
            case CenterTab:
                if (pos.x() >= x-6 && pos.x() <= x+6) {
                    d->m_selected = 4;
                    d->m_selectOffset = x - pos.x();
                    d->m_tabIndex = i;
                }
                break;
            case DelimiterTab:
                if (pos.x() >= x-6 && pos.x() <= x+6) {
                    d->m_selected = 4;
                    d->m_selectOffset = x - pos.x();
                    d->m_tabIndex = i;
                }
                break;
            default:
                break;
            }
            i++;
        }
    }

    if (d->m_rightToLeft) {
        x = int(d->m_viewConverter->documentToViewX(d->m_activeRangeEnd - d->m_firstLineIndent
                - d->m_paragraphIndent) + d->m_offset);
        if (pos.x() >= x-10 && pos.x() <= x && pos.y() <10) {
            d->m_selectOffset = x - pos.x();
            d->m_selected = 1;
        }
    
        x = int(d->m_viewConverter->documentToViewX(d->m_activeRangeEnd - d->m_paragraphIndent)
                            + d->m_offset);
        if (pos.x() >= x-10 && pos.x() <= x && pos.y() > height()-10) {
            d->m_selectOffset = x - pos.x();
            d->m_selected = 2;
        }
    
        x = int(d->m_viewConverter->documentToViewX(d->m_activeRangeStart + d->m_endIndent)
                            + d->m_offset);
        if (pos.x() >= x && pos.x() <= x+10 && pos.y() > height()-10) {
            d->m_selectOffset = x - pos.x();
            d->m_selected = 3;
        }
    } else {
        x = int(d->m_viewConverter->documentToViewX(d->m_activeRangeStart
             + d->m_firstLineIndent + d->m_paragraphIndent) + d->m_offset);
        if (pos.x() >= x && pos.x() <= x+10 && pos.y() <10) {
            d->m_selectOffset = x - pos.x();
            d->m_selected = 1;
        }
    
        x = int(d->m_viewConverter->documentToViewX(d->m_activeRangeStart + d->m_paragraphIndent)
                            + d->m_offset);
        if (pos.x() >= x && pos.x() <= x+10 && pos.y() > height()-10) {
            d->m_selectOffset = x - pos.x();
            d->m_selected = 2;
        }
    
        x = int(d->m_viewConverter->documentToViewX(d->m_activeRangeEnd - d->m_endIndent)
                            + d->m_offset);
        if (pos.x() >= x-10 && pos.x() <= x && pos.y() > height()-10) {
            d->m_selectOffset = x - pos.x();
            d->m_selected = 3;
        }
    }

    if (d->m_showTabs && d->m_selected == 0) {
        // still haven't found something so let assume the user wants to add a tab
        double tabpos = d->m_viewConverter->viewToDocumentX(pos.x() - d->m_offset)
                    - d->m_activeRangeStart;
        Tab t = {tabpos, LeftTab};
        d->m_tabs.append(t);
        d->m_selectOffset = 0;
        d->m_selected = 4;
        d->m_tabIndex = d->m_tabs.count() - 1;
        update();
    }
}

void KoRuler::mouseReleaseEvent ( QMouseEvent* ev )
{
    mouseMoveEvent(ev); // handle any last movement

    if( d->m_selected >0 && d->m_selected < 4)
        emit indentsChanged(true);

    if( d->m_selected == 4)
        emit tabsChanged(true);

    d->m_selected = 0;
}

void KoRuler::mouseMoveEvent ( QMouseEvent* ev )
{
    QPoint pos = ev->pos();
    double activeLength = d->m_activeRangeEnd - d->m_activeRangeStart;

    switch(d->m_selected) {
    case 0:
    default:
        break;
    case 1:
        if (d->m_rightToLeft)
            d->m_firstLineIndent = d->m_activeRangeEnd - d->m_paragraphIndent -
                d->m_viewConverter->viewToDocumentX(pos.x() + d->m_selectOffset - d->m_offset);
        else
            d->m_firstLineIndent = d->m_viewConverter->viewToDocumentX(pos.x() + d->m_selectOffset
                - d->m_offset) - d->m_activeRangeStart - d->m_paragraphIndent;

        emit indentsChanged(false);
        break;
    case 2:
        if (d->m_rightToLeft)
            d->m_paragraphIndent = d->m_activeRangeEnd -
                d->m_viewConverter->viewToDocumentX(pos.x() + d->m_selectOffset - d->m_offset);
        else
            d->m_paragraphIndent = d->m_viewConverter->viewToDocumentX(pos.x() + d->m_selectOffset
                - d->m_offset) - d->m_activeRangeStart;
        if (d->m_paragraphIndent < 0)
            d->m_paragraphIndent = 0;
        if (d->m_paragraphIndent + d->m_endIndent > activeLength)
            d->m_paragraphIndent = activeLength - d->m_endIndent;;
        emit indentsChanged(false);
        break;
    case 3:
        if (d->m_rightToLeft)
            d->m_endIndent = d->m_viewConverter->viewToDocumentX(pos.x()
                 + d->m_selectOffset - d->m_offset) - d->m_activeRangeStart;
        else
            d->m_endIndent = d->m_activeRangeEnd - d->m_viewConverter->viewToDocumentX(pos.x()
                 + d->m_selectOffset - d->m_offset);
        if (d->m_endIndent < 0)
            d->m_endIndent = 0;
        if (d->m_paragraphIndent + d->m_endIndent > activeLength)
            d->m_endIndent = activeLength - d->m_paragraphIndent;;
        emit indentsChanged(false);
        break;
    case 4:
        if (d->m_rightToLeft)
            d->m_tabs[d->m_tabIndex].position = d->m_activeRangeEnd -
                d->m_viewConverter->viewToDocumentX(pos.x() + d->m_selectOffset - d->m_offset);
        else
            d->m_tabs[d->m_tabIndex].position = d->m_viewConverter->viewToDocumentX(pos.x() + d->m_selectOffset
                - d->m_offset) - d->m_activeRangeStart;
        if (d->m_tabs[d->m_tabIndex].position < 0)
            d->m_tabs[d->m_tabIndex].position = 0;
        if (d->m_tabs[d->m_tabIndex].position > activeLength)
            d->m_tabs[d->m_tabIndex].position = activeLength;

        emit tabsChanged(false);
        break;
    }
    update();
}

#include "KoRuler.moc"
