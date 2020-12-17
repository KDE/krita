/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */


#ifndef OVERVIEWWIDGET_H
#define OVERVIEWWIDGET_H
#include <QObject>
#include <QWidget>
#include <QPixmap>
#include <QPointer>

#include <QMutex>
#include "kis_idle_watcher.h"

#include <kis_canvas2.h>

class KisSignalCompressor;
class KoCanvasBase;

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
    void slotThemeChanged();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void recalculatePreviewDimensions();
    ///
    /// \brief isPixelArt checks if the preview is bigger than the image itself
    ///
    /// We assume that if the preview is bigger than the image itself,
    /// (1) the image is very small,
    ///     (might not be true if the docker is huge and the user is on 4k display...)
    /// (2) the image is pixel art, so they would like to see pixels.
    /// In that case we'd like to use a different algorithm (Nearest Neighbour, called "Box")
    ///    in the OverviewThumbnailStrokeStrategy.
    /// \return
    ///
    bool isPixelArt();

    QPointF calculatePreviewOrigin(QSize previewSize);
    QTransform canvasToPreviewTransform();
    QTransform previewToCanvasTransform();
    QPolygonF previewPolygon();

    qreal m_previewScale {1.0};
    QPixmap m_oldPixmap;
    QPixmap m_pixmap;
    QImage m_image;
    QPointer<KisCanvas2> m_canvas;


    QPointF m_previewOrigin; // in the same coordinates space as m_previewSize
    QSize m_previewSize {QSize(100, 100)};


    bool m_dragging {false};
    QPointF m_lastPos {QPointF(0, 0)};

    QColor m_outlineColor;
    KisIdleWatcher m_imageIdleWatcher;
    KisStrokeId strokeId;
    QMutex mutex;
};



#endif /* OVERVIEWWIDGET_H */
