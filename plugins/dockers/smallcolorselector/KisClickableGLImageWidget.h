/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    KisClickableGLImageWidget(QSurfaceFormat::ColorSpace colorSpace,
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
