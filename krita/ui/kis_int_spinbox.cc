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

#include <qtimer.h>
#include <qapplication.h>
#include <qsize.h>
#include <qslider.h>
#include <qstyle.h>
#include <qlabel.h>
#include <q3popupmenu.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qvalidator.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3Frame>

#include <knuminput.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <karrowbutton.h>

#include "kdialog.h"
#include "knumvalidator.h"
#include "kis_int_spinbox.h"

class KisIntSpinbox::KisIntSpinboxPrivate {
public:

    KIntSpinBox * m_numinput;
    KisPopupSlider *m_slider;
    KArrowButton *m_arrow;
    int m_prevValue;
    QValidator *m_validator;
    QTimer m_timer;
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
    Q3BoxLayout * l = new Q3HBoxLayout( this );

    l->insertStretch(0, 1);
    d->m_numinput = new KIntSpinBox(0, 100, 1, val, 10, this, "KisIntSpinbox::KIntSpinBox");

    d->m_numinput->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    d->m_numinput->setSuffix("%");
    l->addWidget( d->m_numinput );

    d->m_slider = new KisPopupSlider(0, 100, 10, val, Qt::Horizontal, this);
    d->m_slider->setFrameStyle(Q3Frame::Panel|Q3Frame::Raised);

    d->m_arrow = new KArrowButton(this, Qt::DownArrow);
    d->m_arrow->setPopup(d->m_slider);
    d->m_arrow->setMaximumHeight( fontMetrics().height() + 4);
    d->m_arrow->setEnabled(true); // Why is the arrow still gray?

    l->addWidget( d->m_arrow );

    d->m_prevValue = val;
    setValue(val);
    setFocusProxy(d->m_numinput);
    layout();

    connect(d->m_numinput, SIGNAL(valueChanged(int)), SLOT(spinboxValueChanged(int)));
    connect(d->m_slider, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)));
    connect(d->m_slider, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
    connect(d->m_slider, SIGNAL(aboutToHide()), SLOT(slotAboutToHide()));

    connect(&(d->m_timer), SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

void KisIntSpinbox::spinboxValueChanged(int val)
{
    setValue(val);
    d->m_timer.start(300, true);

}

void KisIntSpinbox::sliderValueChanged(int val)
{
    setValue(val);
    emit valueChanged(val);
    emit valueChanged(val, true);
}

void KisIntSpinbox::setRange(int lower, int upper, int /*step*/)
{
    upper = qMax(upper, lower);
    lower = qMin(upper, lower);
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
   d->m_slider->blockSignals(true);
   d->m_slider->setValue(val);
   d->m_slider->blockSignals(false);

   d->m_numinput->blockSignals(true);
   d->m_numinput->setValue(val);
   d->m_numinput->blockSignals(false);
}

int  KisIntSpinbox::value() const
{
    return d->m_numinput->value(); // From the numinput: that one isn't in steps of ten
}

void KisIntSpinbox::setLabel(const QString & /*label*/)
{
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

void KisIntSpinbox::slotTimeout()
{
    emit valueChanged(value());
    emit valueChanged(value(), true);
}
#include "kis_int_spinbox.moc"
