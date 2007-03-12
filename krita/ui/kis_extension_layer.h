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
#ifndef KIS_EXTENSION_LAYER_IFACE_
#define KIS_EXTENSION_LAYER_IFACE_

#include "kis_types.h"

#include "kis_external_layer_iface.h"

class QRect;
class QString;
class QIcon;
class QDomDocument;
class QDomElement;
class QRegion;

class KoColorSpace;

class KisFilterStrategy;
class KisProgressDisplayInterface;

/**
   This interface extends the KisExternalLayer interface with the
   methods that are needed for the visitor classes in krita/ui.
 */
class KisExtensionLayer : public KisExternalLayer {

public:


    bool loadVisitorCallback( KisImageSP img, KoStore * store, QMap<KisLayer *, QString> &layerFilenames, bool external )
        {
            Q_UNUSED( img );
            Q_UNUSED( store );
            Q_UNUSED( layerFilenames );
            Q_UNUSED( external );
            return true;
        }

    bool saveXMLVisitorCallback( QDomDocument doc, QDomElement element, quint32 count, bool root )
        {
            Q_UNUSED( doc );
            Q_UNUSED( element );
            Q_UNUSED( count );
            Q_UNUSED( root );

            return true;
        }

    bool saveVisitorCallback( KisImageSP img,  KoStore * store, quint32 & count )
        {
            Q_UNUSED( img );
            Q_UNUSED( store );
            Q_UNUSED( count );

            return true;
        }

    bool oasisSaveVisitorCallback( KoOasisStore * os, KoXmlWriter * writer )
        {
            Q_UNUSED( os );
            Q_UNUSED( writer );

            return true;
        }

    bool oasisSaveDataVisitorCallback( KoOasisStore * os,  KoXmlWriter * manifestWriter )
        {
            Q_UNUSED( os );
            Q_UNUSED( manifestWriter );

            return true;
        }

    bool oasisLoadVisitorCallback( KisDoc2 * doc )
        {
            Q_UNUSED( doc );

            return true;
        }

   bool oasisLoadDataVisitor( KoOasisStore * os,  const QMap<KisLayer *, QString>& filenames )
        {
            Q_UNUSED( os );
            Q_UNUSED( filenames );

            return true;
        }

};

#endif // KIS_EXTENSION_IFACE_LAYER_IFACE_
