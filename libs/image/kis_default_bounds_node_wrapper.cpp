/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_default_bounds_node_wrapper.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_global.h"

struct Q_DECL_HIDDEN KisDefaultBoundsNodeWrapper::Private {
    KisBaseNodeWSP node;
};

const QRect KisDefaultBoundsNodeWrapper::infiniteRect =
    QRect(qint32_MIN/2, qint32_MIN/2, qint32_MAX, qint32_MAX);

KisDefaultBoundsNodeWrapper::KisDefaultBoundsNodeWrapper(KisBaseNodeWSP node)
    : m_d(new Private())
{
    m_d->node = node;
}

KisDefaultBoundsNodeWrapper::KisDefaultBoundsNodeWrapper(KisDefaultBoundsNodeWrapper &rhs):
    m_d(new Private())
{
    m_d->node = rhs.m_d->node;
}

KisDefaultBoundsNodeWrapper::~KisDefaultBoundsNodeWrapper()
{
    delete m_d;
}

QRect KisDefaultBoundsNodeWrapper::bounds() const
{
    return m_d->node->original() ? m_d->node->original()->defaultBounds()->bounds() : KisDefaultBoundsNodeWrapper::infiniteRect;
}

QRect KisDefaultBoundsNodeWrapper::imageBorderRect() const
{
    return m_d->node && m_d->node->image() ? m_d->node->image()->bounds() : KisDefaultBoundsNodeWrapper::infiniteRect;
}

bool KisDefaultBoundsNodeWrapper::wrapAroundMode() const
{
    return m_d->node && m_d->node->image() ? m_d->node->image()->wrapAroundModeActive() : false;
}

int KisDefaultBoundsNodeWrapper::currentLevelOfDetail() const
{
    return m_d->node && m_d->node->image() ? m_d->node->image()->currentLevelOfDetail() : 0;
}

int KisDefaultBoundsNodeWrapper::currentTime() const
{
    const int time = m_d->node && m_d->node->image() ? m_d->node->image()->animationInterface()->currentTime() : 0;
    return time;
}

bool KisDefaultBoundsNodeWrapper::externalFrameActive() const
{
    return m_d->node->original() ? m_d->node->original()->defaultBounds()->externalFrameActive() : false;
}

void *KisDefaultBoundsNodeWrapper::sourceCookie() const
{
    return m_d->node->original() ? m_d->node->original()->defaultBounds()->sourceCookie() : nullptr;
}

