/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2016 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_STOP_GRADIENT_SLIDER_H_
#define _KIS_STOP_GRADIENT_SLIDER_H_

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QScopedPointer>
#include <KoStopGradient.h>
#include <resources/KoStopGradient.h>

class KisStopGradientSlider : public QWidget
{
    Q_OBJECT

public:
    KisStopGradientSlider(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

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
    QRect handlesStripeRect() const;
    QRegion allowedClickRegion(int tolerance) const;

    void updateCursor(const QPoint &pos);
    void paintHandle(qreal position, const QColor &color, bool isSelected, QString text, QPainter *painter);
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
