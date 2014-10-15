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

#include "kis_liquify_transform_worker.h"


ToolTransformArgs::ToolTransformArgs()
{
    m_mode = FREE_TRANSFORM;
    m_transformedCenter = QPointF(0, 0);
    m_originalCenter = QPointF(0, 0);
    m_rotationCenterOffset = QPointF(0, 0);
    m_cameraPos = QVector3D(0,0,1024);
    m_eyePos = -m_cameraPos;
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

    setFilterId("Bicubic");
    m_editTransformPoints = false;
}

void ToolTransformArgs::init(const ToolTransformArgs& args)
{
    m_mode = args.mode();
    m_transformedCenter = args.transformedCenter();
    m_originalCenter = args.originalCenter();
    m_rotationCenterOffset = args.rotationCenterOffset();
    m_cameraPos = QVector3D(0,0,1024);
    m_eyePos = -m_cameraPos;
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
    m_liquifyProperties = args.m_liquifyProperties;

    if (args.m_liquifyWorker) {
        m_liquifyWorker.reset(new KisLiquifyTransformWorker(*args.m_liquifyWorker.data()));
    }
}

void ToolTransformArgs::clear()
{
    m_origPoints.clear();
    m_transfPoints.clear();
}

ToolTransformArgs::ToolTransformArgs(const ToolTransformArgs& args)
{
    init(args);
}

ToolTransformArgs& ToolTransformArgs::operator=(const ToolTransformArgs& args)
{
    clear();
    init(args);

    return *this;
}

ToolTransformArgs::ToolTransformArgs(TransformMode mode,
                                     QPointF transformedCenter,
                                     QPointF originalCenter,
                                     QPointF rotationCenterOffset,
                                     double aX, double aY, double aZ,
                                     double scaleX, double scaleY,
                                     double shearX, double shearY,
                                     KisWarpTransformWorker::WarpType warpType,
                                     double alpha,
                                     bool defaultPoints,
                                     const QString &filterId)
{
    m_mode = mode;
    m_transformedCenter = transformedCenter;
    m_originalCenter = originalCenter;
    m_rotationCenterOffset = rotationCenterOffset;
    m_cameraPos = QVector3D(0,0,1024);
    m_eyePos = -m_cameraPos;
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
        qWarning("Not implemented!");
        return false;
    } else {
        KIS_ASSERT_RECOVER_NOOP(0 && "unknown transform mode");
        return true;
    }
}

void ToolTransformArgs::initLiquifyTransformMode(const QRect &srcRect)
{
    m_liquifyWorker.reset(new KisLiquifyTransformWorker(srcRect, 0, 8));
    m_liquifyProperties.loadMode();
}

void ToolTransformArgs::saveLiquifyTransformMode() const
{
    m_liquifyProperties.saveMode();
}

