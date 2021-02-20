/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
      m_imageCenter(true),
      m_toPixel(false)
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
