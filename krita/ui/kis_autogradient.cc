/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <longamp@reallygood.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_autogradient.h"

#include <QPainter>
#include <QComboBox>

#include <kcolorbutton.h>
#include <knuminput.h>
#include "kis_int_spinbox.h"
#include "kis_gradient.h"
#include "kis_autogradient_resource.h"

#include "kis_gradient_slider_widget.h"

/****************************** KisAutogradient ******************************/

KisAutogradient::KisAutogradient(QWidget *parent, const char* name, const QString& caption) : QWidget(parent)
{
    setObjectName(name);
    setupUi(this);
    setWindowTitle(caption);
    m_autogradientResource = new KisAutogradientResource();
    m_autogradientResource->createSegment( INTERP_LINEAR, COLOR_INTERP_RGB, 0.0, 1.0, 0.5, Qt::black, Qt::white );
    connect(gradientSlider, SIGNAL( sigSelectedSegment( KisGradientSegment* ) ), SLOT( slotSelectedSegment(KisGradientSegment*) ));
    connect(gradientSlider, SIGNAL( sigChangedSegment(KisGradientSegment*) ), SLOT( slotChangedSegment(KisGradientSegment*) ));
    gradientSlider->setGradientResource( m_autogradientResource );
    connect(comboBoxColorInterpolationType, SIGNAL( activated(int) ), SLOT( slotChangedColorInterpolation(int) ));
    connect(comboBoxInterpolationType, SIGNAL( activated(int) ), SLOT( slotChangedInterpolation(int) ));
    connect(leftColorButton, SIGNAL( changed(const QColor&) ), SLOT( slotChangedLeftColor(const QColor&) ));
    connect(rightColorButton, SIGNAL( changed(const QColor&) ), SLOT( slotChangedRightColor(const QColor&) ));

//     intNumInputLeftOpacity->setRange( 0, 100, false);
    connect(intNumInputLeftOpacity, SIGNAL( valueChanged(int) ), SLOT( slotChangedLeftOpacity(int) ));
//     intNumInputRightOpacity->setRange( 0, 100, false);
    connect(intNumInputRightOpacity, SIGNAL( valueChanged(int) ), SLOT( slotChangedRightOpacity(int) ));

}

void KisAutogradient::activate()
{
    paramChanged();
}

void KisAutogradient::slotSelectedSegment(KisGradientSegment* segment)
{
    QColor startColor;
    QColor endColor;
    quint8 startOpacity;
    quint8 endOpacity;

    segment->startColor().toQColor( &startColor, &startOpacity);
    segment->endColor().toQColor( &endColor, &endOpacity);

    leftColorButton->setColor( startColor );
    rightColorButton->setColor( endColor );
    comboBoxColorInterpolationType->setCurrentIndex( segment->colorInterpolation() );
    comboBoxInterpolationType->setCurrentIndex( segment->interpolation() );

    int leftOpacity = qRound( startOpacity / OPACITY_OPAQUE * 100);
    intNumInputLeftOpacity->setValue( leftOpacity );

    int rightOpacity = qRound( endOpacity / OPACITY_OPAQUE * 100);
    intNumInputRightOpacity->setValue( rightOpacity );

    paramChanged();
}

void KisAutogradient::slotChangedSegment(KisGradientSegment*)
{
    paramChanged();
}

void KisAutogradient::slotChangedInterpolation(int type)
{
    KisGradientSegment* segment = gradientSlider->selectedSegment();
    if(segment)
        segment->setInterpolation( type );
    gradientSlider->update();

    paramChanged();
}

void KisAutogradient::slotChangedColorInterpolation(int type)
{
    KisGradientSegment* segment = gradientSlider->selectedSegment();
    if(segment)
        segment->setColorInterpolation( type );
    gradientSlider->update();

    paramChanged();
}

void KisAutogradient::slotChangedLeftColor( const QColor& color)
{
    KisGradientSegment* segment = gradientSlider->selectedSegment();
    if(segment)
    {
        QColor unused;
        quint8 opacity;
        segment->startColor().toQColor( &unused, &opacity);
        segment->setStartColor( KoColor( color, opacity, segment->startColor().colorSpace() ) );
    }
    gradientSlider->update();

    paramChanged();
}

void KisAutogradient::slotChangedRightColor( const QColor& color)
{
    KisGradientSegment* segment = gradientSlider->selectedSegment();
    if(segment)
    {
        QColor unused;
        quint8 opacity;
        segment->endColor().toQColor( &unused, &opacity);
        segment->setEndColor( KoColor( color, opacity, segment->endColor().colorSpace() ) );
    }
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::slotChangedLeftOpacity( int value )
{
    KisGradientSegment* segment = gradientSlider->selectedSegment();
    if(segment)
        segment->setStartColor( KoColor( segment->startColor().toQColor(), value / 100 * OPACITY_OPAQUE, segment->startColor().colorSpace() ) );
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::slotChangedRightOpacity( int value )
{
    KisGradientSegment* segment = gradientSlider->selectedSegment();
    if(segment)
        segment->setEndColor( KoColor( segment->endColor().toQColor(), value / 100 *OPACITY_OPAQUE, segment->endColor().colorSpace() ));
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::paramChanged()
{
    m_autogradientResource->updatePreview ();
    emit activatedResource( m_autogradientResource );
}

#include "kis_autogradient.moc"
