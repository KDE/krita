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
#include <kapplication.h>
#include <kconfig.h>
#include "kis_global.h"
#include "kis_config.h"

namespace {
	const Q_INT32 IMG_WIDTH_MAX = QUANTUM_MAX;
	const Q_INT32 IMG_HEIGHT_MAX = QUANTUM_MAX;
	const Q_INT32 IMG_DEFAULT_WIDTH = 512;
	const Q_INT32 IMG_DEFAULT_HEIGHT = 512;
}

KisConfig::KisConfig()
{
	KApplication *app = KApplication::kApplication();

	Q_ASSERT(app);
	m_cfg = app -> sessionConfig();
}

KisConfig::~KisConfig()
{
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
	return m_cfg -> readNumEntry("imgWidthDef", IMG_DEFAULT_HEIGHT);
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
	return m_cfg -> readNumEntry("layerWidthDef", IMG_DEFAULT_HEIGHT);
}

