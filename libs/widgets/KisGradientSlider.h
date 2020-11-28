/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_SLIDER_H
#define KIS_GRADIENT_SLIDER_H

// Qt includes.

#include <QWidget>
#include <QColor>
#include <QList>
#include <QPair>

#include "kritawidgets_export.h"

/**
 * @brief The KisGradientSlider class is a numerical slider that paints a light-dark
 * gradient for use in filters and histograms.
 */
class KRITAWIDGETS_EXPORT KisGradientSlider : public QWidget
{
    Q_OBJECT

    typedef enum {
        BlackCursor,
        GammaCursor,
        WhiteCursor,
        None
    } eCursor;

public:
    KisGradientSlider(QWidget *parent = 0);

    ~KisGradientSlider() override;

    int black() const;
    int white() const;

public Q_SLOTS:
    void slotModifyBlack(int);
    void slotModifyWhite(int);
    void slotModifyGamma(double);

Q_SIGNALS:
    void sigModifiedBlack(int);
    void sigModifiedWhite(int);
    void sigModifiedGamma(double);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent * e) override;

private:
    void calculateCursorPositions();
    unsigned int calculateGammaCursor();

public:
    void enableGamma(bool b);
    double getGamma(void);

    void enableWhite(bool b);

    void setInverted(bool b);

private:
    int m_leftmost;
    int m_rightmost;
    eCursor m_grabCursor;
    double m_scalingFactor;

    int m_blackCursor;
    int m_whiteCursor;
    int m_gammaCursor;

    int m_black;
    int m_white;

    double m_gamma;
    bool m_gammaEnabled;
    bool m_whiteEnabled;
    bool m_feedback;
    bool m_inverted;
};

#endif /* KIS_GRADIENT_SLIDER_H */
