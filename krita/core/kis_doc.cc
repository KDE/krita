/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2000 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
 *  Copyright (c) 2002, 2003 Patrick Julien <freak@codepimps.org>
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

// Qt
#include <qapplication.h>
#include <qdom.h>
#include <qimage.h>
#include <qpainter.h>
#include <qtl.h>
#include <qstringlist.h>
#include <qwidget.h>
#include <qpaintdevicemetrics.h>

// KDE
#include <dcopobject.h>
#include <kapplication.h>
#include <kcommand.h>
#include <kdebug.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <knotifyclient.h>
#include <klocale.h>

// KOffice
#include <koFilterManager.h>
#include <koMainWindow.h>
#include <koQueryTrader.h>
#include <koStore.h>
#include <koStoreDevice.h>
#include <koTemplateChooseDia.h>
#include <koApplication.h>
#include <kocommandhistory.h>

// Local
#include "kis_types.h"
#include "kis_config.h"
#include "kis_global.h"
#include "kis_dlg_create_img.h"
#include "kis_doc.h"
#include "kis_factory.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_nameserver.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_command.h"
#include "kis_view.h"
#include "kis_strategy_colorspace.h"
#include "kis_colorspace_registry.h"
#include "kis_profile.h"
#include "kis_resource.h"
#include "kis_resourceserver.h"
#include "kis_id.h"
#include "KIsDocIface.h"

static const char *CURRENT_DTD_VERSION = "1.3";

namespace {
	class KisCommandImageMv : public KisCommand {
		typedef KisCommand super;

	public:
		KisCommandImageMv(KisDoc *doc,
				  KisUndoAdapter *adapter,
				  const QString& name,
				  const QString& oldName) : super(i18n("Rename Image"), adapter)
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
				adapter() -> setUndo(false);
				m_doc -> renameImage(m_oldName, m_name);
				adapter() -> setUndo(true);
			}

		virtual void unexecute()
			{
				adapter() -> setUndo(false);
				m_doc -> renameImage(m_name, m_oldName);
				adapter() -> setUndo(true);
			}

	private:
		KisDoc *m_doc;
		QString m_name;
		QString m_oldName;
	};

	class LayerAddCmd : public KisCommand {
		typedef KisCommand super;

	public:
		LayerAddCmd(KisDoc *doc,
			    KisUndoAdapter *adapter,
			    KisImageSP img,
			    KisLayerSP layer) : super(i18n("Add Layer"), adapter)
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
				adapter() -> setUndo(false);
				m_doc -> layerAdd(m_img, m_layer, m_index);
				adapter() -> setUndo(true);
			}

		virtual void unexecute()
			{
				adapter() -> setUndo(false);
				m_doc -> layerRemove(m_img, m_layer);
				adapter() -> setUndo(true);
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
		LayerRmCmd(KisDoc *doc,
			   KisUndoAdapter *adapter,
			   KisImageSP img,
			   KisLayerSP layer) : super(i18n("Remove Layer"))
			{
				m_doc = doc;
				m_adapter = adapter;
				m_img = img;
				m_layer = layer;
				m_index = img -> index(layer);
			}

		virtual ~LayerRmCmd()
			{
			}

		virtual void execute()
			{
				m_adapter -> setUndo(false);
				m_doc -> layerRemove(m_img, m_layer);
				m_adapter -> setUndo(true);
			}

		virtual void unexecute()
			{
				m_adapter -> setUndo(false);
				m_doc -> layerAdd(m_img, m_layer, m_index);
				m_adapter -> setUndo(true);
			}

	private:
		KisDoc *m_doc;
		KisUndoAdapter *m_adapter;
		KisImageSP m_img;
		KisLayerSP m_layer;
		Q_INT32 m_index;
	};

	class LayerPropsCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		LayerPropsCmd(KisLayerSP layer,
			      KisImageSP img,
			      KisDoc *doc,
			      KisUndoAdapter *adapter,
			      const QString& name,
			      Q_INT32 opacity,
			      const KisCompositeOp& compositeOp) : super(i18n("Layer Property Changes"))
			{
				m_layer = layer;
				m_img = img;
				m_doc = doc;
				m_adapter = adapter;
				m_name = name;
				m_opacity = opacity;
				m_compositeOp = compositeOp;
			}

		virtual ~LayerPropsCmd()
			{
			}

	public:
		virtual void execute()
			{
				QString name = m_layer -> name();
				Q_INT32 opacity = m_layer -> opacity();
				KisCompositeOp compositeOp = m_layer -> compositeOp();

				m_adapter -> setUndo(false);
				m_doc -> setLayerProperties(m_img,
							    m_layer,
							    m_opacity,
							    m_compositeOp,
							    m_name);
				m_adapter -> setUndo(true);
				m_name = name;
				m_opacity = opacity;
				m_compositeOp = compositeOp;
				m_img -> notify();
			}

		virtual void unexecute()
			{
				execute();
			}

	private:
		KisUndoAdapter *m_adapter;
		KisLayerSP m_layer;
		KisImageSP m_img;
		KisDoc *m_doc;
		QString m_name;
		Q_INT32 m_opacity;
		KisCompositeOp m_compositeOp;
	};
}

KisDoc::KisDoc(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, bool singleViewMode) :
	super(parentWidget, widgetName, parent, name, singleViewMode)
{

	m_undo = false;
	m_dcop = 0;
	m_cmdHistory = 0;
	m_nserver = 0;
	m_currentMacro = 0;
	m_macroNestDepth = 0;
	m_ioProgressBase = 0;
	m_ioProgressTotalSteps = 0;

	setInstance( KisFactory::global(), false );

	if (name)
		dcopObject();

}

KisDoc::~KisDoc()
{
	// XXX: Now gives a crash, see CRASHES file.
	//delete m_cmdHistory;
        delete m_dcop;
	delete m_nserver;
}

QCString KisDoc::mimeType() const
{
	return APP_MIMETYPE;
}

DCOPObject *KisDoc::dcopObject()
{
	if (!m_dcop) {
		m_dcop = new KIsDocIface(this);
		Q_CHECK_PTR(m_dcop);
	}
	return m_dcop;
}

bool KisDoc::initDoc(InitDocFlags flags, QWidget* parentWidget)
{
	if (!init())
		return false;

	bool ok = false;

        if (flags==KoDocument::InitDocEmpty)
        {
                if ((ok = slotNewImage()))
                        emit imageListUpdated();
                setModified(false);
                setUndo(true);
                return ok;
        }

        QString file;
	KoTemplateChooseDia::DialogType dlgtype;

 	if (flags != KoDocument::InitDocFileNew) {
		dlgtype = KoTemplateChooseDia::Everything;
	} else {
 		dlgtype = KoTemplateChooseDia::OnlyTemplates;
	}

	KoTemplateChooseDia::ReturnType ret =
		KoTemplateChooseDia::choose(KisFactory::global(),
					    file,
					    dlgtype,
					    "krita_template",
					    parentWidget);
	setUndo(false);

	if (ret == KoTemplateChooseDia::Template) {

		resetURL();

		ok = loadNativeFormat( file );

		emit imageListUpdated();

		KoDocument::setEmpty();
		ok = true;
	} else if (ret == KoTemplateChooseDia::File) {
		KURL url( file );
		ok = openURL(url);
	} else if (ret == KoTemplateChooseDia::Empty) {
		if ((ok = slotNewImage())) {
			emit imageListUpdated();
			KoDocument::setEmpty();
		}
	}

	setModified(false);
	setUndo(true);

	return ok;
}

bool KisDoc::init()
{
	if (m_cmdHistory) {
		delete m_cmdHistory;
		m_cmdHistory = 0;
	}

	if (m_nserver) {
		delete m_nserver;
		m_nserver = 0;
	}

	m_cmdHistory = new KoCommandHistory(actionCollection(), true);
	Q_CHECK_PTR(m_cmdHistory);

	connect(m_cmdHistory, SIGNAL(documentRestored()), this, SLOT(slotDocumentRestored()));
	connect(m_cmdHistory, SIGNAL(commandExecuted()), this, SLOT(slotCommandExecuted()));
	m_undo = true;

	m_nserver = new KisNameServer(i18n("Image %1"), 1);
	Q_CHECK_PTR(m_nserver);

	return true;
}

QDomDocument KisDoc::saveXML()
{
	QDomDocument doc = createDomDocument("DOC", CURRENT_DTD_VERSION);
	QDomElement root = doc.documentElement();

	root.setAttribute("editor", "Krita");
	root.setAttribute("depth", sizeof(QUANTUM));
	root.setAttribute("syntaxVersion", "1");

	root.appendChild(saveImage(doc, m_currentImage));

	return doc;
}

bool KisDoc::loadOasis( const QDomDocument&, KoOasisStyles&, const QDomDocument&, KoStore* )
{
	//XXX: todo (and that includes defining an OASIS format for layered 2D raster data!)
	return false;
}


bool KisDoc::saveOasis( KoStore*, KoXmlWriter* )
{
	//XXX: todo (and that includes defining an OASIS format for layered 2D raster data!)
	return false;
}

bool KisDoc::loadXML(QIODevice *, const QDomDocument& doc)
{
	QDomElement root;
	QString attr;
	QDomNode node;
	KisImageSP img;

	if (!init())
		return false;
	if (doc.doctype().name() != "DOC")
		return false;
	root = doc.documentElement();
	attr = root.attribute("syntaxVersion");
 	if (attr.toInt() > 1)
 		return false;
	if ((attr = root.attribute("depth")).isNull())
		return false;
	m_conversionDepth = attr.toInt();

	if (!root.hasChildNodes()) {
		// No children == empty file == show create dialog
		return slotNewImage();
	}
	
	for (node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
		kdDebug() << "Node: " << node.nodeName() << ", element: " << node.isElement() << "\n";
		if (node.isElement()) {
			if (node.nodeName() == "IMAGE") {
				QDomElement elem = node.toElement();
				if (!(img = loadImage(elem)))
					return false;
				m_currentImage = img;
			} else {
				return false;
			}
		}
	}

	return true;
}

QDomElement KisDoc::saveImage(QDomDocument& doc, KisImageSP img)
{
	QDomElement image = doc.createElement("IMAGE");
	vKisLayerSP layers;

	Q_ASSERT(img);
	image.setAttribute("name", img -> name());
	image.setAttribute("mime", "application/x-kra");
	image.setAttribute("width", img -> width());
	image.setAttribute("height", img -> height());
	image.setAttribute("colorspacename", img -> colorStrategy() -> id().id());
	image.setAttribute("description", img -> description());
	// XXX: Save profile as blob inside the image, instead of the product name.
	if (img -> profile() && img -> profile()-> valid())
		image.setAttribute("profile", img -> profile() -> productName());
	image.setAttribute("x-res", img -> xRes());
	image.setAttribute("y-res", img -> yRes());

	layers = img -> layers();

	if (layers.size() > 0) {
		QDomElement elem = doc.createElement("LAYERS");

		image.appendChild(elem);

		for (vKisLayerSP_it it = layers.begin(); it != layers.end(); it++)
			elem.appendChild(saveLayer(doc, *it));
	}


	// TODO Image colormap if any
	return image;
}

KisImageSP KisDoc::loadImage(const QDomElement& element)
{
	
	KisConfig cfg;
	QString attr;
	QDomNode node;
	QDomNode child;
	KisImageSP img;
	QString name;
	Q_INT32 width;
	Q_INT32 height;
	QString description;
	QString profileProductName;
	double xres;
	double yres;
	QString colorspacename;
	KisProfileSP profile;
	

	if ((attr = element.attribute("mime")) == NATIVE_MIMETYPE) {
		if ((name = element.attribute("name")).isNull())
			return 0;
		if ((attr = element.attribute("width")).isNull())
			return 0;
		if ((width = attr.toInt()) < 0 || width > cfg.maxImgWidth())
			return 0;
		if ((attr = element.attribute("height")).isNull())
			return 0;
		if ((height = attr.toInt()) < 0 || height > cfg.maxImgHeight())
			return 0;
		description = element.attribute("description");
		
		if ((attr = element.attribute("x-res")).isNull())
			xres = 100.0;
		else if ((xres = attr.toInt()) < 0 || xres > cfg.maxImgHeight())
			xres = 100.0;

		if ((attr = element.attribute("y-res")).isNull())
			yres = 100.0;
		else if ((yres = attr.toInt()) < 0 || yres > cfg.maxImgHeight())
			yres = 100.0;

		if ((colorspacename = element.attribute("colorspacename")).isNull())
		{
			// And old file: take a reasonable default. 
			// Krita didn't support anything else in those
			// days anyway.
			colorspacename = "RGBA";
		}
		
		KisStrategyColorSpaceSP cs = KisColorSpaceRegistry::instance() -> get(colorspacename);
		if (cs == 0) {
			// return 0;
			if (colorspacename  == "Grayscale + Alpha")
				cs = KisColorSpaceRegistry::instance() -> get("GRAYA");
			else 
				cs = KisColorSpaceRegistry::instance() -> get("RGBA");
		}

		if ((profileProductName = element.attribute("profile")).isNull()) {
			profile = 0;
		}
		else {
			profile = cs -> getProfileByName(profileProductName);
		}

		img = new KisImage(this, width, height, cs, name);
		Q_CHECK_PTR(img);

		img -> setDescription(description);
		img -> setResolution(xres, yres);
		img -> setProfile(profile);


		for (node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
			if (node.isElement()) {
				if (node.nodeName() == "LAYERS") {
					for (child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
						KisLayerSP layer = loadLayer(child.toElement(), img);

						if (!layer) {
							kdDebug(DBG_AREA_FILE) << "Could not load layer\n";
							return 0;
						}
						img -> add(layer, -1);
					}

					if (img -> nlayers()) {
						img -> activateLayer(0);
					}
				} else if (node.nodeName() == "COLORMAP") {
					// TODO
				}
			}
		}
	} else {
		// TODO Try to import it
	}

	return img;
}

QDomElement KisDoc::saveLayer(QDomDocument& doc, KisLayerSP layer)
{
	QDomElement layerElement = doc.createElement("layer");

	layerElement.setAttribute("name", layer -> name());
	layerElement.setAttribute("x", layer->getX());
	layerElement.setAttribute("y", layer-> getY());
	layerElement.setAttribute("opacity", layer -> opacity());
	layerElement.setAttribute("compositeop", layer -> compositeOp().id().id());
	layerElement.setAttribute("visible", layer -> visible());
	layerElement.setAttribute("linked", layer -> linked());
	layerElement.setAttribute("locked", layer -> locked());
	layerElement.setAttribute("colorspacename", layer -> colorStrategy() -> id().id());
	// XXX: Save profile as blob inside the layer, instead of the product name.
	if (layer -> profile() && layer -> profile() -> valid()) {
		layerElement.setAttribute("profile", layer -> profile() -> productName());
	}

	return layerElement;
}

KisLayerSP KisDoc::loadLayer(const QDomElement& element, KisImageSP img)
{
	// Nota bene: If you add new properties to layers, you should
	// ALWAYS define a default value in case the property is not
	// present in the layer definition: this helps a LOT with backward
	// compatibilty.
	kdDebug(DBG_AREA_FILE) << "loadLayer called\n";
	
	KisConfig cfg;
	QString attr;
	QDomNode node;
	QDomNode child;
	QString name;
	Q_INT32 x;
	Q_INT32 y;
	Q_INT32 opacity;
	bool visible;
	bool linked;
	bool locked;
	KisLayerSP layer;

	if ((name = element.attribute("name")).isNull())
		return 0;
	kdDebug(DBG_AREA_FILE) << "Loading layer " << name << "\n";
	
	if ((attr = element.attribute("x")).isNull())
		return 0;
	x = attr.toInt();
	kdDebug(DBG_AREA_FILE) << "X: " << x << "\n";
	
	if ((attr = element.attribute("y")).isNull())
		return 0;

	y = attr.toInt();
	kdDebug(DBG_AREA_FILE) << "Y: " << y << "\n";

	if ((attr = element.attribute("opacity")).isNull())
		return 0;

	if ((opacity = attr.toInt()) < 0 || opacity > QUANTUM_MAX)
		opacity = OPACITY_OPAQUE;

	kdDebug(DBG_AREA_FILE) << "Opacity: " << opacity << "\n";
		
	QString compositeOpName = element.attribute("compositeop");
	KisCompositeOp compositeOp;

	if (compositeOpName.isNull()) {
		compositeOp = COMPOSITE_OVER;
	} else {
		compositeOp = KisCompositeOp(compositeOpName);
	}

	if (!compositeOp.isValid()) {
		return 0;
	}
	kdDebug(DBG_AREA_FILE) << "CompositeOp: " << compositeOpName << "\n";
	
	if ((attr = element.attribute("visible")).isNull())
		attr = "1";

	visible = attr == "0" ? false : true;
	kdDebug(DBG_AREA_FILE) << "Visible: " << visible << "\n";
	
	if ((attr = element.attribute("linked")).isNull())
		attr = "0";

	linked = attr == "0" ? false : true;
	kdDebug(DBG_AREA_FILE) << "Linked: " << linked << "\n";
	
	if ((attr = element.attribute("locked")).isNull())
		attr = "0";

	locked = attr == "0" ? false : true;
	kdDebug(DBG_AREA_FILE) << "Locked: " << locked<< "\n";
	
	QString colorspacename = element.attribute("colorspacename");
	KisStrategyColorSpaceSP colorSpace = img -> colorStrategy();
	
	kdDebug() << "Colorspace name in layer: " << colorspacename << "\n";
	if (!colorspacename.isNull()) {
		colorSpace = KisColorSpaceRegistry::instance() -> get(KisID(colorspacename, ""));
	}
	kdDebug(DBG_AREA_FILE) << "Colorspace: " << colorspacename << "\n";
	
	if (colorSpace == 0) {
		kdDebug() << "Could not get colorspace: aborting\n";
		return 0;
	}

	QString profileProductName = element.attribute("profile");
	KisProfileSP profile = 0;

	if (!profileProductName.isNull()) {
		profile = colorSpace -> getProfileByName(profileProductName);
	}

	layer = new KisLayer(img, name, opacity, colorSpace);
	Q_CHECK_PTR(layer);

	layer -> setCompositeOp(compositeOp);
	layer -> setProfile(profile);
	layer -> setLinked(linked);
	layer -> setVisible(visible);
	layer -> move(x, y);
	return layer;
}


bool KisDoc::completeSaving(KoStore *store)
{
	QString uri = url().url();
	QString location;
	bool external = isStoredExtern();
	Q_INT32 totalSteps = 0;
	KisImageSP img;

	if (!m_currentImage) return false;

	totalSteps = (m_currentImage) -> nlayers();

	img = new KisImage(*m_currentImage);
	Q_CHECK_PTR(img);

	img -> setName((m_currentImage) -> name());

	setIOSteps(totalSteps + 1);

	vKisLayerSP layers = (img) -> layers();

	for (vKisLayerSP_it it2 = layers.begin(); it2 != layers.end(); it2++) {
		connect(*it2, SIGNAL(ioProgress(Q_INT8)), this, SLOT(slotIOProgress(Q_INT8)));
		location = external ? QString::null : uri;
		location += (img) -> name() + "/layers/" + (*it2) -> name();

		// Layer data
		if (store -> open(location)) {
			if (!(*it2) -> write(store)) {
				(*it2) -> disconnect();
				store -> close();
				IODone();
				return false;
			}

			store -> close();
		}
			
		if ((*it2) -> profile()) {
			// save layer profile
			location = external ? QString::null : uri;
			location += (img) -> name() + "/layers/" + (*it2) -> name() + ".icc";
				
			if (store -> open(location)) {
				store -> write((*it2) -> profile() -> annotation() -> annotation());
				store -> close();
			}
		}

		IOCompletedStep();
		(*it2) -> disconnect();
	}
		
	// saving annotations
	// XXX this only saves EXIF and ICC info. This would probably need
	// a redesign of the dtd of the krita file to do this more generally correct
	// e.g. have <ANNOTATION> tags or so.
	KisAnnotationSP annotation = (img) -> annotation("exif");
	if (annotation) {
		location = external ? QString::null : uri;
		location += (img) -> name() + "/annotations/exif";
		if (store -> open(location)) {
			store -> write(annotation -> annotation());
			store -> close();
		}
	}
	if ((img) -> profile()) {
		location = external ? QString::null : uri;
		location += (img) -> name() + "/annotations/icc";
		if (store -> open(location)) {
			store -> write((img) -> profile() -> annotation() -> annotation());
			store -> close();
		}
	}
#if 0 // Composite rendition of the entire image for easier kimgio loading and to speed up loading the image into Krita: show the composite png first, then load the layers
	if (store -> open("composite.png")) {

		QPixmap * pix = new QPixmap(m_currentImage -> width(), m_currentImage -> height());
		QPainter gc(pix);
		m_currentImage -> renderToPainter(0, 0, m_currentImage -> width(), m_currentImage -> height(), gc, m_currentImage -> profile());
		gc.end();
		QImage composite = pix -> convertToImage();

		KoStoreDevice io (store);

		if (!composite.save(&io, "PNG")) {
			store -> close();
			IODone();
			return false;
		}

		delete pix;

		IOCompletedStep();
		store -> close();
	}
#endif
	IODone();
	return true;
}

bool KisDoc::completeLoading(KoStore *store)
{
	QString uri = url().url();
	QString location;
	bool external = isStoredExtern();
	Q_INT32 totalSteps = 0;

	totalSteps = (m_currentImage) -> nlayers();

	setIOSteps(totalSteps);

	vKisLayerSP layers = (m_currentImage) -> layers();

	for (vKisLayerSP_it it2 = layers.begin(); it2 != layers.end(); it2++) {
		connect(*it2, SIGNAL(ioProgress(Q_INT8)), this, SLOT(slotIOProgress(Q_INT8)));
		location = external ? QString::null : uri;
		location += (m_currentImage) -> name() + "/layers/" + (*it2) -> name();

		// Layer data
		if (store -> open(location)) {
			if (!(*it2) -> read(store)) {
				(*it2) -> disconnect();
				store -> close();
				IODone();
				return false;
			}

			store -> close();
		}
			
		// icc profile
		location = external ? QString::null : uri;
		location += (m_currentImage) -> name() + "/layers/" + (*it2) -> name() + ".icc";
		if (store -> hasFile(location)) {
			QByteArray data;
			store -> open(location);
			data = store -> read(store -> size());
			store -> close();
			(*it2) -> setProfile(new KisProfile(data,
				(*it2) -> colorStrategy() -> colorSpaceType()));
			kdDebug(DBG_AREA_FILE) << "Opened icc information, size is " << data.size() << endl;
		}

		IOCompletedStep();
		(*it2) -> disconnect();
	}
		
	// annotations
	// exif
	location = external ? QString::null : uri;
	location += (m_currentImage) -> name() + "/annotations/exif";
	if (store -> hasFile(location)) {
		QByteArray data;
		store -> open(location);
		data = store -> read(store -> size());
		store -> close();
		(m_currentImage) -> addAnnotation(new KisAnnotation("exif", "", data));
		kdDebug(DBG_AREA_FILE) << "Opened exif information, size is " << data.size() << endl;
	}
	// icc profile
	location = external ? QString::null : uri;
	location += (m_currentImage) -> name() + "/annotations/icc";
	if (store -> hasFile(location)) {
		QByteArray data;
		store -> open(location);
		data = store -> read(store -> size());
		store -> close();
		(m_currentImage) -> setProfile(new KisProfile(data,
			(m_currentImage) -> colorStrategy() -> colorSpaceType()));
		kdDebug(DBG_AREA_FILE) << "Opened icc information, size is " << data.size() << endl;
	}

	IODone();
	return true;
}

void KisDoc::renameImage(const QString& oldName, const QString& newName)
{
	(m_currentImage) -> setName(newName);

	if (m_undo)
		addCommand(new KisCommandImageMv(this, this, newName, oldName));

	emit imageListUpdated();
}


KisImageSP KisDoc::newImage(const QString& name, Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorstrategy)
{
	KisImageSP img = new KisImage(this, width, height, colorstrategy, name);
	Q_CHECK_PTR(img);

	KisLayerSP layer = new KisLayer(img, img -> nextLayerName(), OPACITY_OPAQUE);
	Q_CHECK_PTR(layer);

	KisFillPainter painter;

// 	painter.begin(layer.data());
	// XXX: YUse default pixel color, no need to initialize all pixels immediately
	KisColor c(Qt::white);
	layer->dataManager()->setDefaultPixel(c.data());
// 	painter.fillRect(0, 0, width, height, c, OPACITY_OPAQUE);
// 	painter.end();

	img -> add(layer, -1);

	m_currentImage = img;
	return img;
}

bool KisDoc::slotNewImage()
{
	KisConfig cfg;
	KisDlgCreateImg dlg(cfg.maxImgWidth(), cfg.defImgWidth(),
			    cfg.maxImgHeight(), cfg.defImgHeight(),
			    "RGBA",
			    "new image from dlg");
	qApp -> setOverrideCursor(Qt::ArrowCursor);
	if (dlg.exec() == QDialog::Accepted) {
		qApp -> restoreOverrideCursor();
		QString name;
		QUANTUM opacity = dlg.backgroundOpacity();
		QColor c = dlg.backgroundColor();
		KisImageSP img;
		KisLayerSP layer;

		KisStrategyColorSpaceSP cs = KisColorSpaceRegistry::instance()->get(dlg.colorStrategyID());

		if (!cs) return false;

		img = new KisImage(this, dlg.imgWidth(),
				   dlg.imgHeight(),
				   cs,
				   dlg.imgName());
		Q_CHECK_PTR(img);

		img -> setResolution(dlg.imgResolution(), dlg.imgResolution()); // XXX needs to be added to dialog
		img -> setDescription(dlg.imgDescription());

		img -> setProfile(dlg.profile());

		layer = new KisLayer(img, img -> nextLayerName(), OPACITY_OPAQUE);
		Q_CHECK_PTR(layer);

// 		// XXX: Use default color here! No need to fill all pixels immediately
// 		// Default color and opacity: don't allocate memory
// 		if ( c.red() != 0 || c.green() != 0 || c.blue() != 0 || opacity != 0) {
// 			KisFillPainter painter;
// 			painter.begin(layer.data());
// 			painter.fillRect(0, 0, dlg.imgWidth(), dlg.imgHeight(), c, opacity);
// 			painter.end();
// 		}
		KisColor kc(c, opacity, cs, dlg.profile());
		layer->dataManager()->setDefaultPixel(kc.data());
		
		img -> add(layer, -1);

		m_currentImage = img;

		cfg.defImgWidth(dlg.imgWidth());
		cfg.defImgHeight(dlg.imgHeight());

		return true;
	}
	return false;
}

KoView* KisDoc::createViewInstance(QWidget* parent, const char *name)
{
	KisView * v = new KisView(this, this, parent, name);
	Q_CHECK_PTR(v);

	return v;
}

void KisDoc::paintContent(QPainter& painter, const QRect& rect, bool transparent, double zoomX, double zoomY)
{
	KisConfig cfg;
	QString monitorProfileName = cfg.monitorProfile();
	KisProfileSP profile = KisColorSpaceRegistry::instance() -> getProfileByName(monitorProfileName);
	painter.scale(zoomX, zoomY);
	paintContent(painter, rect, profile);
	
}

void KisDoc::paintContent(QPainter& painter, const QRect& rect, KisProfileSP profile)
{
	Q_INT32 x1;
	Q_INT32 y1;
	Q_INT32 x2;
	Q_INT32 y2;

	x1 = CLAMP(rect.x(), 0, m_currentImage -> width());
	y1 = CLAMP(rect.y(), 0, m_currentImage -> height());
	x2 = CLAMP(rect.x() + rect.width(), 0, m_currentImage -> width());
	y2 = CLAMP(rect.y() + rect.height(), 0, m_currentImage -> height());

	m_currentImage -> renderToPainter(x1, y1, x2, y2, painter, profile);
}

void KisDoc::slotImageUpdated()
{
	emit docUpdated();
}

void KisDoc::slotImageUpdated(const QRect& rect)
{
	emit docUpdated(rect);
}

void KisDoc::beginMacro(const QString& macroName)
{
	if (m_undo) {
		if (m_macroNestDepth == 0) {
			Q_ASSERT(m_currentMacro == 0);
			m_currentMacro = new KMacroCommand(macroName);
			Q_CHECK_PTR(m_currentMacro);
		}

		m_macroNestDepth++;
	}
}

void KisDoc::endMacro()
{
	if (m_undo) {
		Q_ASSERT(m_macroNestDepth > 0);

		if (m_macroNestDepth > 0) {
			m_macroNestDepth--;

			if (m_macroNestDepth == 0) {
				Q_ASSERT(m_currentMacro != 0);

				m_cmdHistory -> addCommand(m_currentMacro, false);
				m_currentMacro = 0;
			}
		}
	}
}

void KisDoc::addCommand(KCommand *cmd)
{
	Q_ASSERT(cmd);
	setModified(true);
	if (m_undo) {
		if (m_currentMacro)
			m_currentMacro -> addCommand(cmd);
		else
			m_cmdHistory -> addCommand(cmd, false);
	} else {
		delete cmd;
	}
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
	setModified(true);
}

void KisDoc::slotUpdate(KisImageSP, Q_UINT32 x, Q_UINT32 y, Q_UINT32 w, Q_UINT32 h)
{
	QRect rc(x, y, w, h);

	emit docUpdated(rc);
}

KisLayerSP KisDoc::layerAdd(KisImageSP img, const QString& name, QUANTUM devOpacity)
{
	KisLayerSP layer;

	if (img) {
		layer = new KisLayer(img, name, devOpacity);
		Q_CHECK_PTR(layer);

		// No need to fill the layer with something; it's empty and transparent
		// and doesn't have any pixels by default.
		if (layer && img -> add(layer, -1)) {
			layer = img -> activate(layer);

			if (layer) {
				img -> top(layer);

				if (m_undo)
					addCommand(new LayerAddCmd(this, this, img, layer));
				setModified(true);
				layer -> setVisible(true);
				emit layersUpdated(img);
			}
		}
	}

	return layer;
}

KisLayerSP KisDoc::layerAdd(KisImageSP img,
			    const QString& name,
			    const KisCompositeOp& compositeOp,
			    QUANTUM opacity,
			    KisStrategyColorSpaceSP colorstrategy)
{
	KisLayerSP layer;
	if (img) {
		layer = new KisLayer(colorstrategy, name);
		Q_CHECK_PTR(layer);

		layer -> setOpacity(opacity);
		layer -> setCompositeOp(compositeOp);
		if (layer && img -> add(layer, -1)) {
			layer = img -> activate(layer);

			if (layer) {
				img -> top(layer);

				if (m_undo)
					addCommand(new LayerAddCmd(this, this, img, layer));
				setModified(true);
				layer -> setVisible(true);
				emit layersUpdated(img);
			}
		}
	}

	return layer;
}


KisLayerSP KisDoc::layerAdd(KisImageSP img, KisLayerSP l, Q_INT32 position)
{
	if (!img -> add(l, -1))
		return 0;

	setModified(true);
	img -> pos(l, position);

	if (m_undo)
		addCommand(new LayerAddCmd(this, this, img, l));
	l -> setVisible(true);
	emit layersUpdated(img);

	if (!m_undo)
		emit currentImageUpdated(img);

	return l;
}

void KisDoc::layerRemove(KisImageSP img, KisLayerSP layer)
{
	if (layer) {
		setModified(true);
		img -> rm(layer);

		if (m_undo)
			addCommand(new LayerRmCmd(this, this, img, layer));

		emit layersUpdated(img);

		if (!m_undo)
			emit currentImageUpdated(img);
	}
}

void KisDoc::layerRaise(KisImageSP img, KisLayerSP layer)
{
	if (layer) {
		setModified(true);
		img -> raise(layer);
		emit layersUpdated(img);
	}
}

void KisDoc::layerLower(KisImageSP img, KisLayerSP layer)
{
	if (layer) {
		setModified(true);
		img -> lower(layer);
		emit layersUpdated(img);
	}
}

void KisDoc::layerNext(KisImageSP img, KisLayerSP layer)
{
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
			layer -> setVisible(true);
			setModified(true);
		}

		if (!img -> activate(layer))
			return;

		for (Q_INT32 i = 0; i < npos; i++) {
			layer = img -> layer(i);

			if (layer) {
				layer -> setVisible(false);
			}
		}

		setModified(true);
		emit layersUpdated(img);
	}
}

void KisDoc::layerPrev(KisImageSP img, KisLayerSP layer)
{
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
			layer -> setVisible(true);
			setModified(true);
		}

		if (!img -> activate(layer))
			return;

		for (Q_INT32 i = 0; i < npos; i++) {
			layer = img -> layer(i);

			if (layer) {
				layer -> setVisible(false);
			}
		}

		setModified(true);
		emit layersUpdated(img);
	}
}

void KisDoc::setLayerProperties(KisImageSP img,
				KisLayerSP layer,
				QUANTUM opacity,
				const KisCompositeOp& compositeOp,
				const QString& name)
{
	if (layer) {
		if (m_undo) {
			QString oldname = layer -> name();
			Q_INT32 oldopacity = layer -> opacity();
			KisCompositeOp oldCompositeOp = layer -> compositeOp();
			layer -> setName(name);
			layer -> setOpacity(opacity);
			layer -> setCompositeOp(compositeOp);
			addCommand(new LayerPropsCmd(layer, img, this, this, oldname, oldopacity, oldCompositeOp));
		} else {
			layer -> setName(name);
			layer -> setOpacity(opacity);
			layer -> setCompositeOp(compositeOp);
		}

		setModified(true);
		emit layersUpdated(img);
		emit currentImageUpdated(img);
	}
}

bool KisDoc::undo() const
{
	return m_undo;
}

void KisDoc::setIOSteps(Q_INT32 nsteps)
{
	m_ioProgressTotalSteps = nsteps * 100;
	m_ioProgressBase = 0;
	emitProgress(0);
}

void KisDoc::IOCompletedStep()
{
	m_ioProgressBase += 100;
}

void KisDoc::IODone()
{
	emitProgress(-1);
}

void KisDoc::slotIOProgress(Q_INT8 percentage)
{
	KApplication *app = KApplication::kApplication();

	Q_ASSERT(app);

	if (app -> hasPendingEvents())
		app -> processEvents();

	int totalPercentage = ((m_ioProgressBase + percentage) * 100) / m_ioProgressTotalSteps;

	emitProgress(totalPercentage);
}

void KisDoc::prepareForImport()
{
	if (m_nserver == 0)
		init();

}

void KisDoc::setCurrentImage(KisImageSP image)
{
	m_currentImage = image;
	emit imageListUpdated();
	emit layersUpdated(m_currentImage);
	emit docUpdated();
}

#include "kis_doc.moc"

