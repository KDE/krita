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

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>

#include "kis_global.h"
#include "kis_config.h"

namespace {
	const Q_INT32 IMG_WIDTH_MAX = USHRT_MAX;
	const Q_INT32 IMG_HEIGHT_MAX = USHRT_MAX;
	const Q_INT32 IMG_DEFAULT_WIDTH = 512;
	const Q_INT32 IMG_DEFAULT_HEIGHT = 512;
	const enumCursorStyle DEFAULT_CURSOR_STYLE = CURSOR_STYLE_TOOLICON;
}

KisConfig::KisConfig()
{
	KApplication *app = KApplication::kApplication();

	Q_ASSERT(app);
	m_cfg = app -> config();

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

enumCursorStyle KisConfig::defCursorStyle() const
{
	kdDebug() << "Get cursor: " << m_cfg -> readNumEntry("cursorStyleDef", DEFAULT_CURSOR_STYLE) << "\n";
	return (enumCursorStyle) m_cfg -> readNumEntry("cursorStyleDef", DEFAULT_CURSOR_STYLE);
}

void KisConfig::defCursorStyle(enumCursorStyle style)
{
	kdDebug() << "Set cursor: " << style  << "\n";
	m_cfg -> writeEntry("cursorStyleDef", style);
}
