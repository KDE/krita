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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KIS_IMAGE_H_
#define KIS_IMAGE_H_

#include <qbitarray.h>
#include <qobject.h>
#include <qstring.h>
#include <qimage.h>
#include <qvaluevector.h>
#include <qtimer.h>
#include <qmutex.h>

#include <kurl.h>

#include <koColor.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_render.h"
#include "kis_guide.h"

class KoCommandHistory;
class KisNameServer;
class KisUndoAdapter;
class KisPainter;

class KisImage : public QObject, public KisRenderInterface {
	Q_OBJECT

public:
	KisImage(KisUndoAdapter *undoAdapter, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name);
	KisImage(const KisImage& rhs);
	virtual ~KisImage();

public:
	// Implement KisRenderInterface
	virtual Q_INT32 tileNum(Q_INT32 xpix, Q_INT32 ypix) const;
	virtual void validate(Q_INT32 tileno);
	virtual void invalidate(Q_INT32 tileno);
	virtual void invalidate(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	virtual void invalidate(const QRect& rc);
	virtual void invalidate();
	virtual KisTileMgrSP tiles() const;

public:
	QString name() const;
	void setName(const QString& name);
	QString nextLayerName() const;
	void resize(Q_INT32 w, Q_INT32 h);
	void resize(const QRect& rc);
	void enableUndo(KoCommandHistory *history);

	enumImgType imgType() const;
	enumImgType nativeImgType() const;
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
	KoColor color() const;
	KoColor transformColor() const;
	KisTileMgrSP shadow();

	void activeComponent(CHANNELTYPE type, bool active);
	bool activeComponent(CHANNELTYPE type);

	void visibleComponent(CHANNELTYPE pixel, bool active);
	bool visibleComponent(CHANNELTYPE pixel) const;

	void flush();

	vKisLayerSP layers();
	const vKisLayerSP& layers() const;
	vKisChannelSP channels();
	const vKisChannelSP& channels() const;

	KisPaintDeviceSP activeDevice();

	KisLayerSP activeLayer();
	const KisLayerSP activeLayer() const;
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
	KisLayerSP correlateLayer(Q_INT32 x, Q_INT32 y);

	void setSelection(KisSelectionSP selection);
	void unsetSelection(bool commit = true);
	KisSelectionSP selection() const;
	QRect bounds() const;

	void notify();
	void notify(Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height);
	void notify(const QRect& rc);

	KisUndoAdapter *undoAdapter() const;
	KisGuideMgr *guides() const;

signals:
	void activeLayerChanged(KisImageSP image);
	void activeChannelChanged(KisImageSP image);
	void alphaChanged(KisImageSP image);
	void selectionChanged(KisImageSP image);
	void visibilityChanged(KisImageSP image, CHANNELTYPE type);
	void update(KisImageSP image, const QRect& rc);

private slots:
	void slotUpdateDisplay();

private:
	KisImage& operator=(const KisImage& rhs);
	void expand(KisPaintDeviceSP dev);
	void init(KisUndoAdapter *adapter, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name);
	PIXELTYPE pixelFromChannel(CHANNELTYPE type) const;

	void startUpdateTimer();

private:
	KoCommandHistory *m_undoHistory;
	KURL m_uri;
	QString m_name;

	Q_INT32 m_width;
	Q_INT32 m_height;
	Q_UINT32 m_depth;

	Q_INT32 m_ntileCols;
	Q_INT32 m_ntileRows;

	double m_xres;
	double m_yres;

	KoUnit::Unit m_unit;

	enumImgType m_type;
	KoColorMap m_clrMap;

	bool m_dirty;
	QRect m_dirtyRect;

	KisTileMgrSP m_shadow;
	KisBackgroundSP m_bkg;
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
	KisUndoAdapter *m_adapter;
	KisGuideMgr m_guides;

	QTimer * m_updateTimer;
	QMutex m_displayMutex;
};

#endif // KIS_IMAGE_H_
