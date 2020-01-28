/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2014-11-30
 * @brief  Save space slider widget
 *
 * @author Copyright (C) 2014 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010 by Justin Noel
 *         <a href="mailto:justin at ics dot com">justin at ics dot com</a>
 * @author Copyright (C) 2010 by Cyrille Berger
 *         <a href="mailto:cberger at cberger dot net">cberger at cberger dot net</a>
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

#include "rsliderspinbox.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QStyle>
#include <QLineEdit>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QIntValidator>
#include <QTimer>
#include <QtDebug>
#include <QDoubleSpinBox>

namespace KDcrawIface
{

class RAbstractSliderSpinBoxPrivate
{
public:

    RAbstractSliderSpinBoxPrivate()
    {
        edit            = 0;
        validator       = 0;
        dummySpinBox    = 0;
        upButtonDown    = false;
        downButtonDown  = false;
        shiftMode       = false;
        factor          = 1.0;
        fastSliderStep  = 5;
        slowFactor      = 0.1;
        shiftPercent    = 0.0;
        exponentRatio   = 0.0;
        value           = 0;
        maximum         = 100;
        minimum         = 0;
        singleStep      = 1;
    }

    QLineEdit*        edit;
    QDoubleValidator* validator;
    bool              upButtonDown;
    bool              downButtonDown;
    int               factor;
    int               fastSliderStep;
    double            slowFactor;
    double            shiftPercent;
    bool              shiftMode;
    QString           suffix;
    double            exponentRatio;
    int               value;
    int               maximum;
    int               minimum;
    int               singleStep;
    QSpinBox*         dummySpinBox;
};

RAbstractSliderSpinBox::RAbstractSliderSpinBox(QWidget* const parent, RAbstractSliderSpinBoxPrivate* const q)
    : QWidget(parent),
      d_ptr(q)
{
    Q_D(RAbstractSliderSpinBox);

    d->edit = new QLineEdit(this);
    d->edit->setFrame(false);
    d->edit->setAlignment(Qt::AlignCenter);
    d->edit->hide();
    d->edit->installEventFilter(this);

    // Make edit transparent
    d->edit->setAutoFillBackground(false);
    QPalette pal = d->edit->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    d->edit->setPalette(pal);

    connect(d->edit, SIGNAL(editingFinished()),
            this, SLOT(editLostFocus()));

    d->validator = new QDoubleValidator(d->edit);
    d->edit->setValidator(d->validator);

    setExponentRatio(1.0);

    // Set sane defaults
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // dummy needed to fix a bug in the polyester theme
    d->dummySpinBox = new QSpinBox(this);
    d->dummySpinBox->hide();
}

RAbstractSliderSpinBox::~RAbstractSliderSpinBox()
{
    Q_D(RAbstractSliderSpinBox);
    delete d;
}

void RAbstractSliderSpinBox::showEdit()
{
    Q_D(RAbstractSliderSpinBox);

    if (d->edit->isVisible()) return;

    d->edit->setGeometry(editRect(spinBoxOptions()));
    d->edit->setText(valueString());
    d->edit->selectAll();
    d->edit->show();
    d->edit->setFocus(Qt::OtherFocusReason);
    update();
}

void RAbstractSliderSpinBox::hideEdit()
{
    Q_D(RAbstractSliderSpinBox);
    d->edit->hide();
    update();
}

void RAbstractSliderSpinBox::paintEvent(QPaintEvent* e)
{
    Q_D(RAbstractSliderSpinBox);
    Q_UNUSED(e)

    QPainter painter(this);

    // Create options to draw spin box parts
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    // Draw "SpinBox".Clip off the area of the lineEdit to avoid double borders being drawn

    painter.save();
    painter.setClipping(true);
    QRect eraseRect(QPoint(rect().x(), rect().y()),
                    QPoint(editRect(spinOpts).right(), rect().bottom()));
    painter.setClipRegion(QRegion(rect()).subtracted(eraseRect));
    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, d->dummySpinBox);
    painter.setClipping(false);
    painter.restore();


    // Create options to draw progress bar parts
    QStyleOptionProgressBar progressOpts = progressBarOptions();

    // Draw "ProgressBar" in SpinBox
    style()->drawControl(QStyle::CE_ProgressBar, &progressOpts, &painter, 0);

    // Draw focus if necessary
    if (hasFocus() && d->edit->hasFocus())
    {
        QStyleOptionFocusRect focusOpts;
        focusOpts.initFrom(this);
        focusOpts.rect            = progressOpts.rect;
        focusOpts.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpts, &painter, this);
    }
}

void RAbstractSliderSpinBox::mousePressEvent(QMouseEvent* e)
{
    Q_D(RAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    // Depress buttons or highlight slider. Also used to emulate mouse grab.

    if (e->buttons() & Qt::LeftButton)
    {
        if (upButtonRect(spinOpts).contains(e->pos()))
        {
            d->upButtonDown = true;
        }
        else if (downButtonRect(spinOpts).contains(e->pos()))
        {
            d->downButtonDown = true;
        }
    }
    else if (e->buttons() & Qt::RightButton)
    {
        showEdit();
    }

    update();
}

void RAbstractSliderSpinBox::mouseReleaseEvent(QMouseEvent* e)
{
    Q_D(RAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    // Step up/down for buttons. Emulating mouse grab too.

    if (upButtonRect(spinOpts).contains(e->pos()) && d->upButtonDown)
    {
        setInternalValue(d->value + d->singleStep);
    }
    else if (downButtonRect(spinOpts).contains(e->pos()) && d->downButtonDown)
    {
        setInternalValue(d->value - d->singleStep);
    }
    else if (editRect(spinOpts).contains(e->pos()) &&
             !(d->edit->isVisible())                   &&
             !(d->upButtonDown || d->downButtonDown))
    {
        // Snap to percentage for progress area
        setInternalValue(valueForX(e->pos().x(),e->modifiers()));
    }

    d->upButtonDown   = false;
    d->downButtonDown = false;
    update();
}

void RAbstractSliderSpinBox::mouseMoveEvent(QMouseEvent* e)
{
    Q_D(RAbstractSliderSpinBox);

    if( e->modifiers() & Qt::ShiftModifier )
    {
        if( !d->shiftMode )
        {
            d->shiftPercent = pow(double(d->value - d->minimum)/double(d->maximum - d->minimum),
                                  1/double(d->exponentRatio));
            d->shiftMode    = true;
        }
    }
    else
    {
        d->shiftMode = false;
    }

    // Respect emulated mouse grab.
    if (e->buttons() & Qt::LeftButton && !(d->downButtonDown || d->upButtonDown))
    {
        setInternalValue(valueForX(e->pos().x(),e->modifiers()));
        update();
    }
}

void RAbstractSliderSpinBox::keyPressEvent(QKeyEvent* e)
{
    Q_D(RAbstractSliderSpinBox);

    switch (e->key())
    {
        case Qt::Key_Up:
        case Qt::Key_Right:
            setInternalValue(d->value + d->singleStep);
            break;
        case Qt::Key_Down:
        case Qt::Key_Left:
            setInternalValue(d->value - d->singleStep);
            break;
        case Qt::Key_Shift:
            d->shiftPercent = pow( double(d->value - d->minimum)/double(d->maximum - d->minimum), 1/double(d->exponentRatio) );
            d->shiftMode = true;
            break;
        case Qt::Key_Enter: // Line edit isn't "accepting" key strokes...
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_Super_L:
        case Qt::Key_Super_R:
            break;
        default:
            showEdit();
            d->edit->event(e);
            break;
    }
}

void RAbstractSliderSpinBox::wheelEvent(QWheelEvent *e)
{

    Q_D(RAbstractSliderSpinBox);

    int step = d->fastSliderStep;
    if( e->modifiers() & Qt::ShiftModifier )
    {
        step = d->singleStep;
    }

    if ( e->delta() > 0)
    {
        setInternalValue(d->value + step);
    }
    else
    {
        setInternalValue(d->value - step);
    }

    update();
    e->accept();
}

bool RAbstractSliderSpinBox::eventFilter(QObject* recv, QEvent* e)
{
    Q_D(RAbstractSliderSpinBox);

    if (recv == static_cast<QObject*>(d->edit) && e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* const keyEvent = static_cast<QKeyEvent*>(e);

        switch (keyEvent->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                setInternalValue(QLocale::system().toDouble(d->edit->text()) * d->factor);
                hideEdit();
                return true;
            case Qt::Key_Escape:
                hideEdit();
                return true;
            default:
                break;
        }
    }

    return false;
}

QSize RAbstractSliderSpinBox::sizeHint() const
{
    const Q_D(RAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    QFontMetrics fm(font());

    // We need at least 50 pixels or things start to look bad
    int w = qMax(fm.width(QString::number(d->maximum)), 50);
    QSize hint(w, d->edit->sizeHint().height() + 3);

    // Getting the size of the buttons is a pain as the calcs require a rect
    // that is "big enough". We run the calc twice to get the "smallest" buttons
    // This code was inspired by QAbstractSpinBox.

    QSize extra(35, 6);
    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();

    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();
    hint += extra;

    spinOpts.rect = rect();

    return style()->sizeFromContents(QStyle::CT_SpinBox, &spinOpts, hint, 0);
}

QSize RAbstractSliderSpinBox::minimumSizeHint() const
{
    return sizeHint();
}

QStyleOptionSpinBox RAbstractSliderSpinBox::spinBoxOptions() const
{
    const Q_D(RAbstractSliderSpinBox);

    QStyleOptionSpinBox opts;
    opts.initFrom(this);
    opts.frame         = false;
    opts.buttonSymbols = QAbstractSpinBox::UpDownArrows;
    opts.subControls   = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;

    // Disable non-logical buttons
    if (d->value == d->minimum)
    {
        opts.stepEnabled = QAbstractSpinBox::StepUpEnabled;
    }
    else if (d->value == d->maximum)
    {
        opts.stepEnabled = QAbstractSpinBox::StepDownEnabled;
    }
    else
    {
        opts.stepEnabled = QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
    }

    // Deal with depressed buttons
    if (d->upButtonDown)
    {
        opts.activeSubControls = QStyle::SC_SpinBoxUp;
    }
    else if (d->downButtonDown)
    {
        opts.activeSubControls = QStyle::SC_SpinBoxDown;
    }
    else
    {
        opts.activeSubControls = 0;
    }

    return opts;
}

QStyleOptionProgressBar RAbstractSliderSpinBox::progressBarOptions() const
{
    const Q_D(RAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    // Create opts for drawing the progress portion

    QStyleOptionProgressBar progressOpts;
    progressOpts.initFrom(this);
    progressOpts.maximum       = d->maximum;
    progressOpts.minimum       = d->minimum;

    double minDbl              = d->minimum;
    double dValues             = (d->maximum - minDbl);

    progressOpts.progress      = dValues * pow((d->value - minDbl) / dValues, 1.0 / d->exponentRatio) + minDbl;
    progressOpts.text          = valueString() + d->suffix;
    progressOpts.textAlignment = Qt::AlignCenter;
    progressOpts.textVisible   = !(d->edit->isVisible());

    // Change opts rect to be only the ComboBox's text area
    progressOpts.rect          = editRect(spinOpts);

    return progressOpts;
}

QRect RAbstractSliderSpinBox::editRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxEditField);
}

QRect RAbstractSliderSpinBox::progressRect(const QStyleOptionProgressBar& progressBarOptions) const
{
    return style()->subElementRect(QStyle::SE_ProgressBarGroove, &progressBarOptions);
}

QRect RAbstractSliderSpinBox::upButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxUp);
}

QRect RAbstractSliderSpinBox::downButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxDown);
}

int RAbstractSliderSpinBox::valueForX(int x, Qt::KeyboardModifiers modifiers) const
{
    const Q_D(RAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    QStyleOptionProgressBar progressOpts = progressBarOptions();

    // Adjust for magic number in style code (margins)
    QRect correctedProgRect = progressRect(progressOpts).adjusted(2, 2, -2, -2);

    // Compute the distance of the progress bar, in pixel
    double leftDbl  = correctedProgRect.left();
    double xDbl     = x - leftDbl;

    // Compute the ration of the progress bar used, linearly (ignoring the exponent)
    double rightDbl = correctedProgRect.right();
    double minDbl   = d->minimum;
    double maxDbl   = d->maximum;
    double dValues  = (maxDbl - minDbl);
    double percent  = (xDbl / (rightDbl - leftDbl));

    // If SHIFT is pressed, movement should be slowed.
    if ( modifiers & Qt::ShiftModifier )
    {
        percent = d->shiftPercent + (percent - d->shiftPercent) * d->slowFactor;
    }

    // Final value
    double realvalue = ((dValues * pow(percent, d->exponentRatio)) + minDbl);

    // If key CTRL is pressed, round to the closest step.

    if ( modifiers & Qt::ControlModifier )
    {
        double fstep = d->fastSliderStep;

        if( modifiers & Qt::ShiftModifier )
        {
            fstep *= d->slowFactor;
        }

        realvalue = floor((realvalue + fstep / 2) / fstep) * fstep;
    }

    // Return the value
    return int(realvalue);
}

void RAbstractSliderSpinBox::setSuffix(const QString& suffix)
{
    Q_D(RAbstractSliderSpinBox);
    d->suffix = suffix;
}

void RAbstractSliderSpinBox::setExponentRatio(double dbl)
{
    Q_D(RAbstractSliderSpinBox);
    Q_ASSERT(dbl > 0);
    d->exponentRatio = dbl;
}

void RAbstractSliderSpinBox::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
}

void RAbstractSliderSpinBox::editLostFocus()
{
    // only hide on focus lost, if editing is finished that will be handled in eventFilter
    Q_D(RAbstractSliderSpinBox);

    if (!d->edit->hasFocus())
    {
        hideEdit();
    }
}

// ---------------------------------------------------------------------------------------------

class RSliderSpinBoxPrivate : public RAbstractSliderSpinBoxPrivate
{
};

RSliderSpinBox::RSliderSpinBox(QWidget* const parent)
    : RAbstractSliderSpinBox(parent, new RSliderSpinBoxPrivate)
{
    setRange(0,99);
}

RSliderSpinBox::~RSliderSpinBox()
{
}

void RSliderSpinBox::setRange(int minimum, int maximum)
{
    Q_D(RSliderSpinBox);
    d->minimum        = minimum;
    d->maximum        = maximum;
    d->fastSliderStep = (maximum-minimum+1)/20;
    d->validator->setRange(minimum, maximum, 0);
    update();
}

int RSliderSpinBox::minimum() const
{
    const Q_D(RSliderSpinBox);
    return d->minimum;
}

void RSliderSpinBox::setMinimum(int minimum)
{
    Q_D(RSliderSpinBox);
    setRange(minimum, d->maximum);
}

int RSliderSpinBox::maximum() const
{
    const Q_D(RSliderSpinBox);
    return d->maximum;
}

void RSliderSpinBox::setMaximum(int maximum)
{
    Q_D(RSliderSpinBox);
    setRange(d->minimum, maximum);
}

int RSliderSpinBox::fastSliderStep() const
{
    const Q_D(RSliderSpinBox);
    return d->fastSliderStep;
}

void RSliderSpinBox::setFastSliderStep(int step)
{
    Q_D(RSliderSpinBox);
    d->fastSliderStep = step;
}

int RSliderSpinBox::value() const
{
    const Q_D(RSliderSpinBox);
    return d->value;
}

void RSliderSpinBox::setValue(int value)
{
    setInternalValue(value);
    update();
}

QString RSliderSpinBox::valueString() const
{
    const Q_D(RSliderSpinBox);
    return QLocale::system().toString(d->value);
}

void RSliderSpinBox::setSingleStep(int value)
{
    Q_D(RSliderSpinBox);
    d->singleStep = value;
}

void RSliderSpinBox::setPageStep(int value)
{
    Q_UNUSED(value);
}

void RSliderSpinBox::setInternalValue(int _value)
{
    Q_D(RAbstractSliderSpinBox);
    d->value = qBound(d->minimum, _value, d->maximum);
    emit(valueChanged(value()));
}

// ---------------------------------------------------------------------------------------------

class RDoubleSliderSpinBoxPrivate : public RAbstractSliderSpinBoxPrivate
{
};

RDoubleSliderSpinBox::RDoubleSliderSpinBox(QWidget* const parent)
    : RAbstractSliderSpinBox(parent, new RDoubleSliderSpinBoxPrivate)
{
}

RDoubleSliderSpinBox::~RDoubleSliderSpinBox()
{
}

void RDoubleSliderSpinBox::setRange(double minimum, double maximum, int decimals)
{
    Q_D(RDoubleSliderSpinBox);
    d->factor = pow(10.0, decimals);

    d->minimum = minimum * d->factor;
    d->maximum = maximum * d->factor;

    // This code auto-compute a new step when pressing control.
    // A flag defaulting to "do not change the fast step" should be added, but it implies changing every call

    if (maximum - minimum >= 2.0 || decimals <= 0)
    {
        //Quick step on integers
        d->fastSliderStep = int(pow(10.0, decimals));
    }
    else if(decimals == 1)
    {
        d->fastSliderStep = (maximum-minimum)*d->factor/10;
    }
    else
    {
        d->fastSliderStep = (maximum-minimum)*d->factor/20;
    }

    d->validator->setRange(minimum, maximum, decimals);
    update();
    setValue(value());
}

double RDoubleSliderSpinBox::minimum() const
{
    const Q_D(RAbstractSliderSpinBox);
    return d->minimum / d->factor;
}

void RDoubleSliderSpinBox::setMinimum(double minimum)
{
    Q_D(RAbstractSliderSpinBox);
    setRange(minimum, d->maximum);
}

double RDoubleSliderSpinBox::maximum() const
{
    const Q_D(RAbstractSliderSpinBox);
    return d->maximum / d->factor;
}

void RDoubleSliderSpinBox::setMaximum(double maximum)
{
    Q_D(RAbstractSliderSpinBox);
    setRange(d->minimum, maximum);
}

double RDoubleSliderSpinBox::fastSliderStep() const
{
    const Q_D(RAbstractSliderSpinBox);
    return d->fastSliderStep;
}

void RDoubleSliderSpinBox::setFastSliderStep(double step)
{
    Q_D(RAbstractSliderSpinBox);
    d->fastSliderStep = step * d->factor;
}

double RDoubleSliderSpinBox::value() const
{
    const Q_D(RAbstractSliderSpinBox);
    return (double)d->value / d->factor;
}

void RDoubleSliderSpinBox::setValue(double value)
{
    Q_D(RAbstractSliderSpinBox);
    setInternalValue(d->value = qRound(value * d->factor));
    update();
}

void RDoubleSliderSpinBox::setSingleStep(double value)
{
    Q_D(RAbstractSliderSpinBox);
    d->singleStep = value * d->factor;
}

QString RDoubleSliderSpinBox::valueString() const
{
    const Q_D(RAbstractSliderSpinBox);
    return QLocale::system().toString((double)d->value / d->factor, 'f', d->validator->decimals());
}

void RDoubleSliderSpinBox::setInternalValue(int val)
{
    Q_D(RAbstractSliderSpinBox);
    d->value = qBound(d->minimum, val, d->maximum);
    emit(valueChanged(value()));
}

}  // namespace KDcrawIface
