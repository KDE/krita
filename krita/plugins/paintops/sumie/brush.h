/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef _BRUSH_H_
#define _BRUSH_H_

#include <QVector>
#include <QList>

#include <KoColor.h>

#include "bristle.h"
#include "brush_shape.h"
#include "stroke_sample.h"

#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_random_accessor.h>

class Brush{

public:
	Brush(const BrushShape &initialShape, KoColor inkColor);
	Brush();
	~Brush();
	Brush(KoColor inkColor, BrushShape shape);
	void paint(KisPaintDeviceSP dev, const KisPaintInformation &info);
	void paintLine(KisPaintDeviceSP dev,const KisPaintInformation &pi1, const KisPaintInformation &pi2);
	void setInkDepletion(QList<float> *curveData);
	void setInkColor(const KoColor &color);
	void addStrokeSample(StrokeSample sample);
	void addStrokeSample(float x,float y,float pressure,float tiltX, float tiltY,float rotation);

	void repositionBristles(double angle, double slope);
	void rotateBristles(double angle);

	double getAngleDelta(const KisPaintInformation& info);

	void setRadius(int radius);
	void setSigma(double sigma);
	void setBrushShape(BrushShape brushShape);
	double computeMousePressure(double distance);
	void enableMousePressure(bool enable);

    void setShear(double shearFactor){ m_shearFactor = shearFactor; }
    void setRandom(double randomFactor){ m_randomFactor = randomFactor; }
    void setScale(double scaleFactor){ m_scaleFactor = scaleFactor; }

    /// paints single bristle
    void putBristle(Bristle *bristle, float wx, float wy, const KoColor &color);
    void addBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color);

private:
	QVector<Bristle> m_bristles;
	QVector<StrokeSample> m_stroke;
	QList<float> m_inkDepletion; // array
    QTransform m_transform;


	BrushShape m_initialShape;
	KoColor m_inkColor;

    KisPaintDeviceSP m_dev;
    KisRandomAccessor * m_accessor;
    quint32 m_pixelSize;

	int m_counter;

	int m_radius;
	double m_sigma;

	double m_lastAngle;
	double m_lastSlope;

	double m_angle;
	double m_oldPressure;

	bool m_mousePressureEnabled;

    double m_scaleFactor;
    double m_randomFactor;
    double m_shearFactor;

};

#endif
