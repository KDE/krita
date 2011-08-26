/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_image_view.h"

#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QScrollBar>

//////////////////////////////////////////////////////////////////////////////
// -------- KisImageViewport ---------------------------------------------- //

KisImageViewport::KisImageViewport():
    m_scale(1.0f),
    m_mousePressed(false),
    m_rubberBand(QRubberBand::Rectangle, this)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void KisImageViewport::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(imageRect().topLeft(), m_cachedPixmap);
}

void KisImageViewport::setImage(const QPixmap& pixmap, qreal scale)
{
    m_scale        = scale;
    m_pixmap       = pixmap;
    m_cachedPixmap = pixmap.scaled(imageRect().size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

void KisImageViewport::setScale(qreal scale)
{
    if(!qFuzzyCompare(scale, m_scale)) {
        m_scale        = scale;
        m_cachedPixmap = m_pixmap.scaled(imageRect().size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
}

QColor KisImageViewport::imageColor(const QPoint& pos) const
{
    return m_cachedPixmap.copy(pos.x(), pos.y(), 1, 1).toImage().pixel(0,0);
}

QSize KisImageViewport::sizeHint() const
{
    return imageRect().size();
}

QRect KisImageViewport::imageRect() const
{
    int w = int(m_scale * m_pixmap.width());
    int h = int(m_scale * m_pixmap.height());
    int x = (width()  - w) / 2;
    int y = (height() - h) / 2;
    return QRect(x, y, w, h);
}

QSize KisImageViewport::imageSize() const
{
    return m_pixmap.size();
}

void KisImageViewport::mousePressEvent(QMouseEvent* event)
{
    m_mousePressed = true;
    m_selection    = QRect(event->pos(), QSize(0,0));
    m_rubberBand.setGeometry(m_selection);
    m_rubberBand.show();
}

void KisImageViewport::mouseMoveEvent(QMouseEvent* event)
{
    if(m_mousePressed) {
        QPoint size = event->pos() - m_selection.topLeft();
        m_selection.setSize(QSize(size.x(), size.y()));
        m_rubberBand.setGeometry(m_selection.normalized());
    }
}

void KisImageViewport::mouseReleaseEvent(QMouseEvent* event)
{
    m_selection = m_selection.normalized();
    
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
// -------- KisImageView -------------------------------------------------- //

KisImageView::KisImageView(QWidget* parent):
    QScrollArea(parent),
    m_viewMode(VIEW_MODE_FIT),
    m_minScale(0.05),
    m_maxScale(5.00)
{
    m_imgViewport = new KisImageViewport();
    QScrollArea::setWidgetResizable(true);
    QScrollArea::setWidget(m_imgViewport);
    
    connect(m_imgViewport, SIGNAL(sigImageClicked(const QPoint&)) , SLOT(slotImageClicked(const QPoint&)));
    connect(m_imgViewport, SIGNAL(sigRegionSelected(const QRect&)), SLOT(slotRegionSelected(const QRect&)));
}

void KisImageView::setPixmap(const QPixmap& pixmap, int viewMode, qreal scale)
{
    m_viewMode = viewMode;
    m_scale    = calcScale(scale, viewMode, pixmap.size());
    m_imgViewport->setImage(pixmap, m_scale);
    m_imgViewport->setMinimumSize(m_imgViewport->sizeHint());
    m_imgViewport->adjustSize();
    
    emit sigViewModeChanged(m_viewMode, m_scale);
}

void KisImageView::setViewMode(int viewMode, qreal scale)
{
    m_viewMode = viewMode;
    m_scale    = calcScale(scale, viewMode, m_imgViewport->imageSize());
    m_imgViewport->setScale(m_scale);
    m_imgViewport->setMinimumSize(m_imgViewport->sizeHint());
    m_imgViewport->adjustSize();
    
    emit sigViewModeChanged(m_viewMode, m_scale);
}

void KisImageView::setScrollPos(const QPoint& pos)
{
    horizontalScrollBar()->setValue(pos.x());
    verticalScrollBar()->setValue(pos.y());
}

QPoint KisImageView::getScrollPos() const
{
    return QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
}

qreal KisImageView::getScale() const
{
    return m_scale;
}

qreal KisImageView::calcScale(qreal scale, int viewMode, const QSizeF& imgSize) const
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

QSize KisImageView::viewportSize(bool withScrollbars) const
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

void KisImageView::slotImageClicked(const QPoint& pos)
{
    emit sigColorSelected(m_imgViewport->imageColor(pos));
}

void KisImageView::slotRegionSelected(const QRect& rect)
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

void KisImageView::resizeEvent(QResizeEvent* event)
{
    QScrollArea::resizeEvent(event);
    
    m_scale = calcScale(m_scale, m_viewMode, m_imgViewport->imageSize());
    m_imgViewport->setScale(m_scale);
    m_imgViewport->setMinimumSize(m_imgViewport->sizeHint());
    m_imgViewport->adjustSize();
    
    emit sigViewModeChanged(m_viewMode, m_scale);
}

