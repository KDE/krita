/*
 *  kis_profile.h - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
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

#ifndef KIS_PROFILE_H
#define KIS_PROFILE_H

#include <lcms.h>

#include <qvaluevector.h>
#include <qimage.h>

#include <ksharedptr.h>
#include <kio/job.h>

#include "kis_resource.h"

class KisProfile : public KisResource, public KShared {
	typedef KisResource super;
	Q_OBJECT

public:
	KisProfile();
	KisProfile(const QString& file);
	virtual ~KisProfile();

	virtual bool loadAsync();
	virtual bool saveAsync();
	virtual QImage img();

	icColorSpaceSignature colorSpace() const { return m_colorSpace; }
	QString productName() const { return m_productName; }
	QString productDescription() const { return m_productDescription; }
	QString productInfo() const { return m_productInfo; }
	QString manufacturer() const { return m_manufacturer; }

private:

	cmsHPROFILE m_profile;
	icColorSpaceSignature m_colorSpace;
	QString m_productName;
	QString m_productDescription;
	QString m_productInfo;
	QString m_manufacturer;

};

#endif // KIS_PROFILE_H

