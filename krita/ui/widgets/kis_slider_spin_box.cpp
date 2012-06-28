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
#include <QDoubleSpinBox>

class KisAbstractSliderSpinBoxPrivate {
public:
    QLineEdit* edit;
    QDoubleValidator* validator;
    bool upButtonDown;
    bool downButtonDown;
    int factor;
    QString suffix;
    qreal exponentRatio;
    int value;
    int maximum;
    int minimum;
    int singleStep;
    QSpinBox* dummySpinBox;
};

KisAbstractSliderSpinBox::KisAbstractSliderSpinBox(QWidget* parent, KisAbstractSliderSpinBoxPrivate* _d) :
        QWidget(parent), d_ptr(_d)
{
    Q_D(KisAbstractSliderSpinBox);
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

    connect(d->edit, SIGNAL(lostFocus()), this, SLOT(editLostFocus()));

    d->validator = new QDoubleValidator(d->edit);
    d->edit->setValidator(d->validator);

    d->value = 0;
    d->minimum = 0;
    d->maximum = 100;
    d->factor = 1.0;
    d->singleStep = 1;

    setExponentRatio(1.0);

    //Set sane defaults
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    //dummy needed to fix a bug in the polyester theme
    d->dummySpinBox = new QSpinBox();
    d->dummySpinBox->hide();
}

KisAbstractSliderSpinBox::~KisAbstractSliderSpinBox()
{
    Q_D(KisAbstractSliderSpinBox);
    delete d;
}

void KisAbstractSliderSpinBox::showEdit()
{
    Q_D(KisAbstractSliderSpinBox);
    if (d->edit->isVisible()) return;
    d->edit->setGeometry(progressRect(spinBoxOptions()));
    d->edit->setText(valueString());
    d->edit->selectAll();
    d->edit->show();
    d->edit->setFocus(Qt::OtherFocusReason);
    update();
}

void KisAbstractSliderSpinBox::hideEdit()
{
    Q_D(KisAbstractSliderSpinBox);
    d->edit->hide();
    update();
}

void KisAbstractSliderSpinBox::paintEvent(QPaintEvent* e)
{
    Q_D(KisAbstractSliderSpinBox);
    Q_UNUSED(e)

    QPainter painter(this);

    //Create options to draw spin box parts
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Draw "SpinBox".Clip off the area of the lineEdit to avoid double
    //borders being drawn
    painter.save();
    painter.setClipping(true);
    QRect eraseRect(QPoint(rect().x(), rect().y()),
                    QPoint(progressRect(spinOpts).right(), rect().bottom()));
    painter.setClipRegion(QRegion(rect()).subtracted(eraseRect));
    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, d->dummySpinBox);
    painter.setClipping(false);
    painter.restore();


    //Create options to draw progress bar parts
    QStyleOptionProgressBar progressOpts = progressBarOptions();

    //Draw "ProgressBar" in SpinBox
    style()->drawControl(QStyle::CE_ProgressBar, &progressOpts, &painter, 0);

    //Draw focus if necessary
    if (hasFocus() &&
            d->edit->hasFocus()) {
        QStyleOptionFocusRect focusOpts;
        focusOpts.initFrom(this);
        focusOpts.rect = progressOpts.rect;
        focusOpts.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpts, &painter, this);
    }

}

void KisAbstractSliderSpinBox::mousePressEvent(QMouseEvent* e)
{
    Q_D(KisAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Depress buttons or highlight slider
    //Also used to emulate mouse grab...
    if (e->buttons() & Qt::LeftButton) {
        if (upButtonRect(spinOpts).contains(e->pos())) {
            d->upButtonDown = true;
        } else if (downButtonRect(spinOpts).contains(e->pos())) {
            d->downButtonDown = true;
        }
    } else if (e->buttons() & Qt::RightButton) {
        showEdit();
    }


    update();
}

void KisAbstractSliderSpinBox::mouseReleaseEvent(QMouseEvent* e)
{
    Q_D(KisAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Step up/down for buttons
    //Emualting mouse grab too
    if (upButtonRect(spinOpts).contains(e->pos()) && d->upButtonDown) {
        setInternalValue(d->value + d->singleStep);
    } else if (downButtonRect(spinOpts).contains(e->pos()) && d->downButtonDown) {
        setInternalValue(d->value - d->singleStep);
    } else if (progressRect(spinOpts).contains(e->pos()) &&
               !(d->edit->isVisible()) &&
               !(d->upButtonDown || d->downButtonDown)) {
        //Snap to percentage for progress area
        setInternalValue(valueForX(e->pos().x()));
    }

    d->upButtonDown = false;
    d->downButtonDown = false;
    update();
}

void KisAbstractSliderSpinBox::mouseMoveEvent(QMouseEvent* e)
{
    Q_D(KisAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    //Respect emulated mouse grab.
    if (e->buttons() & Qt::LeftButton &&
        !(d->downButtonDown || d->upButtonDown)) {
        setInternalValue(valueForX(e->pos().x()));
        update();
    }
}

void KisAbstractSliderSpinBox::mouseDoubleClickEvent(QMouseEvent* e)
{
    Q_UNUSED(e);
}

void KisAbstractSliderSpinBox::keyPressEvent(QKeyEvent* e)
{
    Q_D(KisAbstractSliderSpinBox);
    switch (e->key()) {
    case Qt::Key_Up:
    case Qt::Key_Right:
        setInternalValue(d->value + d->singleStep);
        break;
    case Qt::Key_Down:
    case Qt::Key_Left:
        setInternalValue(d->value - d->singleStep);
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

bool KisAbstractSliderSpinBox::eventFilter(QObject* recv, QEvent* e)
{
    Q_D(KisAbstractSliderSpinBox);
    if (recv == static_cast<QObject*>(d->edit) &&
            e->type() == QEvent::KeyRelease) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

        switch (keyEvent->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            setInternalValue(d->edit->text().toDouble()*d->factor);
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

QSize KisAbstractSliderSpinBox::sizeHint() const
{
    const Q_D(KisAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    QFontMetrics fm(font());
    //We need at least 50 pixels or things start to look bad
    int w = qMax(fm.width(QString::number(d->maximum)), 50);
    QSize hint(w, d->edit->sizeHint().height() + 3);

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

QSize KisAbstractSliderSpinBox::minimumSizeHint() const
{
    return sizeHint();
}

QStyleOptionSpinBox KisAbstractSliderSpinBox::spinBoxOptions() const
{
    const Q_D(KisAbstractSliderSpinBox);
    QStyleOptionSpinBox opts;
    opts.initFrom(this);
    opts.frame = false;
    opts.buttonSymbols = QAbstractSpinBox::UpDownArrows;
    opts.subControls = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;

    //Disable non-logical buttons
    if (d->value == d->minimum) {
        opts.stepEnabled = QAbstractSpinBox::StepUpEnabled;
    } else if (d->value == d->maximum) {
        opts.stepEnabled = QAbstractSpinBox::StepDownEnabled;
    } else {
        opts.stepEnabled = QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
    }

    //Deal with depressed buttons
    if (d->upButtonDown) {
        opts.activeSubControls = QStyle::SC_SpinBoxUp;
    } else if (d->downButtonDown) {
        opts.activeSubControls = QStyle::SC_SpinBoxDown;
    } else {
        opts.activeSubControls = 0;
    }

    return opts;
}

QStyleOptionProgressBar KisAbstractSliderSpinBox::progressBarOptions() const
{
    const Q_D(KisAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Create opts for drawing the progress portion
    QStyleOptionProgressBar progressOpts;
    progressOpts.initFrom(this);
    progressOpts.maximum = d->maximum;
    progressOpts.minimum = d->minimum;

    qreal minDbl = d->minimum;

    qreal dValues = (d->maximum - minDbl);

    progressOpts.progress = dValues * pow((d->value - minDbl) / dValues, 1 / d->exponentRatio) + minDbl;
    progressOpts.text = valueString() + d->suffix;
    progressOpts.textAlignment = Qt::AlignCenter;
    progressOpts.textVisible = !(d->edit->isVisible());

    //Change opts rect to be only the ComboBox's text area
    progressOpts.rect = progressRect(spinOpts);

    return progressOpts;
}

QRect KisAbstractSliderSpinBox::progressRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxEditField);
}

QRect KisAbstractSliderSpinBox::upButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxUp);
}

QRect KisAbstractSliderSpinBox::downButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxDown);
}

int KisAbstractSliderSpinBox::valueForX(int x) const
{
    const Q_D(KisAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    //Adjust for magic number in style code (margins)
    QRect correctedProgRect = progressRect(spinOpts).adjusted(2, 2, -2, -2);

    qreal leftDbl = correctedProgRect.left();
    qreal xDbl = x - leftDbl;
    qreal rightDbl = correctedProgRect.right();
    qreal minDbl = d->minimum;
    qreal maxDbl = d->maximum;

    qreal dValues = (maxDbl - minDbl);
    qreal percent = (xDbl / (rightDbl - leftDbl));

    return ((dValues * pow(percent, d->exponentRatio)) + minDbl);
}

void KisAbstractSliderSpinBox::setSuffix(const QString& suffix)
{
    Q_D(KisAbstractSliderSpinBox);
    d->suffix = suffix;
}

void KisAbstractSliderSpinBox::setExponentRatio(qreal dbl)
{
    Q_D(KisAbstractSliderSpinBox);
    Q_ASSERT(dbl > 0);
    d->exponentRatio = dbl;
}

void KisAbstractSliderSpinBox::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
}

void KisAbstractSliderSpinBox::editLostFocus()
{
    hideEdit();
}

class KisSliderSpinBoxPrivate : public KisAbstractSliderSpinBoxPrivate {
};

KisSliderSpinBox::KisSliderSpinBox(QWidget* parent) : KisAbstractSliderSpinBox(parent, new KisSliderSpinBoxPrivate)
{
  setRange(0,99);
}

KisSliderSpinBox::~KisSliderSpinBox()
{
}

void KisSliderSpinBox::setRange(int minimum, int maximum)
{
    Q_D(KisSliderSpinBox);
    d->minimum = minimum;
    d->maximum = maximum;
    d->validator->setRange(minimum, maximum, 0);
    update();
}

int KisSliderSpinBox::minimum() const
{
    const Q_D(KisSliderSpinBox);
    return d->minimum;
}

void KisSliderSpinBox::setMinimum(int minimum)
{
    Q_D(KisSliderSpinBox);
    setRange(minimum, d->maximum);
}

int KisSliderSpinBox::maximum() const
{
    const Q_D(KisSliderSpinBox);
    return d->maximum;
}

void KisSliderSpinBox::setMaximum(int maximum)
{
    Q_D(KisSliderSpinBox);
    setRange(d->minimum, maximum);
}

int KisSliderSpinBox::value()
{
    Q_D(KisSliderSpinBox);
    return d->value;
}

void KisSliderSpinBox::setValue(int value)
{
    setInternalValue(value);
    update();
}

QString KisSliderSpinBox::valueString() const
{
    const Q_D(KisSliderSpinBox);
    return QString::number(d->value, 'f', d->validator->decimals());
}

void KisSliderSpinBox::setSingleStep(int value)
{
    Q_D(KisSliderSpinBox);
    d->singleStep = value;
}

void KisSliderSpinBox::setPageStep(int value)
{
    Q_UNUSED(value);
}

void KisSliderSpinBox::setInternalValue(int _value)
{
    Q_D(KisAbstractSliderSpinBox);
    d->value = qBound(d->minimum, _value, d->maximum);
    emit(valueChanged(value()));
}

class KisDoubleSliderSpinBoxPrivate : public KisAbstractSliderSpinBoxPrivate {
};

KisDoubleSliderSpinBox::KisDoubleSliderSpinBox(QWidget* parent) : KisAbstractSliderSpinBox(parent, new KisDoubleSliderSpinBoxPrivate)
{
}

KisDoubleSliderSpinBox::~KisDoubleSliderSpinBox()
{
}

void KisDoubleSliderSpinBox::setRange(qreal minimum, qreal maximum, int decimals)
{
    Q_D(KisDoubleSliderSpinBox);
    d->factor = pow((double)10, decimals);

    d->minimum = minimum * d->factor;
    d->maximum = maximum * d->factor;
    d->validator->setRange(minimum, maximum, decimals);
    update();
}

qreal KisDoubleSliderSpinBox::value()
{
    Q_D(KisAbstractSliderSpinBox);
    return (qreal)d->value / d->factor;
}

void KisDoubleSliderSpinBox::setValue(qreal value)
{
    Q_D(KisAbstractSliderSpinBox);
    setInternalValue(d->value = value * d->factor);
    update();
}

void KisDoubleSliderSpinBox::setSingleStep(qreal value)
{
    Q_D(KisAbstractSliderSpinBox);
    d->singleStep = value * d->factor;
}

QString KisDoubleSliderSpinBox::valueString() const
{
    const Q_D(KisAbstractSliderSpinBox);
    return QString::number((qreal)d->value / d->factor, 'f', d->validator->decimals());
}

void KisDoubleSliderSpinBox::setInternalValue(int _value)
{
    Q_D(KisAbstractSliderSpinBox);
    d->value = qBound(d->minimum, _value, d->maximum);
    emit(valueChanged(value()));
}
