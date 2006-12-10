/*
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_EXTERNAL_LAYER_IFACE_
#define KIS_EXTERNAL_LAYER_IFACE_

#include "kicon.h"

#include "kis_types.h"

#include "kis_layer.h"

class QRect;
class QString;
class QIcon;
class QDomDocument;
class QDomElement;
class QRegion;

class KoColorSpace;

class KisFilterStrategy;
class KisProgressDisplayInterface;
class KisUndoAdapter;

/**
   A base interface for layers that are implemented outside the Krita
   core.
 */
class KisExternalLayer : public KisLayer {

public:
    KisExternalLayer(KisImageSP img, const QString &name, quint8 opacity)
        : KisLayer(img, name, opacity) {}
    virtual QIcon icon() const { return KIcon("gear"); }
    virtual KisPaintDeviceSP prepareProjection(KisPaintDeviceSP projection, const QRect& r) = 0;
    virtual bool saveToXML(QDomDocument doc, QDomElement elem) = 0;


    // Visitor callbacks
    virtual bool mergeVisitorCallback( KisPaintDeviceSP projection,  const QRect & rc ) 
        { 
            Q_UNUSED(projection);
            Q_UNUSED(rc);
            return true; 
        }

    virtual bool transformVisitorCallback( double sx, double sy, qint32 tx, qint32 ty, KisFilterStrategy * filter, double angle, KisProgressDisplayInterface *progress ) 
        { 
            Q_UNUSED(sx);
            Q_UNUSED(sy);
            Q_UNUSED(tx);
            Q_UNUSED(ty);
            Q_UNUSED(filter);
            Q_UNUSED(angle);
            Q_UNUSED(progress);

            return true; 
        }
    
    
    virtual bool changeProfileVisitorCallback( KoColorSpace * oldCs,  KoColorSpace * newCs ) 
        { 
            Q_UNUSED(oldCs);
            Q_UNUSED(newCs);

            return true; 
        }
    
    
    virtual bool colorspaceConvertVisitorCallback( KoColorSpace * dstCs,  qint32 renderingIntent ) 
        { 
            Q_UNUSED(dstCs);
            Q_UNUSED(renderingIntent);

            return true; 
        }
    
    
    virtual bool cropVisitorCallback( const QRect & rc, bool moveLayers = true ) 
        { 
            Q_UNUSED(rc);
            Q_UNUSED(moveLayers);

            return true; 
        }
    
    
    virtual bool extentVisitorCallback( QRect & imageRect, QRegion & region, bool exact ) 
        { 
            Q_UNUSED(imageRect);
            Q_UNUSED(region);
            Q_UNUSED(exact);

            return true; 
        }
    
    
    virtual bool shearVisitorCallback( double xShear,  double yShear, KisProgressDisplayInterface * progress,  KisFilterStrategy * strategy, KisUndoAdapter * undo ) 
        { 
            Q_UNUSED(xShear);
            Q_UNUSED(yShear);
            Q_UNUSED(progress);
            Q_UNUSED(strategy);
            Q_UNUSED(undo);   

            return true; 
        }
};

#endif // KIS_EXTERNAL_IFACE_LAYER_IFACE_
