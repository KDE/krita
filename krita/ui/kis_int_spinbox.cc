/*
 *     Copyright (c) 2006 Boudewijn Rempt
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
#include <qspinbox.h>
#include <qstyle.h>
#include <qlabel.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "kdialog.h"
#include "knumvalidator.h"
#include "knuminput.h"
#include "kis_int_spinbox.h"

static inline int calcDiffByTen( int x, int y ) {
    // calculate ( x - y ) / 10 without overflowing ints:
    return ( x / 10 ) - ( y / 10 )  +  ( x % 10 - y % 10 ) / 10;
}

class KisPopupSlider : public QPopupMenu {

public:

    KisPopupSlider(nt minValue, int maxValue, int pageStep, int value, Orientation orientation, QWidget * parent, const char * name = 0) 
        : QPopupMenu(parent, name)
    {
    }

};


class KisIntSpinbox::KisIntSpinboxPrivate {
public:

    int referencePoint;
    short blockRelative;
    QLabel * m_label;
    KIntSpinBox * m_spinbox;
    

    KisIntSpinboxPrivate( int r )
    : referencePoint( r ),
      blockRelative( 0 ) {}
};


KisIntSpinbox::KisIntSpinbox(const QString & label, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    init(0, 10);
}

KisIntSpinbox::KisIntSpinbox(const QString & label int val, int _base, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    init(val, _base);
}

void KisIntSpinbox::init(int val, int _base)
{
    d = new KisIntSpinboxPrivate( val );
    m_spin = new KIntSpinBox(INT_MIN, INT_MAX, 1, val, _base, this, "KisIntSpinbox::KIntSpinBox");
    // the KIntValidator is broken beyond belief for
    // spinboxes which have suffix or prefix texts, so
    // better don't use it unless absolutely necessary
    if (_base != 10)
        m_spin->setValidator(new KIntValidator(this, _base, "KNumInput::KIntValidtr"));

    connect(m_spin, SIGNAL(valueChanged(int)), SLOT(spinValueChanged(int)));
    connect(this, SIGNAL(valueChanged(int)),
        SLOT(slotEmitRelativeValueChanged(int)));

    setFocusProxy(m_spin);
    layout(true);
}

void KisIntSpinbox::setReferencePoint( int ref ) {
    // clip to valid range:
    ref = kMin( maxValue(), kMax( minValue(),  ref ) );
    d->referencePoint = ref;
}

int KisIntSpinbox::referencePoint() const {
    return d->referencePoint;
}

void KisIntSpinbox::spinValueChanged(int val)
{
    if(m_slider)
        m_slider->setValue(val);

    emit valueChanged(val);
}

void KisIntSpinbox::slotEmitRelativeValueChanged( int value ) {
    if ( d->blockRelative || !d->referencePoint ) return;
    emit relativeValueChanged( double( value ) / double( d->referencePoint ) );
}

void KisIntSpinbox::setRange(int lower, int upper, int step, bool slider)
{
    upper = kMax(upper, lower);
    lower = kMin(upper, lower);
    m_spin->setMinValue(lower);
    m_spin->setMaxValue(upper);
    m_spin->setLineStep(step);

    step = m_spin->lineStep(); // maybe QRangeControl didn't like out lineStep?

    if(slider) {
    if (m_slider)
        m_slider->setRange(lower, upper);
    else {
        m_slider = new QSlider(lower, upper, step, m_spin->value(),
                   QSlider::Horizontal, this);
        m_slider->setTickmarks(QSlider::Below);
        connect(m_slider, SIGNAL(valueChanged(int)),
            m_spin, SLOT(setValue(int)));
    }

    // calculate (upper-lower)/10 without overflowing int's:
        int major = calcDiffByTen( upper, lower );
    if ( major==0 ) major = step; // #### workaround Qt bug in 2.1-beta4

        m_slider->setSteps(step, major);
        m_slider->setTickInterval(major);
    }
    else {
        delete m_slider;
        m_slider = 0;
    }

    // check that reference point is still inside valid range:
    setReferencePoint( referencePoint() );
    layout(true);
}

void KisIntSpinbox::setMinValue(int min)
{
    setRange(min, m_spin->maxValue(), m_spin->lineStep(), m_slider);
}

int KisIntSpinbox::minValue() const
{
    return m_spin->minValue();
}

void KisIntSpinbox::setMaxValue(int max)
{
    setRange(m_spin->minValue(), max, m_spin->lineStep(), m_slider);
}

int KisIntSpinbox::maxValue() const
{
    return m_spin->maxValue();
}

void KisIntSpinbox::setSuffix(const QString &suffix)
{
    m_spin->setSuffix(suffix);

    layout(true);
}

QString KisIntSpinbox::suffix() const
{
    return m_spin->suffix();
}

void KisIntSpinbox::setPrefix(const QString &prefix)
{
    m_spin->setPrefix(prefix);

    layout(true);
}

QString KisIntSpinbox::prefix() const
{
    return m_spin->prefix();
}

void KisIntSpinbox::setEditFocus(bool mark)
{
    m_spin->setEditFocus(mark);
}

QSize KisIntSpinbox::minimumSizeHint() const
{
    constPolish();

    int w;
    int h;

    h = 2 + QMAX(m_sizeSpin.height(), m_sizeSlider.height());

    // if in extra row, then count it here
    if(m_label && (m_alignment & (AlignBottom|AlignTop)))
        h += 4 + m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
        h = QMAX(h, m_sizeLabel.height() + 2);

    w = m_slider ? m_slider->sizeHint().width() + 8 : 0;
    w += m_colw1 + m_colw2;

    if(m_alignment & (AlignTop|AlignBottom))
        w = QMAX(w, m_sizeLabel.width() + 4);

    return QSize(w, h);
}

void KisIntSpinbox::doLayout()
{
    m_sizeSpin = m_spin->sizeHint();
    m_colw2 = m_sizeSpin.width();

    if (m_label)
        m_label->setBuddy(m_spin);
}

void KisIntSpinbox::resizeEvent(QResizeEvent* e)
{
    int w = m_colw1;
    int h = 0;

    if(m_label && (m_alignment & AlignTop)) {
        m_label->setGeometry(0, 0, e->size().width(), m_sizeLabel.height());
        h += m_sizeLabel.height() + KDialog::spacingHint();
    }

    if(m_label && (m_alignment & AlignVCenter))
        m_label->setGeometry(0, 0, w, m_sizeSpin.height());

    if (qApp->reverseLayout())
    {
        m_spin->setGeometry(w, h, m_slider ? m_colw2 : QMAX(m_colw2, e->size().width() - w), m_sizeSpin.height());
        w += m_colw2 + 8;

        if(m_slider)
            m_slider->setGeometry(w, h, e->size().width() - w, m_sizeSpin.height());
    }
    else if(m_slider) {
        m_slider->setGeometry(w, h, e->size().width() - (w + m_colw2 + KDialog::spacingHint()), m_sizeSpin.height());
        m_spin->setGeometry(w + m_slider->size().width() + KDialog::spacingHint(), h, m_colw2, m_sizeSpin.height());
    }
    else {
       m_spin->setGeometry(w, h, QMAX(m_colw2, e->size().width() - w), m_sizeSpin.height());
    }

    h += m_sizeSpin.height() + 2;

    if(m_label && (m_alignment & AlignBottom))
        m_label->setGeometry(0, h, m_sizeLabel.width(), m_sizeLabel.height());
}

KisIntSpinbox::~KisIntSpinbox()
{
    delete d;
}

void KisIntSpinbox::setValue(int val)
{
    m_spin->setValue(val);
    // slider value is changed by spinValueChanged
}

void KisIntSpinbox::setRelativeValue( double r ) {
    if ( !d->referencePoint ) return;
    ++d->blockRelative;
    setValue( int( d->referencePoint * r + 0.5 ) );
    --d->blockRelative;
}

double KisIntSpinbox::relativeValue() const {
    if ( !d->referencePoint ) return 0;
    return double( value() ) / double ( d->referencePoint );
}

int  KisIntSpinbox::value() const
{
    return m_spin->value();
}

void KisIntSpinbox::setSpecialValueText(const QString& text)
{
    m_spin->setSpecialValueText(text);
    layout(true);
}

QString KisIntSpinbox::specialValueText() const
{
    return m_spin->specialValueText();
}

void KisIntSpinbox::setLabel(const QString & label, int a)
{
    KNumInput::setLabel(label, a);

    if(m_label)
        m_label->setBuddy(m_spin);
}


#include "kis_int_spinbox.moc"
