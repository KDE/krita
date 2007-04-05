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

#include <QTimer>
#include <QApplication>
#include <QSize>
#include <QSlider>
#include <QStyle>
#include <QStylePainter>
#include <QLineEdit>
#include <QValidator>
#include <QHBoxLayout>
#include <QFrame>
#include <QMenu>
#include <QMouseEvent>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "KoSliderCombo.h"

class KoSliderComboContainer : public QFrame
{
public:
    KoSliderComboContainer(KoSliderCombo *parent) : QFrame(parent, Qt::Popup ), m_parent(parent) {}

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
    QFrame::mousePressEvent(e);
}

class KoSliderCombo::KoSliderComboPrivate {
public:

    QLineEdit * lineEdit;
    QValidator *m_validator;
    QTimer m_timer;
    KoSliderComboContainer *container;
    QSlider *slider;
    QStyle::StateFlag arrowState;
    double minimum;
    double maximum;
    int decimals;
};

KoSliderCombo::KoSliderCombo(QWidget *parent)
    : QWidget(parent)
{
    d = new KoSliderComboPrivate();
    d->m_timer.setSingleShot(true);
    d->arrowState = QStyle::State_None;

    d->lineEdit = new QLineEdit(this);
    d->lineEdit->setText("0.00");
    d->minimum = 0.0;
    d->maximum = 100.0;
    d->decimals = 2;
    d->container = new KoSliderComboContainer(this);
    d->container->setAttribute(Qt::WA_WindowPropagation);
    QStyleOptionComboBox opt = styleOption();
    d->container->setFrameStyle(style()->styleHint(QStyle::SH_ComboBox_PopupFrameStyle, &opt, this));

    d->slider = new QSlider(Qt::Horizontal);
    d->slider->setMinimum(0);
    d->slider->setMaximum(256);
    d->slider->setPageStep(10);
    d->slider->setValue(0);
    //d->slider->setTracking(false);
    d->container->resize(200, 30);

    QHBoxLayout * l = new QHBoxLayout();
    l->setMargin(2);
    l->setSpacing(2);
    l->addWidget(d->slider);
    d->container->setLayout(l);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(&(d->m_timer), SIGNAL(timeout()), this, SLOT(slotTimeout()));
    connect(d->slider, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)));
    connect(d->lineEdit, SIGNAL(editingFinished()), SLOT(lineEditFinished()));
}

KoSliderCombo::~KoSliderCombo()
{
    delete d;
}

void KoSliderCombo::updateLineEditGeometry()
{
    QStyleOptionComboBox opt = styleOption();
    QRect editRect = style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                                QStyle::SC_ComboBoxEditField, this);
    d->lineEdit->setGeometry(editRect);
}

void KoSliderCombo::updateArrow(QStyle::StateFlag state)
{
    if (d->arrowState == state)
        return;
    d->arrowState = state;
    QStyleOptionComboBox opt = styleOption();
    update(style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow));
}

void KoSliderCombo::resizeEvent(QResizeEvent *)
{
    updateLineEditGeometry();
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
    QStyleOptionComboBox opt = styleOption();
    sh = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, sh, this);

    return sh.expandedTo(QApplication::globalStrut());
}

QStyleOptionComboBox KoSliderCombo::styleOption() const
{
    QStyleOptionComboBox opt;
    opt.init(this);

    opt.subControls = QStyle::SC_All;
    opt.currentText = KGlobal::locale()->formatNumber(666, 0);
    opt.editable = true;

    if (d->arrowState == QStyle::State_Sunken) {
        opt.activeSubControls = QStyle::SC_ComboBoxArrow;
        opt.state |= d->arrowState;
    }

    if (d->container && d->container->isVisible())
        opt.state |= QStyle::State_On;

    return opt;
}

void KoSliderCombo::showPopup()
{
    QStyleOptionComboBox opt = styleOption();

    QSize popSize = d->container->size();
    QRect popupRect(mapToGlobal(QPoint(size().width() - popSize.width(), size().height())), popSize);
    d->container->setGeometry(popupRect);

    d->container->raise();
    d->container->show();
    d->slider->setFocus();
}

void KoSliderCombo::hidePopup()
{
    d->container->hide();
}

void KoSliderCombo::hideEvent(QHideEvent *)
{
    hidePopup();
}

void KoSliderCombo::changeEvent(QEvent *e)
{
    switch (e->type())
    {
        case QEvent::StyleChange:
            updateLineEditGeometry();
            break;
        case QEvent::EnabledChange:
            if (!isEnabled())
                hidePopup();
            break;
        case QEvent::PaletteChange:
            d->container->setPalette(palette());
            break;
        case QEvent::FontChange:
            d->container->setFont(font());
            updateLineEditGeometry();
            break;
        default:
            break;
    }
    QWidget::changeEvent(e);
}

void KoSliderCombo::focusInEvent(QFocusEvent *e)
{
    update();
    d->lineEdit->event((QEvent *)e);
}

void KoSliderCombo::focusOutEvent(QFocusEvent *e)
{
    update();
    d->lineEdit->event((QEvent *)e);
}

void KoSliderCombo::paintEvent(QPaintEvent *)
{
    QStylePainter gc(this);

    gc.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox opt = styleOption();
    gc.drawComplexControl(QStyle::CC_ComboBox, opt);
    gc.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

bool KoSliderCombo::event(QEvent *event)
{
    switch(event->type())
    {
        case QEvent::LayoutDirectionChange:
        case QEvent::ApplicationLayoutDirectionChange:
            updateLineEditGeometry();
            break;
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
        case QEvent::HoverMove:
/*    if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event))
        d->updateHoverControl(he->pos());
*/
            break;
        case QEvent::ShortcutOverride:
            return d->lineEdit->event(event);
            break;
        default:
            break;
    }
    return QWidget::event(event);
}

void KoSliderCombo::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComboBox opt = styleOption();
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt, e->pos(),
                                                           this);
    if (sc == QStyle::SC_ComboBoxArrow && !d->container->isVisible())
    {
        updateArrow(QStyle::State_Sunken);
        showPopup();
    }
}

void KoSliderCombo::lineEditFinished()
{
    double value = d->lineEdit->text().toDouble();
    d->slider->setValue(int((value - d->minimum) * 256 / d->maximum + 0.5));
}

void KoSliderCombo::sliderValueChanged(int slidervalue)
{
    d->lineEdit->setText(KGlobal::locale()->formatNumber(d->minimum + d->maximum*slidervalue/256, d->decimals));

    double value = d->lineEdit->text().toDouble();
    emit valueChanged(value);
}

double KoSliderCombo::maximum() const
{
    return d->maximum;
}

double KoSliderCombo::minimum() const
{
    return d->minimum;
}

double KoSliderCombo::decimals() const
{
    return d->decimals;
}

void KoSliderCombo::setDecimals(int dec)
{
    d->decimals = dec;
}

void KoSliderCombo::setMinimum(double min)
{
    d->minimum = min;
}

void KoSliderCombo::setMaximum(double max)
{
    d->maximum = max;
}

#include "KoSliderCombo.moc"
