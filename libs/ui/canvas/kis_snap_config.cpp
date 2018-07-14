/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_snap_config.h"

#include "kis_config.h"

KisSnapConfig::KisSnapConfig(bool loadValues)
    : m_orthogonal(false),
      m_node(false),
      m_extension(false),
      m_intersection(false),
      m_boundingBox(false),
      m_imageBounds(true),
      m_imageCenter(true)
{
    if (loadValues) {
        loadStaticData();
    }
}


KisSnapConfig::~KisSnapConfig()
{
}

void KisSnapConfig::saveStaticData() const
{
    KisConfig cfg(false);
    cfg.saveSnapConfig(*this);
}

void KisSnapConfig::loadStaticData()
{
    KisConfig cfg(true);
    cfg.loadSnapConfig(this);
}
