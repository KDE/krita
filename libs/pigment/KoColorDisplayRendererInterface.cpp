/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoColorDisplayRendererInterface.h"

#include "kglobal.h"
#include <KoColorSpaceRegistry.h>

#include <KoChannelInfo.h>


KoColorDisplayRendererInterface::KoColorDisplayRendererInterface()
{
}

KoColorDisplayRendererInterface::~KoColorDisplayRendererInterface()
{
}

QColor KoDumbColorDisplayRenderer::toQColor(const KoColor &c) const
{
    return c.toQColor();
}

KoColor KoDumbColorDisplayRenderer::approximateFromRenderedQColor(const QColor &c) const
{
    KoColor color;
    color.fromQColor(c);
    return color;
}

KoColor KoDumbColorDisplayRenderer::fromHsv(int h, int s, int v, int a) const
{
    h = qBound(0, h, 359);
    s = qBound(0, s, 255);
    v = qBound(0, v, 255);
    a = qBound(0, a, 255);
    QColor qcolor(QColor::fromHsv(h, s, v, a));
    return KoColor(qcolor, KoColorSpaceRegistry::instance()->rgb8());
}

void KoDumbColorDisplayRenderer::getHsv(const KoColor &srcColor, int *h, int *s, int *v, int *a) const
{
    QColor qcolor = toQColor(srcColor);
    qcolor.getHsv(h, s, v, a);
}

KoColorDisplayRendererInterface* KoDumbColorDisplayRenderer::instance()
{
    K_GLOBAL_STATIC(KoDumbColorDisplayRenderer, s_instance);
    return s_instance;
}

qreal KoDumbColorDisplayRenderer::minVisibleFloatValue(const KoChannelInfo *chaninfo) const
{
    Q_ASSERT(chaninfo);
    return chaninfo->getUIMin();
}

qreal KoDumbColorDisplayRenderer::maxVisibleFloatValue(const KoChannelInfo *chaninfo) const
{
    Q_ASSERT(chaninfo);
    return chaninfo->getUIMax();
}
