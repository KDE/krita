/*
 *  colorframe.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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

#include <qpainter.h>
#include <kdebug.h>

#include "colorframe.h"


ColorFrame::ColorFrame(QWidget *parent, int _colorFrameType) 
    : QFrame(parent)
{
    setFrameStyle(Panel | Sunken);
    setBackgroundMode(NoBackground);

    // default values
    m_c1 = QColor(0, 0, 0);
    m_c2 = QColor(255, 255, 255);

    m_Hue = 128;
    m_Saturation = 128;
    m_Value = 128;
    
    m_colorChanged = false;
    m_pixChanged = false;
    m_dragging = false;
    
    m_ColorFrameType = _colorFrameType;
}

const QColor ColorFrame::colorAt (const QPoint& p)
{
    if (m_pixChanged)
	{
	    m_pmImage = m_pm.convertToImage();
	    m_pixChanged = false;
	}
  
    // this shouldn't happen
    if (p.x() >= m_pm.width() || p.y() >= m_pm.height())
	    return QColor(255,255,255);
  
    return QColor(m_pmImage.pixel(p.x(), p.y()));
}


void ColorFrame::slotSetColor1(const QColor& c)
{
    if(m_c1 != c)
    {
        m_c1 = c;
        m_colorChanged = true;
        m_pixChanged = true;
        repaint();
    }    
}

void ColorFrame::slotSetColor2(const QColor& c)
{
    if(m_c2 != c)
    {
        m_c2 = c;
        m_colorChanged = true;
        repaint();
    }    
}

void ColorFrame::slotSetHue(int _hue)
{
    if(m_Hue != _hue)
    {
        m_Hue  = _hue;
        m_colorChanged = true;
        repaint();
    }    
}

void ColorFrame::slotSetSaturation(int _sat)
{
    if(m_Saturation != _sat)
    {
        m_Saturation  = _sat;
        m_colorChanged = true;
        repaint();
    }    
}

void ColorFrame::slotSetValue(int _val)
{
    if(m_Value != _val)
    {
        m_Value  = _val;
        m_colorChanged = true;
        repaint();
    }    
}

void ColorFrame::drawHueFrame(QPixmap *pixmap)
{
	int xSize = pixmap->width(), ySize = pixmap->height();
	QImage image( xSize, ySize, 32 );
	QColor col;
	int h, s;
	uint *p;

	for ( s = ySize-1; s >= 0; s-- )
	{
		p = (uint *) image.scanLine( ySize - s - 1 );
		for( h = 0; h < xSize; h++ )
		{
            // use constant saturation and value 
			col.setHsv( 359*h/(xSize-1), 255, 192 ); 
			*p = col.rgb();
			p++;
		}
	}

	pixmap->convertFromImage( image );
}


void ColorFrame::drawSaturationFrame( QPixmap *pixmap)
{
	int xSize = pixmap->width(), ySize = pixmap->height();
	QImage image( xSize, ySize, 32 );
	QColor col;
	uint *p;
	QRgb rgb;

    for ( int v = 0; v < ySize; v++ )
	{
		p = (uint *) image.scanLine( ySize - v - 1 );

		for( int x = 0; x < xSize; x++ )
		{
            // use hue and value, vary saturation with x position
 			col.setHsv( m_Hue, 255*x/(xSize-1), m_Value );
			rgb = col.rgb();
			*p++ = rgb;
		}
    }

	pixmap->convertFromImage( image );
}


void ColorFrame::drawValueFrame( QPixmap *pixmap)
{
	int xSize = pixmap->width(), ySize = pixmap->height();
	QImage image( xSize, ySize, 32 );
	QColor col;
	uint *p;
	QRgb rgb;

    for ( int v = 0; v < ySize; v++ )
	{
		p = (uint *) image.scanLine( ySize - v - 1 );

		for( int x = 0; x < xSize; x++ )
		{
            // use hue and saturation, vary value with x pos
			col.setHsv( m_Hue, m_Saturation, 255*x/(xSize-1) );
			rgb = col.rgb();
			*p++ = rgb;
		}
    }

	pixmap->convertFromImage( image );
}



void ColorFrame::drawContents(QPainter *p)
{
    QRect r = contentsRect();

    if ((m_pm.size() != r.size()) || m_colorChanged)
	{
        m_pm.resize(r.width() + 1, r.height() + 1);    

        switch(m_ColorFrameType)
        {
            case 0: // normal 2 color rgb gradient
	            KPixmapEffect::gradient(m_pm, m_c1, m_c2, 
                    KPixmapEffect::HorizontalGradient);
                break;

            case 1: // hue gradient - fixed colors
                // this pixmap is constant - only create image
                // if it's never been drawn before - when widget
                // is first shown. It's the only one like that.
                if(!m_pixChanged)
                    drawHueFrame(&m_pm);
                break;               

            case 2:  // saturation gradient
                drawSaturationFrame(&m_pm);
                break;
                
            case 3:  // value gradient
                drawValueFrame(&m_pm);            
                break;

            default:
	            KPixmapEffect::gradient(m_pm, m_c1, m_c2, 
                    KPixmapEffect::HorizontalGradient);
                break;
       }

       m_colorChanged = false;
       m_pixChanged = true;
	}
  
    p->drawPixmap(r.left(), r.top(), m_pm);
}

/*
    mouse press event - only select color on mouse press
*/
void ColorFrame::mousePressEvent (QMouseEvent *e)
{
    if (e->button() & LeftButton)
    {
	    emit clicked(e->pos());

	    m_dragging = true;
	    QPoint pos = QPoint(e->pos().x() - contentsRect().left(),
						  e->pos().y() - contentsRect().top());

	    if (pos.x() < 0)
		    pos.setX(0);
	    else if (pos.x() >= contentsRect().width())
		    pos.setX(contentsRect().width()-1);

	    if (pos.y() < 0)
		    pos.setY(0);
	    else if (pos.y() >= contentsRect().height())
		    pos.setY(contentsRect().height()-1);

	    QColor c = colorAt(pos);
	    emit colorSelected(c);
	}
    else
	    QFrame::mousePressEvent(e);
}


void ColorFrame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() & LeftButton)
	    m_dragging = false;
    else
        QFrame::mouseReleaseEvent(e);
}


void ColorFrame::mouseMoveEvent(QMouseEvent *e)
{
// too many signals - jwc
#if 0
    if (m_dragging)
    {
	    bool set = false;
	    int x = e->pos().x();
	    int y = e->pos().y();
	  
	    int left = contentsRect().left();
	    int right = contentsRect().left() + contentsRect().width();
	    int top = contentsRect().top();
	    int bottom =  contentsRect().top() + contentsRect().height();
	  
	    if (x < left)
		{
		    x = left;
		    set = true;
		}
	    else if (x > right)
		{
		    x = right;
		    set = true;
		}
	  
	    if (y < top)
		{
		    y = top;
		    set = true;
		}
	    else if (y > bottom)
		{
            y = bottom;
		    set = true;
		}
	  
	    if (set)
		    QCursor::setPos(mapToGlobal(QPoint(x,y)));
	  
	    QPoint pos = QPoint(x - contentsRect().left(), 
            y - contentsRect().top());
	  
        // jwc - don't emit colorSelected on mouse move -
        // way too many signals.  Use slider for this.
        
        //QColor c = colorAt(pos);
	    //emit colorSelected(c);
	}
    else
	    QFrame::mouseMoveEvent(e);  
#endif        

    QFrame::mouseMoveEvent(e);  
}

#include "colorframe.moc"
