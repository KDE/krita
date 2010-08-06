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
#include <kis_warptransform_worker.h>

//To store and restore the parameters of a transformation (describes the state of the tool)
class ToolTransformArgs
{
public:
	typedef enum TransfMode_ {FREE_TRANSFORM = 0, WARP} TransfMode;

    ToolTransformArgs();
    ToolTransformArgs(const ToolTransformArgs& args);
    ToolTransformArgs& operator=(const ToolTransformArgs& args);
    ToolTransformArgs(TransfMode mode,
						QPointF translate, QPointF rotationCenterOffset, double aX, double aY, double aZ, double scaleX, double scaleY, double shearX, double shearY,
						int pointsPerLine, KisWarpTransformWorker::WarpType warpType, double alpha, QPointF previewPos, bool defaultPoints);
						//allocates the memory for the points according to pointsPerLine value
    ~ToolTransformArgs();

	TransfMode mode() const;
	void setMode(TransfMode mode);

	//warp-related
	int pointsPerLine() const;
	QPointF &origPoint(int i);
	QPointF &transfPoint(int i);
	const QVector<QPointF> &origPoints() const;
	const QVector<QPointF> &transfPoints() const;
	KisWarpTransformWorker::WarpType warpType() const;
	double alpha() const;
	QPointF previewPos() const;
    bool defaultPoints() const;

    void setPointsPerLine(int pointsPerLine);
	void setPoints(QVector<QPointF>, QVector<QPointF> transfPoints); //makes a COPY of the given points
	void setWarpType(KisWarpTransformWorker::WarpType warpType);
	void setAlpha(double alpha);
	void setPreviewPos(QPointF previewPos);
    void setDefaultPoints(bool defaultPoints);

	//"free transform"-related
    QPointF translate() const;
    QPointF rotationCenterOffset() const;
    double aX() const;
    double aY() const;
    double aZ() const;
    double scaleX() const;
    double scaleY() const;
    double shearX() const;
    double shearY() const;

    void setTranslate(QPointF translate);
    void setRotationCenterOffset(QPointF rotationCenterOffset);
    void setAX(double aX);
    void setAY(double aY);
    void setAZ(double aZ);
    void setScaleX(double scaleX);
    void setScaleY(double scaleY);
    void setShearX(double shearX);
    void setShearY(double shearY);

    bool isIdentity(QPointF originalTranslate) const;

private:
    void clear();
    void init(const ToolTransformArgs& args);
	TransfMode m_mode;

	//warp-related
	int m_pointsPerLine;
	QVector<QPointF> m_origPoints;
	QVector<QPointF> m_transfPoints;
	KisWarpTransformWorker::WarpType m_warpType;
	double m_alpha; //for affine warp type
	QPointF m_previewPos;
    bool m_defaultPoints;

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
