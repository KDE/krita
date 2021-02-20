/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_projection_updates_filter.h"


#include <QtGlobal>
#include <QRect>

KisProjectionUpdatesFilter::~KisProjectionUpdatesFilter()
{
}

bool KisDropAllProjectionUpdatesFilter::filter(KisImage *image, KisNode *node, const QVector<QRect> &rects, bool resetAnimationCache)
{
    Q_UNUSED(image);
    Q_UNUSED(node);
    Q_UNUSED(rects);
    Q_UNUSED(resetAnimationCache);
    return true;
}

bool KisDropAllProjectionUpdatesFilter::filterRefreshGraph(KisImage *image, KisNode *node, const QVector<QRect> &rects, const QRect &cropRect)
{
    Q_UNUSED(image);
    Q_UNUSED(node);
    Q_UNUSED(rects);
    Q_UNUSED(cropRect);
    return true;
}

bool KisDropAllProjectionUpdatesFilter::filterProjectionUpdateNoFilthy(KisImage *image, KisNode *pseudoFilthy, const QVector<QRect> &rects, const QRect &cropRect, const bool resetAnimationCache)
{
    Q_UNUSED(image);
    Q_UNUSED(pseudoFilthy);
    Q_UNUSED(rects);
    Q_UNUSED(cropRect);
    Q_UNUSED(resetAnimationCache);
    return true;
}
