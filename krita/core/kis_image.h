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
#ifndef KIS_IMAGE_H_
#define KIS_IMAGE_H_

#include <qobject.h>
#include <qstring.h>
#include <qvaluevector.h>
#include <qmutex.h>

#include <ksharedptr.h>
#include <kurl.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_guide.h"
#include "kis_scale_visitor.h"
#include "resources/kis_profile.h"
#include <koffice_export.h>
class KoCommandHistory;
class KisNameServer;
class KisUndoAdapter;
class KisPainter;
class DCOPObject;

class KRITACORE_EXPORT KisImage : public QObject, public KShared {
	Q_OBJECT

public:
	KisImage(KisUndoAdapter *undoAdapter, Q_INT32 width, Q_INT32 height,
		 KisStrategyColorSpaceSP colorStrategy, const QString& name);
	KisImage(const KisImage& rhs);
	virtual ~KisImage();
	virtual DCOPObject *dcopObject();

public:
	// Composite the specified tile onto the projection layer.
	virtual void renderToProjection(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

	/// Paint the specified rect onto the painter, adjusting the colors using the
	/// given profile.
	virtual void renderToPainter(Q_INT32 x1,
				     Q_INT32 y1,
				     Q_INT32 x2,
				     Q_INT32 y2,
				     QPainter &painter,
				     KisProfileSP profile);

	// XXX: Add a convertToQImage to KisImage?


	KisLayerSP projection() const;

public:
	QString name() const;
	void setName(const QString& name);

	QString description() const;
	void setDescription(const QString& description);

	QString nextLayerName() const;

	void resize(Q_INT32 w, Q_INT32 h);
	void resize(const QRect& rc);

	void scale(double sx, double sy, KisProgressDisplayInterface *m_progress, enumFilterType ftype = MITCHELL_FILTER);
        void rotate(double angle, KisProgressDisplayInterface *m_progress);
        void shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress);

	void convertTo(KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile, Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

	// Get the profile associated with this image
	KisProfileSP profile() const;

	// Set the profile associated with this image
	void setProfile(const KisProfileSP& profile);


	void enableUndo(KoCommandHistory *history);

	KisStrategyColorSpaceSP colorStrategy() const;

	KURL uri() const;
	void uri(const KURL& uri);

	KoUnit::Unit unit() const;
	void unit(const KoUnit::Unit& u);

	// Resolution of the image == XXX: per inch?
        double xRes();
	double yRes();
	void setResolution(double xres, double yres);

	Q_INT32 width() const;
	Q_INT32 height() const;

	bool empty() const;

	vKisLayerSP layers();
	const vKisLayerSP& layers() const;

	// Get the active painting device
	KisPaintDeviceSP activeDevice();

	KisLayerSP activeLayer();
	const KisLayerSP activeLayer() const;
	KisLayerSP activate(KisLayerSP layer);
	KisLayerSP activateLayer(Q_INT32 n);
	Q_INT32 index(const KisLayerSP &layer);
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
	Q_INT32 nHiddenLayers() const;
	Q_INT32 nLinkedLayers() const;

	// Merge all visible layers and discard hidden ones.
	void flatten();

	void mergeVisibleLayers();
	void mergeLinkedLayers();

	/**
	 * Merge the specified layer with the layer
	 * below this layer, remove the specified layer.
	 */
	void mergeLayer(KisLayerSP l);

	QRect bounds() const;

	void notify();
	void notify(Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height);
	void notify(const QRect& rc);

	void notifyLayersChanged();

	KisUndoAdapter *undoAdapter() const;
	KisGuideMgr *guides() const;

signals:
	void activeLayerChanged(KisImageSP image);
	void activeSelectionChanged(KisImageSP image);
	void selectionCreated(KisImageSP image);
	void selectionChanged(KisImageSP image);
	void update(KisImageSP image, const QRect& rc);
	void layersChanged(KisImageSP image);
	void sizeChanged(KisImageSP image, Q_INT32 w, Q_INT32 h);
	void profileChanged(KisProfileSP profile);

private slots:
	void slotSelectionChanged();
	void slotSelectionCreated();

private:
	KisImage& operator=(const KisImage& rhs);
	void init(KisUndoAdapter *adapter, Q_INT32 width, Q_INT32 height,  KisStrategyColorSpaceSP colorStrategy, const QString& name);

private:
	KoCommandHistory *m_undoHistory;
	KURL m_uri;
	QString m_name;
	QString m_description;

	KisProfileSP m_profile;

	Q_INT32 m_width;
	Q_INT32 m_height;

	double m_xres;
	double m_yres;

	KoUnit::Unit m_unit;

	KisStrategyColorSpaceSP m_colorStrategy;

	bool m_dirty;
	QRect m_dirtyRect;

	KisBackgroundSP m_bkg;
	KisLayerSP m_projection;
	vKisLayerSP m_layers;
	vKisLayerSP m_layerStack;
	KisLayerSP m_activeLayer;

	KisNameServer *m_nserver;
	KisUndoAdapter *m_adapter;
	KisGuideMgr m_guides;

	DCOPObject *m_dcop;

	QPixmap m_pixmap;


};

#endif // KIS_IMAGE_H_
