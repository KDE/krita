/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include "MusicStyle.h"
using namespace MusicCore;

MusicStyle::MusicStyle()
#ifdef Q_WS_MAC
    : m_font("Emmentaler 14")
#else
    : m_font("Emmentaler")
#endif
    , m_textAsPath(false)
{
    m_font.setPixelSize(20);
    m_staffLinePen.setWidthF(0.5);
    m_staffLinePen.setCapStyle(Qt::RoundCap);
    m_staffLinePen.setColor(Qt::black);
    m_stemPen.setWidthF(0.7);
    m_stemPen.setCapStyle(Qt::FlatCap);
    m_stemPen.setColor(Qt::black);
    m_noteDotPen.setWidthF(1.9);
    m_noteDotPen.setCapStyle(Qt::RoundCap);
    m_noteDotPen.setColor(Qt::black);
}

MusicStyle::~MusicStyle()
{
}

QPen MusicStyle::staffLinePen(const QColor& color)
{
    m_staffLinePen.setColor(color);
    return m_staffLinePen;
}

QPen MusicStyle::stemPen(const QColor& color)
{
    m_stemPen.setColor(color);
    return m_stemPen;
}

QPen MusicStyle::noteDotPen(const QColor& color)
{
    m_noteDotPen.setColor(color);
    return m_noteDotPen;
}

qreal MusicStyle::beamLineWidth()
{
    return 3.0;
}

void MusicStyle::renderText(QPainter& painter, qreal x, qreal y, const QString& text)
{
    QPointF p(x, y);
    if(m_textAsPath) {
        QPainterPath textPath;
        textPath.setFillRule(Qt::OddEvenFill);
        textPath.addText(p, m_font, text);
        painter.save();
        painter.setBrush(painter.pen().brush().color());
        painter.drawPath(textPath);
        painter.restore();
    }
    else {
        painter.drawText(p, text);
    }
}

bool MusicStyle::textAsPath() const
{
    return m_textAsPath;
}

void MusicStyle::setTextAsPath(bool drawTextAsPath)
{
    m_textAsPath = drawTextAsPath;
}

void MusicStyle::renderNoteHead(QPainter& painter, qreal x, qreal y, Duration duration, const QColor& color)
{
    painter.setPen(QPen(color));
    painter.setFont(m_font);
    switch (duration) {
        case HundredTwentyEighthNote:
        case SixtyFourthNote:
        case ThirtySecondNote:
        case SixteenthNote:
        case EighthNote:
        case QuarterNote:
            renderText(painter, x, y, QString(0xE125));
            break;
        case HalfNote:
            renderText(painter, x, y, QString(0xE124));
            break;
        case WholeNote:
            renderText(painter, x, y, QString(0xE123));
            break;
        case BreveNote:
            renderText(painter, x, y, QString(0xE122));
            break;
    }
}

void MusicStyle::renderRest(QPainter& painter, qreal x, qreal y, Duration duration, const QColor& color)
{
    painter.setPen(QPen(color));
    painter.setFont(m_font);
    QPointF p(x, y);
    switch (duration) {
        case HundredTwentyEighthNote:
            renderText(painter, x, y, QString(0xE10D));
            break;
        case SixtyFourthNote:
            renderText(painter, x, y, QString(0xE10C));
            break;
        case ThirtySecondNote:
            renderText(painter, x, y, QString(0xE10B));
            break;
        case SixteenthNote:
            renderText(painter, x, y, QString(0xE10A));
            break;
        case EighthNote:
            renderText(painter, x, y, QString(0xE109));
            break;
        case QuarterNote:
            renderText(painter, x, y, QString(0xE107));
            break;
        case HalfNote:
            renderText(painter, x, y, QString(0xE101));
            break;
        case WholeNote:
            renderText(painter, x, y, QString(0xE100));
            break;
        case BreveNote:
            renderText(painter, x, y, QString(0xE106));
            break;
    }
}

void MusicStyle::renderClef(QPainter& painter, qreal x, qreal y, Clef::ClefShape shape, const QColor& color)
{
    painter.setPen(QPen(color));
    painter.setFont(m_font);
    QPointF p(x, y);
    switch (shape) {
        case Clef::GClef:
            renderText(painter, x, y, QString(0xE195));
            break;
        case Clef::FClef:
            renderText(painter, x, y, QString(0xE193));
            break;
        case Clef::CClef:
            renderText(painter, x, y, QString(0xE191));
            break;
    }
}

void MusicStyle::renderAccidental(QPainter& painter, qreal x, qreal y, int accidental, const QColor& color)
{
    painter.setPen(QPen(color));
    painter.setFont(m_font);
    QPointF p(x, y);
    switch (accidental) {
        case 0:
            renderText(painter, x, y, QString(0xE111));
            break;
        case 1:
            renderText(painter, x, y, QString(0xE10E));
            break;
        case 2:
            renderText(painter, x, y, QString(0xE116));
            break;
        case -1:
            renderText(painter, x, y, QString(0xE112));
            break;
        case -2:
            renderText(painter, x, y, QString(0xE114));
            break;
    }
}

void MusicStyle::renderTimeSignatureNumber(QPainter& painter, qreal x, qreal y, qreal w, int number, const QColor& color)
{
    painter.setPen(QPen(color));
    painter.setFont(m_font);
    QFontMetricsF m(m_font);
    QString txt = QString::number(number);

    renderText(painter, x + (w - m.width(txt))/2, y, txt);
}

void MusicStyle::renderNoteFlags(QPainter& painter, qreal x, qreal y, Duration duration, bool stemsUp, const QColor& color)
{
    painter.setPen(QPen(color));
    painter.setFont(m_font);
    QPointF p(x + 0.4, y);
    switch (duration) {
        case HundredTwentyEighthNote:
            // no 128 flag in emmentaler, so stack 16th and 32nd on top of each other...
            renderText(painter, x, y, QString(stemsUp ? 0xE189 : 0xE18F));
            renderText(painter, p.x(), p.y() + (stemsUp ? 13 : -13), QString(stemsUp ? 0xE188 : 0xE18E));
            break;
        case SixtyFourthNote:
            renderText(painter, x, y, QString(stemsUp ? 0xE18A : 0xE190));
            break;
        case ThirtySecondNote:
            renderText(painter, x, y, QString(stemsUp ? 0xE189 : 0xE18F));
            break;
        case SixteenthNote:
            renderText(painter, x, y, QString(stemsUp ? 0xE188 : 0xE18E));
            break;
        case EighthNote:
            renderText(painter, x, y, QString(stemsUp ? 0xE187 : 0xE18B));
            break;
        default:
            // no flags
            break;
    }
}
