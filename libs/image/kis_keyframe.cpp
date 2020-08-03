/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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

#include "kis_image_config.h"
#include "kis_keyframe.h"
#include "kis_keyframe_channel.h"
#include "kis_types.h"

#include <QPointer>


struct KisKeyframeSPStaticRegistrar {
    KisKeyframeSPStaticRegistrar() {
        qRegisterMetaType<KisKeyframeSP>("KisKeyframeSP");
    }
};
static KisKeyframeSPStaticRegistrar __registrar;


struct KisKeyframe::Private
{
    int colorLabel{0}; /**< User-assignable color index associated with a given frame. Used for organization. */
};


KisKeyframe::KisKeyframe()
    : m_d(new Private())
{
}

KisKeyframe::KisKeyframe(const KisKeyframe &rhs)
    : m_d(new Private)
{
    m_d->colorLabel = rhs.m_d->colorLabel;
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
