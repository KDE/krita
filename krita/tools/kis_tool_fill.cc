/*
 *  kis_tool_fill.cc - part of Krayon
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

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_tool_fill.h"

KisToolFill::KisToolFill() 
	: super()
{
	m_subject = 0;

	// set custom cursor.
	setCursor(KisCursor::fillerCursor());

// 	// initialize filler tool settings
// 	m_opacity = 255;
// 	m_usePattern  = false;
// 	m_useGradient = false;
// 	toleranceRed = 0;
// 	toleranceGreen = 0;
// 	toleranceBlue = 0;
// 	layerAlpha = true;
}

KisToolFill::~KisToolFill() 
{
}

// floodfill based on GPL code in gpaint by Li-Cheng (Andy) Tai
int KisToolFill::is_old_pixel_value(struct fillinfo *info, int x, int y)
{
// 	QRgb rgb = fLayer -> pixel(x, y);
// 	unsigned char o_r = qRed(rgb);
// 	unsigned char o_g = qGreen(rgb);
// 	unsigned char o_b = qBlue(rgb);

// 	return o_r == info -> o_r && o_g == info->o_g && o_b == info->o_b;
}

void KisToolFill::set_new_pixel_value(struct fillinfo *info, int x, int y)
{
// 	if(m_useGradient)
// 		m_doc -> frameBuffer() -> setGradientToPixel(fLayer, x, y);
// 	else if(m_usePattern)
// 		m_doc -> frameBuffer() -> setPatternToPixel(fLayer, x, y, 0);
// 	else
// 		fLayer -> setPixel(x, y, qRgba(info -> r, info -> g, info -> b, m_opacity));
    
// 	// alpha adjustment with either fill method
// //	if (layerAlpha)
// //		fLayer -> setPixel(3, x, y, m_opacity);
}


#define STACKSIZE 10000

/* algorithm based on SeedFill.c from GraphicsGems 1 */

void KisToolFill::flood_fill(struct fillinfo *info, int x, int y)
{
   struct fillpixelinfo stack[STACKSIZE];
   struct fillpixelinfo * sp = stack;
   int l, x1, x2, dy;
   
#define PUSH(py, pxl, pxr, pdy) \
{  struct fillpixelinfo *p = sp;\
   if (((py) + (pdy) >= info->top) && ((py) + (pdy) < info->bottom))\
   {\
      p->y = (py);\
      p->xl = (pxl);\
      p->xr = (pxr);\
      p->dy = (pdy);\
      sp++; \
   }\
}
   
#define POP(py, pxl, pxr, pdy) \
{\
   sp--;\
   (py) = sp->y + sp->dy;\
   (pxl) = sp->xl;\
   (pxr) = sp->xr;\
   (pdy) = sp->dy;\
}

   if ((x >= info->left) && (x <= info->right) 
   && (y >= info->top) && (y <= info->bottom))
   {
	   if (is_old_pixel_value(info, x, y))
		   return;

        PUSH(y, x, x, 1);
        PUSH(y + 1, x, x, -1);
      	 
        while (sp > stack)	
        {
            POP(y, x1, x2, dy);
	        for (x = x1; (x >= info->left) && is_old_pixel_value(info, x, y); x--)
	            set_new_pixel_value(info, x, y);
	        if (x >= x1) goto skip;
	        l = x + 1;
	        if (l < x1)
	            PUSH(y, l, x1 - 1, -dy);
	        x = x1 + 1;
	        do
	        {
	            for (; (x <= info->right) && is_old_pixel_value(info, x, y); x++)
	                set_new_pixel_value(info, x, y);
	    
	            PUSH(y, l, x - 1, dy);
	            if (x > x2 + 1)
	                PUSH(y, x2 + 1, x - 1, -dy);
skip:
                for (x++; x <= x2 && !is_old_pixel_value(info, x, y); x++);
	            l = x;
	        } while (x <= x2);
        }
   }

#undef POP
#undef PUSH
}   
   
void KisToolFill::seed_flood_fill(int x, int y, const QRect& frect)
{
    struct fillinfo fillinfo;
   
    fillinfo.left   = frect.left();
    fillinfo.top    = frect.top();
    fillinfo.right  = frect.right();
    fillinfo.bottom = frect.bottom();
    
    // r,g,b are color to set to   
    fillinfo.r = nRed; 
    fillinfo.g = nGreen; 
    fillinfo.b = nBlue; 

    // original color at mouse click position 
    fillinfo.o_r =  sRed; 
    fillinfo.o_g =  sGreen; 
    fillinfo.o_b =  sBlue; 
   
    flood_fill(&fillinfo, x, y);
}


bool KisToolFill::flood(int startX, int startY)
{
//     int startx = startX;
//     int starty = startY;
//     QRgb srgb;
//     KisView *view = getCurrentView();
//     KisImage *img = m_doc->currentImg();

//     if (!img) return false;    

//     KisLayer *lay = img->getCurrentLayer();
//     if (!lay) return false;

//     if (!img->colorMode() == cm_RGB && !img->colorMode() == cm_RGBA)
// 	    return false;
    
//     layerAlpha = (img->colorMode() == cm_RGBA);
//     fLayer = lay;
    
//     // source color values of selected pixed
//     srgb = lay -> pixel(startx, starty);
//     sRed = qRed(srgb);
//     sGreen = qGreen(srgb);
//     sBlue = qBlue(srgb);

//     // new color values from color selector 

//     nRed     = view->fgColor().R();
//     nGreen   = view->fgColor().G();
//     nBlue    = view->fgColor().B();
    
//     int left    = lay->imageExtents().left(); 
//     int top     = lay->imageExtents().top();    
//     int width   = lay->imageExtents().width();    
//     int height  = lay->imageExtents().height();    

//     QRect floodRect(left, top, width, height);
    
//     kdDebug() << "floodRect.left() " << floodRect.left() 
//               << " floodRect.top() "  << floodRect.top() << endl;

//     /* set up gradient - if any.  this should only be done when the
//     currentImg layer is changed or when the fgColor or bgColor are changed,
//     or when the gradient is changed with the gradient settings dialog
//     or by selecting a prexisting gradient from the chooser.
//     Otherwise, it can get slow calculating gradients for every fill
//     operation when this calculation is not needed - when the gradient
//     array is already filled with currentImg values */
    
//     if(m_useGradient)
//     {
//         KoColor startColor(view->fgColor().R(),
//             view->fgColor().G(), view->fgColor().B());
//         KoColor endColor(view->bgColor().R(),
//             view->bgColor().G(), view->bgColor().B());        
            
//         m_doc->frameBuffer()->setGradientPaint(true, startColor, endColor);        
//     }

//     seed_flood_fill( startx, starty, floodRect);
      
//     /* refresh canvas so changes show up */
//     QRect updateRect(0, 0, img->width(), img->height());
//     img->markDirty(updateRect);
//     return true;
}


void KisToolFill::mousePress(QMouseEvent *e)
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
//     need another paramater or nned to set color here and pass in */
    
//     if (e  ->button() == QMouseEvent::LeftButton)
//         flood(pos.x(), pos.y());
//     else if (e->button() == QMouseEvent::RightButton)
//         flood(pos.x(), pos.y());
}

// void KisToolFill::optionsDialog()
// {
//     ToolOptsStruct ts;    
    
//     ts.opacity          = m_opacity;
//     ts.usePattern       = m_usePattern;
//     ts.useGradient      = m_useGradient;

//     unsigned int old_opacity   = m_opacity;
//     bool old_usePattern   = m_usePattern;
//     bool old_useGradient  = m_useGradient;

//     ToolOptionsDialog OptsDialog(tt_filltool, ts);
    
//     OptsDialog.exec();
    
//     if (OptsDialog.result() == QDialog::Rejected)
// 	    return;
        
//     m_opacity     = OptsDialog.fillToolTab()->opacity();
//     m_usePattern      = OptsDialog.fillToolTab()->usePattern();
//     m_useGradient     = OptsDialog.fillToolTab()->useGradient();

//     // User change value ?
//     if (old_usePattern != m_usePattern || old_useGradient != m_useGradient || old_opacity != m_opacity)
// 	    // set filler tool settings
//             m_doc -> setModified( true );
// }


void KisToolFill::setup(KActionCollection *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Filler Tool"), 
						  "fill",
						  0, 
						  this, 
						  SLOT(activate()),
						  collection,
						  "tool_fill");

	toggle -> setExclusiveGroup("tools");
}

// QDomElement KisToolFill::saveSettings(QDomDocument& doc) const
// {
// 	// filler tool element
// 	QDomElement fillerTool = doc.createElement("fillerTool");

// 	fillerTool.setAttribute("opacity", m_opacity);
// 	fillerTool.setAttribute("fillWithPattern", static_cast<int>(m_usePattern));
// 	fillerTool.setAttribute("fillWithGradient", static_cast<int>(m_useGradient));
// 	return fillerTool;
// }

// bool KisToolFill::loadSettings(QDomElement& elem)
// {
// 	bool rc = elem.tagName() == "fillerTool";

// 	if (rc) {
// 		m_opacity = elem.attribute("opacity").toInt();
// 		m_usePattern = static_cast<bool>(elem.attribute("fillWithPattern").toInt());
// 		m_useGradient = static_cast<bool>(elem.attribute("fillWithGradient").toInt());
// 	}

// 	return rc;
// }

#include "kis_tool_fill.moc"
