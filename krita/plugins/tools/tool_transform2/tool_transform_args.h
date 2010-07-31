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

#ifndef TOOL_TRANSFORM_ARGS_H_
#define TOOL_TRANSFORM_ARGS_H_

#include <QPointF>
#include <QVector3D>

//To store and restore the parameters of a transformation (describes the state of the tool)
class ToolTransformArgs
{
public:
	typedef enum TransfMode_ {FREE_TRANSFORM = 0, WARP} TransfMode;
	typedef enum WarpType_ {AFFINE_TRANSFORM = 0} WarpType;

    ToolTransformArgs();
    ToolTransformArgs(ToolTransformArgs& args); //makes a COPY of the points given in args
    ToolTransformArgs(TransfMode mode,
						QPointF translate, QPointF rotationCenterOffset, double aX, double aY, double aZ, double scaleX, double scaleY, double shearX, double shearY,
						int pointsPerLine, WarpType warpType, double alpha, QPointF previewPos);
						//allocates the memory for the points according to pointsPerLine value
    ~ToolTransformArgs();

	TransfMode mode();
	void setMode(TransfMode mode);

	//warp-related
	int pointsPerLine();
	QPointF *origPoints();
	QPointF *transfPoints();
	WarpType warpType();
	double alpha();
	QPointF previewPos();

	void setPoints(int pointsPerLine, QPointF *origPoints, QPointF *transfPoints); //makes a COPY of the given points
	void setWarpType(WarpType warpType);
	void setAlpha(double alpha);
	void setPreviewPos(QPointF previewPos);

	//"free transform"-related
    QPointF translate();
    QPointF rotationCenterOffset();
    double aX();
    double aY();
    double aZ();
    double scaleX();
    double scaleY();
    double shearX();
    double shearY();

    void setTranslate(QPointF translate);
    void setRotationCenterOffset(QPointF rotationCenterOffset);
    void setAX(double aX);
    void setAY(double aY);
    void setAZ(double aZ);
    void setScaleX(double scaleX);
    void setScaleY(double scaleY);
    void setShearX(double shearX);
    void setShearY(double shearY);

private:
	TransfMode m_mode;

	//warp-related
	int m_pointsPerLine;
	QPointF *m_origPoints;
	QPointF *m_transfPoints;
	WarpType m_warpType;
	double m_alpha; //for affine warp type
	QPointF m_previewPos;

	//"free transform"-related
    QPointF m_translate;
    QPointF m_rotationCenterOffset;
    double m_aX;
    double m_aY;
    double m_aZ;
    double m_scaleX;
    double m_scaleY;
    double m_shearX;
    double m_shearY;
};

#endif
