/*
 *  copyright (c) 2002 patrick julien <freak@codepimps.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_PAINT_DEVICE_H_
#define KIS_PAINT_DEVICE_H_

#include <qcolor.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qvaluelist.h>
#include <qstring.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_image.h"
#include "tiles/kis_datamanager.h"
#include "kis_strategy_colorspace.h"
#include "kis_scale_visitor.h"
#include "kis_pixel.h"

#include <koffice_export.h>

class QImage;
class QSize;
class QPoint;
class KoStore;
class KisImage;
class QWMatrix;
class KisTileCommand;
class KisRotateVisitor;
class KisRectIteratorPixel;
class KisVLineIteratorPixel;
class KisHLineIteratorPixel;

/**
 * Class modelled on QPaintDevice.
 */
class KRITACORE_EXPORT KisPaintDevice : public QObject, public KShared {
        Q_OBJECT

public:
	KisPaintDevice(KisStrategyColorSpaceSP colorStrategy,
			const QString& name);
	KisPaintDevice(KisImage *img,
			KisStrategyColorSpaceSP colorStrategy,
			const QString& name);
	KisPaintDevice(const KisPaintDevice& rhs);
	virtual ~KisPaintDevice();

public:
        // Implement KisRenderInterface
        virtual bool write(KoStore *store);
        virtual bool read(KoStore *store);

public:
	virtual void configure(KisImage *image,
			KisStrategyColorSpaceSP colorStrategy,
			const QString& name,
			CompositeOp compositeOp);
public:
        virtual bool shouldDrawBorder() const;

        virtual void move(Q_INT32 x, Q_INT32 y);
        virtual void move(const QPoint& pt);

	virtual const bool visible() const;
        virtual void setVisible(bool v);

        bool contains(Q_INT32 x, Q_INT32 y) const;
        bool contains(const QPoint& pt) const;

	void extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const;
	QRect extent() const;

	Q_UINT8 * readBytes(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	void writeBytes(Q_UINT8 * data, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

	/**
	 *   Converts the paint device to a different colorspace
	 */
	virtual void convertTo(KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile = 0, Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

	/**
	 * Fill this paint device with the data from img;
	 */
	virtual void convertFromImage(const QImage& img);

	/**
	 * Create an RGBA QImage from a rectangle in the paint device.
	 *
	 * x, y left-top point of the rect of pixels
	 * w, h width and height in pixels
	 * profile RGB profile to use in conversion. May be 0, in which
	 * case it's up to the colour strategy to choose a profile (most
	 * like sRGB).
	 */
	virtual QImage convertToQImage(KisProfileSP dstProfile, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

	/**
	 * Create an RGBA QImage from a rectangle in the paint device.
	 *
	 * The dimensions is so that it takes what is currently on screen. relies on the image() to return an image.
	 * profile RGB profile to use in conversion. May be 0, in which
	 * case it's up to the colour strategy to choose a profile (most
	 * like sRGB).
	 */
	virtual QImage convertToQImage(KisProfileSP dstProfile);

        virtual QString name() const;
        virtual void setName(const QString& name);


        /**
	 * Fill c and opacity with the values found at x and y.
	 * The color values will be transformed from the profile of
	 *  this paint device to the display profile.
	 *
	 * @return true if the operation was succesful.
	 */
        bool pixel(Q_INT32 x, Q_INT32 y, QColor *c, QUANTUM *opacity);

        /**
	 * Set the specified pixel to the specified color. Note that this
         * bypasses KisPainter. the PaintDevice is here used as an equivalent
         * to QImage, not QPixmap. This means that this is not undoable; also,
         * there is no compositing with an existing value at this location.
	 *
	 *  The color values will be transformed from the display profile to
	 *  the paint device profile.
	 *
	 * @return true if the operation was succesful
	 */
        bool setPixel(Q_INT32 x, Q_INT32 y, const QColor& c, QUANTUM opacity);

	/**
	 * Return a KisPixel wrapper around these bytes. If there are not enough
	 * bytes, bad things will happen. XXX: use vectors?
	 */
	KisPixel toPixel(Q_UINT8 * bytes);
	KisPixelRO toPixelRO(Q_UINT8 * bytes);

        bool alpha() const;

	KisStrategyColorSpaceSP colorStrategy() const;

	/**
	 * Return the icm profile associated with this layer, or
	 * the profile associated with the image if the color space of
	 * this layer is the same as the color space of the image,
	 * or 0.
	 */
	KisProfileSP profile() const;

	/**
	 * Set the profile associated with this layer to the specified profile
	 * or reset to 0 if the profile does not have the same colorspace signature
	 * as the color model associated with this paint device.
	 */
	void setProfile(KisProfileSP profile);

	CompositeOp compositeOp() { return m_compositeOp; }
	void setCompositeOp(CompositeOp compositeOp) { m_compositeOp = compositeOp; }

	Q_INT32 getX();
	Q_INT32 getY();
        void setX(Q_INT32 x);
        void setY(Q_INT32 y);


	/**
	 * Return the number of bytes a pixel takes
	 */
	Q_INT32 pixelSize() const;

	/**
	 * Return the number of channels a pixel takes
	 */
	Q_INT32 nChannels() const;

	KisImage *image();
        const KisImage *image() const;
        void setImage(KisImage *image);

	// XXX: Do all rotations etc. use the visitor instead of the QMatrix-based code by now?
	void scale(double sx, double sy, KisProgressDisplayInterface *m_progress, enumFilterType ftype=MITCHELL_FILTER);
        void rotate(double angle, KisProgressDisplayInterface *m_progress);
        void shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress);

	/**
	   Apply the transformation matrix _in place_.
	*/
//	void transform(const QWMatrix & matrix);

	/**
	 * Mirror the device along the X axis
	 */
	void mirrorX();
	/**
	 * Mirror the device along the Y axis
	 */
	void mirrorY();

	/**
	 * XXX: Move this undo code back into the tiles/ module and wrap in transactions
	 * CBR: not sure about that, but transaction system needs to be revisited
	 */
	KisMemento * getMemento() { return m_datamanager -> getMemento(); };
	void rollback(KisMemento *memento) { m_datamanager -> rollback(memento); };
	void rollforward(KisMemento *memento) { m_datamanager -> rollforward(memento); };

	/**
	 * This function return an iterator which points to the first pixel of an rectangle
	 */
	KisRectIteratorPixel createRectIterator(Q_INT32 left, Q_INT32 top, Q_INT32 w, Q_INT32 h, bool writable);

	/**
	 * This function return an iterator which points to the first pixel of a horizontal line
	 */
	KisHLineIteratorPixel createHLineIterator(Q_INT32 x, Q_INT32 y, Q_INT32 w, bool writable);

	/**
	 * This function return an iterator which points to the first pixel of a vertical line
	 */
	KisVLineIteratorPixel createVLineIterator(Q_INT32 x, Q_INT32 y, Q_INT32 h, bool writable);

	// Selection stuff. XXX: is it necessary to make the actual
	// selection object available outside the layer? YYY: yes, so
	// selection tools can act on it.

	/** Get the current selection or create one if this layers hasn't got a selection yet. */
	KisSelectionSP selection();

	/** Set the specified selection object as the active selection for this layer */
	void setSelection(KisSelectionSP selection);

	/** Adds the specified selection to the currently active selection for this layer */
	void addSelection(KisSelectionSP selection);

	/** Subtracts the specified selection from the currently active selection for this layer */
	void subtractSelection(KisSelectionSP selection);

	/** Whether there is a valid selection for this layer. */
	bool hasSelection();

	/** Removes the current selection for this layer. */
	void removeSelection();


signals:

        void visibilityChanged(KisPaintDeviceSP device);
        void positionChanged(KisPaintDeviceSP device);
        void ioProgress(Q_INT8 percentage);
	void selectionChanged();
	void selectionCreated();
	void profileChanged(KisProfileSP profile);

private:
	KisPaintDevice& operator=(const KisPaintDevice&);
	void init();

private:
	KisImage *m_owner;
	KisDataManager * m_datamanager;
	Q_INT32 m_x;
	Q_INT32 m_y;
	bool m_visible;
	QString m_name;
	// Operation used to composite this layer with the layers _under_ this layer
	CompositeOp m_compositeOp;
	KisStrategyColorSpaceSP m_colorStrategy;
	KisProfileSP m_profile;

	void accept(KisScaleVisitor &);
	void accept(KisRotateVisitor &);

	// Whether there is a selection valid for this layer
	bool m_hasSelection;
	// Contains the actual selection. For now, there can be only
	// one selection per layer. XXX: is this a limitation?
	KisSelectionSP m_selection;

};

inline Q_INT32 KisPaintDevice::pixelSize() const
{
	return m_colorStrategy -> pixelSize();
}

inline Q_INT32 KisPaintDevice::nChannels() const
{
        return m_colorStrategy -> nChannels();
;
}

inline KisStrategyColorSpaceSP KisPaintDevice::colorStrategy() const
{
	if (m_colorStrategy == 0 && m_owner != 0) {
		return m_owner -> colorStrategy();
	}
        return m_colorStrategy;
}


inline Q_INT32 KisPaintDevice::getX()
{
	return m_x;
}

inline Q_INT32 KisPaintDevice::getY()
{
	return m_y;
}

inline void KisPaintDevice::setX(Q_INT32 x)
{
	m_x = x;
}

inline void KisPaintDevice::setY(Q_INT32 y)
{
	m_y = y;
}

inline const bool KisPaintDevice::visible() const
{
        return m_visible;
}

inline void KisPaintDevice::setVisible(bool v)
{
        if (m_visible != v) {
                m_visible = v;
                emit visibilityChanged(this);
        }
}


inline KisImage *KisPaintDevice::image()
{
        return m_owner;
}

inline const KisImage *KisPaintDevice::image() const
{
        return m_owner;
}

inline void KisPaintDevice::setImage(KisImage *image)
{
        m_owner = image;
}

inline bool KisPaintDevice::alpha() const
{
        return colorStrategy()->alpha();
}

inline KisPixel KisPaintDevice::toPixel(Q_UINT8 * bytes)
{
	return m_colorStrategy -> toKisPixel(bytes, m_profile);
}

inline KisPixelRO KisPaintDevice::toPixelRO(Q_UINT8 * bytes)
{
	return m_colorStrategy -> toKisPixelRO(bytes, m_profile);
}

inline Q_UINT8 * KisPaintDevice::readBytes(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	return m_datamanager -> readBytes(x, y, w, h);
}

inline void KisPaintDevice::writeBytes(Q_UINT8 * data, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	m_datamanager -> writeBytes( data, x, y, w, h);
}

#endif // KIS_PAINT_DEVICE_H_

