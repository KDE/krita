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
#include "kis_layer.h"
#include "kis_background.h"
#include "kis_channel.h"
#include "kis_doc.h"
#include "kis_mask.h"
#include "kis_nameserver.h"
#include "kis_floatingselection.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kispixeldata.h"
#include "visitors/kis_flatten.h"

namespace {

	const bool DISPLAY_TIMER = false; // Whether to repaint the
	// display every
	// DISPLAY_UPDATE_FREQUENCY
	// milliseconds
	const int DISPLAY_UPDATE_FREQUENCY = 50; // in milliseconds

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
				m_img -> notify();
			}

		virtual void unexecute()
			{
				m_adapter -> setUndo(false);
				m_img -> resize(m_before.width(), m_before.height());
				m_adapter -> setUndo(true);
				m_img -> notify();
			}

	private:
		KisUndoAdapter *m_adapter;
		KisImageSP m_img;
		QSize m_before;
		QSize m_after;
	};

	class KisSelectionSet : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		KisSelectionSet(KisUndoAdapter *adapter, KisImageSP img, KisFloatingSelectionSP selection) : super(i18n("Set Floating Selection"))
			{
				m_adapter = adapter;
				m_img = img;
				m_selection = selection;
			}

		virtual ~KisSelectionSet()
			{
			}

	public:
		virtual void execute()
			{
				m_adapter -> setUndo(false);
				m_img -> unsetSelection(false);
				m_adapter -> setUndo(true);
				m_img -> notify();
			}

		virtual void unexecute()
			{
				m_adapter -> setUndo(false);
				m_img -> setSelection(m_selection);
				m_adapter -> setUndo(true);
				m_img -> notify();
			}

	private:
		KisUndoAdapter *m_adapter;
		KisImageSP m_img;
		KisFloatingSelectionSP m_selection;
	};
}

KisImage::KisImage(KisUndoAdapter *undoAdapter, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name)
{
	init(undoAdapter, width, height, imgType, name);
	setName(name);
	startUpdateTimer();
        m_dcop = 0L;
}

KisImage::KisImage(const KisImage& rhs) : QObject(), KisRenderInterface(rhs)
{
	m_dcop = 0L;
	if (this != &rhs) {
		m_undoHistory = rhs.m_undoHistory;
		m_uri = rhs.m_uri;
		m_name = QString::null;
		m_width = rhs.m_width;
		m_height = rhs.m_height;
		m_depth = rhs.m_depth;
		m_ntileCols = rhs.m_ntileCols;
		m_ntileRows = rhs.m_ntileRows;
		m_xres = rhs.m_xres;
		m_yres = rhs.m_yres;
		m_unit = rhs.m_unit;
		m_type = rhs.m_type;
		m_clrMap = rhs.m_clrMap;
		m_dirty = rhs.m_dirty;
		m_adapter = rhs.m_adapter;

		if (rhs.m_shadow)
			m_shadow = new KisTileMgr(*rhs.m_shadow);

		m_bkg = new KisBackground(this, m_width, m_height);
		m_projection = new KisLayer(this, m_width, m_height, "projection", OPACITY_OPAQUE);
		m_layers.reserve(rhs.m_layers.size());

		for (vKisLayerSP_cit it = rhs.m_layers.begin(); it != rhs.m_layers.end(); it++) {
			KisLayerSP layer = new KisLayer(**it);

			layer -> setImage(KisImageSP(this));
			m_layers.push_back(layer);
			m_layerStack.push_back(layer);
			m_activeLayer = layer;
		}

		m_channels.reserve(rhs.m_channels.size());

		for (vKisChannelSP_cit it = rhs.m_channels.begin(); it != rhs.m_channels.end(); it++) {
			KisChannelSP channel = new KisChannel(**it);

			channel -> setImage(KisImageSP(this));
			m_channels.push_back(channel);
			m_activeChannel = channel;
		}

		if (rhs.m_selectionMask)
			m_selectionMask = new KisChannel(*m_selectionMask);

		m_visible = rhs.m_visible;
		m_active = rhs.m_active;
		m_alpha = rhs.m_alpha;
		m_maskEnabled = rhs.m_maskEnabled;
		m_maskInverted = rhs.m_maskInverted;
		m_maskClr = rhs.m_maskClr;
		m_nserver = new KisNameServer(i18n("Layer %1"), rhs.m_nserver -> currentSeed() + 1);
		m_guides = rhs.m_guides;
		startUpdateTimer();
	}

}


DCOPObject *KisImage::dcopObject()
{
	if (!m_dcop)
		m_dcop = new KIsImageIface(this);
	return m_dcop;
}

KisImage::~KisImage()
{
	delete m_nserver;
        delete m_dcop;
	// Not necessary to destroy m_updateTimer
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

QString KisImage::nextLayerName() const
{
	if (m_nserver -> currentSeed() == 0) {
		m_nserver -> number();
		return i18n("background");
	}

	return m_nserver -> name();
}

void KisImage::init(KisUndoAdapter *adapter, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name)
{
	Q_INT32 n;

	m_adapter = adapter;
	m_nserver = new KisNameServer(i18n("Layer %1"), 0);
	n = ::imgTypeDepth(imgType);
	m_active.resize(n);
	m_visible.resize(n);
	m_name = name;
	m_width = width;
	m_height = height;
	m_depth = n;
	m_type = imgType;
	m_bkg = new KisBackground(this, m_width, m_height);
	m_projection = new KisLayer(this, m_width, m_height, "projection", OPACITY_OPAQUE);
	m_xres = 1.0;
	m_yres = 1.0;
	m_unit = KoUnit::U_PT;
	m_dirty = false;
	m_maskEnabled = false;
	m_maskInverted = false;
	m_alpha = false;
	m_undoHistory = 0;
	m_ntileCols = (width + TILE_WIDTH - 1) / TILE_WIDTH;
	m_ntileRows = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;
}

void KisImage::resize(Q_INT32 w, Q_INT32 h)
{
	kdDebug() << "KisImage::resize. From: (" 
		  << m_width 
		  << ", " 
		  << m_height
		  << ") to (" 
		  << w 
		  << ", " 
		  << h 
		  << ")\n";
	if (m_adapter && m_adapter -> undo())
		m_adapter -> addCommand(new KisResizeImageCmd(m_adapter, this, w, h, m_width, m_height));

	m_width = w;
	m_height = h;
	m_ntileCols = (w + TILE_WIDTH - 1) / TILE_WIDTH;
	m_ntileRows = (h + TILE_HEIGHT - 1) / TILE_HEIGHT;
	m_bkg = new KisBackground(this, m_width, m_height);
	m_projection = new KisLayer(this, m_width, m_height, "projection", OPACITY_OPAQUE);
	invalidate();
}

void KisImage::resize(const QRect& rc)
{
	resize(rc.width(), rc.height());
}

void KisImage::scale(double sx, double sy) 
{
	if (m_layers.empty()) return; // Nothing to scale

	// New image size. XXX: Pass along to discourage rounding errors?
	Q_INT32 w, h;	
	w = (Q_INT32)(( m_width * sx) + 0.5);
	h = (Q_INT32)(( m_height * sy) + 0.5); 
	
	vKisLayerSP_it it;
	for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
		KisLayerSP layer = (*it);
		kdDebug() << "Scaling layer " << layer -> name() << "\n";
		layer -> scale(sx, sy);
	}

	// resize(w, h);
}

enumImgType KisImage::imgType() const
{
	switch (m_type) {
	case IMAGE_TYPE_INDEXED:
		return IMAGE_TYPE_INDEXEDA;
	case IMAGE_TYPE_GREY:
		return IMAGE_TYPE_GREYA;
	case IMAGE_TYPE_RGB:
		return IMAGE_TYPE_RGBA;
	case IMAGE_TYPE_CMYK:
		return IMAGE_TYPE_CMYKA;
	case IMAGE_TYPE_LAB:
		return IMAGE_TYPE_LABA;
	case IMAGE_TYPE_YUV:
		return IMAGE_TYPE_YUVA;
	default:
		return m_type;
	}

	return m_type;
}

enumImgType KisImage::nativeImgType() const
{
	return alpha() ? imgTypeWithAlpha() : imgType();
}

enumImgType KisImage::imgTypeWithAlpha() const
{
	switch (m_type) {
	case IMAGE_TYPE_INDEXEDA:
		return IMAGE_TYPE_INDEXED;
	case IMAGE_TYPE_GREYA:
		return IMAGE_TYPE_GREY;
	case IMAGE_TYPE_RGBA:
		return IMAGE_TYPE_RGB;
	case IMAGE_TYPE_CMYKA:
		return IMAGE_TYPE_CMYK;
	case IMAGE_TYPE_LABA:
		return IMAGE_TYPE_LAB;
	case IMAGE_TYPE_YUVA:
		return IMAGE_TYPE_YUV;
	default:
		return m_type;
	}

	return m_type;
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

Q_UINT32 KisImage::depth() const
{
	return m_depth;
}

bool KisImage::alpha() const
{
	return m_alpha;
}

bool KisImage::empty() const
{
	return m_layers.size() > 0;
}

// bool KisImage::colorMap(KoColorMap& cm)
// {
// 	if (m_clrMap.empty())
// 		return false;

// 	cm = m_clrMap;
// 	return true;
// }

// KisChannelSP KisImage::mask()
// {
// 	return m_selectionMask;
// }

// KoColor KisImage::color() const
// {
// 	return KoColor();
// }

// KoColor KisImage::transformColor() const
// {
// 	return KoColor();
// }

KisTileMgrSP KisImage::shadow() const
{
	return m_shadow;
}

void KisImage::activeComponent(CHANNELTYPE type, bool value)
{
	PIXELTYPE i = pixelFromChannel(type);

	if (i != PIXEL_UNDEF)
		m_active[i] = value;
}

bool KisImage::activeComponent(CHANNELTYPE type)
{
	PIXELTYPE i = pixelFromChannel(type);

	if (i != PIXEL_UNDEF)
		return m_active[i];

	return false;
}

void KisImage::visibleComponent(CHANNELTYPE type, bool value)
{
	PIXELTYPE i = pixelFromChannel(type);

	if (i != PIXEL_UNDEF && value != m_visible[i]) {
		m_visible[i] = value;
		emit visibilityChanged(KisImageSP(this), type);
	}
}

bool KisImage::visibleComponent(CHANNELTYPE type) const
{
	PIXELTYPE i = pixelFromChannel(type);

	return i != PIXEL_UNDEF && m_visible[i];
}

void KisImage::flush()
{
}

vKisLayerSP KisImage::layers()
{
	return m_layers;
}

const vKisLayerSP& KisImage::layers() const
{
	return m_layers;
}

vKisChannelSP KisImage::channels()
{
	return m_channels;
}

const vKisChannelSP& KisImage::channels() const
{
	return m_channels;
}

KisPaintDeviceSP KisImage::activeDevice()
{
	if (m_selection)
		return m_selection.data();

	if (m_activeChannel)
		return m_activeChannel.data();

	if (m_activeLayer) {
		if (m_activeLayer -> mask())
			return m_activeLayer -> mask().data();

		return m_activeLayer.data();
	}

	return 0;
}

const KisLayerSP KisImage::activeLayer() const
{
	return m_selection ? static_cast<KisLayerSP>(m_selection) : m_activeLayer;
}

KisLayerSP KisImage::activeLayer()
{
	return m_selection ? static_cast<KisLayerSP>(m_selection) : m_activeLayer;
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
	bool alpha = false;

	if (layer == 0)
		return false;

	if (layer -> image() && layer -> image() != KisImageSP(this))
		return false;

	if (qFind(m_layers.begin(), m_layers.end(), layer) != m_layers.end())
		return false;

	layer -> setImage(KisImageSP(this));

	if (layer -> mask())
		layer -> mask() -> setImage(KisImageSP(this));

	if (position == -1) {
		// Add to bottom of layer stack
		position = m_layers.size();
	}

	if (position == 0 && m_selection && m_selection != layer)
		position = 1;

	if (m_layers.size() == 1 && m_layers[0] -> alpha())
		alpha = true;

	m_layers.insert(m_layers.begin() + position, layer);
	activate(layer);
	layer -> update();

	if (alpha)
		emit alphaChanged(KisImageSP(this));

	m_layerStack.push_back(layer);
	expand(layer.data());
	return true;
}

void KisImage::rm(KisLayerSP layer)
{
	QRect rc;
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

	if (m_selection == layer) {
		m_selection = 0;
		emit selectionChanged(KisImageSP(this));
	}

	if (layer == m_activeLayer) {
		if (m_layers.empty()) {
			m_activeLayer = 0;
			emit activeLayerChanged(KisImageSP(this));
		} else {
			activate(m_layerStack[0]);
		}
	}

	rc = layer -> bounds();
	invalidate(rc.x(), rc.y(), rc.width(), rc.height());

	if (m_layers.size() == 1 && m_layers[0] -> alpha())
		emit alphaChanged(KisImageSP(this));
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

	invalidate();
	return true;
}

Q_INT32 KisImage::nlayers() const
{
	return m_layers.size();
}

KisChannelSP KisImage::activeChannel()
{
	return m_activeChannel;
}

KisChannelSP KisImage::activate(KisChannelSP channel)
{
	if (m_channels.empty() || !channel)
		return 0;

	if (m_selection)
		return 0;

	if (qFind(m_channels.begin(), m_channels.end(), channel) == m_channels.end())
		channel = m_channels[0];

	if (channel != m_activeChannel) {
		m_activeChannel = channel;
		emit activeChannelChanged(KisImageSP(this));
	}

	return channel;
}

KisChannelSP KisImage::activateChannel(Q_INT32 n)
{
	if (n < 0 || static_cast<Q_UINT32>(n) > m_channels.size())
		return 0;

	return activate(m_channels[n]);
}

KisChannelSP KisImage::unsetActiveChannel()
{
	KisChannelSP channel = activeChannel();

	if (channel) {
		m_activeChannel = 0;
		emit activeChannelChanged(KisImageSP(this));

		if (!m_layerStack.empty()) {
			KisLayerSP layer = *m_layerStack.begin();

			activate(layer);
		}
	}

	return channel;
}

Q_INT32 KisImage::index(KisChannelSP channel)
{
	for (Q_UINT32 i = 0; i < m_channels.size(); i++) {
		if (m_channels[i] == channel)
			return i;
	}

	return -1;
}

KisChannelSP KisImage::channel(const QString& name)
{
	for (vKisChannelSP_it it = m_channels.begin(); it != m_channels.end(); it++) {
		if ((*it) -> name() == name)
			return *it;
	}

	return 0;

}

KisChannelSP KisImage::channel(Q_UINT32 npos)
{
	if (npos >= m_channels.size())
		return 0;

	return m_channels[npos];
}

bool KisImage::add(KisChannelSP channel, Q_INT32 position)
{
	if (channel == 0)
		return false;

	if (channel -> image() && channel -> image() != KisImageSP(this))
		return false;

	if (qFind(m_channels.begin(), m_channels.end(), channel) != m_channels.end())
		return false;

	if (position == -1) {
		KisChannelSP active = activeChannel();

		position = active ? index(active) : 0;
	}

	m_channels.insert(m_channels.begin() + position, channel);
	activate(channel);

	if (channel -> visible())
		channel -> update();

	expand(channel.data());
	return true;
}

void KisImage::rm(KisChannelSP channel)
{
	vKisChannelSP_it it;

	if (channel == 0)
		return;

	it = qFind(m_channels.begin(), m_channels.end(), channel);

	if (it == m_channels.end())
		return;

	m_channels.erase(it);

	if (channel == activeChannel()) {
		if (m_channels.empty())
			unsetActiveChannel();
		else
			activate(m_channels[0]);
	}
}

bool KisImage::raise(KisChannelSP channel)
{
	Q_INT32 position;

	if (channel == 0)
		return false;

	position = index(channel);

	if (position == 0)
		return false;

	return pos(channel, position - 1);
}

bool KisImage::lower(KisChannelSP channel)
{
	Q_INT32 position;
	Q_INT32 size;

	if (channel == 0)
		return false;

	position = index(channel);
	size = m_channels.size();

	if (position == size - 1)
		return false;

	return pos(channel, position + 1);
}

bool KisImage::pos(KisChannelSP channel, Q_INT32 position)
{
	Q_INT32 old;
	Q_INT32 nchannels;

	if (channel == 0)
		return false;

	old = index(channel);

	if (old < 0)
		return false;

	nchannels = m_channels.size();
	position = CLAMP(position, 0, nchannels - 1);

	if (position == old)
		return true;

	qSwap(m_channels[old], m_channels[position]);
	channel -> update();
	return true;
}

Q_INT32 KisImage::nchannels() const
{
	return m_channels.size();
}

bool KisImage::boundsLayer()
{
	return false;
}

KisLayerSP KisImage::correlateLayer(Q_INT32 , Q_INT32 )
{
	return 0;
}

void KisImage::enableUndo(KoCommandHistory *history)
{
	m_undoHistory = history;
}

PIXELTYPE KisImage::pixelFromChannel(CHANNELTYPE type) const
{
	PIXELTYPE i = PIXEL_UNDEF;

	switch (type) {
	case REDCHANNEL:
		i = PIXEL_RED;
		break;
	case GREENCHANNEL:
		i = PIXEL_GREEN;
		break;
	case BLUECHANNEL:
		i = PIXEL_BLUE;
		break;
	case GRAYCHANNEL:
		i = PIXEL_GRAY;
		break;
	case INDEXEDCHANNEL:
		i = PIXEL_INDEXED;
		break;
	case ALPHACHANNEL:
		switch (imgType()) {
		case IMAGE_TYPE_GREY:
			i = PIXEL_GRAY_ALPHA;
			break;
		case IMAGE_TYPE_INDEXED:
			i = PIXEL_INDEXED_ALPHA;
			break;
		default:
			i = PIXEL_ALPHA;
			break;
		}
		break;
	default:
		break;
	}

	return i;
}

Q_INT32 KisImage::tileNum(Q_INT32 xpix, Q_INT32 ypix) const
{
	Q_INT32 row;
	Q_INT32 col;
	Q_INT32 num;

	if (xpix >= m_width || ypix >= m_height)
		return -1;

	row = ypix / TILE_HEIGHT;
	col = xpix / TILE_WIDTH;
	num = row * m_ntileCols + col;
	return num;
}

void KisImage::invalidate(Q_INT32 tileno)
{
	m_projection -> invalidate(tileno);
}

void KisImage::invalidate(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	m_projection -> invalidate(x, y, w, h);
}

void KisImage::invalidate(const QRect& rc)
{
	m_projection -> invalidate(rc);
}

void KisImage::invalidate()
{
	m_projection -> invalidate();
}

void KisImage::validate(Q_INT32 tileno)
{
	KisTileMgrSP tm = m_projection -> data();
	KisTileSP dst;
	KisPainter gc;
	QPoint pt;
	QPixmap pix;

	Q_ASSERT(tm);

	if (tileno < 0)
		return;

	if (tileno >= (m_ntileCols * m_ntileRows)) {
		m_ntileCols = (width() + TILE_WIDTH - 1) / TILE_WIDTH;
		m_ntileRows = (height() + TILE_HEIGHT - 1) / TILE_HEIGHT;

		if (tileno >= (m_ntileCols * m_ntileRows))
			return;
	}

	dst = tm -> tile(tileno, TILEMODE_WRITE);
	Q_ASSERT(dst);

	if (!dst || dst -> valid())
		return;

	gc.begin(m_projection.data());
	tm -> tileCoord(dst, pt);
	gc.bitBlt(pt.x(), pt.y(), COMPOSITE_COPY, m_bkg.data(), pt.x(), pt.y(), dst -> width(), dst -> height());

	if (!m_layers.empty()) {
		KisFlatten<flattenAllVisible> visitor(pt.x(), pt.y(), dst -> width(), dst -> height());

		visitor(gc, m_layers);

		if (m_selection)
			visitor(gc, m_selection);
	}

	gc.end();
	dst -> valid(true);
}

void KisImage::setSelection(KisFloatingSelectionSP selection)
{
	if (m_selection != selection) {
		unsetSelection();
		m_selection = selection;
		invalidate(m_selection -> bounds());
		emit selectionChanged(this);
	}
}

void KisImage::unsetSelection(bool commit)
{
	if (m_selection) {
		QRect rc = m_selection -> bounds();

		if (commit) {
			bool inMacro = m_adapter -> inMacro();

			if (m_adapter -> undo()) {
				if (!inMacro)
					m_adapter -> beginMacro(i18n("Anchor Selection"));

				m_adapter -> addCommand(new KisSelectionSet(m_adapter, this, m_selection));
			}

			m_selection -> commit();

			if (m_adapter -> undo() && !inMacro)
				m_adapter -> endMacro();
		}

		m_selection = 0;
		invalidate(rc);
		emit selectionChanged(KisImageSP(this));
	}
}

KisFloatingSelectionSP KisImage::selection() const
{
	return m_selection;
}

void KisImage::expand(KisPaintDeviceSP dev)
{
	Q_INT32 w;
	Q_INT32 h;

	if (dev -> width() >= width() || dev -> height() >= height()) {
		w = QMAX(dev -> width(), width());
		h = QMAX(dev -> height(), height());
		resize(w, h);
		invalidate();
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
	if (DISPLAY_TIMER) {
		m_displayMutex.lock();
		if (m_dirtyRect.isValid()) {
			m_dirtyRect |= rc;
		} else {
			m_dirtyRect = rc;
		}
		m_displayMutex.unlock();
	} else {
		if (rc.isValid()) {
			invalidate(rc);
			emit update(KisImageSP(this), rc);
		}
	}


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

KisTileMgrSP KisImage::tiles() const
{
	return m_projection -> data();
}

void KisImage::slotUpdateDisplay()
{
	if (m_dirtyRect.isValid()) {
		m_displayMutex.lock();
		QRect rect = m_dirtyRect;
		m_dirtyRect = QRect();
		m_displayMutex.unlock();

		invalidate(rect);
		emit update(KisImageSP(this), rect);
	}
}

void KisImage::startUpdateTimer()
{
	if (DISPLAY_TIMER) {
		m_updateTimer = new QTimer(this);
		connect( m_updateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateDisplay()) );
		m_updateTimer -> start( DISPLAY_UPDATE_FREQUENCY );
	}
}

#include "kis_image.moc"

