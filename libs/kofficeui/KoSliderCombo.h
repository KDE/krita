/* This file is part of the KDE project
   Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSLIDERCOMBO_H_
#define KOSLIDERCOMBO_H_

#include <QComboBox>
#include <QStyleOptionComboBox>

#include <kofficeui_export.h>

/**
 * @short A widget for double values with a popup slider
 *
 * KoSliderCombo combines a QLineEdit and a dropdown QSlider
 * to make an easy to use control for setting a double value.
 *
 */
class KOFFICEUI_EXPORT KoSliderCombo : public QComboBox
{

    Q_OBJECT

public:

    /**
     * Constructor for the widget, where value is set to 0
     *
     * @param parent parent QWidget
     */
    KoSliderCombo(QWidget *parent=0);

    /**
     * Destructor
     */
    virtual ~KoSliderCombo();

    /**
     * The precision of values given as the number of digits after the period.
     * default is 2
     */
    double decimals() const;

    /**
     * The minimum value that can be entered.
     * default is 0
     */
    double minimum() const;

    /**
     * The maximum value that can be entered.
     * default is 100
     */
    double maximum() const;

    /**
     * Sets the precision of the entered values.
     * @param d the number of digits after the period
     */

    void setDecimals(int d);

    /**
     * Sets the minimum value that can be entered.
     * @param min the minimum value
     */
    void setMinimum(double min);

    /**
     * Sets the maximum value that can be entered.
     * @param max the maximum value
     */
    void setMaximum(double max);

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

public slots:

signals:

    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     * @param value the new value
     */
    void valueChanged(double value);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void changeEvent(QEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);

private slots:
    void sliderValueChanged(int value);
    void lineEditFinished( const QString & text);

private:
    QStyleOptionComboBox styleOption() const;
    void showPopup();
    void hidePopup();

    class KoSliderComboPrivate;
    KoSliderComboPrivate *d;
};

#endif
