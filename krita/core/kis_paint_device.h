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
#if !defined KIS_PAINT_DEVICE_H_
#define KIS_PAINT_DEVICE_H_

#include <qcolor.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <kcommand.h>
#include "kis_global.h"
#include "kis_types.h"
#include "kis_render.h"

class QImage;
class KoStore;
class KisImage;

class KisPaintDevice : public QObject, public KisRenderInterface {
	Q_OBJECT
	typedef KisRenderInterface super;

public:
	KisPaintDevice(KisImageSP img, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name);
	virtual ~KisPaintDevice();

public:
	// Implement KisRenderInterface
	virtual void invalidate(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	virtual void invalidate(const QRect& rc);
	virtual void invalidate();
	virtual QPixmap pixmap();
	virtual QPixmap recreatePixmap();

public:
	virtual void configure(KisImageSP image, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name);
	virtual void duplicate(KisPaintDevice& rhs, bool addAlpha);
	virtual void update();
	virtual void update(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

public:
	QString name();
	void setName(const QString& name);
	void mergeShadow();
	void fill(const KoColor& clr);
	void maskBounds(Q_INT32 *x1, Q_INT32 *y1, Q_INT32 *x2, Q_INT32 *y2);
	void maskBounds(QRect *rc);
	bool alpha() const;
	enumImgType type() const;
	enumImgType typeWithAlpha() const;
	KisTileMgrSP data();
	const KisTileMgrSP data() const;
	KisTileMgrSP shadow();
	const KisTileMgrSP shadow() const;
	Q_INT32 quantumSize() const;
	Q_INT32 quantumSizeWithAlpha() const;
	Q_INT32 width() const;
	Q_INT32 height() const;
	const bool visible() const;
	void visible(bool v);
	void drawOffset(Q_INT32 *offx, Q_INT32 *offy);
	bool cmap(KoColorMap& cm);
	KoColor colorAt();
	KisImageSP image();
	const KisImageSP image() const;
	void setImage(KisImageSP image);

signals:
	void visibilityChanged(KisPaintDeviceSP device);

private:
	void init();

protected:
	KisImageSP m_owner;
	KisTileMgrSP m_tiles;
	KisTileMgrSP m_shadow;
	bool m_visible;
	Q_INT32 m_width;
	Q_INT32 m_height;
	Q_INT32 m_depth;
	Q_INT32 m_opacity;
	Q_INT32 m_offX;
	Q_INT32 m_offY;
	Q_INT32 m_quantumSize;
	enumImgType m_imgType;
	bool m_alpha;
	QPixmap m_projection;
	bool m_projectionValid;
	QString m_name;
};

#endif // KIS_PAINT_DEVICE_H_

