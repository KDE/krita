/*
 *  kimageshop_config.h - part of KImageShop
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


#ifndef __kis_config_h__
#define __kis_config_h__

#include <qobject.h>
#include <qptrlist.h>

class QFont;

class KConfig;

class LayerDlgConfig;
class KisBrushDlgConfig;
class ColorDlgConfig;
class GradientDlgConfig;
class GradientEditorConfig;

/**
 * Global configuration class for KImageShop
 *
 * There are different types of configuration settings:
 * @li global settings, that are the same for every full @ref KImageShopView
 * @li other settings, specific to one single KImageShopView instance
 *
 * Every KImageShopView instance creates a new object of this class to hold
 * instance specific settings; global settings are shared between all of them
 *
 * Use @see #getNewConfig to create an instance of this class:
 * the first instance will load settings from disk (global settings via
 * @see #loadConfig and instance specific settings via
 * @see loadGlobalSettings), subsequent instances will get the current
 * settings from the first instance.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @version $Id$
 */
class KisConfig : public QObject
{
  Q_OBJECT

public:

  /**
   * A factory for config objects. Use it to create a new config object for
   * each new KImageShopView instance
   * Notice: make sure you delete the object when you don't need it anymore!
   */
  static KisConfig *getNewConfig();

  ~KisConfig();

  /**
   * saves all document specific settings to disk - but for this instance only
   */
  void 	saveConfig();

  /**
   * saves all global settings - called from @see #saveAll and when the last
   * instance is closed
   */
  void 	saveGlobalSettings();

  /**
   * saves all settings for ALL instances - only called from sessionmanagement
   */
  void 	saveAll();

  /**
   * loads all document specific settings for this instance from disk - called
   * from the first constructor and from sessionmanagement (when there are
   * different documents created with different settings)
   */
  void 	loadConfig();


  void 	loadDialogSettings();
  void 	saveDialogSettings();

  
  // accessors for config entries
  
  static QFont smallFont() 			{ return m_smallFont; }
  static QFont tinyFont()  			{ return m_tinyFont;  }
  static const  QStringList& blendings();
  // many more to come...

  
  typedef enum { Normal = 0,
		 Dissolve,
		 Behind,
		 Multiply,
		 Screen,
		 Overlay,
		 Difference,
		 Addition,
		 Subtract,
		 DarkenOnly,
		 LightenOnly,
		 Hue,
		 Saturation,
		 Color,
		 Value
  } Blending;

  
protected:

  /**
   * This constructor creates the first KisConfig object and loads
   * the settings from disk. Subsequent objects get these settings from this
   * object thru the copy constructor, but transparently (@see #getNewConfig)
   */
  KisConfig();

  /**
   *  A copy constructor for the KisConfig class (doh)
   *
   * Used by @see #getNewConfig to create a new KisConfig object for a
   * new KImageShopView
   */
  KisConfig( const KisConfig& );

  /**
   * This method initializes all static members
   * called only once from the very first constructor
   */
  static void 	initStatic();


signals:
  void 	globalConfigChanged();


private:

  // loads the global settings - called only once from the constructor of the
  // first instance
  static void 	loadGlobalSettings();

  
  // static objects - global items that are the same for _all_
  // KImageShopDoc objects
  static KConfig 	*kc;
  static bool		doInit;
  static QPtrList<KisConfig> 	instanceList;

  static QFont 		m_smallFont;
  static QFont		m_tinyFont;
  static QStringList 	m_blendList;


  // document specific configuration (non-static)
  //  LayerDlgConfig 	*m_pLayerDlgConfig;
  // ...
};



// separate configuration classes for all dialogs

/**
 * The interface, every configuration class must implement
 * (pure virtual class)
 */
class BaseConfig : public QObject
{
  Q_OBJECT

public:
  virtual void loadConfig( KConfig * ) = 0;
  virtual void saveConfig( KConfig * ) = 0;
};

class BaseKFDConfig : public BaseConfig
{
  Q_OBJECT

public:
  virtual void loadConfig( KConfig * );
  virtual void saveConfig( KConfig * );

private:
  bool m_docked;
  int m_posX;
  int m_posY;
};


#endif
