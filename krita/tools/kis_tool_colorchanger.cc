/*
 *  kis_tool_colorchanger.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include <koColor.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_tool_colorchanger.h"

KisToolColorChanger::KisToolColorChanger() : super()
{
	setName("tool_colorchanger");
	// set custom cursor.
	setCursor(KisCursor::colorChangerCursor());

	m_subject = 0;

// 	KisView *view = getCurrentView();
// 	// initialize color changer settings
// 	m_opacity = 255;
// 	m_usePattern  = false;
// 	m_useGradient = false;

// 	toleranceRed = 0;
// 	toleranceGreen = 0;
// 	toleranceBlue = 0;

// 	layerAlpha = true;

// 	// get currentImg colors
// 	KoColor startColor( view->fgColor().R(), view->fgColor().G(), view->fgColor().B() );
// 	KoColor endColor( view->bgColor().R(), view->bgColor().G(), view->bgColor().B() );

// 	// prepare for painting with pattern
// 	if( m_usePattern )
// 		m_doc->frameBuffer()->setPattern( view->currentPattern() );

// 	// prepare for painting with gradient
// 	m_doc->frameBuffer()->setGradientPaint( m_useGradient, startColor, endColor );
}

KisToolColorChanger::~KisToolColorChanger()
{
}

bool KisToolColorChanger::changeColors(int startX, int startY)
{
// 	int startx = startX;
// 	int starty = startY;
// 	int sRed;
// 	int sGreen;
// 	int sBlue;
// 	QRgb srgb;
// 	KisView *view = getCurrentView();
// 	KisImage *img = m_doc -> currentImg();

// 	if (!img)
// 		return false;

// 	KisLayer *lay = img->getCurrentLayer();

// 	if (!lay)
// 		return false;

// 	if (!img->colorMode() == cm_RGB && !img->colorMode() == cm_RGBA)
// 		return false;

// 	layerAlpha = (img->colorMode() == cm_RGBA);
// 	fLayer = lay;

// 	// source color values of selected pixed
// 	srgb = lay -> pixel(startx, starty);
// 	sRed = qRed(srgb);
// 	sGreen = qGreen(srgb);
// 	sBlue = qBlue(srgb);

// 	// new color values from color selector
// 	nRed     = view->fgColor().R();
// 	nGreen   = view->fgColor().G();
// 	nBlue    = view->fgColor().B();

// 	int left    = lay->imageExtents().left();
// 	int top     = lay->imageExtents().top();
// 	int width   = lay->imageExtents().width();
// 	int height  = lay->imageExtents().height();

// 	QRect ur(left, top, width, height);

// 	kdDebug() << "ur.left() " << ur.left() << "ur.top() "  << ur.top() << endl;

// 	// prepare for painting with gradient
// 	if (m_useGradient) {
// 		KoColor startColor(view->fgColor().R(), view->fgColor().G(), view->fgColor().B());
// 		KoColor endColor(view->bgColor().R(), view->bgColor().G(), view->bgColor().B());

// 		m_doc->frameBuffer()->setGradientPaint(true, startColor, endColor);
// 	}

// 	// prepare for painting with pattern
// 	if (m_usePattern)
// 		m_doc->frameBuffer()->setPattern(view->currentPattern());

// 	// this does the painting
// 	if (!m_doc->frameBuffer()->changeColors(qRgba(sRed, sGreen, sBlue, m_opacity),
// 				qRgba(nRed, nGreen, nBlue, m_opacity), ur)) {
// 		kdDebug() << "error changing colors" << endl;
// 		return false;
// 	}

// 	/* refresh canvas so changes show up */
// 	img->markDirty(ur);
// 	return true;
}


void KisToolColorChanger::buttonPress(KisButtonPressEvent *e)
{
//     KisImage * img = m_doc->currentImg();
//     if (!img) return;

//     if (e->button() != QMouseEvent::LeftButton
//     && e->button() != QMouseEvent::RightButton)
//         return;

//     QPoint pos = e->pos();
//     pos = zoomed(pos);

//     if( !img->getCurrentLayer()->visible() )
//         return;

//     if( !img->getCurrentLayer()->imageExtents().contains(pos))
//         return;

//     /*  need to fill with foreground color on left click,
//     transparent on middle click, and background color on right click,
//     need another paramater or to set color here and pass in */

//     if (e->button() == QMouseEvent::LeftButton)
//         changeColors(pos.x(), pos.y());
//     else if (e->button() == QMouseEvent::RightButton)
//         changeColors(pos.x(), pos.y());
}


// void KisToolColorChanger::optionsDialog()
// {
// 	ToolOptsStruct ts;

// 	ts.usePattern       = m_usePattern;
// 	ts.useGradient      = m_useGradient;
// 	ts.opacity          = m_opacity;

// 	unsigned int old_m_opacity   = m_opacity;
// 	bool old_usePattern   = m_usePattern;
// 	bool old_useGradient  = m_useGradient;

// 	ToolOptionsDialog OptsDialog(tt_filltool, ts);

// 	OptsDialog.exec();

// 	if (OptsDialog.result() == QDialog::Rejected)
// 		return;

// 	/* the following values should be unique for each tool.
// 	   To change global tool options that will over-ride these
// 	   local ones for individual tools, we need a master tool
// 	   options tabbed dialog */

// 	m_opacity     = OptsDialog.fillToolTab()->opacity();
// 	m_usePattern      = OptsDialog.fillToolTab()->usePattern();
// 	m_useGradient     = OptsDialog.fillToolTab()->useGradient();

// 	// we need HSV tolerances even more
// 	//toleranceRed    = OptsDialog->ToleranceRed();
// 	//toleranceGreen  = OptsDialog->ToleranceGreen();
// 	//toleranceBlue   = OptsDialog->ToleranceBlue();

// 	// User change value ?
// 	if ( old_usePattern != m_usePattern || old_useGradient != m_useGradient || old_m_opacity != m_opacity ) {
// 		KisView *view = getCurrentView();
// 		// note that gradients and patterns are not associated with a
// 		// particular tool, unlike the other options

// 		// get currentImg colors
// 		KoColor startColor( view->fgColor().R(), view->fgColor().G(), view->fgColor().B() );
// 		KoColor endColor( view->bgColor().R(), view->bgColor().G(), view->bgColor().B() );

// 		// prepare for painting with pattern
// 		if( m_usePattern )
// 			m_doc->frameBuffer()->setPattern( view->currentPattern() );

// 		// prepare for painting with gradient
// 		m_doc->frameBuffer()->setGradientPaint( m_useGradient, startColor, endColor );

// 		// set color changer settings
// 		m_doc->setModified( true );
// 	}
// }


void KisToolColorChanger::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Color Changer"), 
					    "colorize", 
					    0, 
					    this, 
					    SLOT(activate()), 
					    collection, 
					    name());
		m_action-> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

// QDomElement KisToolColorChanger::saveSettings(QDomDocument& doc) const
// {
// 	// Color changer element
// 	QDomElement colorChanger = doc.createElement("colorChanger");

// 	colorChanger.setAttribute("opacity", m_opacity);
// 	colorChanger.setAttribute("fillWithPattern", static_cast<int>(m_usePattern));
// 	colorChanger.setAttribute("fillWithGradient", static_cast<int>(m_useGradient));
// 	return colorChanger;
// }

// bool KisToolColorChanger::loadSettings(QDomElement& elem)
// {
// 	bool rc = elem.tagName() == "colorChanger";

// 	if (rc) {
// 		m_opacity = elem.attribute("opacity").toInt();
// 		m_usePattern = static_cast<bool>(elem.attribute("fillWithPattern").toInt());
// 		m_useGradient = static_cast<bool>(elem.attribute("fillWithGradient").toInt());
// 	}

// 	return rc;
// }


#include "kis_tool_colorchanger.moc"
