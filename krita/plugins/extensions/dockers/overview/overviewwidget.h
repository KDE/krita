/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef OVERVIEWWIDGET_H
#define OVERVIEWWIDGET_H

#include <QWidget>
#include <QPixmap>

class KisCanvas2;
class KisSignalCompressor;
class KoCanvasBase;

class OverviewWidget : public QWidget
{
    Q_OBJECT

public:
    OverviewWidget(QWidget * parent = 0);

    virtual ~OverviewWidget();

    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas() { m_canvas = 0; }   

public slots:
    void startUpdateCanvasProjection();

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent *event);
    
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    
private:
    QSize calculatePreviewSize();
    QPointF previewOrigin();
    QTransform imageToPreviewTransform();
    QPolygonF previewPolygon();
    
    KisSignalCompressor *m_compressor;
    QPixmap m_pixmap;
    KisCanvas2 *m_canvas;

    bool m_dragging;
    QPointF m_lastPos;
};


#endif /* OVERVIEWWIDGET_H */
