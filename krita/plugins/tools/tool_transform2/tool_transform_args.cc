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

ToolTransformArgs::ToolTransformArgs()
{
    m_mode = FREE_TRANSFORM;
    m_translate = QPointF(0, 0);
    m_rotationCenterOffset = QPointF(0, 0);
    m_aX = 0;
    m_aY = 0;
    m_aZ = 0;
    m_scaleX = 1.0;
    m_scaleY = 1.0;
    m_shearX = 0.0;
    m_shearY = 0.0;
    m_pointsPerLine = 0;
    m_origPoints = QVector<QPointF>();
    m_transfPoints = QVector<QPointF>();
    m_warpType = KisWarpTransformWorker::AFFINE_TRANSFORM;
    m_alpha = 1.0;
    m_previewPos = QPointF(0, 0);
    m_keepAspectRatio = true;

    setFilterId("Bicubic");
}

void ToolTransformArgs::init(const ToolTransformArgs& args)
{
    m_mode = args.mode();
    m_translate = args.translate();
    m_rotationCenterOffset = args.rotationCenterOffset();
    m_aX = args.aX();
    m_aY = args.aY();
    m_aZ = args.aZ();
    m_scaleX = args.scaleX();
    m_scaleY = args.scaleY();
    m_shearX = args.shearX();
    m_shearY = args.shearY();
    m_pointsPerLine = args.pointsPerLine();
    m_origPoints = args.origPoints(); //it's a copy
    m_transfPoints = args.transfPoints();
    m_warpType = args.warpType();
    m_alpha = args.alpha();
    m_previewPos = args.previewPos();
    m_defaultPoints = args.defaultPoints();
    m_keepAspectRatio = args.keepAspectRatio();
    m_filter = args.m_filter;
}

void ToolTransformArgs::clear()
{
    m_origPoints.clear();
    m_transfPoints.clear();
    m_pointsPerLine = 0;
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
                                     QPointF translate, QPointF rotationCenterOffset, double aX, double aY, double aZ, double scaleX, double scaleY, double shearX, double shearY,
                                     KisWarpTransformWorker::WarpType warpType, double alpha, QPointF previewPos, bool defaultPoints)
{
    m_mode = mode;
    m_translate = translate;
    m_rotationCenterOffset = rotationCenterOffset;
    m_aX = aX;
    m_aY = aY;
    m_aZ = aZ;
    m_scaleX = scaleX;
    m_scaleY = scaleY;
    m_shearX = shearX;
    m_shearY = shearY;
    m_pointsPerLine = 0;
    m_origPoints = QVector<QPointF>();
    m_transfPoints = QVector<QPointF>();

    m_warpType = warpType;
    m_alpha = alpha;
    m_previewPos = previewPos;
    m_defaultPoints = defaultPoints;
    m_keepAspectRatio = true;
    setFilterId("Bicubic");
}


ToolTransformArgs::~ToolTransformArgs()
{
    clear();
}

bool ToolTransformArgs::isIdentity(QPointF originalTranslate) const
{
    if (m_mode == FREE_TRANSFORM) {
        return (m_translate == originalTranslate && m_scaleX == 1
                && m_scaleY == 1 && m_shearX == 0 && m_shearY == 0
                && m_aX == 0 && m_aY == 0 && m_aZ == 0);
    } else {
        for (int i = 0; i < m_origPoints.size(); ++i)
            if (m_origPoints[i] != m_transfPoints[i])
                return false;

        return true;
    }
}

