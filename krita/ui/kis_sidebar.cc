/*
 *  kis_sidebar.cc - part of Krita
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

#include "kis_sidebar.h"

#include "kis_iconwidget.h"
#include "kis_gradientwidget.h"
#include "kis_previewwidget.h"
#include "kis_brush.h"
#include "kis_pattern.h"

BaseDocker::BaseDocker( QWidget* parent, const char* name) : QDockWindow( QDockWindow::OutsideDock, parent ,name )
{
        setCloseMode( QDockWindow::Always );
}

ControlFrame::ControlFrame( QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    /*
    QString defaultPattern = getenv("KDEDIR") + QString("/")
        + KStandardDirs::kde_default("data")
        + "krayon/patterns/wizard.png";
    */

    setFrameStyle(Panel | Raised);
    setLineWidth(1);

    m_pColorButton = new KDualColorButton(this);
    m_pBrushWidget = new KisIconWidget(this);
    m_pPatternWidget = new KisIconWidget(this);
    m_pGradientWidget = new KisGradientWidget(this);
    connect(m_pColorButton, SIGNAL(fgChanged(const QColor &)), this,
	    SLOT(slotFGColorSelected(const QColor &)));

    connect(m_pColorButton, SIGNAL(bgChanged(const QColor &)), this,
	    SLOT(slotBGColorSelected(const QColor &)));

    connect(m_pColorButton, SIGNAL(currentChanged(KDualColorButton::DualColor)),
        this, SLOT(slotActiveColorChanged(KDualColorButton::DualColor )));
}

ActiveColor ControlFrame::activeColor()
{
    if (m_pColorButton->current() == KDualColorButton::Foreground)
	    return ac_Foreground;
    else
	    return ac_Background;
}

void ControlFrame::slotActiveColorChanged(KDualColorButton::DualColor s)
{
    if(s == KDualColorButton::Foreground)
	    slotFGColorSelected(m_pColorButton->currentColor());
    else
	    slotBGColorSelected(m_pColorButton->currentColor());
}

void ControlFrame::slotSetBrush(KoIconItem *item)
{
	if (item)
		m_pBrushWidget -> slotSetItem(*item);
}

void ControlFrame::slotSetPattern(KoIconItem *item)
{
	if (item)
		m_pPatternWidget -> slotSetItem(*item);
}

void ControlFrame::resizeEvent ( QResizeEvent * )
{
    int iw = 34;
    int sp = (width() - iw * 5)/6;
    int x = sp;

    m_pColorButton->setGeometry( x, 4, iw, iw );
    x += (sp + iw);
    m_pBrushWidget->setGeometry( x, 4, iw, iw );
    x += (sp + iw);
    m_pPatternWidget->setGeometry(x, 4, iw, iw );
    x += (sp + iw);
    m_pGradientWidget->setGeometry(x, 4, iw, iw );
}

void ControlFrame::slotSetFGColor(const KoColor& c)
{
    m_pColorButton->setCurrent(KDualColorButton::Foreground);
    m_pColorButton->setForeground( c.color() );
}

void ControlFrame::slotSetBGColor(const KoColor& c)
{
    m_pColorButton->setCurrent(KDualColorButton::Background);
    m_pColorButton->setBackground( c.color() );
}

void ControlFrame::slotFGColorSelected(const QColor& c)
{
    emit fgColorChanged( KoColor(c) );
}

void ControlFrame::slotBGColorSelected(const QColor& c)
{
    emit bgColorChanged( KoColor(c) );
}

DockFrameDocker::DockFrameDocker( QWidget* parent, const char* name ) : BaseDocker( parent, name )
{
    kdDebug() << "DockFrameDocker::DockFrameDocker" << endl;

    setWidget( m_tabwidget = new QTabWidget( this ) );
    
    m_tabwidget -> setFixedSize( 200, 250 );
    kdDebug() << "DockFrameDocker::DockFrameDocker leaving" << endl;
}

DockFrameDocker::~DockFrameDocker()
{
        delete m_tabwidget;
}

void DockFrameDocker::plug (QWidget* w)
{
        m_tabwidget-> addTab( w , w -> caption());
}

void DockFrameDocker::unplug(QWidget *w)
{
        m_tabwidget -> removePage(w);
}

ColorDocker::ColorDocker( QWidget* parent, const char* name ) : BaseDocker( parent, name )
{
        kdDebug() << "ColorDocker::ColorDocker" << endl;
        
        m_ColorChooser = new KoColorChooser(this);
        m_ColorChooser -> setFixedSize( 200, 150 );
        setWidget(m_ColorChooser);

        // connect chooser frame
        connect(m_ColorChooser, SIGNAL(colorChanged(const KoColor &)),
                this, SLOT(slotColorSelected(const KoColor &)));

        kdDebug() << "ColorDocker::ColorDocker leaving" << endl;
}

ColorDocker::~ColorDocker()
{
        delete m_ColorChooser;
}

void ColorDocker::slotSetColor(const KoColor& c)
{
        m_ColorChooser -> slotChangeColor( c );
}

void ColorDocker::slotColorSelected(const KoColor& c)
{
        emit ColorChanged( c );
}

ToolControlDocker::ToolControlDocker( QWidget* parent, const char* name ) : BaseDocker( parent, name )
{
        kdDebug() << "ToolControlDocker::ToolControlDocker" << endl;

        m_controlframe  = new ControlFrame(this);

        // connect control frame
        connect(m_controlframe, SIGNAL(fgColorChanged(const KoColor &)),
                this, SLOT(slotControlFGColorSelected(const KoColor &)));
        connect(m_controlframe, SIGNAL(bgColorChanged(const KoColor &)),
                this, SLOT(slotControlBGColorSelected(const KoColor &)));
        connect(m_controlframe, SIGNAL(activeColorChanged(ActiveColor)),
                this, SLOT(slotControlActiveColorChanged(ActiveColor)));
                
        m_controlframe -> setFixedSize(200,50);
        setWidget(m_controlframe);

        kdDebug() << "ToolControlDocker::ToolControlDocker leaving" << endl;
}

ToolControlDocker::~ToolControlDocker()
{
        delete m_controlframe;
}

void ToolControlDocker::slotSetFGColor(const KoColor& c)
{
        m_controlframe->slotSetFGColor( c );
}

void ToolControlDocker::slotSetBGColor(const KoColor& c)
{
        m_controlframe->slotSetBGColor( c );
}

void ToolControlDocker::slotColorSelected(const KoColor& c)
{
        if (m_controlframe->activeColor() == ac_Foreground)
        {
                m_controlframe->slotSetFGColor( c );
                emit fgColorChanged( c );
        }
        else
        {        
                m_controlframe->slotSetBGColor( c );
                emit bgColorChanged( c );
        }
}

void ToolControlDocker::slotControlFGColorSelected(const KoColor& c)
{
        emit fgColorChanged(c);
}

void ToolControlDocker::slotControlBGColorSelected(const KoColor& c)
{
        emit bgColorChanged(c);
}

void ToolControlDocker::slotSetBrush(KoIconItem *item)
{
        m_controlframe -> slotSetBrush(item);
}

void ToolControlDocker::slotSetPattern(KoIconItem *item)
{
        m_controlframe -> slotSetPattern(item);
}

#include "kis_sidebar.moc"

