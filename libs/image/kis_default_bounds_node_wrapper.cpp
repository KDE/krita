/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_default_bounds_node_wrapper.h"
#include "kis_mask.h"
#include "kis_global.h"

struct Q_DECL_HIDDEN KisDefaultBoundsNodeWrapper::Private {
    KisNodeWSP node;
};

const QRect KisDefaultBoundsNodeWrapper::infiniteRect =
    QRect(qint32_MIN/2, qint32_MIN/2, qint32_MAX, qint32_MAX);

KisDefaultBoundsNodeWrapper::KisDefaultBoundsNodeWrapper(KisNodeWSP node):
    m_d(new Private())
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
    return m_d->node->original() ? m_d->node->original()->defaultBounds()->imageBorderRect() : KisDefaultBoundsNodeWrapper::infiniteRect;
}

bool KisDefaultBoundsNodeWrapper::wrapAroundMode() const
{
    return m_d->node->original() ? m_d->node->original()->defaultBounds()->wrapAroundMode() : false;
}

int KisDefaultBoundsNodeWrapper::currentLevelOfDetail() const
{
    return m_d->node->original() ? m_d->node->original()->defaultBounds()->currentLevelOfDetail() : 0;
}

int KisDefaultBoundsNodeWrapper::currentTime() const
{   
    KisMaskWSP mask = dynamic_cast<KisMask*>(m_d->node.data());
    KisNodeWSP toSample = mask.isValid() ? mask->parent().data() : m_d->node;

    return toSample->original() ? toSample->original()->defaultBounds()->currentTime() : 0;
}

bool KisDefaultBoundsNodeWrapper::externalFrameActive() const
{
    return m_d->node->original() ? m_d->node->original()->defaultBounds()->externalFrameActive() : false;
}

void *KisDefaultBoundsNodeWrapper::sourceCookie() const
{
    return m_d->node->original() ? m_d->node->original()->defaultBounds()->sourceCookie() : nullptr;
}

