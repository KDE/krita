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
#include <stdlib.h>
#include <math.h>

#include <config.h>
#include LCMS_HEADER

#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qtl.h>

#include <kcommand.h>
#include <kocommandhistory.h>
#include <kdebug.h>
#include <klocale.h>

#include "KIsImageIface.h"

#include "kis_guide.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_layer.h"
#include "kis_background.h"
#include "kis_doc.h"
#include "kis_nameserver.h"
#include "visitors/kis_flatten.h"
#include "visitors/kis_merge.h"
#include "kis_transaction.h"
#include "kis_scale_visitor.h"
#include "kis_profile.h"
#include "kis_config.h"
#include "kis_colorspace_registry.h"

#define DEBUG_IMAGES 0

#if DEBUG_IMAGES
static int numImages = 0;
#endif

namespace {

	class KisResizeImageCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		KisResizeImageCmd(KisUndoAdapter *adapter,
				  KisImageSP img,
				  Q_INT32 width,
				  Q_INT32 height,
				  Q_INT32 oldWidth,
				  Q_INT32 oldHeight) : super(i18n("Resize Image"))
			{
				m_adapter = adapter;
				m_img = img;
				m_before = QSize(oldWidth, oldHeight);
				m_after = QSize(width, height);
			}

		virtual ~KisResizeImageCmd()
			{
			}

	public:
		virtual void execute()
			{
				m_adapter -> setUndo(false);
				m_img -> resize(m_after.width(), m_after.height());
				m_adapter -> setUndo(true);
				m_img -> notify(0, 0, QMAX(m_before.width(), m_after.width()), QMAX(m_before.height(), m_after.height()));
			}

		virtual void unexecute()
			{
				m_adapter -> setUndo(false);
				m_img -> resize(m_before.width(), m_before.height());
				m_adapter -> setUndo(true);
				m_img -> notify(0, 0, QMAX(m_before.width(), m_after.width()), QMAX(m_before.height(), m_after.height()));
			}

	private:
		KisUndoAdapter *m_adapter;
		KisImageSP m_img;
		QSize m_before;
		QSize m_after;
	};


	class KisChangeLayersCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		KisChangeLayersCmd(KisUndoAdapter *adapter, KisImageSP img, vKisLayerSP& beforeLayers, vKisLayerSP& afterLayers, const QString& name) : super(name)
			{
				m_adapter = adapter;
				m_img = img;
				m_beforeLayers = beforeLayers;
				m_afterLayers = afterLayers;
			}

		virtual ~KisChangeLayersCmd()
			{
			}

	public:
		virtual void execute()
			{
				m_adapter -> setUndo(false);

				for (vKisLayerSP::const_iterator it = m_beforeLayers.begin(); it != m_beforeLayers.end(); it++) {
					m_img -> rm(*it);
				}

				for (vKisLayerSP::const_iterator it = m_afterLayers.begin(); it != m_afterLayers.end(); it++) {
					m_img -> add(*it, -1);
				}

				m_adapter -> setUndo(true);
				m_img -> notify();
				m_img -> notifyLayersChanged();
			}

		virtual void unexecute()
			{
				m_adapter -> setUndo(false);

				for (vKisLayerSP::const_iterator it = m_afterLayers.begin(); it != m_afterLayers.end(); it++) {
					m_img -> rm(*it);
				}

				for (vKisLayerSP::const_iterator it = m_beforeLayers.begin(); it != m_beforeLayers.end(); it++) {
					m_img -> add(*it, -1);
				}

				m_adapter -> setUndo(true);
				m_img -> notify();
				m_img -> notifyLayersChanged();
			}

	private:
		KisUndoAdapter *m_adapter;
		KisImageSP m_img;
		vKisLayerSP m_beforeLayers;
		vKisLayerSP m_afterLayers;
	};

	class KisConvertImageTypeCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		KisConvertImageTypeCmd(KisUndoAdapter *adapter, KisImageSP img, 
				       KisStrategyColorSpaceSP beforeColorSpace, KisProfileSP beforeProfile, 
				       KisStrategyColorSpaceSP afterColorSpace, KisProfileSP afterProfile
				       ) : super(i18n("&Convert Image Type...")) //XXX: fix when string freeze over
			{
				m_adapter = adapter;
				m_img = img;
				m_beforeColorSpace = beforeColorSpace;
				m_beforeProfile = beforeProfile;
				m_afterColorSpace = afterColorSpace;
				m_afterProfile = afterProfile;
			}

		virtual ~KisConvertImageTypeCmd()
			{
			}

	public:
		virtual void execute()
			{
				m_adapter -> setUndo(false);

				m_img -> setColorStrategy(m_afterColorSpace);
				m_img -> setProfile(m_afterProfile);

				m_adapter -> setUndo(true);
				m_img -> notify();
				m_img -> notifyLayersChanged();
			}

		virtual void unexecute()
			{
				m_adapter -> setUndo(false);

				m_img -> setColorStrategy(m_beforeColorSpace);
				m_img -> setProfile(m_beforeProfile);

				m_adapter -> setUndo(true);
				m_img -> notify();
				m_img -> notifyLayersChanged();
			}

	private:
		KisUndoAdapter *m_adapter;
		KisImageSP m_img;
		KisStrategyColorSpaceSP m_beforeColorSpace;
		KisStrategyColorSpaceSP m_afterColorSpace;
		KisProfileSP m_beforeProfile;
		KisProfileSP m_afterProfile;
	};

}

KisImage::KisImage(KisUndoAdapter *undoAdapter, Q_INT32 width, Q_INT32 height,  KisStrategyColorSpaceSP colorStrategy, const QString& name)
{
#if DEBUG_IMAGES
	numImages++;
	kdDebug() << "IMAGE " << name << " CREATED total now = " << numImages << endl;
#endif
	init(undoAdapter, width, height, colorStrategy, name);
	setName(name);
        m_dcop = 0L;
	m_profile = 0;
}

KisImage::KisImage(const KisImage& rhs) : QObject(), KShared(rhs)
{
#if DEBUG_IMAGES
	numImages++;
	kdDebug() << "IMAGE " << rhs.m_name << " copy CREATED total now = " << numImages << endl;
#endif
	m_dcop = 0L;
	if (this != &rhs) {
		m_undoHistory = rhs.m_undoHistory;
		m_uri = rhs.m_uri;
		m_name = QString::null;
		m_width = rhs.m_width;
		m_height = rhs.m_height;
		m_xres = rhs.m_xres;
		m_yres = rhs.m_yres;
		m_unit = rhs.m_unit;
		m_colorStrategy = rhs.m_colorStrategy;
		m_dirty = rhs.m_dirty;
		m_adapter = rhs.m_adapter;
		m_profile = rhs.m_profile;

		m_bkg = new KisBackground(this, rhs.width(), rhs.height());
		Q_CHECK_PTR(m_bkg);

		m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
		Q_CHECK_PTR(m_projection);

		m_layers.reserve(rhs.m_layers.size());

		for (vKisLayerSP_cit it = rhs.m_layers.begin(); it != rhs.m_layers.end(); it++) {
			KisLayerSP layer = new KisLayer(**it);
			Q_CHECK_PTR(layer);

			layer -> setImage(this);
			m_layers.push_back(layer);
			m_layerStack.push_back(layer);
			m_activeLayer = layer;
		}
		
		m_annotations = rhs.m_annotations; // XXX the annotations would probably need to be deep-copied


		m_nserver = new KisNameServer(i18n("Layer %1"), rhs.m_nserver -> currentSeed() + 1);
		Q_CHECK_PTR(m_nserver);

		m_guides = rhs.m_guides;
		m_pixmap = rhs.m_pixmap;
	}

}


DCOPObject *KisImage::dcopObject()
{
	if (!m_dcop) {
		m_dcop = new KIsImageIface(this);
		Q_CHECK_PTR(m_dcop);
	}
	return m_dcop;
}

KisImage::~KisImage()
{
#if DEBUG_IMAGES
	numImages--;
	kdDebug() << "IMAGE " << name() << " DESTROYED total now = " << numImages << endl;
#endif
	delete m_nserver;
        delete m_dcop;
}

QString KisImage::name() const
{
	return m_name;
}

void KisImage::setName(const QString& name)
{
	if (!name.isEmpty())
		m_name = name;
}

QString KisImage::description() const
{
	return m_description;
}

void KisImage::setDescription(const QString& description)
{
	if (!description.isEmpty())
		m_description = description;
}


QString KisImage::nextLayerName() const
{
	if (m_nserver -> currentSeed() == 0) {
		m_nserver -> number();
		return i18n("background");
	}

	return m_nserver -> name();
}

void KisImage::init(KisUndoAdapter *adapter, Q_INT32 width, Q_INT32 height,  KisStrategyColorSpaceSP colorStrategy, const QString& name)
{
	Q_ASSERT(colorStrategy != 0);
	Q_ASSERT(adapter != 0);

	m_adapter = adapter;
	m_nserver = new KisNameServer(i18n("Layer %1"), 1);
	Q_CHECK_PTR(m_nserver);
	m_name = name;

	m_colorStrategy = colorStrategy;
	m_bkg = new KisBackground(this, width, height);
	Q_CHECK_PTR(m_bkg);

	m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
	Q_CHECK_PTR(m_projection);

	m_xres = 1.0;
	m_yres = 1.0;
	m_unit = KoUnit::U_PT;
	m_dirty = false;
	m_undoHistory = 0;
	m_width = width;
	m_height = height;
}

void KisImage::resize(Q_INT32 w, Q_INT32 h, bool cropLayers)
{
// 	kdDebug() << "KisImage::resize. From: ("
// 		  << width()
// 		  << ", "
// 		  << height()
// 		  << ") to ("
// 		  << w
// 		  << ", "
// 		  << h
// 		  << ")\n";

	if (w != width() || h != height()) {
		if (m_adapter && m_adapter -> undo()) {
			m_adapter -> beginMacro("Resize image");
			m_adapter -> addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));
		}

		m_width = w;
		m_height = h;

		m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
		Q_CHECK_PTR(m_projection);

 		m_bkg = new KisBackground(this, w, h);
		Q_CHECK_PTR(m_bkg);

		if (cropLayers) {
			vKisLayerSP_it it;
			for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
				KisLayerSP layer = (*it);
				KisTransaction * t = new KisTransaction("crop", layer.data());
				Q_CHECK_PTR(t);
				layer -> crop(0, 0, w, h);
				 if (m_adapter && m_adapter -> undo())
					m_adapter -> addCommand(t);
			}
		}

		if (m_adapter && m_adapter -> undo()) {
			m_adapter -> endMacro();
		}

		emit sizeChanged(KisImageSP(this), w, h);
	}
}

void KisImage::resize(const QRect& rc, bool cropLayers)
{
	resize(rc.width(), rc.height(), cropLayers);
}

void KisImage::scale(double sx, double sy, KisProgressDisplayInterface *m_progress, enumFilterType ftype)
{
// 	kdDebug() << "KisImage::scale. SX: "
// 		  << sx
// 		  << ", SY:"
// 		  << sy
// 		  << "\n";

	if (m_layers.empty()) return; // Nothing to scale

	// New image size. XXX: Pass along to discourage rounding errors?
	Q_INT32 w, h;
	w = (Q_INT32)(( width() * sx) + 0.5);
	h = (Q_INT32)(( height() * sy) + 0.5);

// 	kdDebug() << "Scaling from (" << m_projection -> width()
// 		  << ", " << m_projection -> height()
// 		  << "to: (" << w << ", " << h << ")\n";

	if (w != width() || h != height()) {

		undoAdapter() -> beginMacro("Scale image");

		vKisLayerSP_it it;
		for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
			KisLayerSP layer = (*it);
			KisTransaction *cmd = 0;

			if (undoAdapter() -> undo()) {
				cmd = new KisTransaction("", layer.data());
				Q_CHECK_PTR(cmd);
			}

			layer -> scale(sx, sy, m_progress, ftype);

			if (undoAdapter() -> undo()) {
				undoAdapter() -> addCommand(cmd);
			}
		}

		undoAdapter() -> addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));

		m_width = w;
		m_height = h;

		m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
		Q_CHECK_PTR(m_projection);

		m_bkg = new KisBackground(this, w, h);
		Q_CHECK_PTR(m_bkg);

		undoAdapter() -> endMacro();

		emit sizeChanged(KisImageSP(this), w, h);
	}
}

void KisImage::rotate(double angle, KisProgressDisplayInterface *m_progress)
{
        const double pi=3.1415926535897932385;

	if (m_layers.empty()) return; // Nothing to scale
        Q_INT32 w, h;
        w = (Q_INT32)(width()*QABS(cos(angle*pi/180)) + height()*QABS(sin(angle*pi/180)) + 0.5);
        h = (Q_INT32)(height()*QABS(cos(angle*pi/180)) + width()*QABS(sin(angle*pi/180)) + 0.5);

	Q_INT32 oldCentreToNewCentreXOffset = (w - width()) / 2;
	Q_INT32 oldCentreToNewCentreYOffset = (h - height()) / 2;

	undoAdapter() -> beginMacro("Rotate image");

	vKisLayerSP_it it;
	for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
		KisLayerSP layer = (*it);

		KisTransaction * t = 0;
		if (undoAdapter() && undoAdapter() -> undo()) {
			t = new KisTransaction("", layer.data());
			Q_CHECK_PTR(t);
		}

                layer -> rotate(angle, true, m_progress);

		if (t) {
			undoAdapter() -> addCommand(t);
		}

		//XXX: This is very ugly.
		KNamedCommand *moveCommand = layer -> moveCommand(layer -> getX() + oldCentreToNewCentreXOffset, 
								  layer -> getY() + oldCentreToNewCentreYOffset);
		if (undoAdapter() && undoAdapter() -> undo()) {
			undoAdapter() -> addCommand(moveCommand);
		} else {
			delete moveCommand;
		}
	}

	m_adapter -> addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));

	m_width = w;
	m_height = h;

	m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
	Q_CHECK_PTR(m_projection);

	m_bkg = new KisBackground(this, w, h);
	Q_CHECK_PTR(m_bkg);

	undoAdapter()->endMacro();

	emit sizeChanged(KisImageSP(this), w, h);
}

void KisImage::shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress)
{
	const double pi=3.1415926535897932385;

        if (m_layers.empty()) return; // Nothing to scale

        //new image size
	Q_INT32 w=width();
        Q_INT32 h=height();


        if(angleX != 0 || angleY != 0){
                double deltaY=height()*QABS(tan(angleX*pi/180)*tan(angleY*pi/180));
                w = (Q_INT32) ( width() + QABS(height()*tan(angleX*pi/180)) );
                //ugly fix for the problem of having two extra pixels if only a shear along one
                //axis is done. This has to be fixed in the cropping code in KisRotateVisitor!
                if (angleX == 0 || angleY == 0)
                        h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180)) );
                else if (angleX > 0 && angleY > 0)
                        h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180))- 2 * deltaY + 2 );
                else if (angleX < 0 && angleY < 0)
                        h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180))- 2 * deltaY + 2 );
                else
                        h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180)) );
        }

	if (w != width() || h != height()) {

		undoAdapter() -> beginMacro("Shear image");

		vKisLayerSP_it it;
		for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
			KisLayerSP layer = (*it);

			KisTransaction * t = 0;
			if (undoAdapter() && undoAdapter() -> undo()) {
				t = new KisTransaction("", layer.data());
				Q_CHECK_PTR(t);
			}

			layer -> shear(angleX, angleY, m_progress);

			if (t) {
				undoAdapter() -> addCommand(t);
			}

		}

		m_adapter -> addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));

		m_width = w;
		m_height = h;

		m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
		Q_CHECK_PTR(m_projection);

		m_bkg = new KisBackground(this, w, h);
		Q_CHECK_PTR(m_bkg);

		undoAdapter()->endMacro();

		emit sizeChanged(KisImageSP(this), w, h);
	}
}

void KisImage::convertTo(KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile, Q_INT32 renderingIntent)
{
	if (m_colorStrategy -> id() != dstColorStrategy -> id()) {

		if (undoAdapter() && undoAdapter() -> undo()) {
			undoAdapter() -> beginMacro(i18n("&Convert Image Type...")); //XXX: fix when string freeze over
		}

		vKisLayerSP_it it;
		for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
			(*it) -> convertTo(dstColorStrategy, dstProfile, renderingIntent);
		}

		if (undoAdapter() && undoAdapter() -> undo()) {

			undoAdapter() -> addCommand(new KisConvertImageTypeCmd(undoAdapter(), this, m_colorStrategy, m_profile,
									       dstColorStrategy, dstProfile));
			undoAdapter() -> endMacro();
		}

		setColorStrategy(dstColorStrategy);
		setProfile(dstProfile);
		notify();
		notifyLayersChanged();
	}
}

KisProfileSP KisImage::profile() const
{
	return m_profile;
}

void KisImage::setProfile(const KisProfileSP& profile)
{
	if (profile && profile -> valid()) {
		m_profile = profile;
		m_projection -> setProfile(profile);
	}
	else {
		m_profile = 0;
		m_projection -> setProfile(profile);
	}
	emit(profileChanged(m_profile));
}

KURL KisImage::uri() const
{
	return m_uri;
}

void KisImage::uri(const KURL& uri)
{
	if (uri.isValid())
		m_uri = uri;
}

KoUnit::Unit KisImage::unit() const
{
	return m_unit;
}

void KisImage::unit(const KoUnit::Unit& u)
{
	m_unit = u;
}

double KisImage::xRes()
{
	return m_xres;
}

double KisImage::yRes()
{
	return m_yres;
}

void KisImage::setResolution(double xres, double yres)
{
	m_xres = xres;
	m_yres = yres;
}

Q_INT32 KisImage::width() const
{
	return m_width;
}

Q_INT32 KisImage::height() const
{
	return m_height;
}


bool KisImage::empty() const
{
	return m_layers.size() > 0;
}

vKisLayerSP KisImage::layers()
{
	return m_layers;
}

const vKisLayerSP& KisImage::layers() const
{
	return m_layers;
}

KisPaintDeviceSP KisImage::activeDevice()
{
	if (m_activeLayer) {
		return m_activeLayer.data();
	}

	return 0;
}

const KisLayerSP KisImage::activeLayer() const
{
	return m_activeLayer;
}

KisLayerSP KisImage::activeLayer()
{
	return m_activeLayer;
}

KisLayerSP KisImage::activate(KisLayerSP layer)
{
	vKisLayerSP_it it;

	if (m_layers.empty() || !layer)
		return 0;

	it = qFind(m_layers.begin(), m_layers.end(), layer);

	if (it == m_layers.end()) {
		layer = m_layers[0];
		it = m_layers.begin();
	}

	if (layer) {
		it = qFind(m_layerStack.begin(), m_layerStack.end(), layer);

		if (it != m_layerStack.end())
			m_layerStack.erase(it);

		m_layerStack.insert(m_layerStack.begin() + 0, layer);
	}

	if (layer != m_activeLayer) {
		m_activeLayer = layer;
		emit activeLayerChanged(KisImageSP(this));
	}


	return layer;
}

KisLayerSP KisImage::activateLayer(Q_INT32 n)
{
	if (n < 0 || static_cast<Q_UINT32>(n) > m_layers.size())
		return 0;

	return activate(m_layers[n]);
}

Q_INT32 KisImage::index(const KisLayerSP &layer)
{
	for (Q_UINT32 i = 0; i < m_layers.size(); i++) {
		if (m_layers[i] == layer)
			return i;
	}

	return -1;
}

KisLayerSP KisImage::layer(const QString& name)
{
	for (vKisLayerSP_it it = m_layers.begin(); it != m_layers.end(); it++) {
		if ((*it) -> name() == name)
			return *it;
	}

	return 0;
}

KisLayerSP KisImage::layer(Q_UINT32 npos)
{
	if (npos >= m_layers.size())
		return 0;

	return m_layers[npos];
}

bool KisImage::add(KisLayerSP layer, Q_INT32 position)
{
	if (layer == 0)
		return false;

	if (layer -> image() && layer -> image() != KisImageSP(this))
		return false;

	if (qFind(m_layers.begin(), m_layers.end(), layer) != m_layers.end())
		return false;

	layer -> setImage(KisImageSP(this));

	if (position == -1) {
		// Add to bottom of layer stack
		position = m_layers.size();
	}

	m_layers.insert(m_layers.begin() + position, layer);
	activate(layer);

	m_layerStack.push_back(layer);
	return true;
}

void KisImage::rm(KisLayerSP layer)
{
	vKisLayerSP_it it;

	if (layer == 0)
		return;

	it = qFind(m_layers.begin(), m_layers.end(), layer);

	if (it == m_layers.end())
		return;

	m_layers.erase(it);
	it = qFind(m_layerStack.begin(), m_layerStack.end(), layer);

	if (it != m_layerStack.end())
		m_layerStack.erase(it);

	layer -> setImage(0);

	if (layer == m_activeLayer) {
		if (m_layers.empty()) {
			m_activeLayer = 0;
			emit activeLayerChanged(KisImageSP(this));
		} else {
			activate(m_layerStack[0]);
		}
	}
}

bool KisImage::raise(KisLayerSP layer)
{
	Q_INT32 position;

	if (layer == 0)
		return false;

	position = index(layer);

	if (position <= 0)
		return false;

	return pos(layer, position - 1);
}

bool KisImage::lower(KisLayerSP layer)
{
	Q_INT32 position;
	Q_INT32 size;

	if (layer == 0)
		return false;

	position = index(layer);
	size = m_layers.size();

	if (position >= size)
		return false;

	return pos(layer, position + 1);
}

bool KisImage::top(KisLayerSP layer)
{
	Q_INT32 position;

	if (layer == 0)
		return false;

	position = index(layer);

	if (position == 0)
		return false;

	return pos(layer, 0);
}

bool KisImage::bottom(KisLayerSP layer)
{
	Q_INT32 position;
	Q_INT32 size;

	if (layer == 0)
		return false;

	position = index(layer);
	size = m_layers.size();

	if (position >= size - 1)
		return false;

	return pos(layer, size - 1);
}

bool KisImage::pos(KisLayerSP layer, Q_INT32 position)
{
	Q_INT32 old;
	Q_INT32 nlayers;

	if (layer == 0)
		return false;

	old = index(layer);

	if (old < 0)
		return false;

	nlayers = m_layers.size();

	if (position < 0)
		position = 0;

	if (position >= nlayers)
		position = nlayers - 1;

	if (old == position)
		return true;

	if (position < old) {
		m_layers.erase(m_layers.begin() + old);
		m_layers.insert(m_layers.begin() + position, layer);
	}
	else {
		m_layers.insert(m_layers.begin() + position + 1, layer);
		m_layers.erase(m_layers.begin() + old);
	}

	return true;
}

Q_INT32 KisImage::nlayers() const
{
	return m_layers.size();
}

Q_INT32 KisImage::nHiddenLayers() const
{
	Q_INT32 n = 0;

	for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); it++) {
		const KisLayerSP& layer = *it;

		if (!layer -> visible()) {
			n++;
		}
	}

	return n;
}

Q_INT32 KisImage::nLinkedLayers() const
{
	Q_INT32 n = 0;

	for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); it++) {
		const KisLayerSP& layer = *it;

		if (layer -> linked()) {
			n++;
		}
	}

	return n;
}

void KisImage::flatten()
{
	vKisLayerSP beforeLayers = m_layers;

	if (m_layers.empty()) return;

	KisLayerSP dst = new KisLayer(this, nextLayerName(), OPACITY_OPAQUE);
	Q_CHECK_PTR(dst);

	KisFillPainter painter(dst.data());

	vKisLayerSP mergeLayers = layers();

	KisLayerSP bottomLayer = mergeLayers.back();
	QString bottomName =  bottomLayer -> name();

	KisMerge<isVisible, All> visitor(this);
	visitor(painter, mergeLayers);
	dst -> setName(bottomName);

	add(dst, -1);

	notify();
	notifyLayersChanged();

	if (m_adapter && m_adapter -> undo()) {
		m_adapter -> addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("Flatten Image")));
	}
}

void KisImage::mergeVisibleLayers()
{
	vKisLayerSP beforeLayers = m_layers;

	KisLayerSP dst = new KisLayer(this, nextLayerName(), OPACITY_OPAQUE);
	Q_CHECK_PTR(dst);

	KisFillPainter painter(dst.data());

	vKisLayerSP mergeLayers = layers();
	KisMerge<isVisible, isVisible> visitor(this);
	visitor(painter, mergeLayers);

	int insertIndex = -1;

	if (visitor.insertMergedAboveLayer() != 0) {
		insertIndex = index(visitor.insertMergedAboveLayer());
	}

	add(dst, insertIndex);

	notify();
	notifyLayersChanged();

	if (m_adapter && m_adapter -> undo()) {
		m_adapter -> addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("Merge Visible Layers")));
	}
}

void KisImage::mergeLinkedLayers()
{
	vKisLayerSP beforeLayers = m_layers;

	KisLayerSP dst = new KisLayer(this, nextLayerName(), OPACITY_OPAQUE);
	Q_CHECK_PTR(dst);

	KisFillPainter painter(dst.data());

	vKisLayerSP mergeLayers = layers();
	KisMerge<isLinked, isLinked> visitor(this);
	visitor(painter, mergeLayers);

	int insertIndex = -1;

	if (visitor.insertMergedAboveLayer() != 0) {
		insertIndex = index(visitor.insertMergedAboveLayer());
	}

	add(dst, insertIndex);

	notify();
	notifyLayersChanged();

	if (m_adapter && m_adapter -> undo()) {
		m_adapter -> addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("Merge Linked Layers")));
	}
}

void KisImage::mergeLayer(KisLayerSP l)
{
	vKisLayerSP beforeLayers = m_layers;

	KisLayerSP dst = new KisLayer(this, l -> name(), OPACITY_OPAQUE);
	Q_CHECK_PTR(dst);

	KisFillPainter painter(dst.data());

	KisMerge<All, All> visitor(this);
	visitor(painter, layer(index(l) + 1));
	visitor(painter, l);

	int insertIndex = -1;

	if (visitor.insertMergedAboveLayer() != 0) {
		insertIndex = index(visitor.insertMergedAboveLayer());
	}

	add(dst, insertIndex);

	notify();
	notifyLayersChanged();

	if (m_adapter && m_adapter -> undo())
	{
		m_adapter -> addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("&Merge Layers")));
//XXX fix name after string freeze
	}
}


void KisImage::enableUndo(KoCommandHistory *history)
{
	m_undoHistory = history;
}

void KisImage::renderToProjection(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	KisPainter gc;

	gc.begin(m_projection.data());

	gc.bitBlt(x, y, COMPOSITE_COPY, m_bkg.data(), x, y, w, h);

	if (!m_layers.empty()) {
		KisFlatten<flattenAllVisible> visitor(x, y, w, h);

		visitor(gc, m_layers);

		if (m_activeLayer -> hasSelection()) {
			KisSelectionSP s = m_activeLayer -> selection();
			visitor(gc, s);
		}
	}

	gc.end();
}

void KisImage::renderToPainter(Q_INT32 x1,
			       Q_INT32 y1,
			       Q_INT32 x2,
			       Q_INT32 y2,
			       QPainter &painter,
			       KisProfileSP profile)
{
	Q_INT32 x;
	Q_INT32 y;

	// Flatten the layers onto the projection layer of the current image
	for (y = y1; y <= y2; y += RENDER_HEIGHT - (y % RENDER_HEIGHT)) {
		for (x = x1; x <= x2; x += RENDER_WIDTH - (x % RENDER_WIDTH)) {
			Q_INT32 w = QMIN(x2 - x, RENDER_WIDTH);
			Q_INT32 h = QMIN(y2 - y, RENDER_HEIGHT);
			renderToProjection(x, y, w, h);
			QImage img = m_projection -> convertToQImage(profile, x, y, w, h);
			if (!img.isNull()) {
				m_pixmap.convertFromImage(img);
				painter.drawPixmap(x, y, m_pixmap, 0, 0, w, h);
			}
		}
	}
}

void KisImage::notify()
{
	notify(0, 0, width(), height());
}

void KisImage::notify(Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
{
	notify(QRect(x, y, width, height));
}

void KisImage::notify(const QRect& rc)
{
	if (rc.isValid()) {
		emit update(KisImageSP(this), rc);
	}

}

void KisImage::notifyLayersChanged()
{
	emit layersChanged(KisImageSP(this));
}

QRect KisImage::bounds() const
{
	return QRect(0, 0, width(), height());
}

KisUndoAdapter* KisImage::undoAdapter() const
{
	return m_adapter;
}

KisGuideMgr *KisImage::guides() const
{
	return const_cast<KisGuideMgr*>(&m_guides);
}

void KisImage::slotSelectionChanged()
{
 	kdDebug() << "KisImage::slotSelectionChanged\n";
	emit activeSelectionChanged(KisImageSP(this));
}

void KisImage::slotSelectionCreated()
{
 	kdDebug() << "KisImage::slotSelectionCreated\n";
	notify();
	emit selectionCreated(KisImageSP(this));
}


KisStrategyColorSpaceSP KisImage::colorStrategy() const
{
	return m_colorStrategy;
}

void KisImage::setColorStrategy(KisStrategyColorSpaceSP colorStrategy)
{
	m_colorStrategy = colorStrategy;

	m_bkg = new KisBackground(this, m_width, m_height);
	Q_CHECK_PTR(m_bkg);

	m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
	Q_CHECK_PTR(m_projection);
	notify();
}

void KisImage::addAnnotation(KisAnnotationSP annotation)
{
	// Find the icc annotation, if there is one
	vKisAnnotationSP_it it = m_annotations.begin();
	while (it != m_annotations.end()) {
		if ((*it) -> type() == annotation -> type()) {
			*it = annotation;
			return;
		}
		++it;
	}
	m_annotations.push_back(annotation);
}

KisAnnotationSP KisImage::annotation(QString type)
{
	vKisAnnotationSP_it it = m_annotations.begin();
	while (it != m_annotations.end()) {
		if ((*it) -> type() == type) {
			return *it;
		}
		++it;
	}
	return 0;
}

void KisImage::removeAnnotation(QString type)
{
	vKisAnnotationSP_it it = m_annotations.begin();
	while (it != m_annotations.end()) {
		if ((*it) -> type() == type) {
			m_annotations.erase(it);
			return;
		}
		++it;
	}
}

vKisAnnotationSP_it KisImage::beginAnnotations()
{
	if (m_profile) {
		addAnnotation(m_profile -> annotation());
	} else {
		removeAnnotation("icc");
	}

	return m_annotations.begin();
}

vKisAnnotationSP_it KisImage::endAnnotations()
{
	return m_annotations.end();
}

#include "kis_image.moc"

