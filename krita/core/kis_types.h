/*
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
#if !defined KISTYPES_H_
#define KISTYPES_H_

#include <qvaluevector.h>
#include <ksharedptr.h>
#include <koColor.h>

class KisPaintDeviceObserverInterface;
typedef KSharedPtr<KisPaintDeviceObserverInterface> KisPaintDeviceObserverSP;

class KisPaintDeviceSubjectInterface;
typedef KSharedPtr<KisPaintDeviceSubjectInterface> KisPaintDeviceSubjectSP;

class KisImage;
typedef KSharedPtr<KisImage> KisImageSP;
typedef QValueVector<KisImageSP> vKisImageSP;
typedef vKisImageSP::iterator vKisImageSP_it;
typedef vKisImageSP::const_iterator vKisImageSP_cit;

class KisPaintDevice;
typedef KSharedPtr<KisPaintDevice> KisPaintDeviceSP;
typedef QValueVector<KisPaintDeviceSP> vKisPaintDeviceSP;
typedef vKisImageSP::iterator vKisImageSP_it;
typedef vKisImageSP::const_iterator vKisImageSP_cit;

class KisChannel;
typedef KSharedPtr<KisChannel> KisChannelSP;
typedef QValueVector<KisChannelSP> vKisChannelSP;
typedef vKisChannelSP::iterator vKisChannelSP_it;
typedef vKisChannelSP::const_iterator vKisChannelSP_cit;

class KisMask;
typedef KSharedPtr<KisMask> KisMaskSP;
typedef QValueVector<KisMaskSP> vKisMaskSP;
typedef vKisMaskSP::iterator vKisMaskSP_it;
typedef vKisMaskSP::const_iterator vKisMaskSP_cit;

class KisLayer;
typedef KSharedPtr<KisLayer> KisLayerSP;
typedef QValueVector<KisLayerSP> vKisLayerSP;
typedef vKisLayerSP::iterator vKisLayerSP_it;
typedef vKisLayerSP::const_iterator vKisLayerSP_cit;

class KisTile;
typedef KSharedPtr<KisTile> KisTileSP;
typedef QValueVector<KisTileSP> vKisTileSP;
typedef vKisTileSP::iterator vKisTileSP_it;
typedef vKisTileSP::const_iterator vKisTileSPLst_cit;

class KisTileMgr;
typedef KSharedPtr<KisTileMgr> KisTileMgrSP;

class KisPixelData;
typedef KSharedPtr<KisPixelData> KisPixelDataSP;

typedef QValueVector<QPoint> vKisSegments;

class KoColor;
typedef QValueVector<KoColor> KoColorMap;

#endif // KISTYPES_H_

