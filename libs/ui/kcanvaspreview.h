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

    bool xOffsetLocked() const;
    bool yOffsetLocked() const;

public Q_SLOTS:
    void setxOffsetLock(bool);
    void setyOffsetLock(bool);

Q_SIGNALS:
    void sigModifiedXOffset(int);
    void sigModifiedYOffset(int);

protected:
    bool isInRegion(QPoint point);
    double scalingFactor();

protected:
    qint32 m_width {0}, m_height {0};
    qint32 m_imageWidth {0}, m_imageHeight {0};
    qint32 m_xOffset {0}, m_yOffset {0};

    qint16 m_xCanvasOffset {0}, m_yCanvasOffset {0};
    bool m_dragging {false};

    QImage m_image;
    QPoint m_prevDragPoint;

    bool m_xOffsetLocked {false};
    bool m_yOffsetLocked {false};
};


#endif /* KCANVASPREVIEW_H */
