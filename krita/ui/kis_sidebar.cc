/*
 *  kis_sidebar.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>

#include <kglobalsettings.h>
#include <kdualcolorbtn.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>

#include <koColorChooser.h>
#include <koFrameButton.h>

#include "kis_sidebar.h"
#include "kis_krayonwidget.h"
#include "kis_brushwidget.h"
#include "kis_patternwidget.h"
#include "kis_gradientwidget.h"
#include "kis_previewwidget.h"

//KisSideBar::KisSideBar( QWidget* parent, const char* name ) : QWidget( parent, name )
KisSideBar::KisSideBar( QWidget* parent, const char* name )
    : KFloatingDialog( parent, name )
{
    kdDebug() << "KisSideBar::KisSideBar" << endl;

    /* TopTitleFrame and Control frame are always at top of sidebar
    sidabar and are always visible (unless the whole sidebar is hidden */
    m_pTopTitleFrame = new TopTitleFrame(this);
    m_pControlFrame  = new ControlFrame(this);

    // TopColorFrame is just for selecting color chooser -
    // the ColorChooser frame can be hidden
    m_pTopColorFrame        = new TopColorFrame(this);
    m_pColorChooserFrame    = new ColorChooserFrame(this);

    // krayon box
    m_dockFrame = new DockFrame(this);

    // fixed width for sidebar itselt.  When free-floating,
    // there should be no fixed width
//    setFixedWidth( 200 );
    QRect rc = geometry();

    rc.setWidth(200);
    setGeometry(rc);

    // connect chooser frame
    connect(m_pColorChooserFrame, SIGNAL(colorChanged(const KoColor &)),
        this, SLOT(slotColorChooserColorSelected(const KoColor &)));

    // connect top frame for color modes
    connect(m_pTopColorFrame, SIGNAL(greyClicked()), m_pColorChooserFrame,
	    SLOT(slotShowGrey()));
    connect(m_pTopColorFrame, SIGNAL(rgbClicked()), m_pColorChooserFrame,
		SLOT(slotShowRGB()));
    connect(m_pTopColorFrame, SIGNAL(hsbClicked()), m_pColorChooserFrame,
		SLOT(slotShowHSB()));
    connect(m_pTopColorFrame, SIGNAL(cmykClicked()), m_pColorChooserFrame,
		SLOT(slotShowCMYK()));
    connect(m_pTopColorFrame, SIGNAL(labClicked()), m_pColorChooserFrame,
		SLOT(slotShowLAB()));
    connect(m_pTopColorFrame, SIGNAL(hideClicked()),
        this, SLOT(slotHideChooserFrame()));

    // connect control frame
    connect(m_pControlFrame, SIGNAL(fgColorChanged(const KoColor &)),
        this, SLOT(slotControlFGColorSelected(const KoColor &)));
    connect(m_pControlFrame, SIGNAL(bgColorChanged(const KoColor &)),
        this, SLOT(slotControlBGColorSelected(const KoColor &)));
    connect(m_pControlFrame, SIGNAL(activeColorChanged(ActiveColor)),
        this, SLOT(slotControlActiveColorChanged(ActiveColor)));

    kdDebug() << "KisSideBar::KisSideBar leaving" << endl;
}

void KisSideBar::resizeEvent ( QResizeEvent * )
{
    int topTitleFrameHeight = 20;
    int controlHeight = 42;
    int topColorFrameHeight = 18;
    int colorChooserHeight = m_pColorChooserFrame->isVisible() ? 152 : 0;

    int total = 0;

    m_pTopTitleFrame->setGeometry( 0, 0, width(), topTitleFrameHeight );
    total += topTitleFrameHeight;

    m_pControlFrame->setGeometry( 0, total, width(), controlHeight);
    total += controlHeight;

    m_pTopColorFrame->setGeometry( 0, total, width(), topColorFrameHeight );
    total += topColorFrameHeight;

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

void KisSideBar::slotSetKrayon(KisKrayon& k )
{
    m_pControlFrame->slotSetKrayon(k);
}

void KisSideBar::slotSetBrush( KisBrush& b )
{
    m_pControlFrame->slotSetBrush(b);
}

void KisSideBar::slotSetPattern( KisPattern& b )
{
    m_pControlFrame->slotSetPattern(b);
}

void KisSideBar::slotHideChooserFrame( )
{
    if(m_pColorChooserFrame->isVisible())
        m_pColorChooserFrame->hide();
    else
        m_pColorChooserFrame->show();

    resizeEvent(0L);
}

/*
    Top Frame - really just a selector for the color chooser to show,
    or to hide the color chooser entirely
*/

TopTitleFrame::TopTitleFrame( QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    setFrameStyle(Panel | Raised);
    setLineWidth(1);

    // setup buttons
    m_pHideButton = new KoFrameButton(i18n("(*)"), this);
    m_pTitleButton = new KoFrameButton(i18n("Krayon Box"), this);

    QFont font = KGlobalSettings::generalFont();
    font.setPointSize( 8 );

    m_pHideButton->setFont(font);
    m_pTitleButton->setFont(font);

    m_pHideButton->setToggleButton(true);
    m_pTitleButton->setToggleButton(false);

    // connect buttons
    connect(m_pHideButton, SIGNAL(clicked()), this,
		  SLOT(slotHideClicked()));

    setFrameStyle( Panel | Raised );
}

void TopTitleFrame::resizeEvent ( QResizeEvent * )
{
   //int w = width();
    m_pTitleButton->setGeometry(0, 0, 182, 20);
    m_pHideButton->setGeometry(182, 0, 18, 20);
}

void TopTitleFrame::slotHideClicked()
{
    emit hideClicked();
}


/*
    Control Frame - status display with access to
    color selector, brushes, patterns, and preview
*/

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
    m_pBrushWidget = new KisBrushWidget(this);
    m_pPatternWidget = new KisPatternWidget(this /*, defaultPattern.latin1()*/);
    m_pGradientWidget = new KisGradientWidget(this);
    m_pPreviewWidget = new KisPreviewWidget(this);

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

void ControlFrame::slotSetKrayon(KisKrayon& k)
{
    m_pKrayonWidget->slotSetKrayon(k);
}

void ControlFrame::slotSetBrush(KisBrush& b)
{
    m_pBrushWidget->slotSetBrush(b);
}

void ControlFrame::slotSetPattern(KisPattern& b)
{
    m_pPatternWidget->slotSetPattern(b);
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
    x += (sp + iw);
    m_pPreviewWidget->setGeometry(x, 4, iw, iw );
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
    Top Color Frame - really just a selector for the color chooser to show,
    or to hide the color chooser entirely
*/

TopColorFrame::TopColorFrame( QWidget* parent, const char* name )
: QFrame( parent, name )
{
#if 0
    setFrameStyle(Panel | Raised);
    setLineWidth(1);

    // setup buttons
    m_pHideButton = new KoFrameButton(i18n("(*)"), this);
    m_pLABButton = new KoFrameButton(i18n("LAB"), this);
    m_pCMYKButton = new KoFrameButton(i18n("CMYK"), this);
    m_pHSBButton = new KoFrameButton(i18n("HSV"), this);
    m_pGreyButton = new KoFrameButton(i18n("Gray"), this);
    m_pRGBButton = new KoFrameButton(i18n("RGB"), this);

    m_pEmptyFrame = new QFrame(this);
    m_pEmptyFrame->setFrameStyle(Panel | Raised);
    m_pEmptyFrame->setLineWidth(1);

    QFont font = KGlobalSettings::generalFont();
    font.setPointSize( 8 );

    m_pHideButton->setFont(font);
    m_pGreyButton->setFont(font);
    m_pRGBButton->setFont(font);
    m_pHSBButton->setFont(font);
    m_pCMYKButton->setFont(font);
    m_pLABButton->setFont(font);

    m_pHideButton->setToggleButton(true);
    m_pGreyButton->setToggleButton(true);
    m_pRGBButton->setToggleButton(true);
    m_pHSBButton->setToggleButton(true);
    m_pCMYKButton->setToggleButton(true);
    m_pLABButton->setToggleButton(true);

    // connect buttons
    connect(m_pHideButton, SIGNAL(clicked()), this,
		  SLOT(slotHideClicked()));
    connect(m_pGreyButton, SIGNAL(clicked()), this,
		  SLOT(slotGreyClicked()));
    connect(m_pRGBButton, SIGNAL(clicked()), this,
		  SLOT(slotRGBClicked()));
    connect(m_pHSBButton, SIGNAL(clicked()), this,
		  SLOT(slotHSBClicked()));
    connect(m_pCMYKButton, SIGNAL(clicked()), this,
		  SLOT(slotCMYKClicked()));
    connect(m_pLABButton, SIGNAL(clicked()), this,
		  SLOT(slotLABClicked()));

    setFrameStyle( Panel | Raised );

    // RGB is default
    m_pRGBButton->setOn(true);
#endif
}

void TopColorFrame::resizeEvent ( QResizeEvent * )
{
#if 0
    m_pRGBButton->setGeometry  (   0, 0, 30, 18 );
    m_pGreyButton->setGeometry (  30, 0, 30, 18 );
    m_pHSBButton->setGeometry  (  60, 0, 30, 18 );
    m_pCMYKButton->setGeometry (  90, 0, 36, 18 );
    m_pLABButton->setGeometry  ( 126, 0, 30, 18 );
    m_pEmptyFrame->setGeometry ( 156, 0, 26, 18 );
    m_pHideButton->setGeometry ( 182, 0, 18, 18 );
#endif
}

void TopColorFrame::slotHideClicked()
{
    emit hideClicked();
}

void TopColorFrame::slotGreyClicked()
{
    m_pCMYKButton->setOn(false);
    m_pGreyButton->setOn(true);
    m_pRGBButton->setOn(false);
    m_pHSBButton->setOn(false);
    m_pLABButton->setOn(false);

    emit greyClicked();
}

void TopColorFrame::slotRGBClicked()
{
    m_pCMYKButton->setOn(false);
    m_pGreyButton->setOn(false);
    m_pRGBButton->setOn(true);
    m_pHSBButton->setOn(false);
    m_pLABButton->setOn(false);

    emit rgbClicked();
}

void TopColorFrame::slotHSBClicked()
{
    m_pCMYKButton->setOn(false);
    m_pGreyButton->setOn(false);
    m_pRGBButton->setOn(false);
    m_pHSBButton->setOn(true);
    m_pLABButton->setOn(false);

    emit hsbClicked();
}

void TopColorFrame::slotCMYKClicked()
{
    m_pCMYKButton->setOn(true);
    m_pGreyButton->setOn(false);
    m_pRGBButton->setOn(false);
    m_pHSBButton->setOn(false);
    m_pLABButton->setOn(false);

    emit cmykClicked();
}

void TopColorFrame::slotLABClicked()
{
    m_pCMYKButton->setOn(false);
    m_pGreyButton->setOn(false);
    m_pRGBButton->setOn(false);
    m_pHSBButton->setOn(false);
    m_pLABButton->setOn(true);

    emit labClicked();
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

void ColorChooserFrame::slotShowGrey()
{
    m_pColorChooser->slotShowGrey();
}

void ColorChooserFrame::slotShowRGB()
{
    m_pColorChooser->slotShowRGB();
}

void ColorChooserFrame::slotShowHSB()
{
    m_pColorChooser->slotShowHSV();
}

void ColorChooserFrame::slotShowCMYK()
{
    m_pColorChooser->slotShowCMYK();
}

void ColorChooserFrame::slotShowLAB()
{
    m_pColorChooser->slotShowLAB();
}

void ColorChooserFrame::slotSetActiveColor( ActiveColor a )
{
	//    XXX
    //m_pColorChooser->slotSetActiveColor(a);
}

void ColorChooserFrame::resizeEvent ( QResizeEvent * )
{
    m_pColorChooser->setGeometry ( 2, 2, width()-4, height()-4 );
}

void ColorChooserFrame::slotSetFGColor(const KoColor& c)
{
	//    XXX
    //m_pColorChooser->slotSetFGColor( c.color() );
}

void ColorChooserFrame::slotSetBGColor(const KoColor& c)
{
	//    XXX
//    m_pColorChooser->slotSetBGColor( c.color() );
}

void ColorChooserFrame::slotColorSelected(const KoColor& c)
{
    emit colorChanged( c );
}


/*
    Dock Frame - contains tabs for brushes, layers, channels
*/

DockFrame::DockFrame( QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    setFrameStyle(Panel | Raised);
    setLineWidth(1);
    m_wlst.setAutoDelete(true);
    m_blst.setAutoDelete(true);
}

void DockFrame::plug (QWidget* w)
{
	if (!w) 
		return;

	QString name = w -> caption();

	m_wlst.append(w);
	w -> reparent(this, QPoint(0, 0), true);

	KoFrameButton* btn = new KoFrameButton(this);

	btn -> setText(i18n(name.latin1()));
	btn -> setFixedHeight(18);
	btn -> setToggleButton(true);

	connect(btn, SIGNAL(clicked(const QString&)), this, SLOT(slotActivateTab(const QString&)));
	m_blst.append(btn);
	slotActivateTab(name);
}

void DockFrame::unplug (QWidget* w)
{
    if(!w) return;

#if 0
    KoFrameButton *b;

    for ( b = m_blst.first(); b != 0; b = m_blst.next() )
    {
	    if (b->text() == w->caption())
		{
            QObject::disconnect( b, SIGNAL(clicked(const QString&)),
	            this, SLOT(slotActivateTab(const QString&)));

		    m_blst.remove(b);
            delete b;
		    break;
		}
	}

    m_wlst.remove(w);
    //w->reparent ( 0L, QPoint(0, 0), false );
#endif

    kdDebug() << "unplug: reparenting widget" << endl;

    w->reparent(0, WStyle_StaysOnTop, mapToGlobal(QPoint(0,0)), true);
    w->setActiveWindow();

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
