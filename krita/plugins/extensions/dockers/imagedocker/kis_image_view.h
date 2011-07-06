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

#ifndef H_KIS_IMAGE_VIEW_H
#define H_KIS_IMAGE_VIEW_H

#include <QScrollArea>
#include <QPixmap>
#include <QRubberBand>

class KisImageViewport: public QWidget
{
    Q_OBJECT
    
public:
    KisImageViewport();
    
    QRect  imageRect() const;
    QColor imageColor(const QPoint& pos) const;
    QSize  imageSize() const;
    
    void setImage(const QPixmap& pixmap, qreal scale);
    void setScale(qreal scale);
    
    virtual QSize sizeHint() const;
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    
private:
    qreal       m_scale;
    QPixmap     m_pixmap;
    QPixmap     m_cachedPixmap;
    bool        m_mousePressed;
    QRubberBand m_rubberBand;
    QRect       m_selection;
    
signals:
    void sigImageClicked(const QPoint& pos);
    void sigRegionSelected(const QRect& rect);
};

class KisImageView: public QScrollArea
{
    Q_OBJECT
    
public:
    enum { VIEW_MODE_FREE=0, VIEW_MODE_ADJUST=1, VIEW_MODE_FIT=2 };
    
    KisImageView(QWidget* parent=0);
    
    void setPixmap(const QPixmap& pixmap, int viewMode=VIEW_MODE_FIT, qreal scale=1.0);
    void setViewMode(int viewMode, qreal scale=1.0);
    void setScrollPos(const QPoint& pos);
    QPoint getScrollPos() const;
    qreal getScale() const;
    
signals:
    void sigColorSelected(const QColor& color);
    void sigViewModeChanged(int viewMode, qreal scale);
    
private slots:
    void slotImageClicked(const QPoint& pos);
    void slotRegionSelected(const QRect& rect);
    
private:
    qreal calcScale(qreal scale, int viewMode, const QSizeF& imgSize) const;
    QSize viewportSize(bool withScrollbars) const;
    virtual void resizeEvent(QResizeEvent* event);
    
private:
    qreal             m_scale;
    int               m_viewMode;
    qreal             m_minScale;
    qreal             m_maxScale;
    KisImageViewport* m_imgViewport;
};

#endif // H_KIS_IMAGE_VIEW_H
