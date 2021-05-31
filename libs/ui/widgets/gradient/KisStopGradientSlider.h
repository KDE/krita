/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2016 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_STOP_GRADIENT_SLIDER_H_
#define _KIS_STOP_GRADIENT_SLIDER_H_

#include <QWidget>
#include <QScopedPointer>

#include <KoStopGradient.h>
#include <kis_signal_compressor.h>

class QEvent;
class QMouseEvent;
class QKeyEvent;

class KisStopGradientSlider : public QWidget
{
    Q_OBJECT

public:
    KisStopGradientSlider(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    int selectedStop();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    void setGradientResource(KoStopGradientSP gradient);
    void setSelectedStop(int selected);
    void selectPreviousStop();
    void selectNextStop();
    void deleteSelectedStop(bool selectNeighborStop = true);
    void chooseSelectedStopColor();

Q_SIGNALS:
    void sigSelectedStop(int stop);
    void updateRequested();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void leaveEvent(QEvent *e) override;

private Q_SLOTS:
    void updateHandleSize();

private:
    void insertStop(double t);

    QRect sliderRect() const;
    QRect gradientStripeRect() const;
    QRect handlesStripeRect() const;
    QRegion allowedClickRegion(int tolerance) const;

    void updateHoveredStop(const QPoint &pos);
    int handleClickTolerance() const;
    void handleIncrementInput(int direction, Qt::KeyboardModifiers modifiers);
    int minimalHeight() const;

private:
    static constexpr int removeStopDistance{32};

    KoStopGradientSP m_defaultGradient;
    KoStopGradientSP m_gradient;
    int m_selectedStop;
    int m_hoveredStop;
    KoGradientStop m_removedStop;
    bool m_drag;
    QSize m_handleSize;
    KisSignalCompressor m_updateCompressor;
};

#endif
