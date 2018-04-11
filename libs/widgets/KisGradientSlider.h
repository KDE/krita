/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Frederic Coiffier <fcoiffie@gmail.com>
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
