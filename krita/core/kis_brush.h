/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_BRUSH_
#define KIS_BRUSH_

#include <qcstring.h>
#include <qsize.h>
#include <qimage.h>
#include <qvaluevector.h>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_point.h"
#include "kis_alpha_mask.h"

class QPoint;
class QPixmap;
enum enumBrushType {
        INVALID,
	MASK,
	IMAGE,
	PIPE_MASK,
	PIPE_IMAGE,
	AIRBRUSH
};

class KisBrush : public KisResource {
	typedef KisResource super;
	Q_OBJECT

public:
	KisBrush(const QString& filename);
	KisBrush(const QString& filename, 
		 const QByteArray & data,
		 Q_UINT32 & dataPos);
	
	virtual ~KisBrush();

	virtual bool loadAsync();
	virtual bool saveAsync();
	virtual QImage img();

	/**
	   @return a mask computed from the grey-level values of the
	   pixels in the brush.
	*/
	virtual KisAlphaMaskSP mask(double pressure = PRESSURE_DEFAULT, double subPixelX = 0, double subPixelY = 0) const;
	// XXX: return non-tiled simple buffer
	virtual KisLayerSP image(KisStrategyColorSpaceSP colorSpace, double pressure = PRESSURE_DEFAULT, double subPixelX = 0, double subPixelY = 0) const;

	void setHotSpot(KisPoint);
	KisPoint hotSpot(double pressure = PRESSURE_DEFAULT) const;

	void setSpacing(double s) { m_spacing = s; }
	double spacing() const { return m_spacing; }
	double xSpacing(double pressure = PRESSURE_DEFAULT) const;
	double ySpacing(double pressure = PRESSURE_DEFAULT) const;

	virtual void setUseColorAsMask(bool useColorAsMask) { m_useColorAsMask = useColorAsMask; }
	virtual bool useColorAsMask() const { return m_useColorAsMask; }
	virtual bool hasColor() const;

	virtual enumBrushType brushType() const;

protected:
	void setImage(const QImage& img);
	void setBrushType(enumBrushType type) { m_brushType = type; };
	static double scaleForPressure(double pressure);

private slots:
	void ioData(KIO::Job *job, const QByteArray& data);
	void ioResult(KIO::Job *job);

private:
	class ScaledBrush {
	public:
		ScaledBrush();
		ScaledBrush(KisAlphaMaskSP scaledMask, const QImage& scaledImage, double scale, double xScale, double yScale);

		double scale() const { return m_scale; }
		double xScale() const { return m_xScale; }
		double yScale() const { return m_yScale; }
		KisAlphaMaskSP mask() const { return m_mask; }
		QImage image() const { return m_image; }

	private:
		KisAlphaMaskSP m_mask;
		QImage m_image;
		double m_scale;
		double m_xScale;
		double m_yScale;
	};

	void createScaledBrushes() const;

	KisAlphaMaskSP scaleMask(const ScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const;
	QImage scaleImage(const ScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const;

	static QImage scaleImage(const QImage& srcImage, int width, int height);
	static QImage interpolate(const QImage& image1, const QImage& image2, double t);

	static KisAlphaMaskSP scaleSinglePixelMask(double scale, QUANTUM maskValue, double subPixelX, double subPixelY);
	static QImage scaleSinglePixelImage(double scale, QRgb pixel, double subPixelX, double subPixelY);

	// Find the scaled brush(es) nearest to the given scale.
	void findScaledBrushes(double scale, const ScaledBrush **aboveBrush, const ScaledBrush **belowBrush) const;

	QByteArray m_data;
	bool m_ownData;
	KisPoint m_hotSpot;
	double m_spacing;
	bool m_useColorAsMask;
	bool m_hasColor;
	QImage m_img;
	mutable QValueVector<ScaledBrush> m_scaledBrushes;

	Q_UINT32 m_header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
	Q_UINT32 m_version;      /*  brush file version #  */
	Q_UINT32 m_bytes;        /*  depth of brush in bytes */
	Q_UINT32 m_magic_number; /*  GIMP brush magic number  */

	enumBrushType m_brushType;

};
#endif // KIS_BRUSH_

