/*
 * kis_cms.h - part of Krita
 *
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

#ifndef __KIS_CMS_H_
#define __KIS_CMS_H_

#include LCMS_HEADER

#include <qptrdict.h>
#include <qstring.h>

/**
 * KisCMS is a wrapper around the little CMS library. It loads
 * a set of ICM profiles and can convert any KisPaintDevice
 * or QImage to another.
 *
 * XXX: Krita should be able to load the embedded profiles of PNG, TIFF
 * or JPG images. Perhaps ImageMagick can help us here.
 */

/**
 * A simple combination of a CMS colortype and a pointer to enough bytes
 * to interpret as one color.
 */
struct KisColor {
	Q_UINT32 type; // littleCMS Colour type
	Q_UINT8 * value; // Pointer to color channel values. Note that a 
                         // channel need not be a QUANTUM, but might be bigger.
};

/**
 * Manage CMS settings and perform transforms.
 */
class KisCMS {

public:

	KisCMS();

	virtual ~KisCMS();


// 	/**
// 	 * Get a vector with all available profiles
// 	 */
// 	vCMProfilesSP getCMSProfiles();



private:

	QPtrDict<cmsHTRANSFORM> vTransforms;
        QPtrDict<cmsHPROFILE> vProfiles;


	QString inputProfile;
	QString monitorProfile;
	QString printerProfile;

	bool CMSinUse;
	bool SoftProofOn;
	bool GamutCheck;
	bool BlackPoint;

};


#endif // KIS_CMS_H_
