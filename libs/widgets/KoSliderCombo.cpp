/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 C. Boemann <cbo@boemann.dk>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "KoSliderCombo.h"
#include "KoSliderCombo_p.h"

#include <QTimer>
#include <QApplication>
#include <QSize>
#include <QSlider>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QLineEdit>
#include <QValidator>
#include <QHBoxLayout>
#include <QFrame>
#include <QMenu>
#include <QMouseEvent>
#include <QDoubleSpinBox>
#include <QDesktopWidget>


#include <klocalizedstring.h>
#include <WidgetsDebug.h>

KoSliderCombo::KoSliderCombo(QWidget *parent)
   : QComboBox(parent)
    ,d(new KoSliderComboPrivate())
{
    d->thePublic = this;
    d->minimum = 0.0;
    d->maximum = 100.0;
    d->decimals = 2;
    d->container = new KoSliderComboContainer(this);
    d->container->setAttribute(Qt::WA_WindowPropagation);
    QStyleOptionComboBox opt;
    opt.initFrom(this);
//    d->container->setFrameStyle(style()->styleHint(QStyle::SH_ComboBox_PopupFrameStyle, &opt, this));

    d->slider = new QSlider(Qt::Horizontal);
    d->slider->setMinimum(0);
    d->slider->setMaximum(256);
    d->slider->setPageStep(10);
    d->slider->setValue(0);
    // When set to true, causes flicker on Qt 4.6. Any reason to keep it?
    d->firstShowOfSlider = false; //true;

    QHBoxLayout *layout = new QHBoxLayout(d->container);
    layout->setMargin(2);
    layout->setSpacing(2);
    layout->addWidget(d->slider);
    d->container->resize(200, 30);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    setEditable(true);
    setEditText(QLocale().toString(0.0, d->decimals));

    connect(d->slider, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)));
    connect(d->slider, SIGNAL(sliderReleased()), SLOT(sliderReleased()));
    connect(lineEdit(), SIGNAL(editingFinished()), SLOT(lineEditFinished()));
}

KoSliderCombo::~KoSliderCombo()
{
    delete d;
}

QSize KoSliderCombo::sizeHint() const
{
    return minimumSizeHint();
}

QSize KoSliderCombo::minimumSizeHint() const
{
    QSize sh;

    const QFontMetrics &fm = fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
    sh.setWidth(5 * fm.horizontalAdvance(QLatin1Char('8')));
#else
    sh.setWidth(5 * fm.width(QLatin1Char('8')));
#endif
    sh.setHeight(qMax(fm.lineSpacing(), 14) + 2);

    // add style and strut values
    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.subControls = QStyle::SC_All;
    opt.editable = true;
    sh = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, sh, this);

    return sh;
}

void KoSliderCombo::KoSliderComboPrivate::showPopup()
{
    if(firstShowOfSlider) {
        container->show(); //show container a bit early so the slider can be layout'ed
        firstShowOfSlider = false;
    }

    QStyleOptionSlider opt;
    opt.initFrom(slider);
    opt.maximum=256;
    opt.sliderPosition = opt.sliderValue = slider->value();
    int hdlPos = thePublic->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle).center().x();

    QStyleOptionComboBox optThis;
    optThis.initFrom(thePublic);
    optThis.subControls = QStyle::SC_All;
    optThis.editable = true;
    int arrowPos = thePublic->style()->subControlRect(QStyle::CC_ComboBox, &optThis, QStyle::SC_ComboBoxArrow).center().x();

    QSize popSize = container->size();
    QRect popupRect(thePublic->mapToGlobal(QPoint(arrowPos - hdlPos - slider->x(), thePublic->size().height())), popSize);

    // Make sure the popup is not drawn outside the screen area
    QRect screenRect = QApplication::desktop()->availableGeometry(thePublic);
    if (popupRect.right() > screenRect.right())
        popupRect.translate(screenRect.right() - popupRect.right(), 0);
    if (popupRect.left() < screenRect.left())
        popupRect.translate(screenRect.left() - popupRect.left(), 0);
    if (popupRect.bottom() > screenRect.bottom())
        popupRect.translate(0, -(thePublic->height() + container->height()));

    container->setGeometry(popupRect);
    container->raise();
    container->show();
    slider->setFocus();
}

void KoSliderCombo::KoSliderComboPrivate::hidePopup()
{
    container->hide();
}

void KoSliderCombo::hideEvent(QHideEvent *)
{
    d->hidePopup();
}

void KoSliderCombo::changeEvent(QEvent *e)
{
    switch (e->type())
    {
        case QEvent::EnabledChange:
            if (!isEnabled())
                d->hidePopup();
            break;
        case QEvent::PaletteChange:
            d->container->setPalette(palette());
            break;
        default:
            break;
    }
    QComboBox::changeEvent(e);
}

void KoSliderCombo::paintEvent(QPaintEvent *)
{
    QStylePainter gc(this);

    gc.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.subControls = QStyle::SC_All;
    opt.editable = true;
    gc.drawComplexControl(QStyle::CC_ComboBox, opt);
    gc.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void KoSliderCombo::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.subControls = QStyle::SC_All;
    opt.editable = true;
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt, e->pos(),
                                                           this);
    if (sc == QStyle::SC_ComboBoxArrow && !d->container->isVisible())
    {
        d->showPopup();
    }
    else
        QComboBox::mousePressEvent(e);
}

void KoSliderCombo::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up) setValue(value() + d->slider->singleStep() * (maximum() - minimum()) / 256 + 0.5);
    else if (e->key() == Qt::Key_Down) setValue(value() - d->slider->singleStep() * (maximum() - minimum()) / 256 - 0.5);
    else QComboBox::keyPressEvent(e);
}

void KoSliderCombo::wheelEvent(QWheelEvent *e)
{
    if (e->delta() > 0) setValue(value() + d->slider->singleStep() * (maximum() - minimum()) / 256 + 0.5);
    else setValue(value() - d->slider->singleStep() * (maximum() - minimum()) / 256 - 0.5);
}

void KoSliderCombo::KoSliderComboPrivate::lineEditFinished()
{
    qreal value = QLocale().toDouble(thePublic->currentText());
    slider->blockSignals(true);
    slider->setValue(int((value - minimum) * 256 / (maximum - minimum) + 0.5));
    slider->blockSignals(false);
    emit thePublic->valueChanged(value, true);
}

void KoSliderCombo::KoSliderComboPrivate::sliderValueChanged(int slidervalue)
{
    thePublic->setEditText(QLocale().toString(minimum + (maximum - minimum)*slidervalue/256, decimals));

    qreal value = QLocale().toDouble(thePublic->currentText());
    emit thePublic->valueChanged(value, false);
}

void KoSliderCombo::KoSliderComboPrivate::sliderReleased()
{
    qreal value = QLocale().toDouble(thePublic->currentText());
    emit thePublic->valueChanged(value, true);
}

qreal KoSliderCombo::maximum() const
{
    return d->maximum;
}

qreal KoSliderCombo::minimum() const
{
    return d->minimum;
}

qreal KoSliderCombo::decimals() const
{
    return d->decimals;
}

qreal KoSliderCombo::value() const
{
    return QLocale().toDouble(currentText());
}

void KoSliderCombo::setDecimals(int dec)
{
    d->decimals = dec;
    if (dec == 0) lineEdit()->setValidator(new QIntValidator(this));
    else lineEdit()->setValidator(new QDoubleValidator(this));
}

void KoSliderCombo::setMinimum(qreal min)
{
    d->minimum = min;
}

void KoSliderCombo::setMaximum(qreal max)
{
    d->maximum = max;
}

void KoSliderCombo::setValue(qreal value)
{
    if(value < d->minimum)
        value = d->minimum;
    if(value > d->maximum)
        value = d->maximum;
    setEditText(QLocale().toString(value, d->decimals));
    d->slider->blockSignals(true);
    d->slider->setValue(int((value - d->minimum) * 256 / (d->maximum - d->minimum) + 0.5));
    d->slider->blockSignals(false);
    emit valueChanged(value, true);
}

//have to include this because of Q_PRIVATE_SLOT
#include <moc_KoSliderCombo.cpp>
