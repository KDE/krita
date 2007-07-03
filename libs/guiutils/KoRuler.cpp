/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
   Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>

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

class Mark {
    public:
        Mark() {};
        virtual ~Mark() {};
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
        double m_rParagraphIndent;
        double m_endIndent;
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
    setShowIndents(true); 
    d->m_firstLineIndent = d->m_rParagraphIndent = 0;
    d->m_endIndent = 0;
    updateMouseCoordinate(-1);
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
        rectangle.setY(0);
        rectangle.setWidth(qMin((double)width() - 1.0 - rectangle.x(), lengthPixel));
        rectangle.setHeight(height() - 1.0);
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
        rectangle.setHeight(qMin((double)height() - 1.0 - rectangle.y(), lengthPixel));
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

    textLength += 8;  // Add some padding

    // Change number step so all digits fits
    while(textLength > numberStepPixel) {
        numberStepPixel += numberStepPixel;
        numberStep += numberStep;
    }

    int start = 0;
    int stepCount = 0;
    int halfStepCount = 0;
    int quarterStepCount = 0;
    int mouseCoord = d->m_mouseCoordinate;

    // Calc the first number step
    if(d->m_offset < 0) {
        start = qAbs(d->m_offset);
        stepCount = (start / numberStepPixel) + 1;
        halfStepCount = (start / qRound(numberStepPixel * 0.5)) + 1;
        quarterStepCount = (start / qRound(numberStepPixel * 0.25)) + 1;
        mouseCoord = d->m_mouseCoordinate - start;
    }

    int fontHeight = fontMetrics.height();
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
                    painter.drawLine(QPointF(pos, height()-1), QPointF(pos, height() * 0.7));
                }

                QString numberText = QString::number(stepCount * numberStep);
                painter.drawText(QPoint(pos - 0.5*fontMetrics.width(numberText), height()*0.6), numberText);

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
                    painter.drawLine(QPointF(pos, height()-1), QPointF(pos, height() * 0.8));
                }

                ++halfStepCount;
                nextHalfStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
                    numberStep * 0.5 * halfStepCount)));
                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            } else if(i == nextQuarterStep) {
                if(pos != 0) {
                    painter.drawLine(QPointF(pos, height()-1), QPointF(pos, height() * 0.875));
                }

                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewX(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            }
        }

        // Draw the mouse indicator
        if(d->m_showMousePosition && (mouseCoord > 0)) {
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

            // Draw first line start indent
            double x = d->m_viewConverter->documentToViewX(d->m_activeRangeStart
                        + d->m_firstLineIndent) + d->m_offset;
            polygon << QPointF(x, 0.5)
                        << QPointF(x+10.5, 0.5)
                        << QPointF(x+10.5, 2.5)
                        << QPointF(x+3.5, 2.5)
                        << QPointF(x+0.5, 5.5)
                        << QPointF(x+0.5, 0.5);
            painter.drawPolygon(polygon);

            // Draw rest of the lines start indent
            polygon.clear();
            x = d->m_viewConverter->documentToViewX(d->m_activeRangeStart + d->m_rParagraphIndent)
                        + d->m_offset;
            polygon << QPointF(x-2.5, height() - 10.5)
                        << QPointF(x+10.5, height() - 10.5)
                        << QPointF(x+10.5, height() - 3.5)
                        << QPointF(x+3.5, height() - 3.5)
                        << QPointF(x+0.5, height() - 0.5)
                        << QPointF(x-2.5, height() - 3.5)
                        << QPointF(x-2.5, height() - 10.5);
            painter.drawPolygon(polygon);

            // Draw rend indent
            x = d->m_viewConverter->documentToViewX(d->m_activeRangeEnd - d->m_endIndent)
                        + d->m_offset;
            painter.drawLine(QPointF(x-4, 1), QPointF(x+4, 1));
            painter.drawLine(QPointF(x-4, 1), QPointF(x, height() * 0.3));
            painter.drawLine(QPointF(x+4, 1), QPointF(x, height() * 0.3));

            painter.setRenderHint( QPainter::Antialiasing, false );
        }
    } else {
        //int textOffset = 0;

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
                    painter.drawLine(QPointF(0, pos), QPointF(width() * 0.5, pos));
                }

                int textY = pos + textLength + 2;

                if(textY < len) {
                    painter.save();
                    painter.translate(width() - fontHeight + 2, textY);
                    painter.rotate(-90);
                    painter.drawText(QRect(0, 0, textLength, fontHeight),
                                      Qt::AlignRight | Qt::AlignTop,
                                      QString::number(stepCount * numberStep));
                    painter.restore();
                }

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
                    painter.drawLine(QPointF(0, pos), QPointF(width() * 0.25, pos));
                }

                ++halfStepCount;
                nextHalfStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
                    numberStep * 0.5 * halfStepCount)));
                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            } else if(i == nextQuarterStep) {
                if(pos != 0) {
                    painter.drawLine(QPointF(0, pos), QPointF(width() * 0.125, pos));
                }

                ++quarterStepCount;
                nextQuarterStep = qRound(d->m_viewConverter->documentToViewY(d->m_unit.fromUserValue(
                    numberStep * 0.25 * quarterStepCount)));
            }
        }

        // Draw the mouse indicator
        if(d->m_showMousePosition && (mouseCoord > 0)) {
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

    int minimum = qMax(fm.height() + 4, 20);

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
}

void KoRuler::setShowIndents(bool show)
{
    d->m_showIndents = show;
}

void KoRuler::setFirstLineIndent(double indent)
{
    d->m_firstLineIndent = indent;
}

void KoRuler::setParagraphIndent(double indent)
{
    d->m_rParagraphIndent = indent;
}

void KoRuler::setEndIndent(double indent)
{
    d->m_endIndent = indent;
}

double KoRuler::firstLineIndent() const
{
    return d->m_firstLineIndent;
}

double KoRuler::rParagraphIndent() const
{
    return d->m_rParagraphIndent;
}

double KoRuler::endIndent() const
{
    return d->m_endIndent;
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
}

void KoRuler::updateSelectionBorders(double first, double second)
{
    d->m_firstSelectionBorder = first;
    d->m_secondSelectionBorder = second;
    update();
}

#include "KoRuler.moc"
