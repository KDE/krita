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

#include "kis_transform_mask_adapter.h"

#include <QTransform>
#include <QDomElement>

#include "tool_transform_args.h"
#include "kis_transform_utils.h"

#include "kis_node.h"


struct KisTransformMaskAdapter::Private
{
    ToolTransformArgs args;
};


KisTransformMaskAdapter::KisTransformMaskAdapter(const ToolTransformArgs &args)
    : m_d(new Private)
{
    m_d->args = args;
}

KisTransformMaskAdapter::~KisTransformMaskAdapter()
{
}

QTransform KisTransformMaskAdapter::finalAffineTransform() const
{
    KisTransformUtils::MatricesPack m(m_d->args);
    return m.finalTransform();
}

bool KisTransformMaskAdapter::isAffine() const
{
    return m_d->args.mode() == ToolTransformArgs::FREE_TRANSFORM ||
        m_d->args.mode() == ToolTransformArgs::PERSPECTIVE_4POINT;
}

bool KisTransformMaskAdapter::isHidden() const
{
    return false;
}

void KisTransformMaskAdapter::transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst) const
{
    dst->makeCloneFrom(src, src->extent());

    KisProcessingVisitor::ProgressHelper helper(node);
    KisTransformUtils::transformDevice(m_d->args, dst, &helper);
}

const ToolTransformArgs& KisTransformMaskAdapter::savedArgs() const
{
    return m_d->args;
}

QString KisTransformMaskAdapter::id() const
{
    return "tooltransformparams";
}

void KisTransformMaskAdapter::toXML(QDomElement *e) const
{
    m_d->args.toXML(e);
}

KisTransformMaskParamsInterfaceSP KisTransformMaskAdapter::fromXML(const QDomElement &e)
{
    return KisTransformMaskParamsInterfaceSP(
        new KisTransformMaskAdapter(ToolTransformArgs::fromXML(e)));
}

void KisTransformMaskAdapter::translate(const QPointF &offset)
{
    m_d->args.translate(offset);
}

#include "kis_transform_mask_params_factory_registry.h"

struct ToolTransformParamsRegistrar {
    ToolTransformParamsRegistrar() {
        KisTransformMaskParamsFactory f(KisTransformMaskAdapter::fromXML);
        KisTransformMaskParamsFactoryRegistry::instance()->addFactory("tooltransformparams", f);
    }
};
static ToolTransformParamsRegistrar __toolTransformParamsRegistrar;

