/*
 *  blendchooser.cc - part of KImageShop
 *
 *  A Combobox showing all available blendings for KImageShop
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "blendchooser.h"

BlendChooser::BlendChooser( QWidget *parent, const char *name )
  : QComboBox( parent, name )
{
    insertStringList( KisConfig::blendings(), 0 );
    connect( this, SIGNAL( activated( int ) ), 
        SLOT( slotBlendChanged( int ) ) );
}

BlendChooser::~BlendChooser()
{
}

KisConfig::Blending BlendChooser::currentBlending() const
{
    return (KisConfig::Blending) currentItem();
}

void BlendChooser::setCurrentBlending( KisConfig::Blending blending )
{
    setCurrentItem( blending );
}

void BlendChooser::slotBlendChanged( int item )
{
    emit blendingActivated( (KisConfig::Blending) item );
}

#include "blendchooser.moc"
