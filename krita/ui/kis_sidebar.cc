/*
 *  kis_sidebar.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
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

KisSideBar::KisSideBar( QWidget* parent, const char* name ) : super(parent, name)
{
    kdDebug() << "KisSideBar::KisSideBar" << endl;

    m_pControlFrame  = new ControlFrame(this);

    m_pColorChooserFrame    = new ColorChooserFrame(this);

    // krayon box
    m_dockFrame = new DockFrame(this);

    // fixed width for sidebar itself.  When free-floating,
    // there should be no fixed width
//    setFixedWidth( 200 );
    QRect rc = geometry();

    rc.setWidth(200);
    setGeometry(rc);

    // connect chooser frame
    connect(m_pColorChooserFrame, SIGNAL(colorChanged(const KoColor &)),
        this, SLOT(slotColorChooserColorSelected(const KoColor &)));

    // connect control frame
    connect(m_pControlFrame, SIGNAL(fgColorChanged(const KoColor &)),
        this, SLOT(slotControlFGColorSelected(const KoColor &)));
    connect(m_pControlFrame, SIGNAL(bgColorChanged(const KoColor &)),
        this, SLOT(slotControlBGColorSelected(const KoColor &)));
    connect(m_pControlFrame, SIGNAL(activeColorChanged(ActiveColor)),
        this, SLOT(slotControlActiveColorChanged(ActiveColor)));

    kdDebug() << "KisSideBar::KisSideBar leaving" << endl;
}

KisSideBar::~KisSideBar()
{
}

void KisSideBar::plug (QWidget* w)
{
	m_dockFrame -> plug(w);
}

void KisSideBar::unplug(QWidget *w)
{
	m_dockFrame -> unplug(w);
}

QWidget *KisSideBar::dockFrame()
{
	return m_dockFrame;
}

void KisSideBar::resizeEvent ( QResizeEvent * )
{
//    int topTitleFrameHeight = 20;
    int controlHeight = 42;
//    int topColorFrameHeight = 18;
    int colorChooserHeight = 152; //m_pColorChooserFrame->isVisible() ? 152 : 0;

    int total = 0;

//    m_pTopTitleFrame->setGeometry( 0, 0, width(), topTitleFrameHeight );
//    total += topTitleFrameHeight;

    m_pControlFrame->setGeometry( 0, total, width(), controlHeight);
    total += m_pControlFrame -> height();

//    m_pTopColorFrame->setGeometry( 0, total, width(), topColorFrameHeight );
//    total += topColorFrameHeight;

    m_pColorChooserFrame->setGeometry( 0, total, width(), colorChooserHeight );
    total += colorChooserHeight;

    m_dockFrame->setGeometry( 0, total, width(), height() - total);
}

void KisSideBar::closeEvent ( QCloseEvent * )
{
    setDocked(true);
}

void KisSideBar::slotSetFGColor(const KoColor& c)
{
    m_pColorChooserFrame->slotSetFGColor( c );
    m_pControlFrame->slotSetFGColor( c );
}

void KisSideBar::slotSetBGColor(const KoColor& c)
{
    m_pColorChooserFrame->slotSetBGColor( c );
    m_pControlFrame->slotSetBGColor( c );
}

void KisSideBar::slotColorChooserColorSelected(const KoColor& c)
{
    if (m_pControlFrame->activeColor() == ac_Foreground)
	{
	    m_pControlFrame->slotSetFGColor(c);
	    emit fgColorChanged( c );
	}
    else
	{
	    m_pControlFrame->slotSetBGColor(c);
	    emit bgColorChanged( c );
	}
}

void KisSideBar::slotControlActiveColorChanged(ActiveColor s)
{
  m_pColorChooserFrame->slotSetActiveColor(s);
}

void KisSideBar::slotControlFGColorSelected(const KoColor& c)
{
  m_pColorChooserFrame->slotSetFGColor(c);
  emit fgColorChanged( c );
}

void KisSideBar::slotControlBGColorSelected(const KoColor& c)
{
  m_pColorChooserFrame->slotSetBGColor(c);
  emit bgColorChanged( c );
}

void KisSideBar::slotSetBrush(KoIconItem *item)
{
	m_pControlFrame -> slotSetBrush(item);
}

void KisSideBar::slotSetPattern(KoIconItem *item)
{
	m_pControlFrame -> slotSetPattern(item);
}



void KisSideBar::slotHideChooserFrame( )
{
    if(m_pColorChooserFrame->isVisible())
        m_pColorChooserFrame->hide();
    else
        m_pColorChooserFrame->show();

    resizeEvent(0L);
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
	    emit activeColorChanged(ac_Foreground);
    else
	    emit activeColorChanged(ac_Background);
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
    m_pColorButton->setForeground( c.color() );
}

void ControlFrame::slotSetBGColor(const KoColor& c)
{
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

/*
    Chooser Frame - contains color selectors and sliders for the
    different color modes
*/

ColorChooserFrame::ColorChooserFrame( QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    setFrameStyle(Panel | Raised);
    setLineWidth(1);

    m_pColorChooser = new KoColorChooser(this, "Sidebar color chooser");

    connect(m_pColorChooser, SIGNAL(colorChanged(const KoColor &)),
        this, SLOT(slotColorSelected(const KoColor &)));
}

void ColorChooserFrame::slotSetActiveColor( ActiveColor a )
{
    KoColor c;

    if (a == ac_Foreground)
	    c = m_fg;
    else
	    c = m_bg;

    m_pColorChooser->slotChangeColor(c);
}

void ColorChooserFrame::resizeEvent ( QResizeEvent * )
{
    m_pColorChooser->setGeometry ( 2, 2, width()-4, height()-4 );
}

void ColorChooserFrame::slotSetFGColor(const KoColor& c)
{
    m_fg = c;
    m_pColorChooser->slotChangeColor(c);
}

void ColorChooserFrame::slotSetBGColor(const KoColor& c)
{
    m_bg = c;
    m_pColorChooser->slotChangeColor(c);
}

void ColorChooserFrame::slotColorSelected(const KoColor& c)
{
    emit colorChanged( c );
}


/*
    Dock Frame - contains tabs for brushes, layers, channels
*/

DockFrame::DockFrame(QWidget *parent, const char *name) : super(parent, name)
{
	setFrameStyle(Panel | Raised);
	setLineWidth(1);
	m_wlst.setAutoDelete(true);
	m_blst.setAutoDelete(true);
}

void DockFrame::plug(QWidget* w)
{
	if (w) {
		QString name = w -> caption();

		m_wlst.append(w);
		w -> reparent(this, QPoint(0, 0), true);

		KoFrameButton* btn = new KoFrameButton(this);

		btn -> setText(i18n(name.utf8()));
		btn -> setFixedHeight(18);
		btn -> setToggleButton(true);

		connect(btn, SIGNAL(clicked(const QString&)), this, SLOT(slotActivateTab(const QString&)));
		m_blst.append(btn);
		slotActivateTab(name);
	}
}

void DockFrame::unplug(QWidget* w)
{
	if (w) {
		w -> reparent(0, WStyle_StaysOnTop, mapToGlobal(QPoint(0,0)), true);
		w -> setActiveWindow();
	}
}


void DockFrame::slotActivateTab(const QString& tab)
{
    QWidget *w;
    for ( w = m_wlst.first(); w != 0; w = m_wlst.next() )
	{
	    if (w->caption() == tab)
		    w->show();
	    else
		    w->hide();
	}

    KoFrameButton *b;
    for ( b = m_blst.first(); b != 0; b = m_blst.next() )
	    b->setOn(b->text() == tab);
}

void DockFrame::resizeEvent( QResizeEvent * )
{
    int bw = 0;
    int row = 0;

    KoFrameButton *b;

    for ( b = m_blst.first(); b != 0; b = m_blst.next() )
	{
	    if (bw + b->width() >= width())
		{
		    bw = 0;
		    row++;
		}
	    b->move(bw, row*18);
	    bw += b->width();
	}

    QWidget *w;

    int xw = 18 + row*18;

    for ( w = m_wlst.first(); w != 0; w = m_wlst.next() )
	    w->setGeometry(2, xw, width()-4, height()- xw-2);
}

#include "kis_sidebar.moc"

