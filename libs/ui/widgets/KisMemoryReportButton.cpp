/*
 *  Copyright (c) 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisMemoryReportButton.h"
#include <QPaintEvent>
#include <QPalette>
#include <QDebug>
#include <QStyleOptionButton>
#include <QStylePainter>

KisMemoryReportButton::KisMemoryReportButton(QWidget *parent) :
    QPushButton(parent)
    , m_maxbytes(0)
    , m_curbytes(0)
    , m_imgbytes(0)
{

}

void KisMemoryReportButton::setMaximumMemory(qint64 max)
{
    m_maxbytes = max;
}

void KisMemoryReportButton::setCurrentMemory(qint64 memory)
{
    m_curbytes = memory;
}

void KisMemoryReportButton::setImageWeight(qint64 memory)
{
    m_imgbytes = memory;
}

void KisMemoryReportButton::paintEvent(QPaintEvent *e)
{
    qreal ratioCur = qreal(m_curbytes)/qreal(m_maxbytes);
    QStyleOptionButton buttonStyle;
    buttonStyle.initFrom(this);
    QRect area = this->style()->subElementRect(QStyle::SE_PushButtonFocusRect, &buttonStyle);

    int totalWidth = area.width();

    QStylePainter painter(this);

    painter.setPen(Qt::transparent);
    if (style()->objectName() == "breeze") {
        painter.drawPrimitive(QStyle::PE_PanelButtonCommand, buttonStyle);
    } else {
        painter.drawPrimitive(QStyle::PE_Frame, buttonStyle);
    }

    QColor HL = this->palette().highlight().color();
    QColor mid = QColor(220, 220, 0);
    QColor warn = QColor(220, 0, 0);
    if (ratioCur>=0.2 && ratioCur<0.4) {
        qreal newRatio = (ratioCur-0.2)/0.2;
        qreal negRatio = 1-newRatio;
        HL.setRed( qMax(0, qMin(int(HL.red()*negRatio) + int(mid.red()*newRatio), 255)));
        HL.setGreen( qMax(0, qMin(int(HL.green()*negRatio) + int(mid.green()*newRatio), 255)));
        HL.setBlue( qMax(0, qMin(int(HL.blue()*negRatio) + int(mid.blue()*newRatio), 255)));
    }
    else if (ratioCur>=0.4 && ratioCur<0.8) {
        qreal newRatio = (ratioCur-0.4)/0.4;
        qreal negRatio = 1-newRatio;
        HL.setRed( qMax(0, qMin(int(mid.red()*negRatio) + int(warn.red()*newRatio), 255)));
        HL.setGreen( qMax(0, qMin(int(mid.green()*negRatio) + int(warn.green()*newRatio), 255)));
        HL.setBlue( qMax(0, qMin(int(mid.blue()*negRatio) + int(warn.blue()*newRatio), 255)));
    }
    else if (ratioCur>0.8) {
        HL = warn;
    }

    painter.setBrush(HL);
    QRect currentBytes = area;
    currentBytes.setWidth(int(ratioCur*totalWidth));
    painter.setOpacity(0.5);

    painter.drawRoundRect(currentBytes, 2, 2);

    if (m_imgbytes<m_curbytes) {
        QRect imageSize = area;
        imageSize.setWidth(int((qreal(m_imgbytes)/qreal(m_maxbytes))*totalWidth));

        painter.setOpacity(1.0);
        painter.drawRoundRect(imageSize, 2, 2);
    }

    QPushButton::paintEvent(e);

}
