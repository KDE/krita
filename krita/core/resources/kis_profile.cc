/*
 *  kis_profile.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <cfloat>
#include <cmath>
#include <config.h>
#include LCMS_HEADER

#include <qimage.h>
#include <qtextstream.h>

#include <kdebug.h>

#include "kis_profile.h"

#include "ksharedptr.h"

KisProfile::KisProfile(Q_UINT32 colorType)
	: super(QString()),
	  m_lcmsColorType(colorType)
{
}

KisProfile::KisProfile(const QString& file, Q_UINT32 colorType) 
	: super(file),
	  m_lcmsColorType(colorType)
{
}

KisProfile::~KisProfile()
{
	cmsCloseProfile(m_profile);
}


bool KisProfile::loadAsync()
{
	cmsErrorAction(LCMS_ERROR_IGNORE);
	

	m_profile = cmsOpenProfileFromFile(filename().ascii(), "r");

//  	kdDebug() << "loading profile: " << filename() << "\n";
	if (m_profile) {
		m_colorSpaceSignature = cmsGetColorSpace(m_profile);
// 		kdDebug() << "\tColorspaceSignature: " << m_colorSpaceSignature << "\n";

		m_deviceClass = cmsGetDeviceClass(m_profile);

		m_productName = cmsTakeProductName(m_profile);
// 		kdDebug() << "\tProduct name: " << m_productName << "\n";

		m_productDescription = cmsTakeProductDesc(m_profile);
// 		kdDebug() << "\tDescription: " << m_productDescription << "\n";

		m_productInfo = cmsTakeProductInfo(m_profile);
// 		kdDebug() << "\tInfo: " << m_productInfo << "\n";
// #if (LCMS_MAJOR_VERSION > 1) || (LCMS_MAJOR_VERSION == 1 && LCMS_MINOR_VERSION >= 12)
//		m_manufacturer = cmsTakeManufacturer(m_profile);
// 		kdDebug() << "\tManufacturer: " << m_manufacturer << "\n";
// #endif
// 		kdDebug() << "\tCopyright: " << cmsTakeCopyright(m_profile) << "\n";

// 		kdDebug() << "\tModel: " << cmsTakeModel(m_profile) << "\n";

		setValid(true);



		emit loadComplete(this);
		return true;
	}
	emit ioFailed(this);

	return true;
}

bool KisProfile::saveAsync()
{
	return false;
}

QImage KisProfile::img()
{
	return QImage();
}


#include "kis_profile.moc"

