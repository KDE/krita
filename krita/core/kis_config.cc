/*
 *  kis_config.cc - part of Krayon
 *
 *  Global configuration classes for KImageShop
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


#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include "kis_config.h"


// define static attributes and members

bool KisConfig::doInit 	= true;
KConfig * KisConfig::kc	= 0L;
QPtrList<KisConfig> KisConfig::instanceList;
QFont KisConfig::m_smallFont;
QFont KisConfig::m_tinyFont;
QStringList KisConfig::m_blendList;


KisConfig * KisConfig::getNewConfig()
{
    if ( doInit )
    {
        return ( new KisConfig() );
    }
    else
    {
        if ( instanceList.count() == 0 )
        {
            return ( new KisConfig() );
        }

        return ( new KisConfig( *instanceList.first() ) );
    }
}


KisConfig::KisConfig() : QObject( 0L, "krita config" )
{
    // load and init global settings only once
    if ( doInit )
    {
        initStatic();
    }

    instanceList.append( this );

    // now load all the settings for the config objects
    loadConfig();
}


KisConfig::KisConfig( const KisConfig& /*config*/ )
  : QObject()
{
    instanceList.append( this );

    // ...
}


KisConfig::~KisConfig()
{
    instanceList.remove( this );

    // when the last document is closed, save the global settings
    if ( instanceList.isEmpty() )
        saveGlobalSettings();

    // ...
}


void KisConfig::initStatic()
{
    kc = KGlobal::config();

    instanceList.clear();
    instanceList.setAutoDelete( false );

    loadGlobalSettings();

    (void) m_blendList.append( i18n("Normal") );
    (void) m_blendList.append( i18n("Dissolve") );
    (void) m_blendList.append( i18n("Behind") );
    (void) m_blendList.append( i18n("Multiply") );
    (void) m_blendList.append( i18n("Screen") );
    (void) m_blendList.append( i18n("Overlay") );
    (void) m_blendList.append( i18n("Difference") );
    (void) m_blendList.append( i18n("Addition") );
    (void) m_blendList.append( i18n("Subtract") );
    (void) m_blendList.append( i18n("Darken Only") );
    (void) m_blendList.append( i18n("Lighten Only") );
    (void) m_blendList.append( i18n("Hue") );
    (void) m_blendList.append( i18n("Saturation") );
    (void) m_blendList.append( i18n("Color") );
    (void) m_blendList.append( i18n("Value") );

    doInit = false;
}


// a convenience method - load all document specific configuration
void KisConfig::loadConfig()
{
    loadDialogSettings();
    // ...
}


// save all document specific configuration
void KisConfig::saveConfig()
{
    saveDialogSettings();
    // ...
}


void KisConfig::saveAll()
{
    KisConfig *config = 0L;
    for ( config = instanceList.first(); config; config = instanceList.next() )
    {
        config->saveConfig();
    }

    saveGlobalSettings();
}


void KisConfig::loadGlobalSettings()
{
    // read some fonts
    QFont font = KGlobalSettings::generalFont();
    font.setPointSize( 10 );
    m_smallFont = kc->readFontEntry( "Small Font", &font );

    font = KGlobalSettings::generalFont();
    font.setPointSize( 8 );
    m_tinyFont = kc->readFontEntry( "Tiny Font", &font );

  // ...
}


void KisConfig::saveGlobalSettings()
{
    kc->setGroup( "General Settings" );

    kc->writeEntry( "Small Font", m_smallFont );
    kc->writeEntry( "Tiny Font", m_tinyFont );

  // ...
}


void KisConfig::loadDialogSettings()
{
  //m_pLayerDlgConfig->loadConfig( kc );
  // ...
}


void KisConfig::saveDialogSettings()
{
  //m_pLayerDlgConfig->saveConfig( kc );
  // ...
}


const QStringList& KisConfig::blendings()
{
    if ( doInit ) KisConfig::initStatic();

    return KisConfig::m_blendList;
}

// The base configuration class



void BaseKFDConfig::loadConfig( KConfig *_config )
{
  m_docked = _config->readBoolEntry( "Docked", true );
  m_posX   = _config->readUnsignedNumEntry( "PositionX", 0 );
  m_posY   = _config->readUnsignedNumEntry( "PositionY", 0 );
}

void BaseKFDConfig::saveConfig( KConfig *_config )
{
  // TODO: save the right values

  _config->writeEntry( "Docked", false );
  _config->writeEntry( "PositionX", 50 );
  _config->writeEntry( "PositionY", 50 );

  _config->sync();
}

#include "kis_config.moc"
