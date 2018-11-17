/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2016 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef _KIS_STOP_GRADIENT_SLIDER_WIDGET_H_
#define _KIS_STOP_GRADIENT_SLIDER_WIDGET_H_

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QScopedPointer>
#include <KoStopGradient.h>
#include <resources/KoStopGradient.h>

class KisStopGradientSliderWidget : public QWidget
{
    Q_OBJECT

public:
    KisStopGradientSliderWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

public:
    void paintEvent(QPaintEvent *) override;
    void setGradientResource(KoStopGradientSP gradient);

    int selectedStop();

    void setSelectedStop(int selected);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
     void sigSelectedStop(int stop);

protected:
    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent * e) override;

private Q_SLOTS:
     void updateHandleSize();

private:
    void insertStop(double t);

    QRect sliderRect() const;
    QRect gradientStripeRect() const;
    QRect handlesStipeRect() const;
    QRegion allowedClickRegion(int tolerance) const;

    void updateCursor(const QPoint &pos);
    void paintHandle(qreal position, const QColor &color, bool isSelected, QPainter *painter);
    int handleClickTolerance() const;
    int minimalHeight() const;

private:
    KoStopGradientSP m_defaultGradient;
    KoStopGradientSP m_gradient;
    int m_selectedStop;
    KoGradientStop m_removedStop;
    bool m_drag;
    QSize m_handleSize;

};

#endif
