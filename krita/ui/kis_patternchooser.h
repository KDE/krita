/*
 *  kis_patternchooser.h - part of KImageShop
 *
 *  A chooser for KisPatterns. Makes use of the KoIconChooser class and maintains
 *  all available patterns for KIS.
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2000 Matthias Elter   <elter@kde.org>
 *  Copyright (c) 2002 Patrick Julein <freak@codepimps.org>
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

#ifndef __kis_patternchooser_h__
#define __kis_patternchooser_h__

#include <qwidget.h>

#include "kis_pattern.h"

class QHBox;
class QLabel;
class KoIconChooser;
class IntegerWidget;

class KisPatternChooser : public QWidget
{
  Q_OBJECT

public:
  KisPatternChooser( QWidget *parent, const char *name = 0 );
  ~KisPatternChooser();

  KisPattern  *currentPattern();
  void 	setCurrentPattern( KisPattern * );

protected:
  void 		initGUI();

  KoIconChooser 	*chooser;

private:
    QHBox 	*frame;
    QWidget *container;
    QLabel 	*lbSpacing;
    IntegerWidget *slSpacing;

private slots:
  void 		slotItemSelected( KoIconItem * );
  void 		slotSetPatternSpacing( int );

signals:
  void 		selected( KisPattern * );

};

#endif //__kis_patternchooser_h__
