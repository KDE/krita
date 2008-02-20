/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShadowConfigWidget.h"
#include "ui_KoShadowConfigWidget.h"
#include <KoUnit.h>
#include <QtGui/QCheckBox>
#include <math.h>

class KoShadowConfigWidget::Private
{
public:
    Private()
    {
    }
    Ui_KoShadowConfigWidget widget;
};

KoShadowConfigWidget::KoShadowConfigWidget( QWidget * parent )
    : QWidget( parent ), d( new Private() )
{
    d->widget.setupUi(this);
    d->widget.shadowColor->setColor( Qt::gray );
    d->widget.shadowOffset->setValue( 10.0 );
    d->widget.shadowAngle->setValue( 45.0 );
    d->widget.shadowAngle->setMinimum( 0.0 );
    d->widget.shadowAngle->setMaximum( 360.0 );
    d->widget.shadowOptions->setEnabled( false );

    connect( d->widget.shadowVisible, SIGNAL(toggled(bool)), this, SLOT(visibilityChanged()) );
    connect( d->widget.shadowVisible, SIGNAL(toggled(bool)), this, SIGNAL(shadowVisibilityChanged(bool)) );
    connect( d->widget.shadowColor, SIGNAL(changed(const QColor&)), 
        this, SIGNAL(shadowColorChanged(const QColor&)));
    connect( d->widget.shadowAngle, SIGNAL(valueChanged(double,bool)), this, SLOT(offsetChanged()));
    connect( d->widget.shadowOffset, SIGNAL(valueChangedPt(double)), this, SLOT(offsetChanged()));
}

KoShadowConfigWidget::~KoShadowConfigWidget()
{
    delete d;
}

void KoShadowConfigWidget::setShadowColor( const QColor &color )
{
    d->widget.shadowColor->blockSignals(true);
    d->widget.shadowColor->setColor( color );
    d->widget.shadowColor->blockSignals(false);
}

QColor KoShadowConfigWidget::shadowColor() const
{
    return d->widget.shadowColor->color();
}

void KoShadowConfigWidget::setShadowOffset( const QPointF &offset )
{
    qreal length = sqrt( offset.x()*offset.x() + offset.y()*offset.y() );
    qreal angle = atan2( -offset.y(), offset.x() );
    if( angle < 0.0 )
        angle += 2*M_PI;

    d->widget.shadowAngle->blockSignals(true);
    d->widget.shadowAngle->setValue( angle * 180.0 / M_PI );
    d->widget.shadowAngle->blockSignals(false);

    d->widget.shadowOffset->blockSignals(true);
    d->widget.shadowOffset->changeValue( length );
    d->widget.shadowOffset->blockSignals(false);
}

QPointF KoShadowConfigWidget::shadowOffset() const
{
    QPointF offset( d->widget.shadowOffset->value(), 0 );
    QMatrix m;
    m.rotate( 360-d->widget.shadowAngle->value() );
    return m.map( offset );
}

void KoShadowConfigWidget::setShadowVisible( bool visible )
{
    d->widget.shadowVisible->blockSignals(true);
    d->widget.shadowVisible->setChecked(visible);
    d->widget.shadowVisible->blockSignals(false);
    visibilityChanged();
}

bool KoShadowConfigWidget::shadowVisible() const
{
    return d->widget.shadowVisible->isChecked();
}

void KoShadowConfigWidget::visibilityChanged()
{
    d->widget.shadowOptions->setEnabled( d->widget.shadowVisible->isChecked() );
}

void KoShadowConfigWidget::offsetChanged()
{
    emit shadowOffsetChanged( shadowOffset() );
}

void KoShadowConfigWidget::setUnit( const KoUnit &unit )
{
    d->widget.shadowOffset->setUnit( unit );
}

#include "KoShadowConfigWidget.moc"
