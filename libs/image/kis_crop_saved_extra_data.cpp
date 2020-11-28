/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_crop_saved_extra_data.h"

#include "kis_node.h"


KisCropSavedExtraData::KisCropSavedExtraData(Type type,
                                             QRect cropRect,
                                             KisNodeSP cropNode)
    : m_type(type),
      m_cropRect(cropRect),
      m_cropNode(cropNode)
{
}

KisCropSavedExtraData::~KisCropSavedExtraData()
{
}
