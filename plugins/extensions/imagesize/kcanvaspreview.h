/*
 *
 *  SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KCANVASPREVIEW_H
#define KCANVASPREVIEW_H

#include <QWidget>

class KCanvasPreview : public QWidget
{
    Q_OBJECT

public:
    KCanvasPreview(QWidget * parent = 0);

    ~KCanvasPreview() override;

    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void setImageSize(qint32 w, qint32 h);
    void setCanvasSize(qint32 w, qint32 h);
    void setImageOffset(qint32 x, qint32 y);
Q_SIGNALS:
    void sigModifiedXOffset(int);
    void sigModifiedYOffset(int);

protected:
    bool isInRegion(QPoint point);
    double scalingFactor();

protected:
    qint32 m_width, m_height;
    qint32 m_imageWidth, m_imageHeight;
    qint32 m_xOffset, m_yOffset;

    qint16 m_xCanvasOffset, m_yCanvasOffset;
    bool m_dragging;

    QImage m_image;
    QPoint m_prevDragPoint;
};


#endif /* KCANVASPREVIEW_H */
