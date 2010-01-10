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
#include "KoSliderCombo.h"

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

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

class KoSliderComboContainer : public QMenu
{
public:
    KoSliderComboContainer(KoSliderCombo *parent) : QMenu(parent ), m_parent(parent) {}

protected:
    virtual void mousePressEvent(QMouseEvent *e);
private:
    KoSliderCombo *m_parent;
};

void KoSliderComboContainer::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComboBox opt;
    opt.init(m_parent);
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_ComboBoxArrow;
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt,
                                                           m_parent->mapFromGlobal(e->globalPos()),
                                                           m_parent);
    if (sc == QStyle::SC_ComboBoxArrow)
        setAttribute(Qt::WA_NoMouseReplay);
    QMenu::mousePressEvent(e);
}

class KoSliderCombo::KoSliderComboPrivate {
public:
    KoSliderCombo *thePublic;
    QValidator *m_validator;
    QTimer m_timer;
    KoSliderComboContainer *container;
    QSlider *slider;
    QStyle::StateFlag arrowState;
    qreal minimum;
    qreal maximum;
    int decimals;
    bool firstShowOfSlider;

    void showPopup();
    void hidePopup();

    void sliderValueChanged(int value);
    void sliderReleased();
    void lineEditFinished();
};

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
    opt.init(this);
//    d->container->setFrameStyle(style()->styleHint(QStyle::SH_ComboBox_PopupFrameStyle, &opt, this));

    d->slider = new QSlider(Qt::Horizontal);
    d->slider->setMinimum(0);
    d->slider->setMaximum(256);
    d->slider->setPageStep(10);
    d->slider->setValue(0);
    // When set to true, causes flicker on Qt 4.6. Any reason to keep it?
    d->firstShowOfSlider = false; //true;

    QHBoxLayout * l = new QHBoxLayout();
    l->setMargin(2);
    l->setSpacing(2);
    l->addWidget(d->slider);
    d->container->setLayout(l);
    d->container->resize(200, 30);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    setEditable(true);
    setEditText(KGlobal::locale()->formatNumber(0, d->decimals));

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

    sh.setWidth(5 * fm.width(QLatin1Char('8')));
    sh.setHeight(qMax(fm.lineSpacing(), 14) + 2);

    // add style and strut values
    QStyleOptionComboBox opt;
    opt.init(this);
    opt.subControls = QStyle::SC_All;
    opt.editable = true;
    sh = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, sh, this);

    return sh.expandedTo(QApplication::globalStrut());
}

void KoSliderCombo::KoSliderComboPrivate::showPopup()
{
    if(firstShowOfSlider) {
        container->show(); //show container a bit early so the slider can be layout'ed
        firstShowOfSlider = false;
    }

    QStyleOptionSlider opt;
    opt.init(slider);
    opt.maximum=256;
    opt.sliderPosition = opt.sliderValue = slider->value();
    int hdlPos = thePublic->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle).center().x();

    QStyleOptionComboBox optThis;
    optThis.init(thePublic);
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
    opt.init(this);
    opt.subControls = QStyle::SC_All;
    opt.editable = true;
    gc.drawComplexControl(QStyle::CC_ComboBox, opt);
    gc.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void KoSliderCombo::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComboBox opt;
    opt.init(this);
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
    qreal value = thePublic->currentText().toDouble();
    slider->blockSignals(true);
    slider->setValue(int((value - minimum) * 256 / (maximum - minimum) + 0.5));
    slider->blockSignals(false);
    emit thePublic->valueChanged(value, true);
}

void KoSliderCombo::KoSliderComboPrivate::sliderValueChanged(int slidervalue)
{
    thePublic->setEditText(KGlobal::locale()->formatNumber(minimum + (maximum - minimum)*slidervalue/256, decimals));

    qreal value = thePublic->currentText().toDouble();
    emit thePublic->valueChanged(value, false);
}

void KoSliderCombo::KoSliderComboPrivate::sliderReleased()
{
    qreal value = thePublic->currentText().toDouble();
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
    return currentText().toDouble();
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
    setEditText(KGlobal::locale()->formatNumber(value, d->decimals));
    d->slider->blockSignals(true);
    d->slider->setValue(int((value - d->minimum) * 256 / (d->maximum - d->minimum) + 0.5));
    d->slider->blockSignals(false);
    emit valueChanged(value, true);
}

#include <KoSliderCombo.moc>
