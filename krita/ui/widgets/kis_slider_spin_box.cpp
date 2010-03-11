/* This file is part of the KDE project
 * Copyright (c) 2010 Justin Noel <justin@ics.com>
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_slider_spin_box.h"

#include <math.h>
#include <kdebug.h>

#include <QPainter>
#include <QStyle>
#include <QLineEdit>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QIntValidator>
#include <QTimer>
#include <QtDebug>

struct KisSliderSpinBox::Private {
    QLineEdit* edit;
    QDoubleValidator* validator;
    bool upButtonDown;
    bool downButtonDown;
    int factor;
    QString suffix;
    qreal exponentRatio;
};

KisSliderSpinBox::KisSliderSpinBox(QWidget* parent) :
        QAbstractSlider(parent), d(new Private)
{
    d->upButtonDown = false;
    d->downButtonDown = false;
    d->edit = new QLineEdit(this);
    d->edit->setFrame(false);
    d->edit->setAlignment(Qt::AlignCenter);
    d->edit->hide();
    d->edit->installEventFilter(this);

    //Make edit transparent
    d->edit->setAutoFillBackground(false);
    QPalette pal = d->edit->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    d->edit->setPalette(pal);

    d->validator = new QDoubleValidator(d->edit);
    d->edit->setValidator(d->validator);
    setRange(0.0, 100.0, 0);
    setExponentRatio(1.0);
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(internalValueChanged(int)));

    //Set sane defaults
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

KisSliderSpinBox::~KisSliderSpinBox()
{
    delete d;
}

void KisSliderSpinBox::showEdit()
{
    if (d->edit->isVisible()) return;
    d->edit->setGeometry(progressRect(spinBoxOptions()));
    d->edit->setText(valueString());
    d->edit->selectAll();
    d->edit->show();
    d->edit->setFocus(Qt::OtherFocusReason);
    update();
}

void KisSliderSpinBox::hideEdit()
{
    d->edit->hide();
    update();
}

void KisSliderSpinBox::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e)

    QPainter painter(this);

    //Create options to draw spin box parts
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Draw "SpinBox".Clip off the area of the lineEdit to avoid qreal
    //borders being drawn
    painter.setClipping(true);
    QRect eraseRect(QPoint(rect().x(), rect().y()),
                    QPoint(progressRect(spinOpts).right(), rect().bottom()));
    painter.setClipRegion(QRegion(rect()).subtracted(eraseRect));
    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, 0);
    painter.setClipping(false);


    //Create options to draw progress bar parts
    QStyleOptionProgressBar progressOpts = progressBarOptions();

    //Draw "ProgressBar" in SpinBox
    style()->drawControl(QStyle::CE_ProgressBar, &progressOpts, &painter, 0);

    //Draw focus if necessary
    if(hasFocus() &&
       d->edit->hasFocus())
    {
        QStyleOptionFocusRect focusOpts;
        focusOpts.initFrom(this);
        focusOpts.rect = progressOpts.rect;
        focusOpts.backgroundColor = palette().color(QPalette::Background);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpts, &painter, this);
    }

}

void KisSliderSpinBox::mousePressEvent(QMouseEvent* e)
{
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Depress buttons or highlight slider
    //Also used to emulate mouse grab...
    if(e->buttons() & Qt::LeftButton) {
        if(upButtonRect(spinOpts).contains(e->pos()))
        {
            d->upButtonDown = true;
        }
        else if(downButtonRect(spinOpts).contains(e->pos()))
        {
            d->downButtonDown = true;
        }
    } else if(e->buttons() & Qt::RightButton){
        showEdit();
    }


    update();
}

void KisSliderSpinBox::mouseReleaseEvent(QMouseEvent* e)
{
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Step up/down for buttons
    //Emualting mouse grab too
    if(upButtonRect(spinOpts).contains(e->pos()) && d->upButtonDown)
    {
        setValue(value() + singleStep());
    }
    else if(downButtonRect(spinOpts).contains(e->pos()) && d->downButtonDown)
    {
        setValue(value() - singleStep());
    }
    else if( progressRect(spinOpts).contains(e->pos()) &&
             !(d->edit->isVisible()) &&
             !(d->upButtonDown || d->downButtonDown) )
    {
        //Snap to percentage for progress area
        setValue(valueForX(e->pos().x()));
    }

    d->upButtonDown = false;
    d->downButtonDown = false;
    update();
}

void KisSliderSpinBox::mouseMoveEvent(QMouseEvent* e)
{
    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    //Respect emulated mouse grab.
    if(progressRect(spinOpts).contains(e->pos()) &&
       e->buttons() & Qt::LeftButton &&
       !(d->downButtonDown || d->upButtonDown))
    {
        setValue(valueForX(e->pos().x()));
    }
}

void KisSliderSpinBox::mouseDoubleClickEvent(QMouseEvent* e)
{
}

void KisSliderSpinBox::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Right:
        setValue(value() + singleStep());
        break;
    case Qt::Key_Down:
    case Qt::Key_Left:
        setValue(value() - singleStep());
        break;
    case Qt::Key_Enter: //Line edit isn't "accepting" key strokes..
    case Qt::Key_Return:
    case Qt::Key_Escape:
        break;
    default:
        showEdit();
        d->edit->event(e);
        break;
    }
}

bool KisSliderSpinBox::eventFilter(QObject* recv, QEvent* e)
{
    if(recv == static_cast<QObject*>(d->edit) &&
       e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

        switch(keyEvent->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            setValue(d->edit->text().toDouble()*d->factor);
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

QSize KisSliderSpinBox::sizeHint() const
{
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    QFontMetrics fm(font());
    //We need at least 50 pixels or things start to look bad
    int w = qMax(fm.width(QString::number(maximum())), 50);
    QSize hint(w, d->edit->sizeHint().height());

    //Getting the size of the buttons is a pain as the calcs require a rect
    //that is "big enough". We run the calc twice to get the "smallest" buttons
    //This code was inspired by QAbstractSpinBox
    QSize extra(35, 6);
    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();

    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();
    hint += extra;

    spinOpts.rect = rect();
    return style()->sizeFromContents(QStyle::CT_SpinBox, &spinOpts, hint, 0)
            .expandedTo(QApplication::globalStrut());

}

QSize KisSliderSpinBox::minimumSizeHint() const
{
    return sizeHint();
}

QStyleOptionSpinBox KisSliderSpinBox::spinBoxOptions() const
{
    QStyleOptionSpinBox opts;
    opts.initFrom(this);
    opts.frame = false;
    opts.buttonSymbols = QAbstractSpinBox::UpDownArrows;
    opts.subControls = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;

    //Disable non-logical buttons
    if(value() == minimum())
    {
        opts.stepEnabled = QAbstractSpinBox::StepUpEnabled;
    }
    else if(value() == maximum())
    {
        opts.stepEnabled = QAbstractSpinBox::StepDownEnabled;
    }
    else
    {
        opts.stepEnabled = QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
    }

    //Deal with depressed buttons
    if(d->upButtonDown)
    {
        opts.activeSubControls = QStyle::SC_SpinBoxUp;
    }
    else if(d->downButtonDown)
    {
        opts.activeSubControls = QStyle::SC_SpinBoxDown;
    }
    else
    {
        opts.activeSubControls = 0;
    }

    return opts;
}

QStyleOptionProgressBar KisSliderSpinBox::progressBarOptions() const
{
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Create opts for drawing the progress portion
    QStyleOptionProgressBar progressOpts;
    progressOpts.initFrom(this);
    progressOpts.maximum = maximum();
    progressOpts.minimum = minimum();

    qreal minDbl = minimum();

    qreal dValues = (maximum() - minDbl);

    progressOpts.progress = dValues * pow((value() - minDbl) / dValues, 1 / d->exponentRatio) + minDbl;
    progressOpts.text = valueString() + d->suffix;
    progressOpts.textAlignment = Qt::AlignCenter;
    progressOpts.textVisible = !(d->edit->isVisible());

    //Change opts rect to be only the ComboBox's text area
    progressOpts.rect = progressRect(spinOpts);

    return progressOpts;
}

QRect KisSliderSpinBox::progressRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxEditField);
}

QRect KisSliderSpinBox::upButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxUp);
}

QRect KisSliderSpinBox::downButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxDown);
}

int KisSliderSpinBox::valueForX(int x) const
{
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Adjust for magic number in style code (margins)
    QRect correctedProgRect = progressRect(spinOpts).adjusted(2,2,-2,-2);

    qreal leftDbl = correctedProgRect.left();
    qreal xDbl = x - leftDbl;
    qreal rightDbl = correctedProgRect.right();
    qreal minDbl = minimum();
    qreal maxDbl = maximum();

    qreal dValues = (maxDbl - minDbl);
    qreal percent = (xDbl / (rightDbl-leftDbl));

    return ((dValues * pow(percent, d->exponentRatio)) + minDbl);
}

void KisSliderSpinBox::setRange(qreal minimum, qreal maximum, int decimals)
{
    d->validator->setDecimals(decimals);
    d->factor = pow(10,decimals);

    setMinimum(minimum*d->factor);
    setMaximum(maximum*d->factor);
    d->validator->setRange(minimum, maximum);
}

void KisSliderSpinBox::setSuffix(const QString& suffix)
{
    d->suffix = suffix;
}

qreal KisSliderSpinBox::doubleValue()
{
    return (qreal)value()/d->factor;
}

void KisSliderSpinBox::setDoubleValue(qreal value)
{
    setValue(value*d->factor);
    update();
}

QString KisSliderSpinBox::valueString() const
{
    return QString::number((qreal)value()/d->factor, 'f', 2);
}

void KisSliderSpinBox::setExponentRatio(qreal dbl)
{
    Q_ASSERT(dbl > 0);
    d->exponentRatio = dbl;
}

void KisSliderSpinBox::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
}

void KisSliderSpinBox::internalValueChanged(int value)
{
    emit doubleValueChanged(value/d->factor);
}
