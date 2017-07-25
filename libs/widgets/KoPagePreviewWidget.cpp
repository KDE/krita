/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Gary Cramblitt <garycramblitt@comcast.net>
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

#include "KoPagePreviewWidget.h"

#include <KoDpi.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <KoColumns.h>

#include <QPainter>
#include <WidgetsDebug.h>

class Q_DECL_HIDDEN KoPagePreviewWidget::Private
{
public:
    KoPageLayout pageLayout;
    KoColumns columns;
};


KoPagePreviewWidget::KoPagePreviewWidget(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    setMinimumSize( 100, 100 );
}

KoPagePreviewWidget::~KoPagePreviewWidget()
{
    delete d;
}

void KoPagePreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // resolution[XY] is in pixel per pt
    qreal resolutionX = POINT_TO_INCH( static_cast<qreal>(KoDpi::dpiX()) );
    qreal resolutionY = POINT_TO_INCH( static_cast<qreal>(KoDpi::dpiY()) );

    qreal pageWidth = d->pageLayout.width * resolutionX;
    qreal pageHeight = d->pageLayout.height * resolutionY;

    const bool pageSpread = (d->pageLayout.bindingSide >= 0 && d->pageLayout.pageEdge >= 0);
    qreal sheetWidth = pageWidth / (pageSpread?2:1);

    qreal zoomH = (height() * 90 / 100) / pageHeight;
    qreal zoomW = (width() * 90 / 100) / pageWidth;
    qreal zoom = qMin( zoomW, zoomH );

    pageWidth *= zoom;
    sheetWidth *= zoom;
    pageHeight *= zoom;
    QPainter painter( this );

    QRect page = QRectF((width() - pageWidth) / 2.0,
            (height() - pageHeight) / 2.0, sheetWidth, pageHeight).toRect();

    painter.save();
    drawPage(painter, zoom, page, true);
    painter.restore();
    if(pageSpread) {
        page.moveLeft(page.left() + (int) (sheetWidth));
        painter.save();
        drawPage(painter, zoom, page, false);
        painter.restore();
    }

    painter.end();

    // paint scale
}

void KoPagePreviewWidget::drawPage(QPainter &painter, qreal zoom, const QRect &dimensions, bool left)
{
    painter.fillRect(dimensions, QBrush(palette().base()));
    painter.setPen(QPen(palette().color(QPalette::Dark), 0));
    painter.drawRect(dimensions);

    // draw text areas
    QRect textArea = dimensions;
    if ((d->pageLayout.topMargin == 0 && d->pageLayout.bottomMargin == 0 &&
            d->pageLayout.leftMargin == 0 && d->pageLayout.rightMargin == 0) ||
            ( d->pageLayout.pageEdge == 0 && d->pageLayout.bindingSide == 0)) {
        // no margin
        return;
    }
    else {
        textArea.setTop(textArea.top() + qRound(zoom * d->pageLayout.topMargin));
        textArea.setBottom(textArea.bottom() - qRound(zoom * d->pageLayout.bottomMargin));

        qreal leftMargin, rightMargin;
        if(d->pageLayout.bindingSide < 0) { // normal margins.
            leftMargin = d->pageLayout.leftMargin;
            rightMargin = d->pageLayout.rightMargin;
        }
        else { // margins mirrored for left/right pages
            leftMargin = d->pageLayout.bindingSide;
            rightMargin = d->pageLayout.pageEdge;
            if(left)
                std::swap(leftMargin, rightMargin);
        }
        textArea.setLeft(textArea.left() + qRound(zoom * leftMargin));
        textArea.setRight(textArea.right() - qRound(zoom * rightMargin));
    }
    painter.setBrush( QBrush( palette().color(QPalette::ButtonText), Qt::HorPattern ) );
    painter.setPen(QPen(palette().color(QPalette::Dark), 0));

    // uniform columns?
    if (d->columns.columnData.isEmpty()) {
        qreal columnWidth = (textArea.width() + (d->columns.gapWidth * zoom)) / d->columns.count;
        int width = qRound(columnWidth - d->columns.gapWidth * zoom);
        for ( int i = 0; i < d->columns.count; ++i )
            painter.drawRect( qRound(textArea.x() + i * columnWidth), textArea.y(), width, textArea.height());
    } else {
        qreal totalRelativeWidth = 0.0;
        Q_FOREACH (const KoColumns::ColumnDatum &cd, d->columns.columnData) {
            totalRelativeWidth += cd.relativeWidth;
        }
        int relativeColumnXOffset = 0;
        for (int i = 0; i < d->columns.count; i++) {
            const KoColumns::ColumnDatum &columnDatum = d->columns.columnData.at(i);
            const qreal columnWidth = textArea.width() * columnDatum.relativeWidth / totalRelativeWidth;
            const qreal columnXOffset = textArea.width() * relativeColumnXOffset / totalRelativeWidth;

            painter.drawRect( qRound(textArea.x() + columnXOffset + columnDatum.leftMargin * zoom),
                              qRound(textArea.y()  + columnDatum.topMargin * zoom),
                              qRound(columnWidth - (columnDatum.leftMargin + columnDatum.rightMargin) * zoom),
                              qRound(textArea.height() - (columnDatum.topMargin + columnDatum.bottomMargin) * zoom));

            relativeColumnXOffset += columnDatum.relativeWidth;
        }
    }
}

void KoPagePreviewWidget::setPageLayout(const KoPageLayout &layout)
{
    d->pageLayout = layout;
    update();
}

void KoPagePreviewWidget::setColumns(const KoColumns &columns)
{
    d->columns = columns;
    update();
}

