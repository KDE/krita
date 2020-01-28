/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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


#include "kis_global.h"
#include "kis_default_bounds.h"
#include "kis_paint_device.h"
#include "kis_image_animation_interface.h"
#include "kis_image.h"


const QRect KisDefaultBounds::infiniteRect =
    QRect(qint32_MIN/2, qint32_MIN/2, qint32_MAX, qint32_MAX);


/******************************************************************/
/*                  KisDefaultBounds                              */
/******************************************************************/

struct Q_DECL_HIDDEN KisDefaultBounds::Private
{
    KisImageWSP image;
};


KisDefaultBounds::KisDefaultBounds(KisImageWSP image)
    : m_d(new Private())
{
    m_d->image = image;
}

KisDefaultBounds::~KisDefaultBounds()
{
    delete m_d;
}

QRect KisDefaultBounds::bounds() const
{
    /**
     * By default return infinite rect to cover everything
     */
    return m_d->image ? m_d->image->effectiveLodBounds() : infiniteRect;
}

bool KisDefaultBounds::wrapAroundMode() const
{
    return m_d->image ? m_d->image->wrapAroundModeActive() : false;
}

int KisDefaultBounds::currentLevelOfDetail() const
{
    return m_d->image ? m_d->image->currentLevelOfDetail() : 0;
}

int KisDefaultBounds::currentTime() const
{
    KisImageAnimationInterface *interface = m_d->image ? m_d->image->animationInterface() : 0;
    return interface ? interface->currentTime() : 0;
}

bool KisDefaultBounds::externalFrameActive() const
{
    KisImageAnimationInterface *interface = m_d->image ? m_d->image->animationInterface() : 0;
    return interface ? interface->externalFrameActive() : false;
}

void *KisDefaultBounds::sourceCookie() const
{
    return m_d->image.data();
}

/******************************************************************/
/*                  KisSelectionDefaultBounds                     */
/******************************************************************/

struct Q_DECL_HIDDEN KisSelectionDefaultBounds::Private
{
    KisPaintDeviceWSP parentDevice;
};

KisSelectionDefaultBounds::KisSelectionDefaultBounds(KisPaintDeviceSP parentDevice)
    : m_d(new Private())
{
    m_d->parentDevice = parentDevice;
}

KisSelectionDefaultBounds::~KisSelectionDefaultBounds()
{
    delete m_d;
}

QRect KisSelectionDefaultBounds::bounds() const
{
    return m_d->parentDevice ?
        m_d->parentDevice->extent() | m_d->parentDevice->defaultBounds()->bounds() : QRect();
}

bool KisSelectionDefaultBounds::wrapAroundMode() const
{
    return m_d->parentDevice ?
        m_d->parentDevice->defaultBounds()->wrapAroundMode() : false;
}

int KisSelectionDefaultBounds::currentLevelOfDetail() const
{
    return m_d->parentDevice ?
        m_d->parentDevice->defaultBounds()->currentLevelOfDetail() : 0;
}

int KisSelectionDefaultBounds::currentTime() const
{
    return m_d->parentDevice ?
        m_d->parentDevice->defaultBounds()->currentTime() : 0;
}

bool KisSelectionDefaultBounds::externalFrameActive() const
{
    return m_d->parentDevice ?
                m_d->parentDevice->defaultBounds()->externalFrameActive() : false;
}

void *KisSelectionDefaultBounds::sourceCookie() const
{
    return m_d->parentDevice.data();
}

/******************************************************************/
/*                   KisSelectionEmptyBounds                      */
/******************************************************************/

KisSelectionEmptyBounds::KisSelectionEmptyBounds(KisImageWSP image)
    : KisDefaultBounds(image)
{
}

KisSelectionEmptyBounds::~KisSelectionEmptyBounds()
{
}

QRect KisSelectionEmptyBounds::bounds() const
{
    return QRect(0, 0, 0, 0);
}
