/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_IMAGE_H_
#define KIS_IMAGE_H_

#include <qbitarray.h>
#include <qobject.h>
#include <qstring.h>
#include <qvaluevector.h>
#include <ksharedptr.h>
#include <kurl.h>
#include <koColor.h>
#include "kis_global.h"
#include "kis_types.h"
#include "kis_render.h"

class KCommandHistory;
class KisDoc;
class KisNameServer;
class KisPainter;

class KisImage : public QObject, public KisRenderInterface {
	Q_OBJECT

public:
	KisImage(KisDoc *doc, Q_INT32 width, Q_INT32 height, Q_UINT32 depth, QUANTUM opacity, const enumImgType& imgType, const QString& name);
	virtual ~KisImage();

public:
	// Implement KisRenderInterface
	virtual void invalidate(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	virtual void invalidate(const QRect& rc);
	virtual void invalidate();
	virtual QPixmap pixmap();
	virtual QPixmap recreatePixmap();

public:
	QString name() const;
	void setName(const QString& name);
	QString nextLayerName() const;
	QString author() const;
	void setAuthor(const QString& author);
	QString email() const;
	void setEmail(const QString& email);

	void resize(Q_INT32 w, Q_INT32 h);
	void resize(const QRect& rc);
	void enableUndo(KCommandHistory *history);

	enumImgType imgType() const;
	enumImgType imgTypeWithAlpha() const;

	KURL uri() const;
	void uri(const KURL& uri);

	KoUnit::Unit unit() const;
	void unit(const KoUnit::Unit& u);

	void resolution(double xres, double yres);
	void resolution(double *xres, double *yres);

	Q_INT32 width() const;
	Q_INT32 height() const;
	Q_UINT32 depth() const;
	bool alpha() const;
	bool empty() const;
	bool colorMap(KoColorMap& cm);
	KisChannelSP mask();
	KoColor foreground() const;
	KoColor background() const;
	KoColor color() const;
	KoColor transformColor() const;
	KisTileMgrSP shadow();

	void activeComponent(CHANNELTYPE type, bool active);
	bool activeComponent(CHANNELTYPE type);

	void visibleComponent(CHANNELTYPE pixel, bool active);
	bool visibleComponent(CHANNELTYPE pixel) const;

	void flush();

	vKisLayerSP layers() const;
	vKisChannelSP channels() const;

	KisPaintDeviceSP activeDevice();

	KisLayerSP activeLayer();
	KisLayerSP activate(KisLayerSP layer);
	KisLayerSP activateLayer(Q_INT32 n);
	Q_INT32 index(KisLayerSP layer);
	KisLayerSP layer(const QString& name);
	KisLayerSP layer(Q_UINT32 npos);
	bool add(KisLayerSP layer, Q_INT32 position);
	void rm(KisLayerSP layer);
	bool raise(KisLayerSP layer);
	bool lower(KisLayerSP layer);
	bool top(KisLayerSP layer);
	bool bottom(KisLayerSP layer);
	bool pos(KisLayerSP layer, Q_INT32 position);
	Q_INT32 nlayers() const;

	KisChannelSP activeChannel();
	KisChannelSP activate(KisChannelSP channel);
	KisChannelSP activateChannel(Q_INT32 n);
	KisChannelSP unsetActiveChannel();
	Q_INT32 index(KisChannelSP channel);
	KisChannelSP channel(const QString& name);
	KisChannelSP channel(Q_UINT32 npos);
	bool add(KisChannelSP channel, Q_INT32 position);
	void rm(KisChannelSP channel);
	bool raise(KisChannelSP channel);
	bool lower(KisChannelSP channel);
	bool pos(KisChannelSP channel, Q_INT32 position);
	Q_INT32 nchannels() const;

	bool boundsLayer();
	KisLayerSP corrolateLayer(Q_INT32 x, Q_INT32 y);

	void setSelection(KisSelectionSP selection);
	void unsetSelection();
	KisSelectionSP selection() const;

signals:
	void activeLayerChanged(KisImageSP image);
	void activeChannelChanged(KisImageSP image);
	void alphaChanged(KisImageSP image);
	void selectionChanged(KisImageSP image);
	void visibilityChanged(KisImageSP image, CHANNELTYPE type);
	void update(KisImageSP image, Q_UINT32 x, Q_UINT32 y, Q_UINT32 w, Q_UINT32 h);

private:
	KisImage(const KisImage& rhs);
	KisImage& operator=(const KisImage& rhs);
	void copyTile(KisTileSP dst, KisTileSP src);
	void renderLayer(KisPainter& gc, KisLayerSP layer);
	void expand(KisPaintDeviceSP dev);
	void init(KisDoc *doc, Q_INT32 width, Q_INT32 height, Q_UINT32 depth, QUANTUM opacity, const enumImgType& imgType, const QString& name);
	PIXELTYPE pixelFromChannel(CHANNELTYPE type) const;
	void renderProjection();
	void rengerBg();

private:
	KisDoc *m_doc;
	KCommandHistory *m_undoHistory;
	KURL m_uri;
	QString m_name;
	QString m_author;
	QString m_email;
	Q_INT32 m_width;
	Q_INT32 m_height;
	Q_UINT32 m_depth;
	QUANTUM m_opacity;
	double m_xres;
	double m_yres;
	KoUnit::Unit m_unit;
	enumImgType m_type;
	KoColorMap m_clrMap;
	bool m_dirty;
	KisTileMgrSP m_shadow;
	bool m_construct;
	enumImgType m_projType;
	QPixmap m_pixmapProjection;
	KisLayerSP m_projection;
	vKisLayerSP m_layers;
	vKisChannelSP m_channels;
	vKisLayerSP m_layerStack;
	KisLayerSP m_activeLayer;
	KisChannelSP m_activeChannel;
	KisSelectionSP m_selection;
	KisChannelSP m_selectionMask;
	QBitArray m_visible;
	QBitArray m_active;
	bool m_alpha;
	bool m_maskEnabled;
	bool m_maskInverted;
	KoColor m_maskClr;
	KisNameServer *m_nserver;
	KisTileSP m_bgTile;
};

#endif // KIS_IMAGE_H_

