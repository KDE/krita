/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

KisDefaultBounds::KisDefaultBounds()
    : KisDefaultBounds(0)
{
}

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

WrapAroundAxis KisDefaultBounds::wrapAroundModeAxis() const
{
    return m_d->image ? m_d->image->wrapAroundModeAxis() : WRAPAROUND_BOTH;
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
/*                  KisSelectionDefaultBoundsBase                 */
/******************************************************************/

KisSelectionDefaultBoundsBase::KisSelectionDefaultBoundsBase()
{
}

KisSelectionDefaultBoundsBase::~KisSelectionDefaultBoundsBase()
{
}

QRect KisSelectionDefaultBoundsBase::bounds() const
{
    KisPaintDeviceSP parentDevice = this->parentPaintDevice();

    return parentDevice ?
                parentDevice->extent() | parentDevice->defaultBounds()->bounds() : QRect();
}

QRect KisSelectionDefaultBoundsBase::imageBorderRect() const
{
    KisPaintDeviceSP parentDevice = this->parentPaintDevice();

    return parentDevice ? parentDevice->defaultBounds()->bounds() : QRect();
}

bool KisSelectionDefaultBoundsBase::wrapAroundMode() const
{
    KisPaintDeviceSP parentDevice = this->parentPaintDevice();

    return parentDevice ? parentDevice->defaultBounds()->wrapAroundMode() : false;
}

WrapAroundAxis KisSelectionDefaultBoundsBase::wrapAroundModeAxis() const
{
    KisPaintDeviceSP parentDevice = this->parentPaintDevice();

    return parentDevice ? parentDevice->defaultBounds()->wrapAroundModeAxis() : WRAPAROUND_BOTH;
}

int KisSelectionDefaultBoundsBase::currentLevelOfDetail() const
{
    KisPaintDeviceSP parentDevice = this->parentPaintDevice();

    return parentDevice ? parentDevice->defaultBounds()->currentLevelOfDetail() : 0;
}

int KisSelectionDefaultBoundsBase::currentTime() const
{
    KisPaintDeviceSP parentDevice = this->parentPaintDevice();

    return parentDevice ? parentDevice->defaultBounds()->currentTime() : 0;
}

bool KisSelectionDefaultBoundsBase::externalFrameActive() const
{
    KisPaintDeviceSP parentDevice = this->parentPaintDevice();

    return parentDevice ? parentDevice->defaultBounds()->externalFrameActive() : false;
}

void *KisSelectionDefaultBoundsBase::sourceCookie() const
{
    KisPaintDeviceSP parentDevice = this->parentPaintDevice();

    return parentDevice.data();
}

/******************************************************************/
/*                   KisSelectionDefaultBounds                    */
/******************************************************************/

KisSelectionDefaultBounds::KisSelectionDefaultBounds(KisPaintDeviceSP parentPaintDevice)
    : m_paintDevice(parentPaintDevice)
{
}

KisSelectionDefaultBounds::~KisSelectionDefaultBounds()
{
}

KisPaintDeviceSP KisSelectionDefaultBounds::parentPaintDevice() const
{
    return m_paintDevice;
}

/******************************************************************/
/*                   KisMaskDefaultBounds                         */
/******************************************************************/


KisMaskDefaultBounds::KisMaskDefaultBounds(KisNodeSP parentNode)
    : m_parentNode(parentNode)
{
}

KisMaskDefaultBounds::~KisMaskDefaultBounds()
{
}

KisPaintDeviceSP KisMaskDefaultBounds::parentPaintDevice() const
{
    KisNodeSP parentNode = m_parentNode;

    return parentNode ? parentNode->original() : nullptr;
}

/******************************************************************/
/*                   KisSelectionEmptyBounds                      */
/******************************************************************/

KisSelectionEmptyBounds::KisSelectionEmptyBounds()
    : KisSelectionEmptyBounds(nullptr)
{
}

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

/******************************************************************/
/*                 KisWrapAroundBoundsWrapper                     */
/******************************************************************/


struct Q_DECL_HIDDEN KisWrapAroundBoundsWrapper::Private
{
    KisDefaultBoundsBaseSP base;
    QRect bounds;
};

KisWrapAroundBoundsWrapper::KisWrapAroundBoundsWrapper(KisDefaultBoundsBaseSP base, QRect bounds)
: m_d(new Private())
{
    m_d->base = base;
    m_d->bounds = bounds;
}

KisWrapAroundBoundsWrapper::~KisWrapAroundBoundsWrapper()
{
}

QRect KisWrapAroundBoundsWrapper::bounds() const
{
    return m_d->bounds;
}

bool KisWrapAroundBoundsWrapper::wrapAroundMode() const
{
    return true;
}

WrapAroundAxis KisWrapAroundBoundsWrapper::wrapAroundModeAxis() const
{
    return m_d->base->wrapAroundModeAxis();
}

int KisWrapAroundBoundsWrapper::currentLevelOfDetail() const
{
    return m_d->base->currentLevelOfDetail();
}

int KisWrapAroundBoundsWrapper::currentTime() const
{
    return m_d->base->currentTime();
}

bool KisWrapAroundBoundsWrapper::externalFrameActive() const
{
    return m_d->base->externalFrameActive();
}

void *KisWrapAroundBoundsWrapper::sourceCookie() const
{
    return m_d->base->sourceCookie();
}
