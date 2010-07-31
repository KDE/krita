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
	m_origPoints = NULL;
	m_transfPoints = NULL;
	m_warpType = AFFINE_TRANSFORM;
	m_alpha = 1.0;
	m_previewPos = QPointF(0, 0);
}

ToolTransformArgs::ToolTransformArgs(ToolTransformArgs& args)
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
	if (m_pointsPerLine > 0) {
		int nbPoints = m_pointsPerLine * m_pointsPerLine;
		m_origPoints = new QPointF[nbPoints];
		m_transfPoints = new QPointF[nbPoints];
		QPointF *argsOrigPoints = args.origPoints();
		QPointF *argsTransfPoints = args.transfPoints();
		for (int i = 0; i < nbPoints; ++i) {
			m_origPoints[i] = argsOrigPoints[i];
			m_transfPoints[i] = argsTransfPoints[i];
		}
	}
	m_warpType = args.warpType();
	m_alpha = args.alpha();
	m_previewPos = args.previewPos();
}

ToolTransformArgs::ToolTransformArgs(TransfMode mode,
							QPointF translate, QPointF rotationCenterOffset, double aX, double aY, double aZ, double scaleX, double scaleY, double shearX, double shearY,
							int pointsPerLine, WarpType warpType, double alpha, QPointF previewPos)
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
	if (m_pointsPerLine > 0) {
		int nbPoints = m_pointsPerLine * m_pointsPerLine;

		m_origPoints = new QPointF[nbPoints];
		m_transfPoints = new QPointF[nbPoints];
	}

	m_warpType = warpType;
	m_alpha = alpha;
	m_previewPos = previewPos;
}


ToolTransformArgs::~ToolTransformArgs()
{
}

ToolTransformArgs::TransfMode ToolTransformArgs::mode()
{
	return m_mode;
}

void ToolTransformArgs::setMode(TransfMode mode)
{
	m_mode = mode;
}

int ToolTransformArgs::pointsPerLine()
{
	return m_pointsPerLine;
}

QPointF *ToolTransformArgs::origPoints()
{
	return m_origPoints;
}

QPointF *ToolTransformArgs::transfPoints()
{
	return m_transfPoints;
}

ToolTransformArgs::WarpType ToolTransformArgs::warpType()
{
	return m_warpType;
}

double ToolTransformArgs::alpha()
{
	return m_alpha;
}

QPointF ToolTransformArgs::previewPos()
{
	return m_previewPos;
}

void ToolTransformArgs::setPoints(int pointsPerLine, QPointF *origPoints, QPointF *transfPoints)
{
	int nbPoints = pointsPerLine * pointsPerLine;
	if (m_pointsPerLine != pointsPerLine) {
		if (m_pointsPerLine > 0) {
			delete m_origPoints;
			delete m_transfPoints;
		}

		m_pointsPerLine = pointsPerLine;
		m_origPoints = new QPointF[nbPoints];
		m_transfPoints = new QPointF[nbPoints];
	}

	for (int i = 0; i < nbPoints; ++i) {
		m_origPoints[i] = origPoints[i];
		m_transfPoints[i] = transfPoints[i];
	}
}

void ToolTransformArgs::setWarpType(WarpType warpType)
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

QPointF ToolTransformArgs::translate()
{
    return m_translate;
}

QPointF ToolTransformArgs::rotationCenterOffset()
{
    return m_rotationCenterOffset;
}

double ToolTransformArgs::aX()
{
    return m_aX;
}

double ToolTransformArgs::aY()
{
    return m_aY;
}

double ToolTransformArgs::aZ()
{
    return m_aZ;
}

double ToolTransformArgs::scaleX()
{
    return m_scaleX;
}

double ToolTransformArgs::scaleY()
{
    return m_scaleY;
}

double ToolTransformArgs::shearX()
{
    return m_shearX;
}

double ToolTransformArgs::shearY()
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
