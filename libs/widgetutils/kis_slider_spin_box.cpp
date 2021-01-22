/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Justin Noel <justin@ics.com>
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2015 Moritz Molch <kde@moritzmolch.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_slider_spin_box.h"

#include <math.h>

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
#include "kis_cursor.h"

#include "kis_num_parser.h"

class KisAbstractSliderSpinBoxPrivate {
public:
    enum Style {
        STYLE_NOQUIRK,
        STYLE_BREEZE,
        STYLE_FUSION,
    };

    QLineEdit* edit;
    QDoubleValidator* validator;
    bool upButtonDown;
    bool downButtonDown;
    int factor;
    int fastSliderStep;
    qreal slowFactor;
    qreal shiftPercent;
    bool shiftMode;
    QString prefix;
    QString suffix;
    qreal exponentRatio;
    int value;
    int maximum;
    int minimum;
    int singleStep;
    QSpinBox* dummySpinBox;
    Style style;
    bool blockUpdateSignalOnDrag;
    bool isDragging;
    bool parseInt;
};

KisAbstractSliderSpinBox::KisAbstractSliderSpinBox(QWidget* parent, KisAbstractSliderSpinBoxPrivate* _d)
    : QWidget(parent)
    , d_ptr(_d)
{
    Q_D(KisAbstractSliderSpinBox);
    QEvent e(QEvent::StyleChange);
    changeEvent(&e);

    d->upButtonDown = false;
    d->downButtonDown = false;
    d->edit = new QLineEdit(this);
    d->edit->setFrame(false);
    d->edit->setAlignment(Qt::AlignCenter);
    d->edit->hide();
    d->edit->setContentsMargins(0,0,0,0);
    d->edit->installEventFilter(this);

    //Make edit transparent
    d->edit->setAutoFillBackground(false);
    QPalette pal = d->edit->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    d->edit->setPalette(pal);
    d->edit->setContextMenuPolicy(Qt::PreventContextMenu);
    connect(d->edit, SIGNAL(editingFinished()), this, SLOT(editLostFocus()));

    d->validator = new QDoubleValidator(d->edit);

    d->value = 0;
    d->minimum = 0;
    d->maximum = 100;
    d->factor = 1.0;
    d->singleStep = 1;
    d->fastSliderStep = 5;
    d->slowFactor = 0.1;
    d->shiftMode = false;
    d->blockUpdateSignalOnDrag = false;
    d->isDragging = false;

    d->parseInt = false;

    setExponentRatio(1.0);
    setMouseTracking(true);

    //Set sane defaults
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    //dummy needed to fix a bug in the polyester theme
    d->dummySpinBox = new QSpinBox(this);
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
    Q_UNUSED(e);

    QPainter painter(this);
    paintSlider(painter);
    painter.end();
}

void KisAbstractSliderSpinBox::paint(QPainter &painter)
{
    Q_D(KisAbstractSliderSpinBox);

    //Create options to draw spin box parts
    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    spinOpts.rect.adjust(0, 2, 0, -2);

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


    QStyleOptionProgressBar progressOpts = progressBarOptions();
    progressOpts.rect.adjust(0, 2, 0, -2);
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

void KisAbstractSliderSpinBox::paintSlider(QPainter &painter)
{
    Q_D(KisAbstractSliderSpinBox);

    // paint up/down increment arrows
    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    spinOpts.frame = true;
    spinOpts.rect.adjust(0, -1, 0, 1);
    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, d->dummySpinBox);

    painter.save();

    // draw the colored slider bar that is really a progress indicator
    QStyleOptionProgressBar progressOpts = progressBarOptions();
    QRect rect = progressOpts.rect.adjusted(2,2,-2,-2); // this helps when at 100%, it doesn't clear the progress
    QRect progressBarRect;


    // This displays a background color/container, or "groove", for the progress bar
    style()->drawControl(QStyle::CE_ProgressBarGroove, &progressOpts, &painter, this);


    int progressIndicatorPos = (progressOpts.progress - qreal(progressOpts.minimum)) / qMax(qreal(1.0),
                               qreal(progressOpts.maximum) - progressOpts.minimum) * rect.width();

    if (progressIndicatorPos >= 0 && progressIndicatorPos <= rect.width()) {
        progressBarRect = QRect(rect.left(), rect.top(), progressIndicatorPos, rect.height());
    } else if (progressIndicatorPos > rect.width()) {
        painter.setPen(palette().highlightedText().color());
    } else {
        painter.setPen(palette().buttonText().color());
    }

    bool isNotEditingValues = !(d->edit && d->edit->isVisible());
    QRegion rightRect = rect;
    rightRect = rightRect.subtracted(progressBarRect);

    QTextOption textOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter);
    textOption.setWrapMode(QTextOption::NoWrap);

    // setup is done... now determine what things we actually need to paint
    if (isNotEditingValues) {
        painter.setClipRegion(rightRect);
        painter.setClipping(true);
        painter.drawText(rect.adjusted(-2,0,2,0), progressOpts.text, textOption);
        painter.setClipping(false);
    }

    if (!progressBarRect.isNull()) {
        painter.setClipRect(progressBarRect.adjusted(0, -1, 1, 1));
        painter.setPen(palette().highlight().color());
        painter.setBrush(palette().highlight());

        spinOpts.palette.setBrush(QPalette::Base, palette().highlight());
        style()->drawControl(QStyle::CE_ProgressBarContents, &progressOpts, &painter, this);

        if (isNotEditingValues) {
            painter.setPen(palette().highlightedText().color());
            painter.setClipping(true);
            painter.drawText(rect.adjusted(-2,0,2,0), progressOpts.text, textOption);
        }
        painter.setClipping(false);
    }

    painter.restore();
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

    d->isDragging = false;

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
        setInternalValue(valueForX(e->pos().x(),e->modifiers()));
    } else { // Confirm the last known value, since we might be ignoring move events
        setInternalValue(d->value);
    }
    d->upButtonDown = false;
    d->downButtonDown = false;
    update();
}

void KisAbstractSliderSpinBox::mouseMoveEvent(QMouseEvent* e)
{
    Q_D(KisAbstractSliderSpinBox);

    if( e->modifiers() & Qt::ShiftModifier ) {
        if( !d->shiftMode ) {
            d->shiftPercent = pow( qreal(d->value - d->minimum)/qreal(d->maximum - d->minimum), 1/qreal(d->exponentRatio) );
            d->shiftMode = true;
        }
    } else {
        d->shiftMode = false;
    }


    // change cursor shape if we are sliding left and right, or stepping and and down
    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    QPoint localMousePosition = e->localPos().toPoint();
    if (upButtonRect(spinOpts).contains(localMousePosition) ||
        downButtonRect(spinOpts).contains(localMousePosition)) {
        setCursor(KisCursor::arrowCursor());
    } else {
        setCursor(KisCursor::splitHCursor());
    }


    //Respect emulated mouse grab.
    if (e->buttons() & Qt::LeftButton &&
            !(d->downButtonDown || d->upButtonDown)) {
        d->isDragging = true;
        setInternalValue(valueForX(e->pos().x(),e->modifiers()), d->blockUpdateSignalOnDrag);
        update();
    }
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
    case Qt::Key_Shift:
        d->shiftPercent = pow( qreal(d->value - d->minimum)/qreal(d->maximum - d->minimum), 1/qreal(d->exponentRatio) );
        d->shiftMode = true;
        break;
    case Qt::Key_Enter: //Line edit isn't "accepting" key strokes...
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

void KisAbstractSliderSpinBox::wheelEvent(QWheelEvent *e)
{

    Q_D(KisAbstractSliderSpinBox);
    if ( e->delta() > 0) {
        setInternalValue(d->value + d->singleStep);
    } else {
        setInternalValue(d->value - d->singleStep);
    }
    update();
    e->accept();
}

bool KisAbstractSliderSpinBox::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride){
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if (key->modifiers() == Qt::NoModifier){
            switch(key->key()){
            case Qt::Key_Up:
            case Qt::Key_Right:
            case Qt::Key_Down:
            case Qt::Key_Left:
                event->accept();
                return true;
            default: break;
            }
        }
    }
    return QWidget::event(event);
}

void KisAbstractSliderSpinBox::commitEnteredValue()
{
    Q_D(KisAbstractSliderSpinBox);

    //QLocale locale;
    bool ok = false;

    //qreal value = locale.toDouble(d->edit->text(), &ok) * d->factor;
    qreal value;

    if (d->parseInt) {
        value = KisNumericParser::parseIntegerMathExpr(d->edit->text(), &ok) * d->factor;
    } else {
        value = KisNumericParser::parseSimpleMathExpr(d->edit->text(), &ok) * d->factor;
    }

    if (ok) {
        setInternalValue(value);
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
        case Qt::Key_Return: {
            commitEnteredValue();
            hideEdit();
            return true;
        }
        case Qt::Key_Escape:
            d->edit->setText(valueString());
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

    QFont ft(font());
    if (d->style == KisAbstractSliderSpinBoxPrivate::STYLE_NOQUIRK) {
        // Some styles use bold font in progressbars
        // unfortunately there is no reliable way to check for that
        ft.setBold(true);
    }

    QFontMetrics fm(ft);
    QSize hint(fm.boundingRect(d->prefix + QString::number(d->maximum) + d->suffix).size());
    hint += QSize(0, 2);

    switch (d->style) {
    case KisAbstractSliderSpinBoxPrivate::STYLE_FUSION:
        hint += QSize(8, 8);
        break;
    case KisAbstractSliderSpinBoxPrivate::STYLE_BREEZE:
        hint += QSize(2, 0);
        break;
    case KisAbstractSliderSpinBoxPrivate::STYLE_NOQUIRK:
        // almost all "modern" styles have a margin around controls
        hint += QSize(6, 6);
        break;
    default:
        break;
    }

    //Getting the size of the buttons is a pain as the calcs require a rect
    //that is "big enough". We run the calc twice to get the "smallest" buttons
    //This code was inspired by QAbstractSpinBox
    QSize extra(1000, 0);
    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();
    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();
    hint += extra;


    spinOpts.rect.setSize(hint);
    return style()->sizeFromContents(QStyle::CT_SpinBox, &spinOpts, hint)
            .expandedTo(QApplication::globalStrut());

}

QSize KisAbstractSliderSpinBox::minimumSizeHint() const
{
    return sizeHint();
}

QSize KisAbstractSliderSpinBox::minimumSize() const
{
    return QWidget::minimumSize();
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

    progressOpts.progress = dValues * pow((d->value - minDbl) / dValues, 1.0 / d->exponentRatio) + minDbl;
    progressOpts.text = d->prefix + valueString() + d->suffix;
    progressOpts.textAlignment = Qt::AlignCenter;
    progressOpts.textVisible = !(d->edit->isVisible());

    //Change opts rect to be only the ComboBox's text area
    progressOpts.rect = progressRect(spinOpts);

    return progressOpts;
}

QRect KisAbstractSliderSpinBox::progressRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    const Q_D(KisAbstractSliderSpinBox);
    QRect ret = style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                        QStyle::SC_SpinBoxEditField);

    return ret;
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

int KisAbstractSliderSpinBox::valueForX(int x, Qt::KeyboardModifiers modifiers) const
{
    const Q_D(KisAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    QRect correctedProgRect;
    if (d->style == KisAbstractSliderSpinBoxPrivate::STYLE_FUSION) {
        correctedProgRect = progressRect(spinOpts).adjusted(2, 0, -2, 0);
    }
    else if (d->style == KisAbstractSliderSpinBoxPrivate::STYLE_BREEZE) {
        correctedProgRect = progressRect(spinOpts);
    }
    else {
        //Adjust for magic number in style code (margins)
        correctedProgRect = progressRect(spinOpts).adjusted(2, 2, -2, -2);
    }

    //Compute the distance of the progress bar, in pixel
    qreal leftDbl = correctedProgRect.left();
    qreal xDbl = x - leftDbl;

    //Compute the ration of the progress bar used, linearly (ignoring the exponent)
    qreal rightDbl = correctedProgRect.right();
    qreal minDbl = d->minimum;
    qreal maxDbl = d->maximum;

    qreal dValues = (maxDbl - minDbl);
    qreal percent = (xDbl / (rightDbl - leftDbl));

    //If SHIFT is pressed, movement should be slowed.
    if( modifiers & Qt::ShiftModifier ) {
        percent = d->shiftPercent + ( percent - d->shiftPercent ) * d->slowFactor;
    }

    //Final value
    qreal exp_percent = pow(percent, d->exponentRatio);
    qreal realvalue = ((dValues * (percent * exp_percent >= 0 ? exp_percent : -exp_percent)) + minDbl);
    //If key CTRL is pressed, round to the closest step.
    if( modifiers & Qt::ControlModifier ) {
        qreal fstep = d->fastSliderStep;
        if( modifiers & Qt::ShiftModifier ) {
            fstep*=d->slowFactor;
        }
        realvalue = floor( (realvalue+fstep/2) / fstep ) * fstep;
    }
    //Return the value
    return int(realvalue);
}

void KisAbstractSliderSpinBox::setPrefix(const QString& prefix)
{
    Q_D(KisAbstractSliderSpinBox);
    d->prefix = prefix;
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

void KisAbstractSliderSpinBox::setBlockUpdateSignalOnDrag(bool blockUpdateSignal)
{
    Q_D(KisAbstractSliderSpinBox);
    d->blockUpdateSignalOnDrag = blockUpdateSignal;
}

void KisAbstractSliderSpinBox::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
}

void KisAbstractSliderSpinBox::editLostFocus()
{
    Q_D(KisAbstractSliderSpinBox);
    if (!d->edit->hasFocus()) {
        commitEnteredValue();
        hideEdit();
    }
}

void KisAbstractSliderSpinBox::setInternalValue(int value)
{
    setInternalValue(value, false);
}

bool KisAbstractSliderSpinBox::isDragging() const
{
    Q_D(const KisAbstractSliderSpinBox);
    return d->isDragging;
}

void KisAbstractSliderSpinBox::setPrivateValue(int value)
{
    Q_D(KisAbstractSliderSpinBox);
    d->value = qBound(d->minimum, value, d->maximum);
}

class KisSliderSpinBoxPrivate : public KisAbstractSliderSpinBoxPrivate {
};

KisSliderSpinBox::KisSliderSpinBox(QWidget* parent) : KisAbstractSliderSpinBox(parent, new KisSliderSpinBoxPrivate)
{
    Q_D(KisSliderSpinBox);

    d->parseInt = true;

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
    d->fastSliderStep = (maximum-minimum+1)/20;
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

int KisSliderSpinBox::fastSliderStep() const
{
    const Q_D(KisSliderSpinBox);
    return d->fastSliderStep;
}

void KisSliderSpinBox::setFastSliderStep(int step)
{
    Q_D(KisSliderSpinBox);
    d->fastSliderStep = step;
}

int KisSliderSpinBox::value()
{
    Q_D(KisSliderSpinBox);
    return d->value;
}

void KisSliderSpinBox::setValue(int value)
{
    setInternalValue(value, false);
    update();
}

QString KisSliderSpinBox::valueString() const
{
    const Q_D(KisSliderSpinBox);

    QLocale locale;
    return locale.toString((qreal)d->value, 'f', d->validator->decimals());
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

void KisSliderSpinBox::setInternalValue(int _value, bool blockUpdateSignal)
{
    Q_D(KisAbstractSliderSpinBox);
    d->value = qBound(d->minimum, _value, d->maximum);

    if(!blockUpdateSignal) {
        emit(valueChanged(value()));
    }
}

class KisDoubleSliderSpinBoxPrivate : public KisAbstractSliderSpinBoxPrivate {
};

KisDoubleSliderSpinBox::KisDoubleSliderSpinBox(QWidget* parent) : KisAbstractSliderSpinBox(parent, new KisDoubleSliderSpinBoxPrivate)
{
    Q_D(KisDoubleSliderSpinBox);
    d->parseInt = false;
}

KisDoubleSliderSpinBox::~KisDoubleSliderSpinBox()
{
}

void KisDoubleSliderSpinBox::setRange(qreal minimum, qreal maximum, int decimals)
{
    Q_D(KisDoubleSliderSpinBox);
    d->factor = pow(10.0, decimals);

    d->minimum = minimum * d->factor;
    d->maximum = maximum * d->factor;
    //This code auto-compute a new step when pressing control.
    //A flag defaulting to "do not change the fast step" should be added, but it implies changing every call
    if(maximum - minimum >= 2.0 || decimals <= 0) {  //Quick step on integers
        d->fastSliderStep = int(pow(10.0, decimals));
    } else if(decimals == 1) {
        d->fastSliderStep = (maximum-minimum)*d->factor/10;
    } else {
        d->fastSliderStep = (maximum-minimum)*d->factor/20;
    }
    d->validator->setRange(minimum, maximum, decimals);
    update();
    setValue(value());
}

qreal KisDoubleSliderSpinBox::minimum() const
{
    const Q_D(KisAbstractSliderSpinBox);
    return d->minimum / d->factor;
}

void KisDoubleSliderSpinBox::setMinimum(qreal minimum)
{
    Q_D(KisAbstractSliderSpinBox);
    setRange(minimum, d->maximum);
}

qreal KisDoubleSliderSpinBox::maximum() const
{
    const Q_D(KisAbstractSliderSpinBox);
    return d->maximum / d->factor;
}

void KisDoubleSliderSpinBox::setMaximum(qreal maximum)
{
    Q_D(KisAbstractSliderSpinBox);
    setRange(d->minimum, maximum);
}

qreal KisDoubleSliderSpinBox::fastSliderStep() const
{
    const Q_D(KisAbstractSliderSpinBox);
    return d->fastSliderStep;
}
void KisDoubleSliderSpinBox::setFastSliderStep(qreal step)
{
    Q_D(KisAbstractSliderSpinBox);
    d->fastSliderStep = step;
}

qreal KisDoubleSliderSpinBox::value()
{
    Q_D(KisAbstractSliderSpinBox);
    return (qreal)d->value / d->factor;
}

void KisDoubleSliderSpinBox::setValue(qreal value)
{
    Q_D(KisAbstractSliderSpinBox);
    setInternalValue(d->value = qRound(value * d->factor), false);
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

    QLocale locale;
    return locale.toString((qreal)d->value / d->factor, 'f', d->validator->decimals());
}

void KisDoubleSliderSpinBox::setInternalValue(int _value, bool blockUpdateSignal)
{
    Q_D(KisAbstractSliderSpinBox);
    d->value = qBound(d->minimum, _value, d->maximum);

    if(!blockUpdateSignal) {
        emit(valueChanged(value()));
    }
}


void KisAbstractSliderSpinBox::changeEvent(QEvent *e)
{
    Q_D(KisAbstractSliderSpinBox);

    QWidget::changeEvent(e);

    switch (e->type()) {
    case QEvent::StyleChange:
        if (style()->objectName() == "fusion") {
            d->style = KisAbstractSliderSpinBoxPrivate::STYLE_FUSION;
        }
        else if (style()->objectName() == "breeze") {
            d->style = KisAbstractSliderSpinBoxPrivate::STYLE_BREEZE;
        }
        else {
            d->style = KisAbstractSliderSpinBoxPrivate::STYLE_NOQUIRK;
        }
        break;
    default:
        break;
    }
}
