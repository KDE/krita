/*
 *  blendchooser.h - part of KImageShop
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


#ifndef BLENDHOOSER_H
#define BLENDHOOSER_H

#include <qcombobox.h>

#include "kis_config.h"

class BlendChooser : public QComboBox
{
  Q_OBJECT

public:

  BlendChooser( QWidget* parent = 0, const char* name = 0 );
  ~BlendChooser();

  KisConfig::Blending currentBlending()	const;
  void setCurrentBlending( KisConfig::Blending );

signals:

  void blendingActivated( KisConfig::Blending );

private slots:

  void slotBlendChanged( int );
};

#endif // BLENDHOOSER_H
