/*
 *  tool_transform_args.h - part of Krita
 *
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
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

#include "tool_transform_args.h"

#include <QDomElement>

#include <ksharedconfig.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "kis_liquify_transform_worker.h"
#include "kis_dom_utils.h"


ToolTransformArgs::ToolTransformArgs()
    : m_liquifyProperties(new KisLiquifyProperties())
{
    m_mode = FREE_TRANSFORM;
    m_transformedCenter = QPointF(0, 0);
    m_originalCenter = QPointF(0, 0);
    m_rotationCenterOffset = QPointF(0, 0);
    m_cameraPos = QVector3D(0,0,1024);
    m_aX = 0;
    m_aY = 0;
    m_aZ = 0;
    m_scaleX = 1.0;
    m_scaleY = 1.0;
    m_shearX = 0.0;
    m_shearY = 0.0;
    m_origPoints = QVector<QPointF>();
    m_transfPoints = QVector<QPointF>();
    m_warpType = KisWarpTransformWorker::RIGID_TRANSFORM;
    m_alpha = 1.0;
    m_keepAspectRatio = false;
    m_defaultPoints = true;

    KConfigGroup configGroup =  KSharedConfig::openConfig()->group("KisToolTransform");
    QString savedFilterId = configGroup.readEntry("filterId", "Bicubic");
    setFilterId(savedFilterId);
    m_transformAroundRotationCenter = configGroup.readEntry("transformAroundRotationCenter", "0").toInt();

    m_editTransformPoints = false;
}

void ToolTransformArgs::setFilterId(const QString &id) {
    m_filter = KisFilterStrategyRegistry::instance()->value(id);

    if (m_filter) {
        KConfigGroup configGroup =  KSharedConfig::openConfig()->group("KisToolTransform");
        configGroup.writeEntry("filterId", id);
    }
}

void ToolTransformArgs::setTransformAroundRotationCenter(bool value)
{
    m_transformAroundRotationCenter = value;

    KConfigGroup configGroup =  KSharedConfig::openConfig()->group("KisToolTransform");
    configGroup.writeEntry("transformAroundRotationCenter", int(value));
}

void ToolTransformArgs::init(const ToolTransformArgs& args)
{
    m_mode = args.mode();
    m_transformedCenter = args.transformedCenter();
    m_originalCenter = args.originalCenter();
    m_rotationCenterOffset = args.rotationCenterOffset();
    m_transformAroundRotationCenter = args.transformAroundRotationCenter();
    m_cameraPos = args.m_cameraPos;
    m_aX = args.aX();
    m_aY = args.aY();
    m_aZ = args.aZ();
    m_scaleX = args.scaleX();
    m_scaleY = args.scaleY();
    m_shearX = args.shearX();
    m_shearY = args.shearY();
    m_origPoints = args.origPoints(); //it's a copy
    m_transfPoints = args.transfPoints();
    m_warpType = args.warpType();
    m_alpha = args.alpha();
    m_defaultPoints = args.defaultPoints();
    m_keepAspectRatio = args.keepAspectRatio();
    m_filter = args.m_filter;
    m_flattenedPerspectiveTransform = args.m_flattenedPerspectiveTransform;
    m_editTransformPoints = args.m_editTransformPoints;

    if (args.m_liquifyWorker) {
        m_liquifyWorker.reset(new KisLiquifyTransformWorker(*args.m_liquifyWorker.data()));
    }

    m_continuedTransformation.reset(args.m_continuedTransformation ? new ToolTransformArgs(*args.m_continuedTransformation) : 0);
}

void ToolTransformArgs::clear()
{
    m_origPoints.clear();
    m_transfPoints.clear();
}

ToolTransformArgs::ToolTransformArgs(const ToolTransformArgs& args)
    : m_liquifyProperties(args.m_liquifyProperties)
{
    init(args);
}

KisToolChangesTrackerData *ToolTransformArgs::clone() const
{
    return new ToolTransformArgs(*this);
}

ToolTransformArgs& ToolTransformArgs::operator=(const ToolTransformArgs& args)
{
    clear();

    m_liquifyProperties = args.m_liquifyProperties;
    init(args);

    return *this;
}

bool ToolTransformArgs::operator==(const ToolTransformArgs& other) const
{
    return
        m_mode == other.m_mode &&
        m_defaultPoints == other.m_defaultPoints &&
        m_origPoints == other.m_origPoints &&
        m_transfPoints == other.m_transfPoints &&
        m_warpType == other.m_warpType &&
        m_alpha == other.m_alpha &&
        m_transformedCenter == other.m_transformedCenter &&
        m_originalCenter == other.m_originalCenter &&
        m_rotationCenterOffset == other.m_rotationCenterOffset &&
        m_transformAroundRotationCenter == other.m_transformAroundRotationCenter &&
        m_aX == other.m_aX &&
        m_aY == other.m_aY &&
        m_aZ == other.m_aZ &&
        m_cameraPos == other.m_cameraPos &&
        m_scaleX == other.m_scaleX &&
        m_scaleY == other.m_scaleY &&
        m_shearX == other.m_shearX &&
        m_shearY == other.m_shearY &&
        m_keepAspectRatio == other.m_keepAspectRatio &&
        m_flattenedPerspectiveTransform == other.m_flattenedPerspectiveTransform &&
        m_editTransformPoints == other.m_editTransformPoints &&
        (m_liquifyProperties == other.m_liquifyProperties ||
         *m_liquifyProperties == *other.m_liquifyProperties) &&

        // pointer types

        ((m_filter && other.m_filter &&
          m_filter->id() == other.m_filter->id())
         || m_filter == other.m_filter) &&

        ((m_liquifyWorker && other.m_liquifyWorker &&
          *m_liquifyWorker == *other.m_liquifyWorker)
         || m_liquifyWorker == other.m_liquifyWorker);
}

bool ToolTransformArgs::isSameMode(const ToolTransformArgs& other) const
{
    if (m_mode != other.m_mode) return false;

    bool result = true;

    if (m_mode == FREE_TRANSFORM) {
        result &= m_transformedCenter == other.m_transformedCenter;
        result &= m_originalCenter == other.m_originalCenter;
        result &= m_scaleX == other.m_scaleX;
        result &= m_scaleY == other.m_scaleY;
        result &= m_shearX == other.m_shearX;
        result &= m_shearY == other.m_shearY;
        result &= m_aX == other.m_aX;
        result &= m_aY == other.m_aY;
        result &= m_aZ == other.m_aZ;

    } else if (m_mode == PERSPECTIVE_4POINT) {
        result &= m_transformedCenter == other.m_transformedCenter;
        result &= m_originalCenter == other.m_originalCenter;
        result &= m_scaleX == other.m_scaleX;
        result &= m_scaleY == other.m_scaleY;
        result &= m_shearX == other.m_shearX;
        result &= m_shearY == other.m_shearY;
        result &= m_flattenedPerspectiveTransform == other.m_flattenedPerspectiveTransform;

    } else if(m_mode == WARP || m_mode == CAGE) {
        result &= m_origPoints == other.m_origPoints;
        result &= m_transfPoints == other.m_transfPoints;

    } else if (m_mode == LIQUIFY) {
        result &= m_liquifyProperties &&
            (m_liquifyProperties == other.m_liquifyProperties ||
             *m_liquifyProperties == *other.m_liquifyProperties);

        result &=
            (m_liquifyWorker && other.m_liquifyWorker &&
             *m_liquifyWorker == *other.m_liquifyWorker)
            || m_liquifyWorker == other.m_liquifyWorker;

    } else {
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "unknown transform mode");
    }

    return result;
}

ToolTransformArgs::ToolTransformArgs(TransformMode mode,
                                     QPointF transformedCenter,
                                     QPointF originalCenter,
                                     QPointF rotationCenterOffset,
                                     bool transformAroundRotationCenter,
                                     double aX, double aY, double aZ,
                                     double scaleX, double scaleY,
                                     double shearX, double shearY,
                                     KisWarpTransformWorker::WarpType warpType,
                                     double alpha,
                                     bool defaultPoints,
                                     const QString &filterId)
    : m_liquifyProperties(new KisLiquifyProperties())
{
    m_mode = mode;
    m_transformedCenter = transformedCenter;
    m_originalCenter = originalCenter;
    m_rotationCenterOffset = rotationCenterOffset;
    m_transformAroundRotationCenter = transformAroundRotationCenter;
    m_cameraPos = QVector3D(0,0,1024);
    m_aX = aX;
    m_aY = aY;
    m_aZ = aZ;
    m_scaleX = scaleX;
    m_scaleY = scaleY;
    m_shearX = shearX;
    m_shearY = shearY;
    m_origPoints = QVector<QPointF>();
    m_transfPoints = QVector<QPointF>();
    m_warpType = warpType;
    m_alpha = alpha;
    m_defaultPoints = defaultPoints;
    m_keepAspectRatio = false;
    setFilterId(filterId);
    m_editTransformPoints = false;
}


ToolTransformArgs::~ToolTransformArgs()
{
    clear();
}

void ToolTransformArgs::translate(const QPointF &offset)
{
    if (m_mode == FREE_TRANSFORM || m_mode == PERSPECTIVE_4POINT) {
        m_originalCenter += offset;
        m_rotationCenterOffset += offset;
        m_transformedCenter += offset;
    } else if(m_mode == WARP || m_mode == CAGE) {
        for (auto &pt : m_origPoints) {
            pt += offset;
        }

        for (auto &pt : m_transfPoints) {
            pt += offset;
        }
    } else if (m_mode == LIQUIFY) {
        KIS_ASSERT_RECOVER_RETURN(m_liquifyWorker);
        m_liquifyWorker->translate(offset);
    } else {
        KIS_ASSERT_RECOVER_NOOP(0 && "unknown transform mode");
    }
}

bool ToolTransformArgs::isIdentity() const
{
    if (m_mode == FREE_TRANSFORM) {
        return (m_transformedCenter == m_originalCenter && m_scaleX == 1
                && m_scaleY == 1 && m_shearX == 0 && m_shearY == 0
                && m_aX == 0 && m_aY == 0 && m_aZ == 0);
    } else if (m_mode == PERSPECTIVE_4POINT) {
            return (m_transformedCenter == m_originalCenter && m_scaleX == 1
                    && m_scaleY == 1 && m_shearX == 0 && m_shearY == 0
                    && m_flattenedPerspectiveTransform.isIdentity());
    } else if(m_mode == WARP || m_mode == CAGE) {
        for (int i = 0; i < m_origPoints.size(); ++i)
            if (m_origPoints[i] != m_transfPoints[i])
                return false;

        return true;
    } else if (m_mode == LIQUIFY) {
        // Not implemented!
        return false;
    } else {
        KIS_ASSERT_RECOVER_NOOP(0 && "unknown transform mode");
        return true;
    }
}

void ToolTransformArgs::initLiquifyTransformMode(const QRect &srcRect)
{
    m_liquifyWorker.reset(new KisLiquifyTransformWorker(srcRect, 0, 8));
    m_liquifyProperties->loadAndResetMode();
}

void ToolTransformArgs::saveLiquifyTransformMode() const
{
    m_liquifyProperties->saveMode();
}

void ToolTransformArgs::toXML(QDomElement *e) const
{
    e->setAttribute("mode", (int) m_mode);

    QDomDocument doc = e->ownerDocument();

    if (m_mode == FREE_TRANSFORM || m_mode == PERSPECTIVE_4POINT) {

        QDomElement freeEl = doc.createElement("free_transform");
        e->appendChild(freeEl);

        KisDomUtils::saveValue(&freeEl, "transformedCenter", m_transformedCenter);
        KisDomUtils::saveValue(&freeEl, "originalCenter", m_originalCenter);
        KisDomUtils::saveValue(&freeEl, "rotationCenterOffset", m_rotationCenterOffset);
        KisDomUtils::saveValue(&freeEl, "transformAroundRotationCenter", m_transformAroundRotationCenter);

        KisDomUtils::saveValue(&freeEl, "aX", m_aX);
        KisDomUtils::saveValue(&freeEl, "aY", m_aY);
        KisDomUtils::saveValue(&freeEl, "aZ", m_aZ);

        KisDomUtils::saveValue(&freeEl, "cameraPos", m_cameraPos);

        KisDomUtils::saveValue(&freeEl, "scaleX", m_scaleX);
        KisDomUtils::saveValue(&freeEl, "scaleY", m_scaleY);

        KisDomUtils::saveValue(&freeEl, "shearX", m_shearX);
        KisDomUtils::saveValue(&freeEl, "shearY", m_shearY);

        KisDomUtils::saveValue(&freeEl, "keepAspectRatio", m_keepAspectRatio);
        KisDomUtils::saveValue(&freeEl, "flattenedPerspectiveTransform", m_flattenedPerspectiveTransform);

        KisDomUtils::saveValue(&freeEl, "filterId", m_filter->id());

    } else if (m_mode == WARP || m_mode == CAGE) {
        QDomElement warpEl = doc.createElement("warp_transform");
        e->appendChild(warpEl);

        KisDomUtils::saveValue(&warpEl, "defaultPoints", m_defaultPoints);
        KisDomUtils::saveValue(&warpEl, "originalPoints", m_origPoints);
        KisDomUtils::saveValue(&warpEl, "transformedPoints", m_transfPoints);

        KisDomUtils::saveValue(&warpEl, "warpType", (int)m_warpType); // limited!
        KisDomUtils::saveValue(&warpEl, "alpha", m_alpha);

    } else if (m_mode == LIQUIFY) {
        QDomElement liqEl = doc.createElement("liquify_transform");
        e->appendChild(liqEl);

        m_liquifyProperties->toXML(&liqEl);
        m_liquifyWorker->toXML(&liqEl);
    } else {
        KIS_ASSERT_RECOVER_RETURN(0 && "Unknown transform mode");
    }

    // m_editTransformPoints should not be saved since it is reset explicitly
}

ToolTransformArgs ToolTransformArgs::fromXML(const QDomElement &e)
{
    ToolTransformArgs args;

    int newMode = e.attribute("mode", "0").toInt();
    if (newMode < 0 || newMode >= N_MODES) return ToolTransformArgs();

    args.m_mode = (TransformMode) newMode;

    // reset explicitly
    args.m_editTransformPoints = false;

    bool result = false;

    if (args.m_mode == FREE_TRANSFORM || args.m_mode == PERSPECTIVE_4POINT) {

        QDomElement freeEl;

        QString filterId;

        result =
            KisDomUtils::findOnlyElement(e, "free_transform", &freeEl) &&

            KisDomUtils::loadValue(freeEl, "transformedCenter", &args.m_transformedCenter) &&
            KisDomUtils::loadValue(freeEl, "originalCenter", &args.m_originalCenter) &&
            KisDomUtils::loadValue(freeEl, "rotationCenterOffset", &args.m_rotationCenterOffset) &&
            KisDomUtils::loadValue(freeEl, "transformAroundRotationCenter", &args.m_transformAroundRotationCenter) &&

            KisDomUtils::loadValue(freeEl, "aX", &args.m_aX) &&
            KisDomUtils::loadValue(freeEl, "aY", &args.m_aY) &&
            KisDomUtils::loadValue(freeEl, "aZ", &args.m_aZ) &&

            KisDomUtils::loadValue(freeEl, "cameraPos", &args.m_cameraPos) &&

            KisDomUtils::loadValue(freeEl, "scaleX", &args.m_scaleX) &&
            KisDomUtils::loadValue(freeEl, "scaleY", &args.m_scaleY) &&

            KisDomUtils::loadValue(freeEl, "shearX", &args.m_shearX) &&
            KisDomUtils::loadValue(freeEl, "shearY", &args.m_shearY) &&

            KisDomUtils::loadValue(freeEl, "keepAspectRatio", &args.m_keepAspectRatio) &&
            KisDomUtils::loadValue(freeEl, "flattenedPerspectiveTransform", &args.m_flattenedPerspectiveTransform) &&

            KisDomUtils::loadValue(freeEl, "filterId", &filterId);

        if (result) {
            args.m_filter = KisFilterStrategyRegistry::instance()->value(filterId);
            result = (bool) args.m_filter;
        }

    } else if (args.m_mode == WARP || args.m_mode == CAGE) {
        QDomElement warpEl;

        int warpType = 0;

        result =
            KisDomUtils::findOnlyElement(e, "warp_transform", &warpEl) &&

            KisDomUtils::loadValue(warpEl, "defaultPoints", &args.m_defaultPoints) &&

            KisDomUtils::loadValue(warpEl, "originalPoints", &args.m_origPoints) &&
            KisDomUtils::loadValue(warpEl, "transformedPoints", &args.m_transfPoints) &&

            KisDomUtils::loadValue(warpEl, "warpType", &warpType) &&
            KisDomUtils::loadValue(warpEl, "alpha", &args.m_alpha);

        if (result && warpType >= 0 && warpType < KisWarpTransformWorker::N_MODES) {
            args.m_warpType = (KisWarpTransformWorker::WarpType_) warpType;
        } else {
            result = false;
        }

    } else if (args.m_mode == LIQUIFY) {
        QDomElement liquifyEl;

        result =
            KisDomUtils::findOnlyElement(e, "liquify_transform", &liquifyEl);

        *args.m_liquifyProperties = KisLiquifyProperties::fromXML(e);
        args.m_liquifyWorker.reset(KisLiquifyTransformWorker::fromXML(e));
    } else {
        KIS_ASSERT_RECOVER_NOOP(0 && "Unknown transform mode");
    }

    if (!result) {
        args = ToolTransformArgs();
    }

    return args;
}

void ToolTransformArgs::saveContinuedState()
{
    m_continuedTransformation.reset();
    m_continuedTransformation.reset(new ToolTransformArgs(*this));
}

void ToolTransformArgs::restoreContinuedState()
{
    QScopedPointer<ToolTransformArgs> tempTransformation(
        new ToolTransformArgs(*m_continuedTransformation));

    *this = *tempTransformation;
    m_continuedTransformation.swap(tempTransformation);
}

const ToolTransformArgs* ToolTransformArgs::continuedTransform() const
{
    return m_continuedTransformation.data();
}
