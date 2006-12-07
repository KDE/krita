/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_DUMMY_SHAPE_
#define _KIS_DUMMY_SHAPE_

#include <KoShape.h>
#include <KoShapeUserData.h>
#include <KoShapeControllerBase.h>

#include <kis_types.h>
#include <kis_image.h>

const QString KisDummyShape_SHAPEID = "KisDummyShapeID";

class KisShapeUserData : public KoShapeUserData {

public:

    KisShapeUserData( KisImageSP image )
        {
            m_image = image;
        }

    KisImageSP image() const
        {
            return m_image;
        }
private:

    KisImageSP m_image;
};

/**
   The dummy shape is a simple shape for internal use in Krita. It
   contains a reference to the KisImage object -- this way Krita tools
   can get the image from the canvas, using the KoShapeUserData route
 */
class KisDummyShape : public KoShape, public KoShapeControllerBase {

public:

    /**
       Create the dummy shape around a KisImage
    */
    KisDummyShape()
        {
            setShapeId(KisDummyShape_SHAPEID);
        }

    void setImage( KisImageSP image)
        {
            setUserData( new KisShapeUserData( image ) );
        }

    KisImageSP image() const
        {
            return static_cast<KisShapeUserData*>( userData() )->image();
        }

    QSizeF size() const { return QSizeF( 320000,320000 ); }

    /**
       Don't paint this shape: it's a mere dummy shape
    */
    virtual void paint( QPainter & painter,  const KoViewConverter & converter )
        {
            Q_UNUSED( painter );
            Q_UNUSED( converter );
        }

    /// Temporary implementation of KoShapeControllerBase, awaiting
    /// the shapification of the krita layer hierarchy
    virtual void addShape( KoShape* shape )
        {
            Q_UNUSED(shape);
            // We don't want no shapes
        }

    virtual void removeShape( KoShape* shape )
        {
            Q_UNUSED(shape);
            // You ain't gonna remove any of my kids!
        }

};


#endif // _KIS_DUMMY_SHAPE_
