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

#include LCMS_HEADER

#include <qvaluevector.h>
#include <qimage.h>
#include <qcstring.h>

#include <ksharedptr.h>
#include <kio/job.h>

#include "kis_resource.h"
#include "kis_types.h"




//XXX: Profiles should be loaded by the color strategies
//     and be available only through the color strategy 
//     that matches the profile's color model
class KisProfile : public KisResource, public KShared {
	typedef KisResource super;
	Q_OBJECT

public:
	KisProfile(Q_UINT32 colorType);
	KisProfile(QByteArray rawData, Q_UINT32 colorType);
	KisProfile(const QString& file, Q_UINT32 colorType);
	KisProfile(const cmsHPROFILE profile, QByteArray rawData, Q_UINT32 colorType);

	virtual ~KisProfile();

	virtual bool load();
	virtual bool save();
	virtual QImage img();

	icColorSpaceSignature colorSpaceSignature() const { return m_colorSpaceSignature; }
	icProfileClassSignature deviceClass() const { return m_deviceClass; }
	QString productName() const { return m_productName; }
	QString productDescription() const { return m_productDescription; }
	QString productInfo() const { return m_productInfo; }
	QString manufacturer() const { return m_manufacturer; }
	cmsHPROFILE profile() const { return m_profile; }
	Q_UINT32 colorType() { return m_lcmsColorType; }
	KisAnnotationSP annotation() const;

public:

	static KisProfileSP getScreenProfile(int screen = -1);

private:
	bool init();
	
	cmsHPROFILE m_profile;
	icColorSpaceSignature m_colorSpaceSignature;
	icProfileClassSignature m_deviceClass;
	QString m_productName;
	QString m_productDescription;
	QString m_productInfo;
	QString m_manufacturer;
	Q_UINT32 m_lcmsColorType;

	QByteArray m_rawData;
};



#endif // KIS_PROFILE_H

