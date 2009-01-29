/* This file is part of the KDE project
 * Copyright (C)  2008 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#include "FormattingPreview.h"

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QRectF>
#include <QString>

#include <math.h>

#include <klocale.h>
#include "kdebug.h"

 FormattingPreview::FormattingPreview(QWidget* parent)
        : QFrame(parent),
        m_font(QFont("Times", 12)),
        m_fontCapitalisation(QFont::MixedCase),
        m_sampleText(i18n("Font")),
        m_strikethroughType(KoCharacterStyle::NoLineType),
        m_strikethroughStyle(KoCharacterStyle::NoLineStyle),
        m_strikethroughColor(QColor(Qt::black)),
        m_underlineType(KoCharacterStyle::NoLineType),
        m_underlineStyle(KoCharacterStyle::NoLineStyle),
        m_underlineColor(QColor(Qt::black)),
        m_textColor(QColor(Qt::black)),
        m_backgroundColor(QColor(Qt::transparent))
{
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setMinimumSize( 500, 150 );
}

FormattingPreview::~FormattingPreview()
{
}

void FormattingPreview::setBackgroundColor(QColor color)
{
    m_backgroundColor = color;
    update();
}

void FormattingPreview::setFont(const QFont &font)
{
    m_font = font;
    update();
}

void FormattingPreview::setFontCapitalisation(QFont::Capitalization capitalisation)
{
    m_fontCapitalisation = capitalisation;
    update();
}

void FormattingPreview::setStrikethrough(KoCharacterStyle::LineType strikethroughType, KoCharacterStyle::LineStyle strikethroughStyle, const QColor &color)
{
    m_strikethroughType = strikethroughType;
    m_strikethroughStyle = strikethroughStyle;
    m_strikethroughColor = color;
    update();
}

void FormattingPreview::setText(const QString &sampleText)
{
    m_sampleText = sampleText;
    update();
}

void FormattingPreview::setTextColor(QColor color)
{
    m_textColor = color;
    update();
}

void FormattingPreview::setUnderline(KoCharacterStyle::LineType underlineType, KoCharacterStyle::LineStyle underlineStyle, const QColor &underlineColor)
{
    m_underlineType = underlineType;
    m_underlineStyle = underlineStyle;
    m_underlineColor = underlineColor;
    update();
}

//Painting related methods

void FormattingPreview::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(contentsRect(), QBrush(Qt::white));

//set up the Font properties
    QFont displayFont = m_font;
    displayFont.setCapitalization(m_fontCapitalisation);
    painter.setFont(displayFont);

//draw the preview text
    QRect boundingRect = painter.boundingRect(contentsRect(), Qt::AlignCenter, m_sampleText);
    if ((boundingRect.width() > contentsRect().width()) || (boundingRect.height() > contentsRect().height())) {
        displayFont.setPointSize(12);
        painter.setFont(displayFont);
        boundingRect = painter.boundingRect(contentsRect(), Qt::AlignCenter, m_sampleText);
    }
    QRectF displayRect = QRectF((contentsRect().width()-boundingRect.width())/2, (contentsRect().height()-boundingRect.height())/2, boundingRect.width(), boundingRect.height());

    QFontMetrics fm = QFontMetrics(displayFont);

    painter.setPen(m_textColor);
    painter.fillRect(displayRect, QBrush(m_backgroundColor));
    painter.drawText(displayRect,  Qt::AlignLeft | Qt::AlignVCenter, m_sampleText);

    //draw underline
    if ((m_underlineType != KoCharacterStyle::NoLineType) && (m_underlineStyle != KoCharacterStyle::NoLineStyle)) {

        qreal xstart = displayRect.x();
        qreal xend = displayRect.x() + displayRect.width();
        qreal y = displayRect.y() + fm.ascent() + fm.underlinePos() + 1;

        qreal width;
        switch (m_font.weight()) {
        case QFont::Light:
            width = fm.lineWidth() /2;
            break;
        case QFont::Normal: //Falltrhough
        case QFont::DemiBold:
            width = fm.lineWidth();
            break;
        case QFont::Bold: //Fallthrough
        case QFont::Black:
            width = fm.lineWidth() * 2;
            break;
        default:
            width = fm.lineWidth();
        }
        drawLine(painter, xstart, xend, y, width, fm.underlinePos(), m_underlineType, m_underlineStyle, m_underlineColor);
    }
    //draw strikethrough
    if ((m_strikethroughType != KoCharacterStyle::NoLineType) && (m_strikethroughStyle != KoCharacterStyle::NoLineStyle)) {

        qreal xstart = displayRect.x();
        qreal xend = displayRect.x() + displayRect.width();
        qreal y = displayRect.y() + fm.ascent() - fm.strikeOutPos();

        qreal width;
        switch (m_font.weight()) {
        case QFont::Light:
            width = fm.lineWidth() /2;
            break;
        case QFont::Normal: //Falltrhough
        case QFont::DemiBold:
            width = fm.lineWidth();
            break;
        case QFont::Bold: //Fallthrough
        case QFont::Black:
            width = fm.lineWidth() * 2;
            break;
        default:
            width = fm.lineWidth();
        }
        drawLine(painter, xstart, xend, y, width, fm.strikeOutPos(), m_strikethroughType, m_strikethroughStyle, m_strikethroughColor);
    }
}

void FormattingPreview::drawLine(QPainter &painter, qreal xstart, qreal xend, qreal y, qreal width, int distToBaseLine, KoCharacterStyle::LineType lineType, KoCharacterStyle::LineStyle lineStyle, QColor lineColor)
{
//Following code derived from plugins/textShape/Layout::drawDecorationLine
    painter.save();
    QPen pen = painter.pen();
    pen.setColor(lineColor);
    pen.setWidthF(width);

    if (lineStyle == KoCharacterStyle::WaveLine) {
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const qreal halfWaveWidth = 2 * distToBaseLine;
        const qreal halfWaveLength = 4 * distToBaseLine;
        const int startAngle = 45; //in °
        const int middleAngle = 225; //in °
        const int spanAngle = 90; //in °
        qreal xbound = xstart - (1 - cos(startAngle))*halfWaveLength / 2;
        qreal ybound1 = y - (1 - sin(startAngle))*halfWaveWidth / 2;
        qreal ybound2 = y - (1 + sin(startAngle))*halfWaveWidth / 2;
        while (xbound < xend) {
            QRectF rectangle1(xbound, ybound1, halfWaveLength, halfWaveWidth);
            if (lineType == KoCharacterStyle::DoubleLine) {
                painter.translate(0, -pen.width());
                painter.drawArc(rectangle1, startAngle * 16, spanAngle * 16);
                painter.translate(0, 2*pen.width());
                painter.drawArc(rectangle1, startAngle * 16, spanAngle * 16);
                painter.translate(0, -pen.width());
            } else {
                painter.drawArc(rectangle1, startAngle * 16, spanAngle * 16);
            }
            if (xbound + (1 - cos(startAngle)) * halfWaveLength > xend)
                break;
            xbound = xbound + (1 - cos(startAngle)) * halfWaveLength;
            QRectF rectangle2(xbound, ybound2, halfWaveLength, halfWaveWidth);
            if (lineType == KoCharacterStyle::DoubleLine) {
                painter.translate(0, -pen.width());
                painter.drawArc(rectangle2, middleAngle * 16, spanAngle * 16);
                painter.translate(0, 2*pen.width());
                painter.drawArc(rectangle2, middleAngle * 16, spanAngle * 16);
                painter.translate(0, -pen.width());
            } else {
                painter.drawArc(rectangle2, middleAngle * 16, spanAngle * 16);
            }
            xbound = xbound + (1 - cos(startAngle)) * halfWaveLength;
        }
    }
    else {
        if (lineStyle == KoCharacterStyle::SolidLine)
            pen.setStyle(Qt::SolidLine);
        else if (lineStyle == KoCharacterStyle::DashLine)
            pen.setStyle(Qt::DashLine);
        else if (lineStyle == KoCharacterStyle::DottedLine)
            pen.setStyle(Qt::DotLine);
        else if (lineStyle == KoCharacterStyle::DotDashLine)
            pen.setStyle(Qt::DashDotLine);
        else if (lineStyle == KoCharacterStyle::DotDotDashLine)
            pen.setStyle(Qt::DashDotDotLine);
        painter.setPen(pen);
        if (lineType == KoCharacterStyle::DoubleLine) {
            painter.translate(0, -pen.width());
            painter.drawLine(QPointF(xstart, y), QPointF(xend, y));
            painter.translate(0, 2*pen.width());
            painter.drawLine(QPointF(xstart, y), QPointF(xend, y));
            painter.translate(0, -pen.width());
        } else {
            painter.drawLine(QPointF(xstart, y), QPointF(xend, y));
        }
    }
    painter.restore();
}

#include "FormattingPreview.moc"
