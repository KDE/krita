/*
 *  kis_colordocker.h - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Sven Langkamp  <longamp@reallygood.de>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef __kis_colordocker_h__
#define __kis_colordocker_h__

#include <qframe.h>
#include <qtabwidget.h>

#include <koColor.h>

#include "kis_basedocker.h"

class KisRGBWidget;
class KisHSVWidget;
class KisGrayWidget;

class ColorDocker : public BaseDocker 
{
        Q_OBJECT

public:
        ColorDocker(QWidget *parent = 0, const char *name = 0);
        ~ColorDocker();

public slots:
        void slotSetFGColor(const KoColor& c);
        void slotSetBGColor(const KoColor& c);

signals:   
        void fgColorChanged(const KoColor& c);
        void bgColorChanged(const KoColor& c);

protected slots:
        void slotFGColorSelected(const KoColor& c);
        void slotBGColorSelected(const KoColor& c);
        void slotCurrentChanged(QWidget*);

private:
        QTabWidget *m_tabwidget;
        KisRGBWidget *m_rgbChooser;
        KisHSVWidget *m_hsvChooser;
        KisGrayWidget *m_grayChooser;
        KoColor m_fgColor;
        KoColor m_bgColor;
};

#endif //__kis_colordocker_h__
