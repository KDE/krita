/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_mask_params_interface.h"

#include <QTransform>


KisTransformMaskParamsInterface::~KisTransformMaskParamsInterface()
{
}


KisAnimatedTransformParamsInterface::~KisAnimatedTransformParamsInterface()
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

void KisDumbTransformMaskParams::translateSrcAndDst(const QPointF &offset)
{
    Q_UNUSED(offset);

    /**
     * Normal translation doesn't change affine transformations
     * in full-featured KisTransformMaskAdapter, so we should resemble
     * this behavior in the dumb one
     */
}

void KisDumbTransformMaskParams::transformSrcAndDst(const QTransform &t)
{
    Q_UNUSED(t);

    /**
     * Normal translation doesn't change affine transformations
     * in full-featured KisTransformMaskAdapter, so we should resemble
     * this behavior in the dumb one
     */
}

void KisDumbTransformMaskParams::translateDstSpace(const QPointF &offset)
{
    m_d->transform.translate(offset.x(), offset.y());
}

QRect KisDumbTransformMaskParams::nonAffineChangeRect(const QRect &rc)
{
    return rc;
}

QRect KisDumbTransformMaskParams::nonAffineNeedRect(const QRect &rc, const QRect &srcBounds)
{
    Q_UNUSED(srcBounds);
    return rc;
}

bool KisDumbTransformMaskParams::isAnimated() const
{
    return false;
}

KisKeyframeChannel *KisDumbTransformMaskParams::getKeyframeChannel(const QString&, KisDefaultBoundsBaseSP)
{
    return 0;
}

void KisDumbTransformMaskParams::clearChangedFlag()
{}

bool KisDumbTransformMaskParams::hasChanged() const
{
    return false;
}

KisTransformMaskParamsInterfaceSP KisDumbTransformMaskParams::clone() const
{
    return toQShared(new KisDumbTransformMaskParams(m_d->transform));
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
