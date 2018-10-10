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


#ifndef OVERVIEWWIDGET_H
#define OVERVIEWWIDGET_H
#include <QObject>
#include <QWidget>
#include <QPixmap>
#include <QPointer>

#include <QMutex>
#include "kis_idle_watcher.h"
#include "kis_simple_stroke_strategy.h"

#include <kis_canvas2.h>

class KisSignalCompressor;
class KoCanvasBase;

class OverviewThumbnailStrokeStrategy : public QObject, public KisSimpleStrokeStrategy
{
    Q_OBJECT
public:
    OverviewThumbnailStrokeStrategy(KisImageWSP image);
    ~OverviewThumbnailStrokeStrategy() override;

    static QList<KisStrokeJobData*> createJobsData(KisPaintDeviceSP dev, const QRect& imageRect, KisPaintDeviceSP thumbDev, const QSize &thumbnailSize);

private:
    void initStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;

Q_SIGNALS:
    //Emitted when thumbnail is updated and overviewImage is fully generated.
    void thumbnailUpdated(QImage pixmap);


private:
    struct Private;
    const QScopedPointer<Private> m_d;
    QMutex m_thumbnailMergeMutex;
    KisImageSP m_image;
};

class OverviewWidget : public QWidget
{
    Q_OBJECT

public:
    OverviewWidget(QWidget * parent = 0);

    ~OverviewWidget() override;

    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas()
    {
        m_canvas = 0;
    }

public Q_SLOTS:
    void startUpdateCanvasProjection();
    void generateThumbnail();
    void updateThumbnail(QImage pixmap);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QSize recalculatePreviewSize();
    QPointF previewOrigin();
    QTransform canvasToPreviewTransform();
    QTransform previewToCanvasTransform();
    QPolygonF previewPolygon();

    qreal m_previewScale;
    QPixmap m_oldPixmap;
    QPixmap m_pixmap;
    QPointer<KisCanvas2> m_canvas;

    bool m_dragging;
    QPointF m_lastPos;

    QColor m_outlineColor;
    KisIdleWatcher m_imageIdleWatcher;
    KisStrokeId strokeId;
    QMutex mutex;
};



#endif /* OVERVIEWWIDGET_H */
