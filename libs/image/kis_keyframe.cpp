/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_config.h"
#include "kis_keyframe.h"
#include "kis_keyframe_channel.h"
#include "kis_types.h"

#include <QPointer>

struct KisKeyframe::Private
{
    int colorLabel{0}; /**< User-assignable color index associated with a given frame. Used for organization. */
};

KisKeyframe::KisKeyframe()
    : m_d(new Private())
{
}

KisKeyframe::~KisKeyframe()
{
}

int KisKeyframe::colorLabel() const
{
    return m_d->colorLabel;
}

void KisKeyframe::setColorLabel(int colorIndex)
{
    m_d->colorLabel = colorIndex;
}
