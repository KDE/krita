/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Christian Muehlhaeuser <chris@chris.de>
 *  Copyright (c) 2004-2006 Seb Ruiz <ruiz@kde.org>
 *  Copyright (c) 2004,2005 Max Howell <max.howell@methylblue.com>
 *  Copyright (c) 2005 Gabor Lehel <illissius@gmail.com>
 *  Copyright (c) 2008,2009 Mark Kretschmann <kretschmann@kde.org>
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
#include "kis_floating_message.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QRegExp>

#include <kis_icon.h>
#include <kis_debug.h>
#include "kis_global.h"


/* Code copied from kshadowengine.cpp
 *
 * Copyright (C) 2003 Laur Ivan <laurivan@eircom.net>
 *
 * Many thanks to:
 *  - Bernardo Hung <deciare@gta.igs.net> for the enhanced shadow
 *    algorithm (currently used)
 *  - Tim Jansen <tim@tjansen.de> for the API updates and fixes.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

namespace ShadowEngine
{
    // Not sure, doesn't work above 10
    static const int    MULTIPLICATION_FACTOR = 3;
    // Multiplication factor for pixels directly above, under, or next to the text
    static const double AXIS_FACTOR = 2.0;
    // Multiplication factor for pixels diagonal to the text
    static const double DIAGONAL_FACTOR = 0.1;
    // Self explanatory
    static const int    MAX_OPACITY = 200;

    double decay( QImage&, int, int );

    QImage makeShadow( const QPixmap& textPixmap, const QColor &bgColor )
    {
        const int w   = textPixmap.width();
        const int h   = textPixmap.height();
        const int bgr = bgColor.red();
        const int bgg = bgColor.green();
        const int bgb = bgColor.blue();

        int alphaShadow;

        // This is the source pixmap
        QImage img = textPixmap.toImage();

        QImage result( w, h, QImage::Format_ARGB32 );
        result.fill( 0 ); // fill with black

        static const int M = 5;
        for( int i = M; i < w - M; i++) {
            for( int j = M; j < h - M; j++ )
            {
                alphaShadow = (int) decay( img, i, j );

                result.setPixel( i,j, qRgba( bgr, bgg , bgb, qMin( MAX_OPACITY, alphaShadow ) ) );
            }
        }

        return result;
    }

    double decay( QImage& source, int i, int j )
    {
        double alphaShadow;
        alphaShadow =(qGray(source.pixel(i-1,j-1)) * DIAGONAL_FACTOR +
                qGray(source.pixel(i-1,j  )) * AXIS_FACTOR +
                qGray(source.pixel(i-1,j+1)) * DIAGONAL_FACTOR +
                qGray(source.pixel(i  ,j-1)) * AXIS_FACTOR +
                0                         +
                qGray(source.pixel(i  ,j+1)) * AXIS_FACTOR +
                qGray(source.pixel(i+1,j-1)) * DIAGONAL_FACTOR +
                qGray(source.pixel(i+1,j  )) * AXIS_FACTOR +
                qGray(source.pixel(i+1,j+1)) * DIAGONAL_FACTOR) / MULTIPLICATION_FACTOR;

        return alphaShadow;
    }
}

#define OSD_WINDOW_OPACITY 0.74

KisFloatingMessage::KisFloatingMessage(const QString &message, QWidget *parent, bool showOverParent, int timeout, Priority priority, int alignment)
    : QWidget(parent)
    , m_message(message)
    , m_showOverParent(showOverParent)
    , m_timeout(timeout)
    , m_priority(priority)
    , m_alignment(alignment)
    , widgetQueuedForDeletion(false)
{
    m_icon = KisIconUtils::loadIcon("calligrakrita").pixmap(256, 256).toImage();

    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setFont(QFont("sans-serif"));

    m_timer.setSingleShot( true );
    connect(&m_timer, SIGNAL(timeout()), SLOT(startFade()));
    connect(this, SIGNAL(destroyed()), SLOT(widgetDeleted()));
}

void KisFloatingMessage::tryOverrideMessage(const QString message,
                                            const QIcon& icon,
                                            int timeout,
                                            KisFloatingMessage::Priority priority,
                                            int alignment)
{
    if ((int)priority > (int)m_priority) return;

    m_message = message;
    setIcon(icon);
    m_timeout = timeout;
    m_priority = priority;
    m_alignment = alignment;
    showMessage();
    update();
}

void KisFloatingMessage::showMessage()
{
    if (widgetQueuedForDeletion) return;

    setGeometry(determineMetrics(fontMetrics().width('x')));
    setWindowOpacity(OSD_WINDOW_OPACITY);

    QWidget::setVisible(true);
    m_timer.start(m_timeout);
}

void KisFloatingMessage::setShowOverParent(bool show)
{
    m_showOverParent = show;
}

void KisFloatingMessage::setIcon(const QIcon& icon)
{
    m_icon = icon.pixmap(256, 256).toImage();
}

const int MARGIN = 20;

QRect KisFloatingMessage::determineMetrics( const int M )
{
    m_m = M;

    const QSize minImageSize = m_icon.size().boundedTo(QSize(100, 100));

    // determine a sensible maximum size, don't cover the whole desktop or cross the screen
    const QSize margin( (M + MARGIN) * 2, (M + MARGIN) * 2); //margins
    const QSize image = m_icon.isNull() ? QSize(0, 0) : minImageSize;
    const QSize max = QApplication::desktop()->availableGeometry(parentWidget()).size() - margin;

    // If we don't do that, the boundingRect() might not be suitable for drawText() (Qt issue N67674)
    m_message.replace(QRegExp( " +\n"), "\n");
    // remove consecutive line breaks
    m_message.replace(QRegExp( "\n+"), "\n");

    // The osd cannot be larger than the screen
    QRect rect = fontMetrics().boundingRect(0, 0, max.width() - image.width(), max.height(),
           m_alignment, m_message);

    if (!m_icon.isNull()) {
        const int availableWidth = max.width() - rect.width() - M; //WILL be >= (minImageSize.width() - M)

        m_scaledIcon = QPixmap::fromImage(m_icon.scaled(qMin(availableWidth, m_icon.width()),
                                                        qMin( rect.height(), m_icon.height()),
                                                        Qt::KeepAspectRatio, Qt::SmoothTransformation));

        const int widthIncludingImage = rect.width() + m_scaledIcon.width() + M; //margin between text + image
        rect.setWidth( widthIncludingImage );
    }

    // expand in all directions by 2*M
    //
    // take care with this rect, because it must be *bigger*
    // than the rect we paint the message in
    rect = kisGrowRect(rect, 2 * M);



    const QSize newSize = rect.size();
    QRect screen = QApplication::desktop()->screenGeometry(parentWidget());

    QPoint newPos(MARGIN, MARGIN);

    if (parentWidget() && m_showOverParent) {
        screen = parentWidget()->geometry();
        screen.setTopLeft(parentWidget()->mapToGlobal(QPoint(MARGIN, MARGIN + 50)));
        newPos = screen.topLeft();
    }
    else {
        // move to the right
        newPos.rx() = screen.width() - MARGIN - newSize.width();

        //ensure we don't dip below the screen
        if (newPos.y() + newSize.height() > screen.height() - MARGIN) {
            newPos.ry() = screen.height() - MARGIN - newSize.height();
        }
        // correct for screen position
        newPos += screen.topLeft();

        if (parentWidget()) {
            // Move a bit to the left as there could be a scrollbar
            newPos.setX(newPos.x() - MARGIN);
        }
    }

    QRect rc(newPos, rect.size());

    return rc;
}

void KisFloatingMessage::paintEvent( QPaintEvent *e )
{
    const int& M = m_m;

    QPoint point;
    QRect rect(point, size());
    rect.adjust(0, 0, -1, -1);

    QPainter p(this);
    p.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing );
    p.setClipRect(e->rect());

    //QPixmap background = The::svgHandler()->renderSvgWithDividers( "service_list_item", width(), height(), "service_list_item" );
    //p.drawPixmap( 0, 0, background );

    p.setPen(Qt::white);
    rect.adjust(M, M, -M, -M);

    if (!m_icon.isNull()) {
        QRect r(rect);
        r.setTop((size().height() - m_scaledIcon.height() ) / 2);
        r.setSize(m_scaledIcon.size());
        p.drawPixmap(r.topLeft(), m_scaledIcon);
        rect.setLeft(rect.left() + m_scaledIcon.width() + M);
    }

    int graphicsHeight = 0;

    rect.setBottom(rect.bottom() - graphicsHeight);

    // Draw "shadow" text effect (black outline)
    QPixmap pixmap( rect.size() + QSize( 10, 10 ) );
    pixmap.fill( Qt::black );

    QPainter p2(&pixmap);
    p2.setFont(font());
    p2.setPen(Qt::white);
    p2.setBrush(Qt::white);
    p2.drawText(QRect( QPoint( 5, 5 ), rect.size() ), m_alignment, m_message);
    p2.end();

    QColor shadowColor;
    {
        int h, s, v;
        palette().color( QPalette::Normal, QPalette::Foreground ).getHsv( &h, &s, &v );
        shadowColor = v > 128 ? Qt::black : Qt::white;
    }
    p.drawImage(rect.topLeft() - QPoint(5, 5), ShadowEngine::makeShadow(pixmap, shadowColor));

    p.setPen( palette().color(QPalette::Active, QPalette::WindowText ));
    p.drawText(rect, m_alignment, m_message);
}

void KisFloatingMessage::startFade()
{
    m_fadeTimeLine.setDuration(250);
    m_fadeTimeLine.setCurveShape(QTimeLine::EaseInCurve);
    m_fadeTimeLine.setLoopCount(1);
    m_fadeTimeLine.setFrameRange(0, 0);
    m_fadeTimeLine.setFrameRange(0, 10);
    connect(&m_fadeTimeLine, SIGNAL(finished()), SLOT(removeMessage()));
    connect(&m_fadeTimeLine, SIGNAL(frameChanged(int)), SLOT(updateOpacity(int)));
    m_fadeTimeLine.start();
}

void KisFloatingMessage::removeMessage()
{
    m_timer.stop();
    m_fadeTimeLine.stop();
    widgetQueuedForDeletion = true;

    hide();
    deleteLater();
}

void KisFloatingMessage::updateOpacity(int /*value*/)
{
    setWindowOpacity(OSD_WINDOW_OPACITY - 0.1);
}

void KisFloatingMessage::widgetDeleted()
{
    widgetQueuedForDeletion = false;
}
