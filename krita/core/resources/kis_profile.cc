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

#include <lcms.h>

#include <qimage.h>
#include <qtextstream.h>

#include <kdebug.h>

#include "kis_profile.h"

KisProfile::KisProfile(const QString& file) : super(file)
{
}

KisProfile::~KisProfile()
{
}


bool KisProfile::loadAsync()
{
	cmsErrorAction(LCMS_ERROR_IGNORE);
	
	m_profile = cmsOpenProfileFromFile(filename().ascii(), "r");

// 	kdDebug() << "loading profile: " << filename() << "\n";
	if (m_profile) {
		m_colorSpace = cmsGetColorSpace(m_profile);
// 		kdDebug() << "\tColorspace: " << m_colorSpace << "\n";

		m_productName = cmsTakeProductName(m_profile);
// 		kdDebug() << "\tProduct name: " << m_productName << "\n";

		m_productDescription = cmsTakeProductDesc(m_profile);
// 		kdDebug() << "\tDescription: " << m_productDescription << "\n";

		m_productInfo = cmsTakeProductInfo(m_profile);
// 		kdDebug() << "\tInfo: " << m_productInfo << "\n";

		m_manufacturer = cmsTakeManufacturer(m_profile);
// 		kdDebug() << "\tManufacturer: " << m_manufacturer << "\n";

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

