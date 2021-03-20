/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCLICKABLEGLIMAGEWIDGET_H
#define KISCLICKABLEGLIMAGEWIDGET_H

#include <KisGLImageWidget.h>
#include <QScopedPointer>


class KisClickableGLImageWidget : public KisGLImageWidget
{
    Q_OBJECT
public:
    struct HandlePaintingStrategy {
        virtual void drawHandle(QPainter *p, const QPointF &normalizedPoint, const QRect &rect, bool useOpacity) = 0;
        virtual ~HandlePaintingStrategy() {}
    };

    struct VerticalLineHandleStrategy : public HandlePaintingStrategy {
        void drawHandle(QPainter *p, const QPointF &normalizedPoint, const QRect &rect, bool useOpacity) override;
    };

    struct CircularHandleStrategy : public HandlePaintingStrategy {
        void drawHandle(QPainter *p, const QPointF &normalizedPoint, const QRect &rect, bool useOpacity) override;
    };

public:
    KisClickableGLImageWidget(QWidget *parent = nullptr);
    KisClickableGLImageWidget(KisSurfaceColorSpace colorSpace,
                              QWidget *parent = nullptr);
    ~KisClickableGLImageWidget();

    void setHandlePaintingStrategy(HandlePaintingStrategy *strategy);
    void setUseHandleOpacity(bool value);

    QPointF normalizedPos() const;
    void setNormalizedPos(const QPointF &pos, bool update = true);

    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void selected(const QPointF &normalizedPos);

private:
    QPointF normalizePoint(const QPointF &pos) const;

private:
    QPointF m_normalizedClickPoint;
    QScopedPointer<HandlePaintingStrategy> m_handleStrategy;
    bool m_useHandleOpacity = true;
};

#endif // KISCLICKABLEGLIMAGEWIDGET_H
