/*
 *  kis_layer.h - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __kis_layer_h__
#define __kis_layer_h__

#include <qimage.h>
#include <qobject.h>
#include <qptrlist.h>

#include <koColor.h>

#include "kis_channel.h"
#include "kis_global.h"
#include "kis_paint_device.h"

class KisLayer;

typedef KSharedPtr<KisLayer> KisLayerSP;
typedef QValueVector<KisLayerSP> KisLayerSPLst;
typedef KisLayerSPLst::iterator KisLayerSPLstIterator;
typedef KisLayerSPLst::const_iterator KisLayerSPLstConstIterator;

class KisLayer : public KisPaintDevice {
	Q_OBJECT
	typedef KisPaintDevice super;

public:
	KisLayer(const QString& name, uint width, uint height, uint bpp, cMode cm, const QRgb& defaultColor);
	virtual ~KisLayer();

	bool    linked()  const { return m_linked; }
	cMode   colorMode()   const { return m_cMode; }

	void    setLinked(bool l)  { m_linked = l; }

	int     channelLastTileOffsetX() const;
	int     channelLastTileOffsetY() const;
	QRect   tileRect(int tileNo);

	// information about where the layer rectange (not the
	// entire image rectangle) is in canvas coords -jwc-

	void    loadRGBImage(QImage img, QImage alpha);
	void    loadGrayImage(QImage img, QImage alpha);

	bool    boundryTileX(int tile) const;
	bool    boundryTileY(int tile) const;
    
signals:
	void layerPropertiesChanged();

public:
	uchar    m_current;
	bool     m_linked;
	uchar    m_bitDepth;
    
	QRect    m_imgRect;
};

#endif // __kis_layer_h__

