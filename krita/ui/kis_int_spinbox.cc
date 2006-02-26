/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Casper Boemann <cbr@boemann.dk>
 *
 *  Requires the Qt widget libraries, available at no cost at
 *  http://www.troll.no/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <assert.h>
#include <math.h>
#include <algorithm>

#include <qapplication.h>
#include <qsize.h>
#include <qslider.h>
#include <qstyle.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qvalidator.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <karrowbutton.h>

#include "kdialog.h"
#include "knumvalidator.h"
#include "kis_int_spinbox.h"

class KisIntSpinbox::KisIntSpinboxPrivate {
public:

    QLineEdit * m_numinput;
    KisPopupSlider *m_slider;
    KArrowButton *m_arrow;
    int m_prevValue;
    QValidator *m_validator;
};


KisIntSpinbox::KisIntSpinbox(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    init(0);
}

KisIntSpinbox::KisIntSpinbox(const QString & /*label*/, int val, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    init(val);
}

void KisIntSpinbox::init(int val)
{
    d = new KisIntSpinboxPrivate( );
    QBoxLayout * l = new QHBoxLayout( this );

    l->insertStretch(0, 1);
    d->m_numinput = new QLineEdit(this, "KisIntSpinbox::QLineEdit");
    //d->m_numinput->setInputMask("009%"); //makes it use overwrite mode and be uneditable, bizarre.
    d->m_validator = new QRegExpValidator(QRegExp("^((100)|([0-9]?[0-9]))\\%$"), this);
    d->m_numinput->setValidator(d->m_validator);
    d->m_numinput->setAlignment(Qt::AlignRight);
    //d->m_numinput->setMaximumWidth(d->m_numinput->fontMetrics().width("100%"));
    //d->m_numinput->setMinimumWidth(d->m_numinput->fontMetrics().width("100%"));
    d->m_numinput->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    l->addWidget( d->m_numinput );

    connect(d->m_numinput, SIGNAL(textChanged(const QString &)), SLOT(numValueChanged(const QString &)));

    //d->m_slider = new KisPopupSlider(INT_MIN, INT_MAX, 1, val, QSlider::Horizontal, this);
    d->m_slider = new KisPopupSlider(0, 100, 1, val, QSlider::Horizontal, this);
    d->m_slider->setFrameStyle(QFrame::Panel|QFrame::Raised);
    connect(d->m_slider, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)));
    connect(d->m_slider, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
    connect(d->m_slider, SIGNAL(aboutToHide()), SLOT(slotAboutToHide()));

    d->m_arrow = new KArrowButton(this, Qt::DownArrow);
    d->m_arrow->setPopup(d->m_slider);
    d->m_arrow->setMaximumHeight( fontMetrics().height() + 2);
    d->m_arrow->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->m_arrow->setEnabled(true);

    l->addWidget( d->m_arrow );

    d->m_prevValue = val;
    setValue(val);
    setFocusProxy(d->m_numinput);
    layout();
}

void KisIntSpinbox::numValueChanged(const QString &text)
{
    int val;
    if (text.length() >= 2)
    {
        val = text.left(text.length()-1).toInt();
        if (val == 0)
        {
            d->m_numinput->setText(QString("%1%").arg(val));
            return;
        }
    }
    else
    {
        if (text.toInt() != 0)
            val = text.toInt();
        else
            val = 0;
        d->m_numinput->setText(QString("%1%").arg(val));
        return;
    }
    d->m_slider->blockSignals(true);
    d->m_slider->setValue(val);
    d->m_slider->blockSignals(false);

    emit valueChanged(val);
    emit valueChanged(val, false);
}

void KisIntSpinbox::sliderValueChanged(int val)
{
    d->m_numinput->blockSignals(true);
    QString valstr = QString("%1%").arg(val);
    d->m_numinput->setText(valstr);
    d->m_numinput->blockSignals(false);

    emit valueChanged(val);
    emit valueChanged(val, true);
}

void KisIntSpinbox::setRange(int lower, int upper, int /*step*/)
{
    upper = kMax(upper, lower);
    lower = kMin(upper, lower);
    d->m_slider->setRange(lower, upper);

    layout();
}

void KisIntSpinbox::setMinValue(int min)
{
    setRange(min, maxValue(), d->m_slider->lineStep());
}

int KisIntSpinbox::minValue() const
{
    return d->m_slider->minValue();
}

void KisIntSpinbox::setMaxValue(int max)
{
    setRange(minValue(), max, d->m_slider->lineStep());
}

int KisIntSpinbox::maxValue() const
{
    return d->m_slider->maxValue();
}

KisIntSpinbox::~KisIntSpinbox()
{
    delete d;
}

void KisIntSpinbox::setValue(int val)
{
    QString valstr = QString("%1%").arg(val);
    if (d->m_numinput->text() != valstr)
        d->m_numinput->setText(valstr);
    // slider value is changed by numValueChanged
}

int  KisIntSpinbox::value() const
{
    return d->m_slider->value();
}

void KisIntSpinbox::setLabel(const QString & /*label*/)
{
//    d->m_numinput->setLabel(label);
}

void KisIntSpinbox::slotAboutToShow()
{
    d->m_prevValue = value();
}

void KisIntSpinbox::slotAboutToHide()
{
    if( d->m_prevValue != value() )
    {
        emit finishedChanging( d->m_prevValue, value() );
        d->m_prevValue = value();
    }
}


#include "kis_int_spinbox.moc"
