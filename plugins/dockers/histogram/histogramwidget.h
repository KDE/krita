/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QPixmap>
#include "kis_histogram_view.h"

class KisCanvas2;
class KisSignalCompressor;
class KoCanvasBase;

class HistogramWidget : public QLabel
{
    Q_OBJECT

public:
    HistogramWidget(QWidget * parent = 0);

    virtual ~HistogramWidget();

    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas() { m_canvas = 0; }   

public Q_SLOTS:
    void startUpdateHistogram();

protected:
    void showEvent(QShowEvent *event);

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    
private:
    KisSignalCompressor *m_compressor;
    QPixmap m_pixmap;
    KisCanvas2 *m_canvas;
    KisHistogram *m_histogram;
    KoHistogramProducer *m_current_producer;
};


#endif /* HISTOGRAMWIDGET_H */
