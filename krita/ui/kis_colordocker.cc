/*
 *  kis_colordocker.cc - part of Krita
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
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>

#include <kglobalsettings.h>
#include <kdualcolorbutton.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>

#include <koColorChooser.h>
#include <koFrameButton.h>

#include "kis_colordocker.h"

//#include "kis_rgb_widget.h"
//#include "kis_hsv_widget.h"
#include "kis_gray_widget.h"
#include "kis_color_widget.h"

ColorDocker::ColorDocker( QWidget* parent, const char* name ) : BaseDocker( parent, name )
{
        setWidget( m_tabwidget = new QTabWidget( this ) );
        m_tabwidget -> setBaseSize( 200, 150 );
        
       // m_rgbChooser = new KisRGBWidget(this);
   //     m_hsvChooser = new KisHSVWidget(this);
        m_grayChooser = new KisGrayWidget(this);
      //  m_tabwidget-> addTab( m_rgbChooser, i18n("RGB"));
     //   m_tabwidget-> addTab( m_hsvChooser, i18n("HSV"));
        m_tabwidget-> addTab( m_grayChooser, i18n("Gray"));

        connect(m_tabwidget, SIGNAL(currentChanged(QWidget*)), SLOT(slotCurrentChanged(QWidget*)));
        
        // connect chooser frames
    //    connect(m_rgbChooser, SIGNAL(fgColorChanged(const KoColor &)), SLOT(slotFGColorSelected(const KoColor &)));
    //    connect(m_hsvChooser, SIGNAL(fgColorChanged(const KoColor &)), SLOT(slotFGColorSelected(const KoColor &)));
        connect(m_grayChooser, SIGNAL(fgColorChanged(const KoColor &)), SLOT(slotFGColorSelected(const KoColor &)));
  
      //  connect(m_rgbChooser, SIGNAL(bgColorChanged(const KoColor &)), SLOT(slotBGColorSelected(const KoColor &)));
      //  connect(m_hsvChooser, SIGNAL(bgColorChanged(const KoColor &)), SLOT(slotBGColorSelected(const KoColor &)));
        connect(m_grayChooser, SIGNAL(bgColorChanged(const KoColor &)), SLOT(slotBGColorSelected(const KoColor &))); 
 
}

ColorDocker::~ColorDocker()
{
        delete m_tabwidget;
}


void ColorDocker::slotSetFGColor(const KoColor& c)
{
        KisColorWidget *current = static_cast<KisColorWidget*>(m_tabwidget -> currentPage());
        current -> slotSetFGColor(c);
        m_fgColor = c;
}

void ColorDocker::slotSetBGColor(const KoColor& c)
{
        KisColorWidget *current = static_cast<KisColorWidget*>(m_tabwidget -> currentPage());
        current -> slotSetBGColor(c);
        m_bgColor = c;
}

void ColorDocker::slotFGColorSelected(const KoColor& c)
{
        emit fgColorChanged(c);
        m_fgColor = c;

}

void ColorDocker::slotBGColorSelected(const KoColor& c)
{
        emit bgColorChanged(c);
        m_bgColor = c;
}

void ColorDocker::slotCurrentChanged(QWidget *w)
{
        KisColorWidget *current = static_cast<KisColorWidget*>(w);
        current -> slotSetFGColor(m_fgColor);
        current -> slotSetBGColor(m_bgColor);
}       

#include "kis_colordocker.moc"
