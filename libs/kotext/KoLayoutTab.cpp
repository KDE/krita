/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include "KoLayoutTab.h"
#include "KoTextFormat.h"

#include <knuminput.h>

#include <q3buttongroup.h>
#include <QCheckBox>
#include <QSpinBox>

#include "KoLayoutTab.moc"

KoLayoutTab::KoLayoutTab( bool withSubSuperScript, QWidget* parent, const char* name, Qt::WFlags fl ) 
        : KoLayoutTabBase( parent, name, fl )
{
    if ( !withSubSuperScript ) positionButtonGroup->hide();

    connect( positionButtonGroup, SIGNAL( clicked( int ) ), this, SLOT( slotSubSuperScriptChanged( int ) ) );
    connect( offsetSpinBox, SIGNAL( valueChanged( int ) ), this, SIGNAL( offsetChanged( int ) ) );
    connect( relativeSizeKDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( slotRelativeSizeChanged( double ) ) );
    connect( hyphenateCheckBox, SIGNAL( toggled( bool ) ), this, SIGNAL( hyphenationChanged( bool ) ) );
}

KoLayoutTab::~KoLayoutTab()
{
}

KoTextFormat::VerticalAlignment KoLayoutTab::getSubSuperScript() const
{
    switch ( positionButtonGroup->selectedId() )
    {
    case 0:
        return KoTextFormat::AlignNormal;
    case 1:
        return KoTextFormat::AlignSubScript;
    case 2:
        return KoTextFormat::AlignSuperScript;
    case 3:
        return KoTextFormat::AlignCustom;
    default:
        return KoTextFormat::AlignNormal;
    }
}

int KoLayoutTab::getOffsetFromBaseline() const
{
    switch ( positionButtonGroup->selectedId() )
    {
    case 0:
        return 0;
    case 1:
        return 0;	// subscript has a default
    case 2:
        return 0;	// superscript has got a default
    case 3:
        return offsetSpinBox->value();
    default:
        return 0;
    }
}

double KoLayoutTab::getRelativeTextSize() const
{
    switch ( positionButtonGroup->selectedId() )
    {
    case 0:
        return 1.0;
    case 1:
        return relativeSizeKDoubleSpinBox->value() / 100;
    case 2:
        return relativeSizeKDoubleSpinBox->value() / 100;
    case 3:
        return relativeSizeKDoubleSpinBox->value() / 100;
    default:
        return 1.0;
    }
}

bool KoLayoutTab::getAutoHyphenation() const
{
    return hyphenateCheckBox->isOn();
}

void KoLayoutTab::setSubSuperScript( KoTextFormat::VerticalAlignment subSuperScript, int offset, double relativeSize )
{
    switch ( static_cast< int >( subSuperScript ) )
    {
    case 0:
        positionButtonGroup->setButton( 0 );
        break;
    case 1:
        positionButtonGroup->setButton( 1 );
        relativeSizeKDoubleSpinBox->setValue( relativeSize * 100 );
        break;
    case 2:
        positionButtonGroup->setButton( 2 );
        relativeSizeKDoubleSpinBox->setValue( relativeSize * 100 );
        break;
    case 3:
        positionButtonGroup->setButton( 3 );
        offsetSpinBox->setValue( offset );
        relativeSizeKDoubleSpinBox->setValue( relativeSize * 100 );
        break;
    default:
        positionButtonGroup->setButton( 0 );
        break;
    }
    slotSubSuperScriptChanged( static_cast< int >( subSuperScript ) );
}

void KoLayoutTab::setAutoHyphenation( bool state )
{
    hyphenateCheckBox->setChecked( state );
}

void KoLayoutTab::slotSubSuperScriptChanged( int item )
{
    if ( item == 1 || item == 2 ) offsetSpinBox->setValue( 0 );
    emit subSuperScriptChanged();
    if ( item ) {
        emit relativeSizeChanged( relativeSizeKDoubleSpinBox->value() / 100 );
        emit offsetChanged( offsetSpinBox->value() );
    }
}

void KoLayoutTab::slotRelativeSizeChanged( double relativeSize )
{
    emit relativeSizeChanged( relativeSize / 100 );
}
