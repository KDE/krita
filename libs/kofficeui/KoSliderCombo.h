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

#include <QWidget>
#include <QStyleOptionComboBox>

#include <kofficeui_export.h>

/**
 * @short A widget for double values with a popup slider
 *
 * KoSliderCombo combines a QLineedit and a dropdown QSlider
 * to make an easy to use control for setting a value.
 *
 */
class KOFFICEUI_EXPORT KoSliderCombo : public QWidget
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
     *
     *
     */
    virtual ~KoSliderCombo();

    double maximum() const;
    double minimum() const;
    double decimals() const;
    void setDecimals(int d);
    void setMinimum(double min);
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
    virtual void resizeEvent(QResizeEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void changeEvent(QEvent *e);
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    virtual bool event(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *e);

private slots:
    void sliderValueChanged(int value);
    void lineEditFinished();

private:
    QStyleOptionComboBox styleOption() const;
    void updateLineEditGeometry();
    void updateArrow(QStyle::StateFlag state);
    void showPopup();
    void hidePopup();

    class KoSliderComboPrivate;
    KoSliderComboPrivate *d;
};

#endif
