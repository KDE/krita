/*
 *  tool_transform_args.h - part of Krita
 *
 *  Copyright (c) 2010 Marc Pegon
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
	m_mode = WARP;
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

ToolTransformArgs::ToolTransformArgs(TransfMode mode,
							QPointF translate, QPointF rotationCenterOffset, double aX, double aY, double aZ, double scaleX, double scaleY, double shearX, double shearY,
							int pointsPerLine, KisWarpTransformWorker::WarpType warpType, double alpha, QPointF previewPos, bool defaultPoints)
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
	m_pointsPerLine = pointsPerLine;
    int nbPoints = m_pointsPerLine * m_pointsPerLine;
    m_origPoints.resize(nbPoints);
    m_transfPoints.resize(nbPoints);

	m_warpType = warpType;
	m_alpha = alpha;
	m_previewPos = previewPos;
    m_defaultPoints = defaultPoints;
}


ToolTransformArgs::~ToolTransformArgs()
{
    clear();
}

ToolTransformArgs::TransfMode ToolTransformArgs::mode() const
{
	return m_mode;
}

void ToolTransformArgs::setMode(TransfMode mode)
{
	m_mode = mode;
}

int ToolTransformArgs::pointsPerLine() const
{
	return m_pointsPerLine;
}

const QVector<QPointF> &ToolTransformArgs::origPoints() const
{
	return m_origPoints;
}

QPointF &ToolTransformArgs::origPoint(int i)
{
    return m_origPoints[i];
}

const QVector<QPointF> &ToolTransformArgs::transfPoints() const
{
	return m_transfPoints;
}

QPointF &ToolTransformArgs::transfPoint(int i)
{
    return m_transfPoints[i];
}

KisWarpTransformWorker::WarpType ToolTransformArgs::warpType() const
{
	return m_warpType;
}

double ToolTransformArgs::alpha() const
{
	return m_alpha;
}

QPointF ToolTransformArgs::previewPos() const
{
	return m_previewPos;
}

bool ToolTransformArgs::defaultPoints() const
{
    return m_defaultPoints;
}

void ToolTransformArgs::setPointsPerLine(int pointsPerLine)
{
    m_pointsPerLine = pointsPerLine;
}

void ToolTransformArgs::setPoints(QVector<QPointF> origPoints, QVector<QPointF> transfPoints)
{
    m_origPoints = QVector<QPointF>(origPoints);
    m_transfPoints = QVector<QPointF>(transfPoints);
}

void ToolTransformArgs::setWarpType(KisWarpTransformWorker::WarpType warpType)
{
	m_warpType = warpType;
}

void ToolTransformArgs::setAlpha(double alpha)
{
	m_alpha = alpha;
}

void ToolTransformArgs::setPreviewPos(QPointF previewPos)
{
	m_previewPos = previewPos;
}

void ToolTransformArgs::setDefaultPoints(bool defaultPoints)
{
    m_defaultPoints = defaultPoints;
}

QPointF ToolTransformArgs::translate() const
{
    return m_translate;
}

QPointF ToolTransformArgs::rotationCenterOffset() const
{
    return m_rotationCenterOffset;
}

double ToolTransformArgs::aX() const
{
    return m_aX;
}

double ToolTransformArgs::aY() const
{
    return m_aY;
}

double ToolTransformArgs::aZ() const
{
    return m_aZ;
}

double ToolTransformArgs::scaleX() const
{
    return m_scaleX;
}

double ToolTransformArgs::scaleY() const
{
    return m_scaleY;
}

double ToolTransformArgs::shearX() const
{
    return m_shearX;
}

double ToolTransformArgs::shearY() const
{
    return m_shearY;
}

void ToolTransformArgs::setTranslate(QPointF translate)
{
    m_translate = translate;
}

void ToolTransformArgs::setRotationCenterOffset(QPointF rotationCenterOffset)
{
    m_rotationCenterOffset = rotationCenterOffset;
}

void ToolTransformArgs::setAX(double aX)
{
    m_aX = aX;
}

void ToolTransformArgs::setAY(double aY)
{
    m_aY = aY;
}

void ToolTransformArgs::setAZ(double aZ)
{
    m_aZ = aZ;
}

void ToolTransformArgs::setScaleX(double scaleX)
{
    m_scaleX = scaleX;
}

void ToolTransformArgs::setScaleY(double scaleY)
{
    m_scaleY = scaleY;
}

void ToolTransformArgs::setShearX(double shearX)
{
    m_shearX = shearX;
}

void ToolTransformArgs::setShearY(double shearY)
{
    m_shearY = shearY;
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

