/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2000 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
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

// Qt
#include <qapplication.h>
#include <qclipboard.h>
#include <qdom.h>
#include <qimage.h>
#include <qpainter.h>
#include <qtl.h>
#include <qstringlist.h>
#include <qwidget.h>

// KDE
#include <dcopobject.h>
#include <kcommand.h>
#include <kdebug.h>
#include <kimageio.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <klocale.h>
#include <ksharedptr.h>

// KOffice
#include <koFilterManager.h>
#include <koMainWindow.h>
#include <koQueryTrader.h>
#include <koStore.h>
#include <koStoreDevice.h>
#include <koTemplateChooseDia.h>

// Local
#include "kis_types.h"
#include "kis_global.h"
#include "kis_channel.h"
#include "kis_dlg_create_img.h"
#include "kis_doc.h"
#include "kis_factory.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_nameserver.h"
#include "kis_painter.h"
#include "kis_mask.h"
#include "kis_selection.h"
#include "kis_command.h"
#include "kis_view.h"
#include "kistilemgr.h"

static const char *CURRENT_DTD_VERSION = "1.2";

namespace {
	class KisCommandImageAdd : public KisCommand {
		typedef KisCommand super;

	public:
		KisCommandImageAdd(KisDoc *doc, KisImageSP img) : super("Add Image", doc)
		{
			m_doc = doc;
			m_img = img;
		}

		virtual ~KisCommandImageAdd()
		{
		}

		virtual void execute()
		{
			m_doc -> setUndo(false);
			m_doc -> addImage(m_img);
			m_doc -> setUndo(true);
		}

		virtual void unexecute()
		{
			m_doc -> setUndo(false);
			m_doc -> removeImage(m_img);
			m_doc -> setUndo(true);
		}

	private:
		KisDoc *m_doc;
		KisImageSP m_img;
	};

	class KisCommandImageMv : public KisCommand {
		typedef KisCommand super;

	public:
		KisCommandImageMv(KisDoc *doc, const QString& name, const QString& oldName) : super(i18n("Rename image"), doc)
		{
			m_doc = doc;
			m_name = name;
			m_oldName = oldName;
		}

		virtual ~KisCommandImageMv()
		{
		}

		virtual void execute()
		{
			m_doc -> setUndo(false);
			m_doc -> renameImage(m_oldName, m_name);
			m_doc -> setUndo(true);
		}

		virtual void unexecute()
		{
			m_doc -> setUndo(false);
			m_doc -> renameImage(m_name, m_oldName);
			m_doc -> setUndo(true);
		}

	private:
		KisDoc *m_doc;
		QString m_name;
		QString m_oldName;
	};

	class KisCommandImageRm : public KisCommand {
		typedef KisCommand super;

	public:
		KisCommandImageRm(KisDoc *doc, KisImageSP img) : super("Remove Image", doc)
		{
			m_doc = doc;
			m_img = img;
		}

		virtual ~KisCommandImageRm()
		{
		}

		virtual void execute()
		{
			m_doc -> setUndo(false);
			m_doc -> removeImage(m_img);
			m_doc -> setUndo(true);
		}

		virtual void unexecute()
		{
			m_doc -> setUndo(false);
			m_doc -> addImage(m_img);
			m_doc -> setUndo(true);
		}

	private:
		KisDoc *m_doc;
		KisImageSP m_img;
	};

	class LayerAddCmd : public KisCommand {
		typedef KisCommand super;

	public:
		LayerAddCmd(KisDoc *doc, KisImageSP img, KisLayerSP layer) : super("Add Layer", doc)
		{
			m_doc = doc;
			m_img = img;
			m_layer = layer;
			m_index = img -> index(layer);
		}

		virtual ~LayerAddCmd()
		{
		}

		virtual void execute()
		{
			m_doc -> setUndo(false);
			m_doc -> layerAdd(m_img, m_layer, m_index);
			m_doc -> setUndo(true);
		}

		virtual void unexecute()
		{
			m_doc -> setUndo(false);
			m_doc -> layerRemove(m_img, m_layer);
			m_doc -> setUndo(true);
		}

	private:
		KisDoc *m_doc;
		KisImageSP m_img;
		KisLayerSP m_layer;
		Q_INT32 m_index;
	};

	class LayerRmCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		LayerRmCmd(KisDoc *doc, KisImageSP img, KisLayerSP layer) : super("Remove Layer")
		{
			m_doc = doc;
			m_img = img;
			m_layer = layer;
			m_index = img -> index(layer);
		}

		virtual ~LayerRmCmd()
		{
		}

		virtual void execute()
		{
			m_doc -> setUndo(false);
			m_doc -> layerRemove(m_img, m_layer);
			m_doc -> setUndo(true);
		}

		virtual void unexecute()
		{
			m_doc -> setUndo(false);
			m_doc -> layerAdd(m_img, m_layer, m_index);
			m_doc -> setUndo(true);
		}

	private:
		KisDoc *m_doc;
		KisImageSP m_img;
		KisLayerSP m_layer;
		Q_INT32 m_index;
	};
}

KisDoc::KisDoc(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, bool singleViewMode) : 
	super(parentWidget, widgetName, parent, name, singleViewMode)
{
	m_undo = false;
	m_dcop = 0;
	setInstance(KisFactory::global(), true);
	m_cmdHistory = 0;
	QPixmap::setDefaultOptimization(QPixmap::BestOptim);
	m_nserver = 0;
	m_pushedClipboard = false;

	if (name)
		dcopObject();
}

KisDoc::~KisDoc()
{
	delete m_cmdHistory;
        delete m_dcop;
	delete m_nserver;
}

QCString KisDoc::mimeType() const
{
	return APP_MIMETYPE;
}

DCOPObject *KisDoc::dcopObject()
{
#if 0
	if (!m_dcop)
		m_dcop = new KIsDocIface(this);

	return m_dcop;
#endif
	return 0;
}

QDomDocument KisDoc::saveXML()
{
	// FIXME: implement saving of non-RGB modes.
        // This creates the DOCTYPE stuff, as well as the document-element named <image>.
	QDomDocument doc = createDomDocument("image", CURRENT_DTD_VERSION);

	doc.documentElement().appendChild(saveImages(doc));
	setModified(false);
	return doc;
}

bool KisDoc::initDoc()
{
	bool ok = false;
	QString templ;
	KoTemplateChooseDia::ReturnType ret;

	m_cmdHistory = new KCommandHistory(actionCollection(), false);
	connect(m_cmdHistory, SIGNAL(documentRestored()), this, SLOT(slotDocumentRestored()));
	connect(m_cmdHistory, SIGNAL(commandExecuted()), this, SLOT(slotCommandExecuted()));
	m_undo = true;
	m_nserver = new KisNameServer(i18n("Image %1"), 0);
	ret = KoTemplateChooseDia::choose(KisFactory::global(), templ, "application/x-krita", "*.kra",
			i18n("Krita"), KoTemplateChooseDia::NoTemplates, "krita_template");

	if (ret == KoTemplateChooseDia::Template) {
		QString name = nextImageName();
		KisImageSP img = new KisImage(this, IMG_DEFAULT_WIDTH, IMG_DEFAULT_HEIGHT, OPACITY_OPAQUE, IMAGE_TYPE_RGBA, name);
		KisLayerSP layer = new KisLayer(img, IMG_DEFAULT_WIDTH, IMG_DEFAULT_DEPTH, img -> nextLayerName(), OPACITY_OPAQUE);

		layer -> visible(true);
		addImage(img);
		ok = true;
	} else if (ret == KoTemplateChooseDia::File) {
		KURL url;

		url.setPath(templ);
		ok = openURL(url);
	} else if (ret == KoTemplateChooseDia::Empty) {
		if ((ok = slotNewImage()))
			emit imageListUpdated();
	}

	return ok;
}

QDomElement KisDoc::saveImages(QDomDocument& doc)
{
	QDomElement images = doc.createElement("images");
#if 0
	QStringList imageNames = images();
	QString tmp_currentImageName = currentImgName();

	images.setAttribute("editor", "Krita");
	images.setAttribute("mime", "application/x-krita");
	images.setAttribute("version", "1.3");

	for (QStringList::Iterator it = imageNames.begin(); it != imageNames.end(); it++) {
		setImage(*it);
		KisImageSP img = m_currentImg;

		// image element
		QDomElement image = doc.createElement("image");

		image.setAttribute("name", img -> name());
		image.setAttribute("author", img -> author());
		image.setAttribute("email", img -> email());
		image.setAttribute("width", img -> width());
		image.setAttribute("height", img -> height());
		image.setAttribute("depth", img -> depth());
		image.setAttribute("type", img -> imgType());
		images.appendChild(image);
		image.appendChild(saveLayers(doc, img));
	}

	setImage(tmp_currentImageName);
	images.appendChild(saveToolSettings(doc));
#endif
	return images;
}

// save layers
QDomElement KisDoc::saveLayers(QDomDocument& doc, KisImageSP)
{
	// layers element - variable
	QDomElement layersElement = doc.createElement("layers");

#if 0
	// layer elements
	kdDebug(0) << "layer elements" << endl;
	KisLayerSPLst layers = img -> layerList();

	for (KisLayerSPLstIterator it = layers.begin(); it != layers.end(); it++) {
		KisLayerSP lay = *it;
		QDomElement layer = doc.createElement( "layer" );

		layer.setAttribute("name", lay -> name());
		layer.setAttribute("x", lay -> imageExtents().x());
		layer.setAttribute("y", lay -> imageExtents().y());
		layer.setAttribute("width", lay -> imageExtents().width());
		layer.setAttribute("height", lay -> imageExtents().height());
		layer.setAttribute("opacity", static_cast<int>(lay -> opacity()));

		kdDebug(0) << "name: " <<  lay -> name() << endl;
		kdDebug(0) << "x: " << lay -> imageExtents().x() << endl;
		kdDebug(0) << "y: " << lay -> imageExtents().y() << endl;
		kdDebug(0) << "width: " << lay -> imageExtents().width() << endl;
		kdDebug(0) << "height: " << lay -> imageExtents().height() << endl;
		kdDebug(0) << "opacity: " <<  static_cast<int>(lay -> opacity()) << endl;

		layer.setAttribute("visible", lay -> visible());
		layer.setAttribute("linked", lay -> linked());
		layer.setAttribute("bitDepth", static_cast<int>(lay -> depth()) );
		layer.setAttribute("cMode", static_cast<int>(lay -> colorMode()) );

		kdDebug(0) << "bitDepth: " <<  static_cast<int>(lay -> depth())  << endl;
		kdDebug(0) << "colorMode: " <<  static_cast<int>(lay -> colorMode())  << endl;

		layersElement.appendChild(layer);
		layer.appendChild(saveChannels(doc, lay));
	}
#endif

	return layersElement;
}

// save channels
QDomElement KisDoc::saveChannels( QDomDocument &doc, KisLayer * /*lay*/ )
{
    // channels element - variable, normally maximum of 4 channels
    QDomElement channels = doc.createElement( "channels" );

    // XXX
#if 0
    kdDebug(0) << "channel elements" << endl;

    // channel elements
    for ( KisChannel* ch = lay->firstChannel(); ch != 0; ch = lay->nextChannel() )
    {
        QDomElement channel = doc.createElement( "channel" );

        channel.setAttribute( "cId", static_cast<int>(ch->channelId()) );
        channel.setAttribute( "bitDepth", static_cast<int>(ch->bitDepth()) );

        kdDebug(0) << "cId: " <<  static_cast<int>(ch->channelId())  << endl;
        kdDebug(0) << "bitDepth: " <<  static_cast<int>(ch->bitDepth())  << endl;

        channels.appendChild( channel );
    } // end of channels loop
#endif

    return channels;
}

QDomElement KisDoc::saveToolSettings(QDomDocument& doc) const
{
	QDomElement tool = doc.createElement("tool");

	return tool;
}

#if 0
// save Gradients settings
QDomElement KisDoc::saveGradientsSettings( QDomDocument &doc )
{
    // gradients element
    QDomElement gradients = doc.createElement( "gradients" );

    gradients.setAttribute( "opacity", gradientsSettings.opacity );
    gradients.setAttribute( "offset", gradientsSettings.offset );
    gradients.setAttribute( "mode", gradientsSettings.mode );
    gradients.setAttribute( "blend", gradientsSettings.blend );
    gradients.setAttribute( "gradient", gradientsSettings.gradient );
    gradients.setAttribute( "repeat", gradientsSettings.repeat );

    return gradients;
}

#endif

/*
    Save extra, document-specific data outside xml format as defined by
    DTD for this document type.  It is appended to the saved
    xml document in gzipped format using the store methods of koffice
    common code koffice/lib/store/ as an internal file, not a real,
    separate file in the filesystem.

    In this case it's the binary image data
    Krayon can only handle rgb and rgba formats for now,
    by channel for each layer of the image saved in binary format

    ko virtual method implemented
*/

bool KisDoc::completeSaving(KoStore * /*store*/)
{
#if 0
	if (!m_currentImg)
		return false;

	QStringList imageNames = images();
	QString tmp_currentImageName = currentImgName();
	uint imageNumbers = 1;

	for (QStringList::Iterator it = imageNames.begin(); it != imageNames.end(); it++) {
		setImage(*it);

		KisLayerSPLst layers = m_currentImg -> layerList();
		uint layerNumbers = 0;

		for (KisLayerSPLstIterator it = layers.begin(); it != layers.end(); it++) {
			KisLayerSP lay = *it;
			QString image = QString("image%1").arg(imageNumbers);
			QString layerName;

			if (layerNumbers == 0)
				layerName = QString::fromLatin1("background");
			else
				layerName = QString("layer%1").arg(layerNumbers);

			QString url = QString("images/%1/layers/%2.bin").arg(image).arg(layerName);

			if (store -> open(url)) {
				lay -> writeToStore(store);
				store -> close();
			}

			layerNumbers++;
		}

		imageNumbers++;
	}

	setImage(tmp_currentImageName);
	return true;
#endif
	return false;
}

/*
    loadXML - reimplements ko method
*/

bool KisDoc::loadXML(QIODevice *, const QDomDocument& doc)
{
	kdDebug(0) << "KisDoc::loadXML() entering" << endl;

	if (doc.doctype().name() != "image") {
		kdDebug(0) << "KisDoc::loadXML() no doctype name error" << endl;
		return false;
	}

	QDomElement images = doc.documentElement();

	if (images.attribute("mime") != "application/x-krita" && images.attribute("mime") != "application/vnd.kde.krita") {
		kdDebug(0) << "KisDoc::loadXML() no mime name error" << endl;
		return false;
	}

	// load images
	if (!loadImages(images))
		return false;

	kdDebug(0) << "KisDoc::loadXML() leaving succesfully" << endl;
	return true;
}

// load images
bool KisDoc::loadImages(QDomElement& element)
{
	for (QDomElement elem = element.firstChild().toElement(); !elem.isNull(); elem = elem.nextSibling().toElement()) {
		if (elem.tagName() == "image")
			return loadImgSettings(elem);
		else if (elem.tagName() == "tool")
			loadToolSettings(elem);
	}

	return true;
}

// load layers
bool KisDoc::loadLayers(QDomElement& /*element*/, KisImageSP /*img*/)
{
	return false;
#if 0
	QDomElement layers = element.namedItem("layers").toElement();

	if (layers.isNull()) {
		kdDebug(0) << "KisDoc::loadXML(): layers.isNull() error!" << endl;
		return false;
	}

	for (QDomNode l = layers.firstChild(); !l.isNull(); l = l.nextSibling()) {
		QDomElement layer = l.toElement();

		if (layer.tagName() == "layer") {
			QString layerName = layer.attribute("name");
			int w = layer.attribute("width").toInt();
			int h = layer.attribute("height").toInt();
			int x = layer.attribute("x").toInt();
			int y = layer.attribute("y").toInt();

			img -> addLayer(QRect(x, y, w, h), KoColor::white(), true, layerName);
			img -> markDirty(QRect(x, y, w, h));

			KisLayerSP lay = img -> getCurrentLayer();

			Q_ASSERT(lay);

			lay -> setOpacity(layer.attribute("opacity").toInt());
			lay -> setVisible(layer.attribute("visible") == "1");
			lay -> setLinked(layer.attribute("linked") == "1");

			// load channels
			loadChannels(layer, lay);
		}
	}

	slotLayersUpdated();
	return true;
#endif
}

// load channels
void KisDoc::loadChannels(QDomElement& /*element*/, KisLayerSP /*lay*/)
{
#if 0
	QDomElement channels = element.namedItem("channels").toElement();

	if (channels.isNull())
		return;

	// channel elements
	for (QDomNode c = channels.firstChild(); !c.isNull(); c = c.nextSibling()) {
		QDomElement channel = c.toElement();

		if (channel.tagName() == "channel") {
			kdDebug(0) << "channel" << endl;
			// TODO
		}
	}
#endif
}

bool KisDoc::loadImgSettings(QDomElement& /*elem*/)
{
#if 0
	QString name = elem.attribute("name");
	int w = elem.attribute("width").toInt();
	int h = elem.attribute("height").toInt();
	int cm = elem.attribute("cMode").toInt();
	int bd = elem.attribute("bitDepth").toInt();
	cMode colorMode = static_cast<cMode>(cm);

	kdDebug(0) << "name: " << name << endl;
	kdDebug(0) << "width: " << w << endl;
	kdDebug(0) << "height: " << w << endl;
	kdDebug(0) << "cMode: " << cm << endl;
	kdDebug(0) << "bitDepth: " << bd << endl;

	KisImageSP img = newImage(name, w, h, colorMode, bd);

	if (!img)
		return false;

	img -> setAuthor(elem.attribute("author"));
	img -> setEmail(elem.attribute("email" ));

	if (!loadLayers(elem, img))
		return false;

	return true;
#endif
	return false;
}

// load tool settings
void KisDoc::loadToolSettings(QDomElement& /*elem*/)
{
#if 0
	KisTool *p;

	kdDebug() << "KisDoc::loadToolSettings\n";
	Q_ASSERT(m_tools.size());

	for (QDomElement tool = elem.firstChild().toElement(); !tool.isNull(); tool = tool.nextSibling().toElement()) {
		for (ktvector_size_type i = 0; i < m_tools.size(); i++) {
			p = m_tools[i];
			Q_ASSERT(p);
			kdDebug() << "TAGNAME = " << tool.tagName() << endl;

			if (p && p -> loadSettings(tool)) {
				kdDebug() << "Loaded Settings\n\n";
				break;
			}
		}
	}
#endif
}

#if 0
// load Gradients settings
void KisDoc::loadGradientsSettings( QDomElement &elem )
{
    gradientsSettings.opacity = elem.attribute( "opacity" ).toInt();
    gradientsSettings.offset = elem.attribute( "offset" ).toInt();
    gradientsSettings.mode = elem.attribute( "mode" );
    gradientsSettings.blend = elem.attribute( "blend" );
    gradientsSettings.gradient = elem.attribute( "gradient" );
    gradientsSettings.repeat = elem.attribute( "repeat" );
}
#endif

bool KisDoc::completeLoading(KoStore * /*store*/)
{
#if 0
	QStringList imageNames = images(); // XXX why go by image names?
	QString tmp_currentImageName = currentImgName();
	uint imageNumbers = 1;

	for (QStringList::Iterator it = imageNames.begin(); it != imageNames.end(); it++) {
		setImage(*it);
		Q_ASSERT(*it == m_currentImg -> name());

		KisLayerSPLst layers = m_currentImg -> layerList();
		uint layerNumbers = 0;

		for (KisLayerSPLstIterator it = layers.begin(); it != layers.end(); it++) {
			KisLayerSP lay = *it;
			QString image = QString("image%1").arg(imageNumbers);
			QString layerName;

			if (layerNumbers == 0)
				layerName = QString::fromLatin1("background");
			else
				layerName = QString("layer%1").arg(layerNumbers);

			QString url = QString("images/%1/layers/%2.bin").arg(image).arg(layerName);

			if (store -> open(url)) {
				lay -> loadFromStore(store);
				store -> close();
			}

			// TODO Load Channels
			layerNumbers++;
		}

		imageNumbers++;
		m_currentImg -> markDirty(QRect(0, 0, m_currentImg -> width(), m_currentImg -> height()));
	}

	return true;
#endif
	return false;
}

void KisDoc::renameImage(const QString& oldName, const QString& newName)
{
	for (vKisImageSP_it it = m_images.begin(); it != m_images.end(); it++) {
		if ((*it) -> name() == oldName) {
			(*it) -> setName(newName);

			if (m_undo)
				addCommand(new KisCommandImageMv(this, newName, oldName));

			emit imageListUpdated();
			break;
		}
	}
}

QStringList KisDoc::images()
{
	QStringList lst;

	for (vKisImageSP_it it = m_images.begin(); it != m_images.end(); it++)
		lst.append((*it) -> name());

	return lst;
}

bool KisDoc::isEmpty() const
{
	return m_images.size() == 0;
}

KisImageSP KisDoc::imageNum(unsigned int num) const
{
	if (m_images.empty() || num > m_images.size())
		return 0;

	return m_images[num];
}

Q_INT32 KisDoc::nimages() const
{
	return m_images.size();
}

KisImageSP KisDoc::findImage(const QString& name)
{
	for (vKisImageSP_it it = m_images.begin(); it != m_images.end(); it++) {
		if ((*it) -> name() == name) {
			return *it;
		}
	}

	return 0;
}

bool KisDoc::contains(KisImageSP img) const
{
	return qFind(m_images.begin(), m_images.end(), img) != m_images.end();
}

KisImageSP KisDoc::newImage(const QString& name, Q_INT32 width, Q_INT32 height, enumImgType type)
{
	KisImageSP img = new KisImage(this, width, height, OPACITY_OPAQUE, type, name);

	m_images.push_back(img);

	if (m_undo)
		addCommand(new KisCommandImageAdd(this, img));

	return img;
}

void KisDoc::addImage(KisImageSP img)
{
	m_images.push_back(img);

	if (m_undo)
		addCommand(new KisCommandImageAdd(this, img));

	img -> invalidate();
	emit imageListUpdated();
	emit layersUpdated(img);
	emit docUpdated();
	setModified(true);
}

void KisDoc::removeImage(KisImageSP img)
{
	vKisImageSP_it it = qFind(m_images.begin(), m_images.end(), img);

	if (it != m_images.end()) {
		m_images.erase(it);
		setModified(true);
	}

	emit imageListUpdated();
	emit docUpdated();

	if (m_undo)
		addCommand(new KisCommandImageRm(this, img));
}

void KisDoc::removeImage(const QString& name)
{
	KisImageSP img = findImage(name);

	if (img)
		removeImage(img);
}

bool KisDoc::slotNewImage()
{
	KisDlgCreateImg dlg(IMG_WIDTH_MAX, IMG_DEFAULT_WIDTH, IMG_HEIGHT_MAX, IMG_DEFAULT_HEIGHT);

	if (dlg.exec() == QDialog::Accepted) {
		QUANTUM opacity = dlg.backgroundOpacity();
		KoColor c = dlg.backgroundColor();
		KisImageSP img = new KisImage(this, dlg.imgWidth(), dlg.imgHeight(), OPACITY_OPAQUE, dlg.colorSpace(), nextImageName());
		KisLayerSP layer = new KisLayer(img, dlg.imgWidth(), dlg.imgHeight(), img -> nextLayerName(), OPACITY_OPAQUE);
		KisPainter gc(layer.data());

		gc.fillRect(0, 0, layer -> width(), layer -> height(), c, opacity);
		gc.end();
		img -> add(layer, -1);
		addImage(img);
		return true;
	}

	return false;
}

KoView* KisDoc::createViewInstance(QWidget* parent, const char *name)
{
	KisView *view = new KisView(this, parent, name);

	return view;
}

void KisDoc::paintContent(QPainter& painter, const QRect& rect, bool transparent, double zoomX, double zoomY)
{
	if (m_projection) {
		QPixmap pixmap;
		Q_INT32 x1 = rect.x();
		Q_INT32 y1 = rect.y();
		Q_INT32 x2 = x1 + rect.width() - 1;
		Q_INT32 y2 = y1 + rect.height() - 1;
		Q_INT32 tileno;

		if (transparent)
			painter.eraseRect(rect);

		if (zoomX != 1.0 || zoomY != 1.0)
			painter.scale(zoomX, zoomY);

		for (Q_INT32 y = y1; y <= y2; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
			for (Q_INT32 x = x1; x <= x2; x += TILE_WIDTH - (x % TILE_WIDTH)) {
				if ((tileno = m_projection -> tileNum(x, y)) < 0)
					continue;

				pixmap = m_projection -> pixmap(tileno);

				if (!pixmap.isNull()) {
					painter.drawPixmap(x, y, pixmap, 
							(x % TILE_WIDTH),
							(y % TILE_HEIGHT),
							TILE_WIDTH - (x % TILE_WIDTH),
							TILE_HEIGHT - (y % TILE_HEIGHT));
				}
			}	
		}

#if 0
		// TODO : Use QPainter here to draw whatever is left (outlines, borders) until KisPainter gets the missing functionality
		{
			KisSelectionSP selection = m_projection -> selection();

			if (selection) {
				QPen pen(Qt::DotLine);
				QRect rc = selection -> bounds();
				QRect clip = selection -> clip();

				if (!clip.isEmpty()) {
					rc.setX(rc.x() + clip.x());
					rc.setY(rc.y() + clip.y());
					rc.setWidth(clip.width());
					rc.setHeight(clip.height());
				}

	//			painter.setClipRect(rect);
				painter.setPen(pen);
				painter.drawRect(rc);
			}
		}
		
#endif

	}
}

void KisDoc::slotImageUpdated()
{
	emit docUpdated();
}

void KisDoc::slotImageUpdated(const QRect& rect)
{
	emit docUpdated(rect);
}

void KisDoc::addCommand(KCommand *cmd)
{
	Q_ASSERT(cmd);

	if (m_undo)
		m_cmdHistory -> addCommand(cmd, false);
	else
		delete cmd;
}

void KisDoc::setUndo(bool undo)
{
	m_undo = undo;
}

Q_INT32 KisDoc::undoLimit() const
{
	return m_cmdHistory -> undoLimit();
}

void KisDoc::setUndoLimit(Q_INT32 limit)
{
	m_cmdHistory -> setUndoLimit(limit);
}

Q_INT32 KisDoc::redoLimit() const
{
	return m_cmdHistory -> redoLimit();
}

void KisDoc::setRedoLimit(Q_INT32 limit)
{
	m_cmdHistory -> setRedoLimit(limit);
}

void KisDoc::slotDocumentRestored()
{
	setModified(false);
}

void KisDoc::slotCommandExecuted()
{
}

QString KisDoc::nextImageName() const
{
	return m_nserver -> name();
}

bool KisDoc::loadNativeFormat(const QString& file)
{
	kdDebug() << "KisDoc::loadNativeFormat : " << file << endl;
	return false;
}

void KisDoc::slotActiveLayerChanged(KisImageSP)
{
}

void KisDoc::slotAlphaChanged(KisImageSP)
{
}

void KisDoc::slotVisibilityChanged(KisImageSP, CHANNELTYPE)
{
}

void KisDoc::slotUpdate(KisImageSP, Q_UINT32 x, Q_UINT32 y, Q_UINT32 w, Q_UINT32 h)
{
	QRect rc(x, y, w, h);

	emit docUpdated(rc);
}

void KisDoc::setProjection(KisImageSP img)
{
	m_projection = img;
}

KisLayerSP KisDoc::layerAdd(KisImageSP img, Q_INT32 width, Q_INT32 height, const QString& name, QUANTUM devOpacity)
{
	KisLayerSP layer;

	if (!contains(img))
		return 0;

	if (img) {
		layer = new KisLayer(img, width, height, name, devOpacity);

		if (layer && img -> add(layer, -1)) {
			layer = img -> activate(layer);

			if (layer) {
				KisPainter gc(layer.data());

				gc.fillRect(0, 0, layer -> width(), layer -> height(), KoColor::black(), OPACITY_TRANSPARENT);
				gc.end();
				img -> top(layer);
				
				if (m_undo)
					addCommand(new LayerAddCmd(this, img, layer)); 
				
				img -> invalidate();
				setModified(true);
				layer -> visible(true);
				emit layersUpdated(img);
			}
		} 
	}

	return layer;
}

KisLayerSP KisDoc::layerAdd(KisImageSP img, const QString& name, KisSelectionSP selection)
{
	KisLayerSP layer;

	if (contains(img) && selection) {
		layer = new KisLayer(selection -> data(), img, name, OPACITY_OPAQUE);

		if (selection -> mask())
			layer -> addMask(selection -> mask());

		if (!img -> add(layer, -1))
			return 0;

		img -> top(layer);
		setModified(true);
	
		if (m_undo)
			addCommand(new LayerAddCmd(this, img, layer)); 

		img -> invalidate(layer -> bounds());
		layer -> move(selection -> x(), selection -> y());
		layer -> visible(true);
		emit layersUpdated(img);
	}

	return layer;
}

KisLayerSP KisDoc::layerAdd(KisImageSP img, KisLayerSP l, Q_INT32 position)
{
	if (!contains(img) || !l)
		return 0;

	if (img -> layer(l -> name()))
		return 0;

	if (!img -> add(l, -1))
		return 0;

	setModified(true);
	img -> pos(l, position);

	if (m_undo)
		addCommand(new LayerAddCmd(this, img, l)); 

	img -> invalidate(l -> bounds());
	l -> visible(true);
	emit layersUpdated(img);

	if (!m_undo)
		emit projectionUpdated(img);

	return l;
}

void KisDoc::layerRemove(KisImageSP img, KisLayerSP layer)
{
	if (!contains(img))
		return;

	if (layer) {
		setModified(true);
		img -> rm(layer);

		if (m_undo)
			addCommand(new LayerRmCmd(this, img, layer));

		img -> invalidate(layer -> bounds());
		emit layersUpdated(img);

		if (!m_undo)
			emit projectionUpdated(img);
	}
}

void KisDoc::layerRaise(KisImageSP img, KisLayerSP layer)
{
	if (!contains(img))
		return;

	if (layer) {
		setModified(true);
		img -> raise(layer);
		emit layersUpdated(img);
	}
}

void KisDoc::layerLower(KisImageSP img, KisLayerSP layer)
{
	if (!contains(img))
		return;

	if (layer) {
		setModified(true);
		img -> lower(layer);
		emit layersUpdated(img);
	}
}

void KisDoc::layerNext(KisImageSP img, KisLayerSP layer)
{
	if (!contains(img))
		return;

	if (layer) {
		Q_INT32 npos = img -> index(layer);
		Q_INT32 n = img -> nlayers();

		if (npos < 0 || npos >= n - 1)
			return;

		npos--;
		layer = img -> layer(npos);

		if (!layer)
			return;

		if (!layer -> visible()) {
			layer -> visible(true);
			setModified(true);
		}

		if (!img -> activate(layer))
			return;

		for (Q_INT32 i = 0; i < npos; i++) {
			layer = img -> layer(i);

			if (layer) {
				layer -> visible(false);
				img -> invalidate(layer -> x(), layer -> y(), layer -> width(), layer -> height());
			}
		}

		setModified(true);
		emit layersUpdated(img);
	}
}

void KisDoc::layerPrev(KisImageSP img, KisLayerSP layer)
{
	if (!contains(img))
		return;

	if (layer) {
		Q_INT32 npos = img -> index(layer);
		Q_INT32 n = img -> nlayers();

		if (npos < 0 || npos >= n - 1)
			return;

		npos++;
		layer = img -> layer(npos);

		if (!layer)
			return;

		if (!layer -> visible()) {
			layer -> visible(true);
			setModified(true);
		}

		if (!img -> activate(layer))
			return;

		for (Q_INT32 i = 0; i < npos; i++) {
			layer = img -> layer(i);

			if (layer) {
				layer -> visible(false);
				img -> invalidate(layer -> x(), layer -> y(), layer -> width(), layer -> height());
			}
		}

		setModified(true);
		emit layersUpdated(img);
	}
}

void KisDoc::layerProperties(KisImageSP img, KisLayerSP layer, QUANTUM opacity, const QString& name)
{
	if (!contains(img))
		return;

	if (layer) {
		layer -> setName(name);
		layer -> opacity(opacity);
		img -> invalidate();
		setModified(true);
		emit layersUpdated(img);
		emit projectionUpdated(img);
	}
}

void KisDoc::setClipboardSelection(KisSelectionSP selection)
{
	m_clipboard = selection;

	if (selection) {
		QImage qimg = selection -> toImage();
		QClipboard *cb = QApplication::clipboard();

		cb -> setImage(qimg);
		m_pushedClipboard = true;
	}
}

KisSelectionSP KisDoc::clipboardSelection()
{
	return m_clipboard;
}

void KisDoc::clipboardDataChanged()
{
	if (!m_pushedClipboard) {
		QClipboard *cb = QApplication::clipboard();
		QImage qimg = cb -> image();

		if (!qimg.isNull()) {
			m_clipboard = new KisSelection(qimg.width(), qimg.height(), 
					qimg.hasAlphaBuffer() ? IMAGE_TYPE_RGBA : IMAGE_TYPE_RGB,
					"KisDoc created clipboard selection");

			m_clipboard -> fromImage(qimg);
		}
	}

	m_pushedClipboard = false;
}

bool KisDoc::undo() const
{
	return m_undo;
}

#include "kis_doc.moc"

