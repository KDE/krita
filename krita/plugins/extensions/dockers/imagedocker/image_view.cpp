/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "image_view.h"
#include <kis_cursor.h>

#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QScrollBar>

//////////////////////////////////////////////////////////////////////////////
// -------- ImageViewport ---------------------------------------------- //

ImageViewport::ImageViewport():
    m_scale(1.0f),
    m_mousePressed(false),
    m_rubberBand(QRubberBand::Rectangle, this)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setCursor(KisCursor::pickerCursor());
}

void ImageViewport::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(imageRect().topLeft(), m_cachedPixmap);
}

void ImageViewport::setImage(const QPixmap& pixmap, qreal scale)
{
    m_scale        = scale;
    m_pixmap       = pixmap;
    m_cachedPixmap = pixmap.scaled(imageRect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void ImageViewport::setScale(qreal scale)
{
    if(!qFuzzyCompare(scale, m_scale)) {
        m_scale        = scale;
        m_cachedPixmap = m_pixmap.scaled(imageRect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}

QColor ImageViewport::imageColor(const QPoint& pos) const
{
    return m_cachedPixmap.copy(pos.x(), pos.y(), 1, 1).toImage().pixel(0,0);
}

QSize ImageViewport::sizeHint() const
{
    return imageRect().size();
}

QRect ImageViewport::imageRect() const
{
    int w = int(m_scale * m_pixmap.width());
    int h = int(m_scale * m_pixmap.height());
    int x = (width()  - w) / 2;
    int y = (height() - h) / 2;
    return QRect(x, y, w, h);
}

QSize ImageViewport::imageSize() const
{
    return m_pixmap.size();
}

void ImageViewport::mousePressEvent(QMouseEvent* event)
{
    m_mousePressed = true;
    m_selection    = QRect(event->pos(), QSize(0,0));
    m_rubberBand.setGeometry(m_selection);
    m_rubberBand.show();
}

void ImageViewport::mouseMoveEvent(QMouseEvent* event)
{
    if(m_mousePressed) {
        setCursor(KisCursor::arrowCursor());
        QPoint size = event->pos() - m_selection.topLeft();
        m_selection.setSize(QSize(size.x(), size.y()));
        m_rubberBand.setGeometry(m_selection.normalized());
    }
}

void ImageViewport::mouseReleaseEvent(QMouseEvent* event)
{
    m_selection = m_selection.normalized();
    setCursor(KisCursor::pickerCursor());

    if(m_selection.width() > 5 && m_selection.height() > 5) {
        QRect imgRect = imageRect();
        QRect rect    = imgRect.intersected(m_selection).translated(-imgRect.topLeft());
        emit sigRegionSelected(rect);
    }
    else if(imageRect().contains(event->pos(), true)) {
        emit sigImageClicked(event->pos() - imageRect().topLeft());
    }

    m_mousePressed = false;
    m_rubberBand.hide();
}


//////////////////////////////////////////////////////////////////////////////
// -------- ImageView -------------------------------------------------- //

ImageView::ImageView(QWidget* parent):
    QScrollArea(parent),
    m_viewMode(VIEW_MODE_FIT),
    m_minScale(0.05),
    m_maxScale(5.00)
{
    m_imgViewport = new ImageViewport();
    QScrollArea::setWidgetResizable(true);
    QScrollArea::setWidget(m_imgViewport);

    connect(m_imgViewport, SIGNAL(sigImageClicked(const QPoint&)) , SLOT(slotImageClicked(const QPoint&)));
    connect(m_imgViewport, SIGNAL(sigRegionSelected(const QRect&)), SLOT(slotRegionSelected(const QRect&)));
}

void ImageView::setPixmap(const QPixmap& pixmap, int viewMode, qreal scale)
{
    m_viewMode = viewMode;
    m_scale    = calcScale(scale, viewMode, pixmap.size());
    m_imgViewport->setImage(pixmap, m_scale);
    m_imgViewport->setMinimumSize(m_imgViewport->sizeHint());
    m_imgViewport->adjustSize();

    emit sigViewModeChanged(m_viewMode, m_scale);
}

void ImageView::setViewMode(int viewMode, qreal scale)
{
    m_viewMode = viewMode;
    m_scale    = calcScale(scale, viewMode, m_imgViewport->imageSize());
    m_imgViewport->setScale(m_scale);
    m_imgViewport->setMinimumSize(m_imgViewport->sizeHint());
    m_imgViewport->adjustSize();

    emit sigViewModeChanged(m_viewMode, m_scale);
}

void ImageView::setScrollPos(const QPoint& pos)
{
    horizontalScrollBar()->setValue(pos.x());
    verticalScrollBar()->setValue(pos.y());
}

QPoint ImageView::getScrollPos() const
{
    return QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
}

qreal ImageView::getScale() const
{
    return m_scale;
}

qreal ImageView::calcScale(qreal scale, int viewMode, const QSizeF& imgSize) const
{
    QSizeF viewSize  = viewportSize(viewMode == VIEW_MODE_ADJUST);
    qreal  wdgAspect = viewSize.width() / viewSize.height();
    qreal  imgAspect = imgSize.width() / imgSize.height();

    switch(viewMode)
    {
    case VIEW_MODE_FIT:
        if(wdgAspect > imgAspect) { scale = viewSize.height() / imgSize.height(); }
        else                      { scale = viewSize.width()  / imgSize.width();  }
        break;

    case VIEW_MODE_ADJUST:
        if(wdgAspect > imgAspect) { scale = viewSize.width()  / imgSize.width();  }
        else                      { scale = viewSize.height() / imgSize.height(); }
        break;
    }

    return qBound(m_minScale, scale, m_maxScale);
}

QSize ImageView::viewportSize(bool withScrollbars) const
{
    int width  = viewport()->width();
    int height = viewport()->height();
    int xAdd   = verticalScrollBar()->width();
    int yAdd   = horizontalScrollBar()->height();

    if(withScrollbars) {
        width  -= verticalScrollBar()->isVisible()   ? 0 : xAdd;
        height -= horizontalScrollBar()->isVisible() ? 0 : yAdd;
    }
    else {
        width  += verticalScrollBar()->isVisible()   ? xAdd : 0;
        height += horizontalScrollBar()->isVisible() ? yAdd : 0;
    }

    return QSize(width, height);
}

void ImageView::slotImageClicked(const QPoint& pos)
{
    emit sigColorSelected(m_imgViewport->imageColor(pos));
}

void ImageView::slotRegionSelected(const QRect& rect)
{
    QSizeF viewSize = viewportSize(true);
    QRectF selRect  = rect;

    selRect = QRectF(selRect.topLeft() / m_scale, selRect.size() / m_scale);

    qreal wdgAspect = viewSize.width() / viewSize.height();
    qreal selAspect = selRect.width() / selRect.height();

    if(wdgAspect > selAspect)
        m_scale = viewSize.height() / selRect.height();
    else
        m_scale = viewSize.width() / selRect.width();

    m_scale    = qBound(m_minScale, m_scale, m_maxScale);
    m_viewMode = VIEW_MODE_FREE;
    m_imgViewport->setScale(m_scale);
    m_imgViewport->setMinimumSize(m_imgViewport->sizeHint());
    m_imgViewport->adjustSize();

    selRect = QRectF(selRect.topLeft() * m_scale, selRect.size() * m_scale);

    QSize  offset    = ((viewSize - selRect.size()) / 2.0).toSize();
    QPoint scrollPos = selRect.topLeft().toPoint() - QPoint(offset.width(), offset.height());
    setScrollPos(scrollPos);

    emit sigViewModeChanged(m_viewMode, m_scale);
}

void ImageView::resizeEvent(QResizeEvent* event)
{
    QScrollArea::resizeEvent(event);

    m_scale = calcScale(m_scale, m_viewMode, m_imgViewport->imageSize());
    m_imgViewport->setScale(m_scale);
    m_imgViewport->setMinimumSize(m_imgViewport->sizeHint());
    m_imgViewport->adjustSize();

    emit sigViewModeChanged(m_viewMode, m_scale);
}

