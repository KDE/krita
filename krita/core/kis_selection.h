/*
 *  kis_selection.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
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

#ifndef __kis_selection_h__
#define __kis_selection_h__

#include <qobject.h>
#include <qimage.h>

#include "kis_doc.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_pattern.h"
#include "kis_gradient.h"


class KisSelection : public QObject 
{
    Q_OBJECT

public:

    KisSelection(KisDoc *doc);
    ~KisSelection();

    // set up the containers 
    void setBounds(const QRect& r);
    
    // set the selection rectangle size to 0
    void setNull();

    // erase contents of the selection
    bool erase();

    // mark all the selected pixels unselected and visa-versa
    void reverse(); 

    // fill the selection with a color, gradient and pattern
    void fill(uint color, KisPattern *pattern = 0, KisGradient *gradient = 0);
    
    QImage & getImage() { return image; }
    void setImage(QImage & img);
    
    QRect & getRectangle() { return rectangle; } 
    void setRect(QRect & rect);

    // each type of selection can be set externally, typically
    // with the appropriate selection tool, but other objects
    // can also set a selection, nullify it, fill it or reverse it
    
    void setRectangularSelection(QRect & r, KisLayer *lay = 0);
    void setEllipticalSelection(QRect & r, KisLayer *lay = 0);
    void setPolygonalSelection(QRect & r, QPointArray & a, KisLayer *lay = 0);
    void setContiguousSelection(QRect & r, KisLayer *lay = 0);      

protected:

    /* the selection image is merely an image of everything in the 
    selection rectangle, taken from the layer, with non-selected 
    pixels given an alpha value of 0.  This can be determined from 
    seeing which pixels in the selection array have a value of 0. */

    QImage image;
    
    /* this rectangle bounds the selection.  In the case of a
     rectangular selection, it is equivalent to the selection.  
     For others it limits the selection. */
    
    QRect rectangle;
          
private:

    QPointArray getBundedPointArray( QPointArray & points, QPoint & topLeft );

    /* the document, image and layer for the selection
    Note that a selection can be attached to a specific
    layer and left in place in that layer, or it can be 
    the chosen selection for the entire document, which 
    then goes to the the clipboard.  Only the chosen, document-
    wide selection is used for paste operations, indirectly
    via the clipboard. Refer to KisDoc::getSelection() in 
    kis_doc.cc */
    
    KisDoc      *pDoc;
    KisImage    *pImage;
    KisLayer    *pLayer;

   /* An array of points representing every pixel in the 
   selection rectangle.  Those which are unselected have
   a value of 0.   Those selected a value of 1.  In the case 
   of elliptical, polygonal, and contiguous selections 
   not include every point in the selection rectangle is
   selected */   
   
   QMemArray <int> array; 

};

#endif

