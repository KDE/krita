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

#include "kis_transform_mask_params_interface.h"

#include <QTransform>


KisTransformMaskParamsInterface::~KisTransformMaskParamsInterface()
{
}

///////////////// KisDumbTransformMaskParams ////////////////////////////

#include <QDomElement>
#include "kis_dom_utils.h"
#include "kis_node.h"
#include "kis_painter.h"
#include "KoCompositeOpRegistry.h"



struct Q_DECL_HIDDEN KisDumbTransformMaskParams::Private
{
    Private() : isHidden(false) {}

    QTransform transform;
    bool isHidden;
};

KisDumbTransformMaskParams::KisDumbTransformMaskParams()
    : m_d(new Private)
{
}

KisDumbTransformMaskParams::KisDumbTransformMaskParams(const QTransform &transform)
    : m_d(new Private)
{
    m_d->isHidden = false;
    m_d->transform = transform;
}

KisDumbTransformMaskParams::KisDumbTransformMaskParams(bool isHidden)
    : m_d(new Private)
{
    m_d->isHidden = isHidden;
}

KisDumbTransformMaskParams::~KisDumbTransformMaskParams()
{
}

QTransform KisDumbTransformMaskParams::finalAffineTransform() const
{
    return m_d->transform;
}

bool KisDumbTransformMaskParams::isAffine() const
{
    return true;
}

bool KisDumbTransformMaskParams::isHidden() const
{
    return m_d->isHidden;
}

void KisDumbTransformMaskParams::transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst) const
{
    Q_UNUSED(node);

    QRect rc = src->exactBounds();
    QPoint dstTopLeft = rc.topLeft();

    QTransform t = finalAffineTransform();
    if (t.isTranslating()) {
        dstTopLeft = t.map(dstTopLeft);
    } else if (!t.isIdentity()) {
        warnKrita << "KisDumbTransformMaskParams::transformDevice does not support this kind of transformation";
        warnKrita << ppVar(t);
    }

    KisPainter::copyAreaOptimized(dstTopLeft, src, dst, rc);
}

QString KisDumbTransformMaskParams::id() const
{
    return "dumbparams";
}

void KisDumbTransformMaskParams::toXML(QDomElement *e) const
{
    QDomDocument doc = e->ownerDocument();
    QDomElement transformEl = doc.createElement("dumb_transform");
    e->appendChild(transformEl);

    KisDomUtils::saveValue(&transformEl, "transform", m_d->transform);
}

KisTransformMaskParamsInterfaceSP KisDumbTransformMaskParams::fromXML(const QDomElement &e)
{
    QDomElement transformEl;
    bool result = false;

    QTransform transform;

    result =
        KisDomUtils::findOnlyElement(e, "dumb_transform", &transformEl) &&
        KisDomUtils::loadValue(transformEl, "transform", &transform);

    if (!result) {
        warnKrita << "WARNING: couldn't load dumb transform. Ignoring...";
    }

    return KisTransformMaskParamsInterfaceSP(
        new KisDumbTransformMaskParams(transform));
}

void KisDumbTransformMaskParams::translate(const QPointF &offset)
{
    m_d->transform *= QTransform::fromTranslate(offset.x(), offset.y());
}

QTransform KisDumbTransformMaskParams::testingGetTransform() const
{
    return m_d->transform;
}

void KisDumbTransformMaskParams::testingSetTransform(const QTransform &t)
{
    m_d->transform = t;
}

#include "kis_transform_mask_params_factory_registry.h"

struct DumbParamsRegistrar {
    DumbParamsRegistrar() {
        KisTransformMaskParamsFactory f(KisDumbTransformMaskParams::fromXML);
        KisTransformMaskParamsFactoryRegistry::instance()->addFactory("dumbparams", f);
    }
};
static DumbParamsRegistrar __dumbParamsRegistrar;
