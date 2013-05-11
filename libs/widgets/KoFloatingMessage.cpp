/*
 *  This file is part of the KDE project
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
#include "KoFloatingMessage.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QRegExp>

#include <kapplication.h>
#include <kwindowsystem.h>

#include <KoIcon.h>

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

class KoFloatingMessage::Private
{
public:
    Private(const QString &message, bool showOverParent)
        :   message(message)
        ,   showOverParent(showOverParent)
    {
    }

    QString message;
    QImage icon;
    QPixmap scaledIcon;
    QTimer timer;
    int m;
    QTimeLine fadeTimeLine;
    bool showOverParent;
};

KoFloatingMessage::KoFloatingMessage(const QString &message, QWidget *parent, bool showOverParent)
    : QWidget(parent)
    , d(new Private(message, showOverParent))
{
    d->icon = koIcon("dialog-information").pixmap(256, 256).toImage();

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setFont(QFont("sans-serif"));

    #ifdef Q_WS_X11
    KWindowSystem::setType( winId(), NET::Notification );
    #endif

    d->timer.setSingleShot( true );
    connect(&d->timer, SIGNAL(timeout()), SLOT(startFade()));
}

KoFloatingMessage::~KoFloatingMessage()
{
    delete d;
}

void KoFloatingMessage::showMessage()
{
    setGeometry(determineMetrics(fontMetrics().width('x')));
    setWindowOpacity(OSD_WINDOW_OPACITY);

    QWidget::setVisible(true);
    d->timer.start(4500);
}

void KoFloatingMessage::setShowOverParent(bool show)
{
    d->showOverParent = show;
}

void KoFloatingMessage::setIcon(const QIcon& icon)
{
    d->icon = icon.pixmap(256, 256).toImage();
}

const int MARGIN = 20;

QRect KoFloatingMessage::determineMetrics( const int M )
{
    d->m = M;

    const QSize minImageSize = d->icon.size().boundedTo(QSize(100, 100));

    // determine a sensible maximum size, don't cover the whole desktop or cross the screen
    const QSize margin( (M + MARGIN) * 2, (M + MARGIN) * 2); //margins
    const QSize image = d->icon.isNull() ? QSize(0, 0) : minImageSize;
    const QSize max = QApplication::desktop()->availableGeometry(parentWidget()).size() - margin;

    // If we don't do that, the boundingRect() might not be suitable for drawText() (Qt issue N67674)
    d->message.replace(QRegExp( " +\n"), "\n");
    // remove consecutive line breaks
    d->message.replace(QRegExp( "\n+"), "\n");

    // The osd cannot be larger than the screen
    QRect rect = fontMetrics().boundingRect(0, 0, max.width() - image.width(), max.height(),
            Qt::AlignCenter | Qt::TextWordWrap, d->message);
    rect.setHeight(rect.height() + M + M);

    if (!d->icon.isNull()) {
        const int availableWidth = max.width() - rect.width() - M; //WILL be >= (minImageSize.width() - M)

        d->scaledIcon = QPixmap::fromImage(d->icon.scaled(qMin(availableWidth, d->icon.width()),
                                                        qMin( rect.height(), d->icon.height()),
                                                        Qt::KeepAspectRatio, Qt::SmoothTransformation));

        const int widthIncludingImage = rect.width() + d->scaledIcon.width() + M; //margin between text + image
        rect.setWidth( widthIncludingImage );
    }

    // expand in all directions by M
    rect.adjust( -M, -M, M, M );

    const QSize newSize = rect.size();
    QRect screen = QApplication::desktop()->screenGeometry(parentWidget());
    if (parentWidget() && d->showOverParent) {
        screen = parentWidget()->geometry();
        screen.setTopLeft(parentWidget()->mapToGlobal(QPoint(0, 0)));
    }
    QPoint newPos(MARGIN, MARGIN);

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

    QRect rc(newPos, rect.size());

    return rc;
}

void KoFloatingMessage::paintEvent( QPaintEvent *e )
{
    const int& M = d->m;

    QPoint point;
    QRect rect(point, size());
    rect.adjust(0, 0, -1, -1);

    int align = Qt::AlignCenter | Qt::TextWordWrap;

    QPainter p(this);
    p.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing );
    p.setClipRect(e->rect());

    //QPixmap background = The::svgHandler()->renderSvgWithDividers( "service_list_item", width(), height(), "service_list_item" );
    //p.drawPixmap( 0, 0, background );

    p.setPen(Qt::white);
    rect.adjust(M, M, -M, -M);

    if (!d->icon.isNull()) {
        QRect r(rect);
        r.setTop((size().height() - d->scaledIcon.height() ) / 2);
        r.setSize(d->scaledIcon.size());
        p.drawPixmap(r.topLeft(), d->scaledIcon);
        rect.setLeft(rect.left() + d->scaledIcon.width() + M);
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
    p2.drawText(QRect( QPoint( 5, 5 ), rect.size() ), align, d->message);
    p2.end();

    QColor shadowColor;
    {
        int h, s, v;
        palette().color( QPalette::Normal, QPalette::Foreground ).getHsv( &h, &s, &v );
        shadowColor = v > 128 ? Qt::black : Qt::white;
    }
    p.drawImage(rect.topLeft() - QPoint(5, 5), ShadowEngine::makeShadow(pixmap, shadowColor));

    p.setPen( palette().color(QPalette::Active, QPalette::WindowText ));
    p.drawText(rect, align, d->message);
}

void KoFloatingMessage::startFade()
{
    d->fadeTimeLine.setDuration(250);
    d->fadeTimeLine.setCurveShape(QTimeLine::EaseInCurve);
    d->fadeTimeLine.setLoopCount(1);
    d->fadeTimeLine.setFrameRange(OSD_WINDOW_OPACITY, 0);
    d->fadeTimeLine.setFrameRange(0, 10);
    connect(&d->fadeTimeLine, SIGNAL(finished()), SLOT(removeMessage()));
    connect(&d->fadeTimeLine, SIGNAL(frameChanged(int)), SLOT(updateOpacity(int)));
    d->fadeTimeLine.start();
}

void KoFloatingMessage::removeMessage()
{
    hide();
    deleteLater();
}

void KoFloatingMessage::updateOpacity(int value)
{
    Q_UNUSED(value);
    setWindowOpacity(OSD_WINDOW_OPACITY - 0.1);
}
