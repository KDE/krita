/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2014-11-30
 * @brief  Save space slider widget
 *
 * @author Copyright (C) 2014 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010 by Justin Noel
 *         <a href="mailto:justin at ics dot com">justin at ics dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RSLIDERSPINBOX_H
#define RSLIDERSPINBOX_H

// Qt includes

#include <QWidget>
#include <QString>
#include <QRect>
#include <QStyleOptionSpinBox>
#include <QStyleOptionProgressBar>

namespace KDcrawIface
{

class RAbstractSliderSpinBoxPrivate;
class RSliderSpinBoxPrivate;
class RDoubleSliderSpinBoxPrivate;

/**
 * TODO: when inactive, also show the progress bar part as inactive!
 */
class RAbstractSliderSpinBox : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(RAbstractSliderSpinBox)
    Q_DECLARE_PRIVATE(RAbstractSliderSpinBox)

protected:

    explicit RAbstractSliderSpinBox(QWidget* const parent, RAbstractSliderSpinBoxPrivate* const q);

public:

    virtual ~RAbstractSliderSpinBox();

    void showEdit();
    void hideEdit();

    void setSuffix(const QString& suffix);

    void setExponentRatio(double dbl);

protected:

    virtual void paintEvent(QPaintEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void wheelEvent(QWheelEvent *);
    virtual bool eventFilter(QObject* recv, QEvent* e);

    virtual QSize sizeHint()        const;
    virtual QSize minimumSizeHint() const;

    QStyleOptionSpinBox spinBoxOptions()         const;
    QStyleOptionProgressBar progressBarOptions() const;

    QRect editRect(const QStyleOptionSpinBox& spinBoxOptions)   const;
    QRect progressRect(const QStyleOptionProgressBar& progressBarOptions)   const;
    QRect upButtonRect(const QStyleOptionSpinBox& spinBoxOptions)   const;
    QRect downButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const;

    int valueForX(int x, Qt::KeyboardModifiers modifiers = Qt::NoModifier) const;

    virtual QString valueString() const = 0;
    virtual void setInternalValue(int value) = 0;

protected Q_SLOTS:

    void contextMenuEvent(QContextMenuEvent* event);
    void editLostFocus();

protected:

    RAbstractSliderSpinBoxPrivate* const d_ptr;
};

// ---------------------------------------------------------------------------------

class RSliderSpinBox : public RAbstractSliderSpinBox
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RSliderSpinBox)
    Q_PROPERTY( int minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( int maximum READ maximum WRITE setMaximum )

public:

    RSliderSpinBox(QWidget* const parent = 0);
    ~RSliderSpinBox();

    void setRange(int minimum, int maximum);

    int  minimum() const;
    void setMinimum(int minimum);
    int  maximum() const;
    void setMaximum(int maximum);
    int  fastSliderStep() const;
    void setFastSliderStep(int step);

    int  value() const;
    void setValue(int value);

    void setSingleStep(int value);
    void setPageStep(int value);

Q_SIGNALS:

    void valueChanged(int value);

protected:

    virtual QString valueString() const;
    virtual void setInternalValue(int value);
};

// ---------------------------------------------------------------------------------

class RDoubleSliderSpinBox : public RAbstractSliderSpinBox
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RDoubleSliderSpinBox)

public:

    RDoubleSliderSpinBox(QWidget* const parent = 0);
    ~RDoubleSliderSpinBox();

    void setRange(double minimum, double maximum, int decimals = 0);

    double minimum() const;
    void   setMinimum(double minimum);
    double maximum() const;
    void   setMaximum(double maximum);
    double fastSliderStep() const;
    void   setFastSliderStep(double step);

    double value() const;
    void   setValue(double value);

    void   setSingleStep(double value);

Q_SIGNALS:

    void valueChanged(double value);

protected:

    virtual QString valueString() const;
    virtual void setInternalValue(int val);
};

}  // namespace KDcrawIface

#endif // RSLIDERSPINBOX_H
