/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <cfloat>

#include "qbrush.h"
#include "qcolor.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpen.h"
#include "qregion.h"
#include "qwmatrix.h"
#include <qimage.h>
#include <qmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qstring.h>

#include <kdebug.h>
#include <kcommand.h>
#include <klocale.h>

#include <koColor.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_gradient.h"
#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "kis_strategy_colorspace.h"
#include "kis_tile_command.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kispixeldata.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kis_selection.h"
#include "kis_gradient_painter.h"

namespace {

	class GradientShapeStrategy {
	public:
		GradientShapeStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);
		virtual ~GradientShapeStrategy() {}

		virtual double valueAt(double x, double y) const = 0;

	protected:
		KisPoint m_gradientVectorStart;
		KisPoint m_gradientVectorEnd;
	};

	GradientShapeStrategy::GradientShapeStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
		: m_gradientVectorStart(gradientVectorStart), m_gradientVectorEnd(gradientVectorEnd)
	{
	}


	class LinearGradientStrategy : public GradientShapeStrategy {
		typedef GradientShapeStrategy super;
	public:
		LinearGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

		virtual double valueAt(double x, double y) const;

	protected:
		double m_normalisedVectorX;
		double m_normalisedVectorY;
		double m_vectorLength;
	};

	LinearGradientStrategy::LinearGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
		: super(gradientVectorStart, gradientVectorEnd)
	{
		double dx = gradientVectorEnd.x() - gradientVectorStart.x();
		double dy = gradientVectorEnd.y() - gradientVectorStart.y();

		m_vectorLength = sqrt((dx * dx) + (dy * dy));

		if (m_vectorLength < DBL_EPSILON) {
			m_normalisedVectorX = 0;
			m_normalisedVectorY = 0;
		}
		else {
			m_normalisedVectorX = dx / m_vectorLength;
			m_normalisedVectorY = dy / m_vectorLength;
		}
	}

	double LinearGradientStrategy::valueAt(double x, double y) const
	{
		double vx = x - m_gradientVectorStart.x();
		double vy = y - m_gradientVectorStart.y();

		// Project the vector onto the normalised gradient vector.
		double t = vx * m_normalisedVectorX + vy * m_normalisedVectorY;

		if (m_vectorLength < DBL_EPSILON) {
			t = 0;
		}
		else {
			// Scale to 0 to 1 over the gradient vector length.
			t /= m_vectorLength;
		}

		return t;
	}


	class BiLinearGradientStrategy : public LinearGradientStrategy {
		typedef LinearGradientStrategy super;
	public:
		BiLinearGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

		virtual double valueAt(double x, double y) const;
	};

	BiLinearGradientStrategy::BiLinearGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
		: super(gradientVectorStart, gradientVectorEnd)
	{
	}

	double BiLinearGradientStrategy::valueAt(double x, double y) const
	{
		double t = super::valueAt(x, y);

		// Reflect
		if (t < -DBL_EPSILON) {
			t = -t;
		}

		return t;
	}


	class RadialGradientStrategy : public GradientShapeStrategy {
		typedef GradientShapeStrategy super;
	public:
		RadialGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

		virtual double valueAt(double x, double y) const;

	protected:
		double m_radius;
	};

	RadialGradientStrategy::RadialGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
		: super(gradientVectorStart, gradientVectorEnd)
	{
		double dx = gradientVectorEnd.x() - gradientVectorStart.x();
		double dy = gradientVectorEnd.y() - gradientVectorStart.y();

		m_radius = sqrt((dx * dx) + (dy * dy));
	}

	double RadialGradientStrategy::valueAt(double x, double y) const
	{
		double dx = x - m_gradientVectorStart.x();
		double dy = y - m_gradientVectorStart.y();

		double distance = sqrt((dx * dx) + (dy * dy));

		double t;

		if (m_radius < DBL_EPSILON) {
			t = 0;
		}
		else {
			t = distance / m_radius;
		}

		return t;
	}


	class SquareGradientStrategy : public GradientShapeStrategy {
		typedef GradientShapeStrategy super;
	public:
		SquareGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

		virtual double valueAt(double x, double y) const;

	protected:
		double m_normalisedVectorX;
		double m_normalisedVectorY;
		double m_vectorLength;
	};

	SquareGradientStrategy::SquareGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
		: super(gradientVectorStart, gradientVectorEnd)
	{
		double dx = gradientVectorEnd.x() - gradientVectorStart.x();
		double dy = gradientVectorEnd.y() - gradientVectorStart.y();

		m_vectorLength = sqrt((dx * dx) + (dy * dy));

		if (m_vectorLength < DBL_EPSILON) {
			m_normalisedVectorX = 0;
			m_normalisedVectorY = 0;
		}
		else {
			m_normalisedVectorX = dx / m_vectorLength;
			m_normalisedVectorY = dy / m_vectorLength;
		}
	}

	double SquareGradientStrategy::valueAt(double x, double y) const
	{
		double px = x - m_gradientVectorStart.x();
		double py = y - m_gradientVectorStart.y();

		double distance1 = 0;
		double distance2 = 0;

		if (m_vectorLength > DBL_EPSILON) {

			// Point to line distance is:
			// distance = ((l0.y() - l1.y()) * p.x() + (l1.x() - l0.x()) * p.y() + l0.x() * l1.y() - l1.x() * l0.y()) / m_vectorLength;
			//
			// Here l0 = (0, 0) and |l1 - l0| = 1

			distance1 = -m_normalisedVectorY * px + m_normalisedVectorX * py;
			distance1 = fabs(distance1);

			// Rotate point by 90 degrees and get the distance to the perpendicular
			distance2 = -m_normalisedVectorY * -py + m_normalisedVectorX * px;
			distance2 = fabs(distance2);
		}

		double t = QMAX(distance1, distance2) / m_vectorLength;

		return t;
	}


	class ConicalGradientStrategy : public GradientShapeStrategy {
		typedef GradientShapeStrategy super;
	public:
		ConicalGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

		virtual double valueAt(double x, double y) const;

	protected:
		double m_vectorAngle;
	};

	ConicalGradientStrategy::ConicalGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
		: super(gradientVectorStart, gradientVectorEnd)
	{
		double dx = gradientVectorEnd.x() - gradientVectorStart.x();
		double dy = gradientVectorEnd.y() - gradientVectorStart.y();

		// Get angle from 0 to 2 PI.
		m_vectorAngle = atan2(dy, dx) + M_PI;
	}

	double ConicalGradientStrategy::valueAt(double x, double y) const
	{
		double px = x - m_gradientVectorStart.x();
		double py = y - m_gradientVectorStart.y();

		double angle = atan2(py, px) + M_PI;

		angle -= m_vectorAngle;

		if (angle < 0) {
			angle += 2 * M_PI;
		}

		double t = angle / (2 * M_PI);

		return t;
	}


	class ConicalSymetricGradientStrategy : public GradientShapeStrategy {
		typedef GradientShapeStrategy super;
	public:
		ConicalSymetricGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

		virtual double valueAt(double x, double y) const;

	protected:
		double m_vectorAngle;
	};

	ConicalSymetricGradientStrategy::ConicalSymetricGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
		: super(gradientVectorStart, gradientVectorEnd)
	{
		double dx = gradientVectorEnd.x() - gradientVectorStart.x();
		double dy = gradientVectorEnd.y() - gradientVectorStart.y();

		// Get angle from 0 to 2 PI.
		m_vectorAngle = atan2(dy, dx) + M_PI;
	}

	double ConicalSymetricGradientStrategy::valueAt(double x, double y) const
	{
		double px = x - m_gradientVectorStart.x();
		double py = y - m_gradientVectorStart.y();

		double angle = atan2(py, px) + M_PI;

		angle -= m_vectorAngle;

		if (angle < 0) {
			angle += 2 * M_PI;
		}

		double t;

		if (angle < M_PI) {
			t = angle / M_PI;
		}
		else {
			t = 1 - ((angle - M_PI) / M_PI);
		}

		return t;
	}


	class GradientRepeatStrategy {
	public:
		GradientRepeatStrategy() {}
		virtual ~GradientRepeatStrategy() {}

		virtual double valueAt(double t) const = 0;
	};


	class GradientRepeatNoneStrategy : public GradientRepeatStrategy {
	public:
		static GradientRepeatNoneStrategy *instance();

		virtual double valueAt(double t) const;

	private:
		GradientRepeatNoneStrategy() {}

		static GradientRepeatNoneStrategy *m_instance;
	};

	GradientRepeatNoneStrategy *GradientRepeatNoneStrategy::m_instance = 0;

	GradientRepeatNoneStrategy *GradientRepeatNoneStrategy::instance()
	{
		if (m_instance == 0) {
			m_instance = new GradientRepeatNoneStrategy();
		}

		return m_instance;
	}

	// Output is clamped to 0 to 1.
	double GradientRepeatNoneStrategy::valueAt(double t) const
	{
		double value = t;

		if (t < DBL_EPSILON) {
			value = 0;
		}
		else
			if (t > 1 - DBL_EPSILON) {
				value = 1;
			}

		return value;
	}


	class GradientRepeatForwardsStrategy : public GradientRepeatStrategy {
	public:
		static GradientRepeatForwardsStrategy *instance();

		virtual double valueAt(double t) const;

	private:
		GradientRepeatForwardsStrategy() {}

		static GradientRepeatForwardsStrategy *m_instance;
	};

	GradientRepeatForwardsStrategy *GradientRepeatForwardsStrategy::m_instance = 0;

	GradientRepeatForwardsStrategy *GradientRepeatForwardsStrategy::instance()
	{
		if (m_instance == 0) {
			m_instance = new GradientRepeatForwardsStrategy();
		}

		return m_instance;
	}

	// Output is 0 to 1, 0 to 1, 0 to 1...
	double GradientRepeatForwardsStrategy::valueAt(double t) const
	{
		int i = static_cast<int>(t);

		if (t < DBL_EPSILON) {
			i--;
		}

		double value = t - i;

		return value;
	}


	class GradientRepeatAlternateStrategy : public GradientRepeatStrategy {
	public:
		static GradientRepeatAlternateStrategy *instance();

		virtual double valueAt(double t) const;

	private:
		GradientRepeatAlternateStrategy() {}

		static GradientRepeatAlternateStrategy *m_instance;
	};

	GradientRepeatAlternateStrategy *GradientRepeatAlternateStrategy::m_instance = 0;

	GradientRepeatAlternateStrategy *GradientRepeatAlternateStrategy::instance()
	{
		if (m_instance == 0) {
			m_instance = new GradientRepeatAlternateStrategy();
		}

		return m_instance;
	}

	// Output is 0 to 1, 1 to 0, 0 to 1, 1 to 0...
	double GradientRepeatAlternateStrategy::valueAt(double t) const
	{
		if (t < 0) {
			t = -t;
		}

		int i = static_cast<int>(t);

                double value = t - i;

		if (i % 2 == 1) {
			value = 1 - value;
		}

		return value;
	}
}

KisGradientPainter::KisGradientPainter() 
	: super()
{ 
	m_gradient = 0; 
}

KisGradientPainter::KisGradientPainter(KisPaintDeviceSP device) : super(device), m_gradient(0) 
{
}

bool KisGradientPainter::paintGradient(const KisPoint& gradientVectorStart,
				       const KisPoint& gradientVectorEnd,
				       enumGradientShape shape,
				       enumGradientRepeat repeat,
				       double antiAliasThreshold,
				       bool reverseGradient)
{
	m_cancelRequested = false;

	GradientShapeStrategy *shapeStrategy = 0;

	switch (shape) {
	case GradientShapeLinear:
		shapeStrategy = new LinearGradientStrategy(gradientVectorStart, gradientVectorEnd);
		break;
	case GradientShapeBiLinear:
		shapeStrategy = new BiLinearGradientStrategy(gradientVectorStart, gradientVectorEnd);
		break;
	case GradientShapeRadial:
		shapeStrategy = new RadialGradientStrategy(gradientVectorStart, gradientVectorEnd);
		break;
	case GradientShapeSquare:
		shapeStrategy = new SquareGradientStrategy(gradientVectorStart, gradientVectorEnd);
		break;
	case GradientShapeConical:
		shapeStrategy = new ConicalGradientStrategy(gradientVectorStart, gradientVectorEnd);
		break;
	case GradientShapeConicalSymetric:
		shapeStrategy = new ConicalSymetricGradientStrategy(gradientVectorStart, gradientVectorEnd);
		break;
	}

	Q_ASSERT(shapeStrategy != 0);

	GradientRepeatStrategy *repeatStrategy = 0;

	switch (repeat) {
	case GradientRepeatNone:
		repeatStrategy = GradientRepeatNoneStrategy::instance();
		break;
	case GradientRepeatForwards:
		repeatStrategy = GradientRepeatForwardsStrategy::instance();
		break;
	case GradientRepeatAlternate:
		repeatStrategy = GradientRepeatAlternateStrategy::instance();
		break;
	}

	Q_ASSERT(repeatStrategy != 0);

	KisLayerSP layer = new KisLayer(m_device -> width(), m_device -> height(), m_device -> colorStrategy(), "gradient");
	KisPainter painter(layer.data());

	int totalPixels = layer -> width() * layer -> height();

	if (antiAliasThreshold < 1 - DBL_EPSILON) {
		totalPixels *= 2;
	}

	int pixelsProcessed = 0;
	int lastProgressPercent = 0;

	emit notifyProgressStage(this, i18n("Rendering gradient..."), 0);

	for (int x = 0; x < layer -> width(); x++) {
		for (int y = 0; y < layer -> height(); y++) {

			double t = shapeStrategy -> valueAt(x, y);
			t = repeatStrategy -> valueAt(t);

			if (reverseGradient) {
				t = 1 - t;
			}

			KoColor color;
			QUANTUM opacity;

			m_gradient -> colorAt(t, &color, &opacity);
			layer ->setPixel(x, y, color, opacity);

			pixelsProcessed++;

			int progressPercent = (pixelsProcessed * 100) / totalPixels;

			if (progressPercent > lastProgressPercent) {
				emit notifyProgress(this, progressPercent);
				lastProgressPercent = progressPercent;

				if (m_cancelRequested) {
					break;
				}
			}
		}

		if (m_cancelRequested) {
			break;
		}
	}

	if (!m_cancelRequested && antiAliasThreshold < 1 - DBL_EPSILON) {

		KisLayerSP antiAliasedLayer = new KisLayer(*layer);

		emit notifyProgressStage(this, i18n("Anti-aliasing gradient..."), lastProgressPercent);

		//QImage distanceImage(layer -> width(), layer -> height(), 32);

		for (int y = 0; y < layer -> height(); y++) {
			for (int x = 0; x < layer -> width(); x++) {

				double maxDistance = 0;

				KoColor thisPixel;
				QUANTUM thisPixelOpacity;

				layer -> pixel(x, y, &thisPixel, &thisPixelOpacity);

				for (int yOffset = -1; yOffset < 2; yOffset++) {
					for (int xOffset = -1; xOffset < 2; xOffset++) {

						if (xOffset != 0 || yOffset != 0) {

							int sampleX = x + xOffset;
							int sampleY = y + yOffset;

							if (sampleX >= 0 && sampleX < layer -> width() && sampleY >=0 && sampleY < layer -> height()) {
								KoColor color;
								QUANTUM opacity;

								layer -> pixel(sampleX, sampleY, &color, &opacity);

								double dRed = (color.R() * opacity - thisPixel.R() * thisPixelOpacity) / 65535.0;
								double dGreen = (color.G() * opacity - thisPixel.G() * thisPixelOpacity) / 65535.0;
								double dBlue = (color.B() * opacity - thisPixel.B() * thisPixelOpacity) / 65535.0;

#define SQRT_3 1.7320508

								double distance = sqrt(dRed * dRed + dGreen * dGreen + dBlue * dBlue) / SQRT_3;

								if (distance > maxDistance) {
									maxDistance = distance;
								}
							}
						}
					}
				}

				//int distCol = (int)(maxDistance * 255 + 0.5);
				//distanceImage.setPixel(x, y, qRgb(distCol, distCol, distCol));

				if (maxDistance > antiAliasThreshold) {
					const int numSamples = 4;

					int totalRed = 0;
					int totalGreen = 0;
					int totalBlue = 0;
					int totalOpacity = 0;

					for (int ySample = 0; ySample < numSamples; ySample++) {
						for (int xSample = 0; xSample < numSamples; xSample++) {

							double sampleWidth = 1.0 / numSamples;

							double sampleX = x - 0.5 + (sampleWidth / 2) + xSample * sampleWidth;
							double sampleY = y - 0.5 + (sampleWidth / 2) + ySample * sampleWidth;

							double t = shapeStrategy -> valueAt(sampleX, sampleY);
							t = repeatStrategy -> valueAt(t);

							if (reverseGradient) {
								t = 1 - t;
							}

							KoColor color;
							QUANTUM opacity;

							m_gradient -> colorAt(t, &color, &opacity);

							totalRed += color.R();
							totalGreen += color.G();
							totalBlue += color.B();
							totalOpacity += opacity;
						}
					}

					int red = totalRed / (numSamples * numSamples);
					int green = totalGreen / (numSamples * numSamples);
					int blue = totalBlue / (numSamples * numSamples);
					int opacity = totalOpacity / (numSamples * numSamples);

					KoColor color(red, green,  blue);

					antiAliasedLayer ->setPixel(x, y, color, opacity);
				}

				pixelsProcessed++;

				int progressPercent = (pixelsProcessed * 100) / totalPixels;

				if (progressPercent > lastProgressPercent) {
					emit notifyProgress(this, progressPercent);
					lastProgressPercent = progressPercent;

					if (m_cancelRequested) {
						break;
					}
				}
			}

			if (m_cancelRequested) {
				break;
			}
		}

		layer = antiAliasedLayer;
	}

	//distanceImage.save("distance.png",  "PNG");

	if (!m_cancelRequested) {
		KisLayer * l = dynamic_cast<KisLayer*>(m_device.data());
		if (l -> hasSelection()) {
			// apply mask...
			KisSelectionSP selection = l -> selection();
			for (int y = 0; y < layer -> height(); y++) {
				for (int x = 0; x < layer -> width(); x++) {
					KoColor c;
					QUANTUM opacity;
					layer -> pixel(x, y, &c, &opacity);
					opacity = ((OPACITY_OPAQUE - selection -> selected(x, y)) * opacity) / QUANTUM_MAX;
					layer -> setPixel(x, y, c, opacity); // XXX: we need a setOpacity in KisPaintDevice!
					
				}
			}

			
		}

		bitBlt(0, 0, m_compositeOp, layer.data(), m_opacity, 0, 0, layer -> width(), layer -> height());
	}

	delete shapeStrategy;

	emit notifyProgressDone(this);

	return !m_cancelRequested;
}
