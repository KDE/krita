/*
 *  kis_doc.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2000 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@ideasandassociates.com>
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

#include <Magick++.h>

#include <qdom.h>
#include <qimage.h>
#include <qpainter.h>
#include <qwidget.h>

#include <kcommand.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <kimageio.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <koFilterManager.h>
#include <koMainWindow.h>
#include <koQueryTrader.h>
#include <koTemplateChooseDia.h>
#include <koStore.h>
#include <koStoreDevice.h>

#include "kis_channel.h"
#include "kis_doc.h"
#include "kis_dlg_new.h"
#include "kis_factory.h"
#include "kis_framebuffer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_pixel_packet.h"
#include "kis_selection.h"
#include "kis_undo.h"
#include "kis_view.h"
#include "KIsDocIface.h"

// Make sure an appropriate DTD is available in www/koffice/DTD if changing this value
static const char * CURRENT_DTD_VERSION = "1.2";

class KisCommandImageAdd : public KisCommand {
	typedef KisCommand super;

public:
	KisCommandImageAdd(KisDoc *doc, KisImageSP img);
	virtual ~KisCommandImageAdd();

	virtual void execute();
	virtual void unexecute();

private:
	KisDoc *m_doc;
	KisImageSP m_img;
};

class KisCommandImageRm : public KisCommand {
	typedef KisCommand super;

public:
	KisCommandImageRm(KisDoc *doc, KisImageSP img);
	virtual ~KisCommandImageRm();

	virtual void execute();
	virtual void unexecute();

private:
	KisDoc *m_doc;
	KisImageSP m_img;
};

class KisCommandImageMv : public KisCommand {
	typedef KisCommand super;

public:
	KisCommandImageMv(KisDoc *doc, const QString& name, const QString& oldName);
	virtual ~KisCommandImageMv();

	virtual void execute();
	virtual void unexecute();

private:
	KisDoc *m_doc;
	QString m_name;
	QString m_oldName;
};

KisCommandImageAdd::KisCommandImageAdd(KisDoc *doc, KisImageSP img) : super("Add Image", doc)
{
	m_doc = doc;
	m_img = img;
}

KisCommandImageAdd::~KisCommandImageAdd()
{
}

void KisCommandImageAdd::execute()
{
	m_doc -> setUndo(false);
	m_doc -> addImage(m_img);
	m_doc -> setUndo(true);
}

void KisCommandImageAdd::unexecute()
{
	m_doc -> setUndo(false);
	m_doc -> removeImage(m_img);
	m_doc -> setUndo(true);
}

KisCommandImageRm::KisCommandImageRm(KisDoc *doc, KisImageSP img) : super("Remove Image", doc)
{
	m_doc = doc;
	m_img = img;
}

KisCommandImageRm::~KisCommandImageRm()
{
}

void KisCommandImageRm::execute()
{
	m_doc -> setUndo(false);
	m_doc -> removeImage(m_img);
	m_doc -> setUndo(true);
}

void KisCommandImageRm::unexecute()
{
	m_doc -> setUndo(false);
	m_doc -> addImage(m_img);
	m_doc -> setUndo(true);
}

KisCommandImageMv::KisCommandImageMv(KisDoc *doc, const QString& name, const QString& oldName) : super("Rename Image", doc)
{
	m_doc = doc;
	m_name = name;
	m_oldName = oldName;
}

KisCommandImageMv::~KisCommandImageMv()
{
}

void KisCommandImageMv::execute()
{
	m_doc -> setUndo(false);
	m_doc -> renameImage(m_oldName, m_name);
	m_doc -> setUndo(true);
}

void KisCommandImageMv::unexecute()
{
	m_doc -> setUndo(false);
	m_doc -> renameImage(m_name, m_oldName);
	m_doc -> setUndo(true);
}

/*
    KisDoc - constructor ko virtual method implemented
*/

KisDoc::KisDoc(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, bool singleViewMode)
	: KoDocument(parentWidget, widgetName, parent, name, singleViewMode)
{
	bool loadPlugins = true;

	m_undo = true;
        dcop = 0;
	setInstance(KisFactory::global(), loadPlugins);
	m_cmdHistory = new KCommandHistory(actionCollection(), false);
	m_currentView = 0;
	m_currentImg = 0L;
	m_pClipImage = 0L;
	m_pSelection = new KisSelection(this);
	m_pFrameBuffer = new KisFrameBuffer(this);

	connect(m_cmdHistory, SIGNAL(documentRestored()), this, SLOT(slotDocumentRestored()));
	connect(m_cmdHistory, SIGNAL(commandExecuted()), this, SLOT(slotCommandExecuted()));

        if (name)
            dcopObject();
}

/*
    KisDoc destructor - Note that since this is MDI, each image
    in the list must be deleted to free up all memory.  While
    there is only ONE Koffice "document", there are multiple
    images you can load and work with in any session
*/

KisDoc::~KisDoc()
{
	unsetCurrentImage();
	delete m_pClipImage;
	delete m_pSelection;
	delete m_pFrameBuffer;
	delete m_cmdHistory;
        delete dcop;
}

DCOPObject* KisDoc::dcopObject()
{
	if (!dcop)
		dcop = new KIsDocIface(this);

	return dcop;
}

KisImageSP KisDoc::imageNum(unsigned int num)
{
	if (m_images.empty() || num > m_images.size())
		return 0;

	return m_images[num];
}

/*
    initDoc - ko virtual method implemented
*/

bool KisDoc::initDoc()
{
	bool ok = false;
	QString name = i18n("image %1").arg(m_images.size() + 1);

	kdDebug() << "KisDoc::initDoc\n";

	// choose dialog for open mode
	QString templ;
	KoTemplateChooseDia::ReturnType ret;

	ret = KoTemplateChooseDia::choose (KisFactory::global(),
			templ,
			"application/x-krita", "*.kra",
			i18n("Krayon"),
			KoTemplateChooseDia::NoTemplates,
			"krita_template");

	// create document from template - use default
	// 512x512 RGBA image util we have real templates
	// however, this will never happen because KoTemplateChossDia
	// returns false if there is no templae selected

	if (m_currentView) {
		removeView(m_currentView);
		delete m_currentView;
		m_currentView = 0;
	}

	if (ret == KoTemplateChooseDia::Template) {
		KisImageSP img = newImage(name, 512, 512, cm_RGBA, 8);

		if (!img)
			return false;

		// add background layer
		img -> addLayer(QRect(0, 0, 512, 512), KoColor::white(), false, i18n("background"));
		img -> markDirty(QRect(0, 0, 512, 512));

		// list of images - mdi document
		setCurrentImage(img);

		// signal to tabbar for iages
		emit imageListUpdated();

		setModified (true);
		ok = true;
	}
	else if (ret == KoTemplateChooseDia::File) {
		KURL url;
		url.setPath (templ);
		ok = openURL (url);
	}
	// create a new document from scratch
	else if (ret == KoTemplateChooseDia::Empty) {
		// NewDialog for entering parameters
		ok = slotNewImage();
		// signal to tabbar for images
		if(ok)
			emit imageListUpdated();
	}

	return ok;
}


/*
    Save document (image) to xml format using QDomDocument
    ko virtual method implemented
*/

QDomDocument KisDoc::saveXML()
{
	kdDebug(0) << "KisDoc::saveXML" << endl;

	// FIXME: implement saving of non-RGB modes.

        // This creates the DOCTYPE stuff, as well as the document-element named <image>.
        QDomDocument doc = createDomDocument( "image", CURRENT_DTD_VERSION );
	doc.documentElement().appendChild( saveImages( doc ) );

	setModified( false );
	return doc;
}

QDomElement KisDoc::saveImages(QDomDocument& doc)
{
	QStringList imageNames = images();
	QString tmp_currentImageName = currentImgName();
	QDomElement images = doc.createElement("images");

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
		image.setAttribute("bitDepth", static_cast<int>(img -> bitDepth()));
		image.setAttribute("cMode", static_cast<int>(img -> colorMode()));
		images.appendChild(image);
		image.appendChild(saveLayers(doc, img));
	}

	setImage(tmp_currentImageName);
	images.appendChild(saveToolSettings(doc));
	return images;
}

// save layers
QDomElement KisDoc::saveLayers(QDomDocument& doc, KisImageSP img)
{
	// layers element - variable
	QDomElement layersElement = doc.createElement("layers");

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

// save tool settings
QDomElement KisDoc::saveToolSettings(QDomDocument& doc) const
{
	QDomElement tool = doc.createElement("tool");

	for (ktvector_size_type i = 0; i < m_tools.size(); i++) {
		kdDebug() << "KisDoc::saveToolSettings\n";
		tool.appendChild(m_tools[i] -> saveSettings(doc));
   	}

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

bool KisDoc::completeSaving(KoStore *store)
{
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
}

/*
    loadXML - reimplements ko method
*/

bool KisDoc::loadXML(QIODevice *, const QDomDocument& doc)
{
	kdDebug(0) << "KisDoc::loadXML() entering" << endl;

	if (!m_currentView) {
		m_currentView = new KisView(this);
		m_currentView -> setupTools();
	}

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
bool KisDoc::loadLayers(QDomElement& element, KisImageSP img)
{
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

bool KisDoc::loadImgSettings(QDomElement& elem)
{
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

	setCurrentImage(img);
	return true;
}

// load tool settings
void KisDoc::loadToolSettings(QDomElement& elem)
{
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

bool KisDoc::completeLoading(KoStore *store)
{
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
		setCurrentImage(m_currentImg);
	}

	return true;
}

/*
    setCurrentImage - using pointer to a KisImage - this is
    normally done from the view.
*/

void KisDoc::setCurrentImage(KisImageSP img)
{
	unsetCurrentImage();
	m_currentImg = img;

	if (m_currentImg) {
		// connect new currentImg image
		QObject::connect(m_currentImg, SIGNAL(updated()), this, SLOT(slotImageUpdated()));
		QObject::connect(m_currentImg, SIGNAL(updated(const QRect&)), this, SLOT(slotImageUpdated(const QRect&)));
		QObject::connect(m_currentImg, SIGNAL(layersUpdated()), this, SLOT(slotLayersUpdated()));
	}

	// signal to tabbar for images - kis_view.cc
	emit imageListUpdated();
	// signal to currentImg image - kis_image.cc
	emit layersUpdated();
	// signal to view to update contents - kis_view.cc
	emit docUpdated();
}

/*
    setCurrentImage - by name
*/

void KisDoc::setCurrentImage(const QString& name)
{
	for (KisImageSPLstIterator it = m_images.begin(); it != m_images.end(); it++) {
		if ((*it) -> name() == name) {
			setCurrentImage(*it);
			return;
		}
	}
}

void KisDoc::unsetCurrentImage()
{
	if (m_currentImg) {
		// disconnect old currentImg image
		QObject::disconnect(m_currentImg, SIGNAL(updated()), this, SLOT(slotImageUpdated()));
		QObject::disconnect(m_currentImg, SIGNAL(updated(const QRect&)), this, SLOT(slotImageUpdated(const QRect&)));
		QObject::disconnect(m_currentImg, SIGNAL(layersUpdated()), this, SLOT(slotLayersUpdated()));
	}

	m_currentImg = 0;
}

/*
    renameImage - from menu or click on image tab
*/

void KisDoc::renameImage(const QString& oldName, const QString& newName)
{
	for (KisImageSPLstIterator it = m_images.begin(); it != m_images.end(); it++) {
		if ((*it) -> name() == oldName) {
			(*it) -> setName(newName);
			break;
		}
	}

	if (m_undo)
		addCommand(new KisCommandImageMv(this, newName, oldName));

	emit imageListUpdated();
}

/*
    images - build list of images by name
*/

QStringList KisDoc::images()
{
	QStringList lst;

	for (KisImageSPLstIterator it = m_images.begin(); it != m_images.end(); it++) {
		kdDebug() << "(*it) -> name() == " << (*it) -> name() << endl;
		lst.append((*it) -> name());
	}

	return lst;
}

/*
    isEmpty - has no image.  This is not an error condition but
    still need to be check occasionally for operations which
    require an image
*/

bool KisDoc::isEmpty() const
{
	return !isModified();
}

/*
    currentView - pointer to currentImg view for this doc
*/

KisView* KisDoc::currentView()
{
	const QPtrList<KoView>& v = views();

	return v.count() ? dynamic_cast<KisView*>(v.getFirst()) : m_currentView;
}

const KisView* KisDoc::currentView() const
{
	const QPtrList<KoView>& v = views();

	return v.count() ? dynamic_cast<KisView*>(v.getFirst()) : m_currentView;
}

/*
    currentImgName - name of currentImg image
*/

QString KisDoc::currentImgName()
{
	if (m_currentImg)
		return m_currentImg -> name();

	return QString("");
}


/*
    currentImg - pointer to currentImg image
*/
KisImageSP KisDoc::currentImg() const
{
	return m_currentImg;
}

/*
    Save currentImg document (image) in a standard image format
    Note that only the currentImg visible layer(s) will be saved
    usually one needs to merge all layers first, as with Gimp

    The format the image is saved in is determined solely by
    the file extension used. (Although the name of the original
    file, if any, should be the default file name and its extension
    the default extension.  An imagi information dialog and
    extended file save dialog giving the user more choice about
    the exact save format to use is needed, eventually).

    To insure 32 bit images being exported, before the converstion to
    a specific file format, we get the data directly from the layer
    channels, and put it into a QImage, without using the QPimap paint
    device.  Therefore the user's hardware has no effect on the depth
    of the save file.

    There also need to be another method just to save the currentImg view
    as an image, mostly for debugging and documentation purposes, but it
    will only save at the display depth of the hardware -  often 16 bit,
    because it gets a QPixmap from the canvas instead of a QImage from
    the layer(s).

*/

bool KisDoc::saveAsQtImage(const QString& file, bool wholeImage)
{
	int x, y, w, h;
	KisImageSP img = currentImg();

	if (!img) {
		kdDebug() << "No Image\n";
		return false;
	}

	if (wholeImage) {
		x = 0;
		y = 0;
		w = img -> width();
		h = img -> height();
	}
	else {
		KisLayerSP lay = img -> getCurrentLayer();

		if (!lay) {
			kdDebug() << "No layer\n";
			return false;
		}

		const QRect& rc = lay -> imageExtents();

		x = rc.left();
		y = rc.top();
		w = rc.width();
		h = rc.height();
	}

	QImage qimg(w, h, 32);

	qimg.setAlphaBuffer(currentImg() -> colorMode() == cm_RGBA);
	QRect saveRect = QRect(x, y, w, h);
	LayerToQtImage(&qimg, saveRect);
	return qimg.save(file, KImageIO::type(file).ascii());
}

bool KisDoc::MagickImageToLayer(const Magick::Image& mimg, KisView * /*view*/)
{
	KisImageSP img = currentImg();
	QImage cI;
	KisLayerSP lay = img -> getCurrentLayer();
	QRect clipRect(0, 0, mimg.columns(), mimg.rows());

	if (!clipRect.intersects(lay -> imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay -> imageExtents());

	int sx = clipRect.left();
	int sy = clipRect.top();
	int ex = clipRect.right();
	int ey = clipRect.bottom();
	KisPixelPacket *dst = lay -> getPixels(sx, sy, ex, ey);
	const Magick::PixelPacket *src = mimg.getConstPixels(sx, sy, ex, ey);

	memcpy(dst, src, ey * ex * sizeof(KisPixelPacket));
	lay -> syncPixels(dst);
	return true;
}

/*
    Copy a QImage exactly into the currentImg image's active layer.
*/

bool KisDoc::QtImageToLayer(QImage *qimg, KisView * /*pView*/)
{
	KisImageSP img = currentImg();
	QImage cI;

	if (!img)
		return false;

	KisLayerSP lay = img -> getCurrentLayer();

	if (!lay)
		return false;

	qimg -> setAlphaBuffer(true);

	int startx = 0;
	int starty = 0;

	QRect clipRect(startx, starty, qimg -> width(), qimg -> height());

	if (!clipRect.intersects(lay -> imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay -> imageExtents());

	int sx = clipRect.left() - startx;
	int sy = clipRect.top() - starty;
	int ex = clipRect.right() - startx;
	int ey = clipRect.bottom() - starty;
	KisPixelPacket *region = lay -> getPixels(sx, sy, ex, ey);
	QRgb rgba;

	for (int y = sy; y <= ey; y++)
		for (int x = sx; x <= ex; x++) {
			rgba = qimg -> pixel(x, y);
			*(region + (y - sy) * ex + (x - sx)) = rgba;
		}

	lay -> syncPixels(region);
	return true;
}


/*
    Copy a rectangular area of a layer into a QImage, pixel by pixel
    using scanlines, fully 32 bit even if the alpha channel isn't used.
    This provides a basis for a clipboard buffer and a Krayon blit
    routine, with custom modifiers to blend, apply various filters and
    raster operations, and many other neat effects.
*/

bool KisDoc::LayerToQtImage(QImage *qimg, const QRect& clipRect)
{
#if 0
	QRect clip;

	KisImageSP img = currentImg();

	if (!img)
		return false;

	KisLayerSP lay = img -> getCurrentLayer();

	if (!lay)
		return false;

	// this may not always be zero, but for currentImg
	// uses it will be, as the entire layer is copied
	// from its offset into the image
	int startx = 0;
	int starty = 0;

	// this insures getting the layer from its offset
	// into the image, which may not be zero if the
	// layer has been moved
	if (!clipRect.intersects(lay -> imageExtents()))
		return false;

	clip = clipRect.intersect(lay -> imageExtents());

	int sx = clip.left();
	int sy = clip.top();
	int ex = clip.right();
	int ey = clip.bottom();
	uchar *sl;
//	uchar r, g, b;
//	uchar a = 255;
//	uint rgba;

//	bool alpha = (img->colorMode() == cm_RGBA);

	for (int y = sy; y <= ey; y++) {
		sl = qimg -> scanLine(y);

		for (int x = sx; x <= ex; x++) {
			QRgb *p = (QRgb*)qimg -> scanLine(y) + x;
			uchar color[MAX_CHANNELS];

			color[PIXEL_RED] = qRed(*p);
			color[PIXEL_GREEN] = qGreen(*p);
			color[PIXEL_BLUE] = qBlue(*p);
			color[PIXEL_ALPHA] = qAlpha(*p);

			lay -> setPixel(startx + x, starty + y, color);
		}
	}

	return true;
#endif
}

/*
    setSelection - set selection for document
*/
void KisDoc::setSelection(const QRect& r)
{
	Q_ASSERT(m_pSelection);
	m_pSelection -> setBounds(r);
}

/*
    clearSelection - clear selection for document
*/
void KisDoc::clearSelection()
{
	m_pSelection -> setNull();
}


/*
   hasSelection - does this document have a document-wide selection?
*/
bool KisDoc::hasSelection()
{
	return !m_pSelection -> getRectangle().isNull();
}

/*
    removeClipImage - delete the currentImg clip image and nullify it
*/
void KisDoc::removeClipImage()
{
	delete m_pClipImage;
	m_pClipImage = 0L;
}

/*
    setClipImage - set currentImg clip image for the document
    from the selection
*/
bool KisDoc::setClipImage()
{
    KisImageSP img = currentImg();
    if(!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if(!lay) return false;

    if(m_pClipImage != 0L)
    {
        delete m_pClipImage;
        m_pClipImage = 0L;
    }

    QRect selectRect = getSelection()->getRectangle();

    // create a clip image same size, depth, etc., as selection image
    m_pClipImage = new QImage(selectRect.width(), selectRect.height(), 32);
    if(!m_pClipImage) return false;

    // we will need alpha channel for masking the unselected pixels out
    m_pClipImage->setAlphaBuffer(true);

    // make a deep copy of the selection image, if there is one
    if(getSelection()->getImage().isNull())
        return false;
    else
        *m_pClipImage = getSelection()->getImage();

    return true;
}

KisImageSP KisDoc::newImage(const QString& n, int width, int height, cMode cm, uchar bitDepth)
{
	KisImageSP img = new KisImage(this, n, width, height, cm, bitDepth);

	m_images.push_back(img);

	if (m_undo)
		addCommand(new KisCommandImageAdd(this, img));

	return img;
}

void KisDoc::addImage(KisImageSP img)
{
	m_images.push_back(img);
	img -> markDirty(QRect(0, 0, img -> width(), img -> height()));
	setCurrentImage(img);
}

void KisDoc::removeImage(KisImageSP img)
{
	KisImageSPLstIterator it;

	for (it = m_images.begin(); it != m_images.end(); it++)
		if (img == (*it)) {
			m_images.erase(it);
			break;
		}


	if (m_images.empty()) {
		unsetCurrentImage();
		emit imageListUpdated();
		emit layersUpdated();
		emit docUpdated();
	}
	else
		setCurrentImage(*m_images.begin());

	if (m_undo)
		addCommand(new KisCommandImageRm(this, img));
}

void KisDoc::slotRemoveImage(const QString& name)
{
	for (KisImageSPLstIterator it = m_images.begin(); it != m_images.end(); it++)
		if ((*it) -> name() == name) {
			removeImage(*it);
			return;
		}
}

/*
    slotnewImage - Create a new image for this document and set the
    currentImg image to it. There can be more than one image for each doc
*/

bool KisDoc::slotNewImage()
{
	NewDialog dlg;
	KisImageSP img;

	/* This dialog causes bad drawable or invalid window paramater.
	   It seems harmless, though, just a message about an Xerror.
	   Error only occurs when document is first created and has no
	   content, not when adding new image to an existing document */

	dlg.exec();

	if (!dlg.result() == QDialog::Accepted)
		return false;

	int w = dlg.newwidth();
	int h = dlg.newheight();
	bgMode bg = dlg.backgroundMode();
	cMode cm = dlg.colorMode();

	kdDebug() << "KisDoc::slotNewImage: w: "<< w << "h: " << h << endl;

	int n = m_images.size() + 1;
	QString	name = i18n("image %1").arg(n);

	if (!(img = newImage(name, w, h, cm, 8)))
		return false;

	// XXX background color
	if (bg == bm_White)
		img -> addLayer(QRect(0, 0, w, h), KoColor::white(), false, i18n("background"));
	else if (bg == bm_Transparent)
		img -> addLayer(QRect(0, 0, w, h), KoColor::white(), true, i18n("background"));
	else if (bg == bm_ForegroundColor)
		img -> addLayer(QRect(0, 0, w, h), KoColor::white(), false, i18n("background"));
	else if (bg == bm_BackgroundColor)
		img -> addLayer(QRect(0, 0, w, h), KoColor::white(), false, i18n("background"));

	img -> markDirty(QRect(0, 0, w, h));
	setCurrentImage(img);
	emit layersUpdated();
	return true;
}

/*
    Mime type for this app - not same as file type, but file types
    can be associated with a mime type and are opened with applications
    associated with the same mime type
*/

QCString KisDoc::mimeType() const
{
	return "application/x-krita";
}


/*
    Create view instance for this document - there can be more than
    one view of a document open at any time - a list of the views for
    this document can be obtained somehow - perhaps from the factory
    or some kind of view manager - needed
*/

KoView* KisDoc::createViewInstance(QWidget* parent, const char *name)
{
	KisView *view;

	if ((view = currentView()) && !view -> parentWidget()) {
		view -> reparent(parent, QPoint(0, 0));
		view -> setName(name);
	}
	else {
		view = m_currentView = new KisView(this, parent, name);
		view -> setupTools();
	}

	return view;
}

/*
    Draw currentImg image on canvas - indirect approach
*/

void KisDoc::paintContent( QPainter& painter,
        const QRect& rect, bool /*transparent*/, double /*zoomX*/, double /*zoomY*/ )
{
    // TODO support zooming
    if (m_currentImg)
    {
        m_currentImg->paintPixmap( &painter, rect );
    }
    else
    {
        kdDebug(0) <<  "KisDoc::paintContent() - no m_currentImg" << endl;
    }
}

/*
    Draw currentImg image on canvas - direct apprach
*/

void KisDoc::paintPixmap(QPainter *p, QRect area)
{
    if (m_currentImg)
    {
        m_currentImg->paintPixmap(p, area);
    }
    else
    {
        kdDebug(0) <<  "KisDoc::paintPixmap() - no m_currentImg" << endl;
    }
}

/*
    let document update view when image is changed
*/

void KisDoc::slotImageUpdated()
{
	emit docUpdated();
}

/*
    let document update specific area of view when image is changed
*/

void KisDoc::slotImageUpdated(const QRect& rect)
{
	emit docUpdated(rect);
}

void KisDoc::slotLayersUpdated()
{
	emit layersUpdated();
}

QRect KisDoc::getImageRect()
{
	return QRect(0, 0, m_currentImg -> width(), m_currentImg -> height());
}

void KisDoc::setImage(const QString& imageName)
{
	for (KisImageSPLstIterator it = m_images.begin(); it != m_images.end(); it++) {
		if ((*it) -> name() == imageName) {
			setCurrentImage((*it));
			return;
		}
	}
}

ktvector KisDoc::getTools() const
{
	return m_tools;
}

void KisDoc::setTools(const ktvector& tools)
{
	Q_ASSERT(tools.size());

	m_tools.erase(m_tools.begin(), m_tools.end());
	m_tools.reserve(tools.size());

	for (ktvector_size_type i = 0; i < tools.size(); i++)
		m_tools.push_back(tools[i]);
}

void KisDoc::addCommand(KCommand *cmd)
{
	Q_ASSERT(cmd);
	m_cmdHistory -> addCommand(cmd, false);
}

int KisDoc::undoLimit() const
{
	return m_cmdHistory -> undoLimit();
}

void KisDoc::setUndoLimit(int limit)
{
	m_cmdHistory -> setUndoLimit(limit);
}

int KisDoc::redoLimit() const
{
	return m_cmdHistory -> redoLimit();
}

void KisDoc::setRedoLimit(int limit)
{
	m_cmdHistory -> setRedoLimit(limit);
}

void KisDoc::slotDocumentRestored()
{
	setModified(false);
}

void KisDoc::slotCommandExecuted()
{
	setModified(true);
}

void KisDoc::setCanvasCursor(const QCursor& cursor)
{
	QPtrList<KoView> l = views();

	for (KoView *view = l.first(); view; view = l.next()) {
		KisView *p = dynamic_cast<KisView*>(view);

		if (p)
			p -> setCanvasCursor(cursor);
	}
}

QString KisDoc::nextImageName() const
{
	return i18n("image %1").arg(m_images.size() + 1);
}

#include "kis_doc.moc"

