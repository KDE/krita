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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <limits.h>
#include <map>
#include <kapplication.h>
#include <kconfig.h>
#include "kis_global.h"
#include "kis_config.h"

namespace {
	const Q_INT32 IMG_WIDTH_MAX = USHRT_MAX;
	const Q_INT32 IMG_HEIGHT_MAX = USHRT_MAX;
	const Q_INT32 IMG_DEFAULT_WIDTH = 512;
	const Q_INT32 IMG_DEFAULT_HEIGHT = 512;
	const enumImgType IMG_DEFAULT_TYPE = IMAGE_TYPE_RGBA;
}

std::map<enumImgType, QString> KisConfig::m_imgTypeName;

KisConfig::KisConfig()
{
	KApplication *app = KApplication::kApplication();

	Q_ASSERT(app);
	m_cfg = app -> sessionConfig();

	if (m_imgTypeName.empty()) {
		setupImgTypeNames();
	}
}

KisConfig::~KisConfig()
{
	m_cfg -> sync();
}

Q_INT32 KisConfig::maxImgWidth() const
{
	return m_cfg -> readNumEntry("imgWidthMax", IMG_WIDTH_MAX);
}

Q_INT32 KisConfig::defImgWidth() const
{
	return m_cfg -> readNumEntry("imgWidthDef", IMG_DEFAULT_WIDTH);
}

Q_INT32 KisConfig::maxImgHeight() const
{
	return m_cfg -> readNumEntry("imgHeightMax", IMG_HEIGHT_MAX);
}

Q_INT32 KisConfig::defImgHeight() const
{
	return m_cfg -> readNumEntry("imgHeightDef", IMG_DEFAULT_HEIGHT);
}

Q_INT32 KisConfig::maxLayerWidth() const
{
	return m_cfg -> readNumEntry("layerWidthMax", IMG_WIDTH_MAX);
}

Q_INT32 KisConfig::defLayerWidth() const
{
	return m_cfg -> readNumEntry("layerWidthDef", IMG_DEFAULT_WIDTH);
}

Q_INT32 KisConfig::maxLayerHeight() const
{
	return m_cfg -> readNumEntry("layerHeightMax", IMG_HEIGHT_MAX);
}

Q_INT32 KisConfig::defLayerHeight() const
{
	return m_cfg -> readNumEntry("layerHeightDef", IMG_DEFAULT_HEIGHT);
}

enumImgType KisConfig::defImgType() const
{
	return imgType(m_cfg -> readEntry("imgTypeDef", imgTypeName(IMG_DEFAULT_TYPE)));
}

void KisConfig::defImgType(enumImgType type)
{
	m_cfg -> writeEntry("imgTypeDef", imgTypeName(type));
}

void KisConfig::defImgWidth(Q_INT32 width)
{
	m_cfg -> writeEntry("imgWidthDef", width);
}

void KisConfig::defImgHeight(Q_INT32 height)
{
	m_cfg -> writeEntry("imgHeightDef", height);
}

void KisConfig::defLayerWidth(Q_INT32 width)
{
	m_cfg -> writeEntry("layerWidthDef", width);
}

void KisConfig::defLayerHeight(Q_INT32 height)
{
	m_cfg -> writeEntry("layerHeightDef", height);
}

void KisConfig::setupImgTypeNames() const
{
	m_imgTypeName[IMAGE_TYPE_UNKNOWN] = "Unknown";
	m_imgTypeName[IMAGE_TYPE_INDEXED] = "Indexed";
	m_imgTypeName[IMAGE_TYPE_INDEXEDA] = "IndexedA";
	m_imgTypeName[IMAGE_TYPE_GREY] = "GreyScale";
	m_imgTypeName[IMAGE_TYPE_GREYA] = "GreyScaleA";
	m_imgTypeName[IMAGE_TYPE_RGB] = "RGB";
	m_imgTypeName[IMAGE_TYPE_RGBA] = "RGBA";
	m_imgTypeName[IMAGE_TYPE_CMYK] = "CMYK";
	m_imgTypeName[IMAGE_TYPE_CMYKA] = "CMYKA";
	m_imgTypeName[IMAGE_TYPE_LAB] = "LAB";
	m_imgTypeName[IMAGE_TYPE_LABA] = "LABA";
	m_imgTypeName[IMAGE_TYPE_YUV] = "YUV";
	m_imgTypeName[IMAGE_TYPE_YUVA] = "YUVA";
}

QString KisConfig::imgTypeName(enumImgType imgType) const
{
	QString name = "Unknown";
	std::map<enumImgType, QString>::const_iterator it = m_imgTypeName.find(imgType);

	if (it != m_imgTypeName.end()) {
		name = (*it).second;
	}

	return name;
}

enumImgType KisConfig::imgType(const QString& name) const
{
	enumImgType type = IMAGE_TYPE_UNKNOWN;
	std::map<enumImgType, QString>::const_iterator it;

	for (it = m_imgTypeName.begin(); it != m_imgTypeName.end(); it++) {
		if ((*it).second == name) {
			type = (*it).first;
			break;
		}
	}

	return type;
}


