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
#include <qclipboard.h>
#include <qdom.h>
#include <qimage.h>
#include <qpainter.h>
#include <qtl.h>
#include <qstringlist.h>
#include <qwidget.h>
#include <qpaintdevicemetrics.h>
#include <qmessagebox.h>

// KDE
#include <dcopobject.h>
#include <kapplication.h>
#include <kcommand.h>
#include <kdebug.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kmessagebox.h>
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
#include "builder/kis_builder_subject.h"
#include "builder/kis_builder_monitor.h"
#include "builder/kis_image_magick_converter.h"
#include "kis_strategy_colorspace.h"
#include "kis_colorspace_registry.h"
#include "tiles/kistilemgr.h"

#include "KIsDocIface.h"

static const char *CURRENT_DTD_VERSION = "1.3";

namespace {
	class KisCommandImageAdd : public KisCommand {
		typedef KisCommand super;

	public:
		KisCommandImageAdd(KisDoc *doc,
				   KisUndoAdapter *adapter,
				   KisImageSP img) : super(i18n("Add Image"), adapter)
			{
				m_doc = doc;
				m_img = img;
			}

		virtual ~KisCommandImageAdd()
			{
			}

		virtual void execute()
			{
				adapter() -> setUndo(false);
				m_doc -> addImage(m_img);
				adapter() -> setUndo(true);
			}

		virtual void unexecute()
			{
				adapter() -> setUndo(false);
				m_doc -> removeImage(m_img);
				adapter() -> setUndo(true);
			}

	private:
		KisDoc *m_doc;
		KisImageSP m_img;
	};

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

	class KisCommandImageRm : public KisCommand {
		typedef KisCommand super;

	public:
		KisCommandImageRm(KisDoc *doc,
				  KisUndoAdapter *adapter,
				  KisImageSP img) : super(i18n("Remove Image"), adapter)
			{
				m_doc = doc;
				m_img = img;
			}

		virtual ~KisCommandImageRm()
			{
			}

		virtual void execute()
			{
				adapter() -> setUndo(false);
				m_doc -> removeImage(m_img);
				adapter() -> setUndo(true);
			}

		virtual void unexecute()
			{
				adapter() -> setUndo(false);
				m_doc -> addImage(m_img);
				adapter() -> setUndo(true);
			}

	private:
		KisDoc *m_doc;
		KisImageSP m_img;
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
			      CompositeOp compositeOp) : super(i18n("Layer Property Changes"))
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
				CompositeOp compositeOp = m_layer -> compositeOp();

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
		CompositeOp m_compositeOp;
	};
}

KisDoc::KisDoc(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, bool singleViewMode) :
	super(parentWidget, widgetName, parent, name, singleViewMode),
	m_pixmap(RENDER_WIDTH, RENDER_HEIGHT)
{
// 	kdDebug() << "KisDoc created for " << widgetName << "\n";

	m_undo = false;
	m_dcop = 0;
	m_cmdHistory = 0;
	m_nserver = 0;
	m_pushedClipboard = false;
	m_currentMacro = 0;
	m_macroNestDepth = 0;
	m_ioProgressBase = 0;
	m_ioProgressTotalSteps = 0;

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
	if (!m_dcop)
		m_dcop = new KIsDocIface(this);

	return m_dcop;
}

bool KisDoc::initDoc(InitDocFlags flags, QWidget* parentWidget)
{
// 	kdDebug() << "KisDoc::initDoc\n";
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

		QFileInfo fileInfo( file );
		QString fileName( fileInfo.dirPath( TRUE ) + "/" + fileInfo.baseName() + ".kra" );
		resetURL();
		ok = loadNativeFormat( fileName );
		emit imageListUpdated();
		if (nimages() == 0) {
			if ((ok = slotNewImage()))
				emit imageListUpdated();
		}
		ok = true;
	} else if (ret == KoTemplateChooseDia::File) {
		KURL url( file );
		ok = openURL(url);
	} else if (ret == KoTemplateChooseDia::Empty) {
		if ((ok = slotNewImage()))
			emit imageListUpdated();
	}

	setModified(false);
	setUndo(true);

	return ok;
}

bool KisDoc::init()
{
// 	kdDebug() << "KisDoc::init\n";
	if (m_cmdHistory) {
		delete m_cmdHistory;
		m_cmdHistory = 0;
	}

	if (m_nserver) {
		delete m_nserver;
		m_nserver = 0;
	}

	m_cmdHistory = new KoCommandHistory(actionCollection(), true);
	connect(m_cmdHistory, SIGNAL(documentRestored()), this, SLOT(slotDocumentRestored()));
	connect(m_cmdHistory, SIGNAL(commandExecuted()), this, SLOT(slotCommandExecuted()));
	m_undo = true;
	m_nserver = new KisNameServer(i18n("Image %1"), 1);
	return true;
}

QDomDocument KisDoc::saveXML()
{
	QDomDocument doc = createDomDocument("DOC", CURRENT_DTD_VERSION);
	QDomElement root = doc.documentElement();

	root.setAttribute("editor", "Krita");
	root.setAttribute("depth", sizeof(QUANTUM));
	root.setAttribute("syntaxVersion", "1");

	for (vKisImageSP_it it = m_images.begin(); it != m_images.end(); it++)
		root.appendChild(saveImage(doc, *it));

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

	for (node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
		if (node.isElement()) {
			if (node.nodeName() == "IMAGE") {
				QDomElement elem = node.toElement();

				if (!(img = loadImage(elem)))
					return false;

				m_images.push_back(img);
			} else {
// 				kdDebug(DBG_AREA_CORE) << "KisDoc::loadXML nodeName == " << node.nodeName() << endl;
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
	image.setAttribute("colorspacename", img -> colorStrategy() -> name());
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
	QString colorspacename;
	Q_INT32 colorspace_int; // used to keep compatibility with old document

	if ((attr = element.attribute("mime")) == NATIVE_MIMETYPE) {
		if ((name = element.attribute("name")).isNull())
			return 0;

		if (namePresent(name))
			name = nextImageName();

		if ((attr = element.attribute("width")).isNull())
			return 0;

		if ((width = attr.toInt()) < 0 || width > cfg.maxImgWidth())
			return 0;

		if ((attr = element.attribute("height")).isNull())
			return 0;

		if ((height = attr.toInt()) < 0 || height > cfg.maxImgHeight())
			return 0;

		if ((colorspacename = element.attribute("colorspacename")).isNull())
		{
			// TODO: This code is used for compatibility with old files,
			// it should be removed before alpha
			if ((attr = element.attribute("colorspace")).isNull())
				return 0;
			colorspace_int = attr.toInt();
// 			kdDebug() << "colorspace_int = " << colorspace_int << endl;
			if( colorspace_int <= IMAGE_TYPE_UNKNOWN || colorspace_int > IMAGE_TYPE_YUVA)
				return 0;
			switch(colorspace_int)
			{
			case IMAGE_TYPE_GREYA:
			case IMAGE_TYPE_GREY:
				colorspacename = "Grayscale + Alpha";
				break;
			case IMAGE_TYPE_RGB:
			case IMAGE_TYPE_RGBA:
				colorspacename = "RGBA";
				break;
			case IMAGE_TYPE_CMYK:
			case IMAGE_TYPE_CMYKA:
				colorspacename = "CMYKA";
				break;
			default:
				return 0;
			}
		}
		img = new KisImage(this, width, height, KisColorSpaceRegistry::instance()->colorSpace(colorspacename), name);

		for (node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
			if (node.isElement()) {
				if (node.nodeName() == "LAYERS") {
					for (child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
						KisLayerSP layer = loadLayer(child.toElement(), img);

						if (!layer)
							return 0;

						img -> add(layer, -1);
					}

					if (img -> nlayers()) {
						img -> activateLayer(0);
					}
				} else if (node.nodeName() == "COLORMAP") {
					// TODO
				} //else {
// 					kdDebug(DBG_AREA_CORE) << "KisDoc::loadImage nodeName == " << node.nodeName() << endl;
// 				}
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
	layerElement.setAttribute("x", layer -> x());
	layerElement.setAttribute("y", layer -> y());
	layerElement.setAttribute("width", layer -> width());
	layerElement.setAttribute("height", layer -> height());
	layerElement.setAttribute("opacity", layer -> opacity());
	layerElement.setAttribute("visible", layer -> visible());
	layerElement.setAttribute("linked", layer -> linked());
	return layerElement;
}

KisLayerSP KisDoc::loadLayer(const QDomElement& element, KisImageSP img)
{
	KisConfig cfg;
	QString attr;
	QDomNode node;
	QDomNode child;
	QString name;
	Q_INT32 x;
	Q_INT32 y;
	Q_INT32 width;
	Q_INT32 height;
	Q_INT32 opacity;
	bool visible;
	bool linked;
	KisLayerSP layer;

	if ((name = element.attribute("name")).isNull())
		return 0;

	if ((attr = element.attribute("x")).isNull())
		return 0;

	x = attr.toInt();

	if ((attr = element.attribute("y")).isNull())
		return 0;

	y = attr.toInt();

	if ((attr = element.attribute("width")).isNull())
		return 0;

	if ((width = attr.toInt()) < 0 || x + width > cfg.maxImgWidth())
		return 0;

	if ((attr = element.attribute("height")).isNull())
		return 0;

	if ((height = attr.toInt()) < 0 || y + height > cfg.maxImgHeight())
		return 0;

	if ((attr = element.attribute("opacity")).isNull())
		return 0;

	if ((opacity = attr.toInt()) < 0 || opacity > QUANTUM_MAX)
		return 0;

	if ((attr = element.attribute("visible")).isNull())
		return 0;

	visible = attr == "0" ? false : true;

	if ((attr = element.attribute("linked")).isNull())
		return 0;

	linked = attr == "0" ? false : true;
	layer = new KisLayer(img, width, height, name, opacity);
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
	vKisImageSP images;
	KisImageSP img;

	for (vKisImageSP_it it = m_images.begin(); it != m_images.end(); it++) {
		totalSteps += (*it) -> nlayers();
		img = new KisImage(**it);
		img -> setName((*it) -> name());
		images.push_back(img);
	}

	setIOSteps(totalSteps);

	for (vKisImageSP_it it = images.begin(); it != images.end(); it++) {
		vKisLayerSP layers = (*it) -> layers();

		for (vKisLayerSP_it it2 = layers.begin(); it2 != layers.end(); it2++) {
			connect(*it2, SIGNAL(ioProgress(Q_INT8)), this, SLOT(slotIOProgress(Q_INT8)));
			location = external ? QString::null : uri;
			location += (*it) -> name() + "/layers/" + (*it2) -> name();

			if (store -> open(location)) {
				if (!(*it2) -> write(store)) {
					(*it2) -> disconnect();
					store -> close();
					IODone();
					return false;
				}

				store -> close();
			}

			IOCompletedStep();
			(*it2) -> disconnect();
		}

	}

	IODone();
	return true;
}

bool KisDoc::completeLoading(KoStore *store)
{
	QString uri = url().url();
	QString location;
	bool external = isStoredExtern();
	Q_INT32 totalSteps = 0;

	for (vKisImageSP_it it = m_images.begin(); it != m_images.end(); it++)
		totalSteps += (*it) -> nlayers();

	setIOSteps(totalSteps);

	for (vKisImageSP_it it = m_images.begin(); it != m_images.end(); it++) {
		vKisLayerSP layers = (*it) -> layers();

		for (vKisLayerSP_it it2 = layers.begin(); it2 != layers.end(); it2++) {
			connect(*it2, SIGNAL(ioProgress(Q_INT8)), this, SLOT(slotIOProgress(Q_INT8)));
			location = external ? QString::null : uri;
			location += (*it) -> name() + "/layers/" + (*it2) -> name();

			if (store -> open(location)) {
				if (!(*it2) -> read(store)) {
					(*it2) -> disconnect();
					store -> close();
					IODone();
					return false;
				}

				store -> close();
			}

			IOCompletedStep();
			(*it2) -> disconnect();
		}

	}

	IODone();
	return true;
}

bool KisDoc::namePresent(const QString& name) const
{
	for (vKisImageSP_cit it = m_images.begin(); it != m_images.end(); it++)
		if ((*it) -> name() == name)
			return true;

	return false;
}

void KisDoc::renameImage(const QString& oldName, const QString& newName)
{
	for (vKisImageSP_it it = m_images.begin(); it != m_images.end(); it++) {
		if ((*it) -> name() == oldName) {
			(*it) -> setName(newName);

			if (m_undo)
				addCommand(new KisCommandImageMv(this, this, newName, oldName));

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

Q_INT32 KisDoc::imageIndex(KisImageSP img) const
{
	for (vKisImageSP_cit it = m_images.begin(); it != m_images.end(); it++) {
		if (*it == img)
			return it - m_images.begin();
	}

	return -1;
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

KisImageSP KisDoc::findImage(const QString& name) const
{
	for (vKisImageSP_cit it = m_images.begin(); it != m_images.end(); it++) {
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

KisImageSP KisDoc::newImage(const QString& name, Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorstrategy)
{
	KisImageSP img = new KisImage(this, width, height, colorstrategy, name);

	KisLayerSP layer = new KisLayer(img, width, height, img -> nextLayerName(), OPACITY_OPAQUE);

	KisFillPainter painter;

	painter.begin(layer.data());
	painter.fillRect(0, 0, layer -> width(), layer -> height(), KoColor::white(), OPACITY_OPAQUE);
	painter.end();

	img -> add(layer, -1);

	addImage(img);
	return img;
}

void KisDoc::addImage(KisImageSP img)
{
	if (contains(img))
		return;

	m_images.push_back(img);

	if (m_undo)
		addCommand(new KisCommandImageAdd(this, this, img));

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
		addCommand(new KisCommandImageRm(this, this, img));
}

void KisDoc::removeImage(const QString& name)
{
	KisImageSP img = findImage(name);

	if (img)
		removeImage(img);
}

bool KisDoc::slotNewImage()
{
	KisConfig cfg;
	KisDlgCreateImg dlg(cfg.maxImgWidth(), cfg.defImgWidth(), 
			    cfg.maxImgHeight(), cfg.defImgHeight(), 
			    "RGBA",
			    nextImageName());

	if (dlg.exec() == QDialog::Accepted) {
		QString name;
		QUANTUM opacity = dlg.backgroundOpacity();
		KoColor c = dlg.backgroundColor();
		KisImageSP img;
		KisLayerSP layer;
		KisFillPainter painter;

		img = new KisImage(this, dlg.imgWidth(), 
				   dlg.imgHeight(), 
				   KisColorSpaceRegistry::instance()->colorSpace(dlg.colorStrategyName()), 
				   dlg.imgName());
		img -> setResolution(dlg.imgResolution(), dlg.imgResolution()); // XXX needs to be added to dialog
		img -> setDescription(dlg.imgDescription());
		img -> setProfile(dlg.profile());

		layer = new KisLayer(img, dlg.imgWidth(), dlg.imgHeight(), img -> nextLayerName(), OPACITY_OPAQUE);

		painter.begin(layer.data());
		painter.fillRect(0, 0, layer -> width(), layer -> height(), c, opacity);
		painter.end();

		img -> add(layer, -1);

		addImage(img);

		cfg.defImgWidth(dlg.imgWidth());
		cfg.defImgHeight(dlg.imgHeight());
		cfg.defLayerWidth(dlg.imgWidth());
		cfg.defLayerHeight(dlg.imgHeight());

		return true;
	}
	return false;
}

KoView* KisDoc::createViewInstance(QWidget* parent, const char *name)
{
	KisView * v = new KisView(this, this, parent, name);
	return v;
}

void KisDoc::paintContent(QPainter& painter, const QRect& rect)
{
// 	kdDebug() << "KisDoc::paintContent called with rect: "
// 		  << rect.x() << ","
// 		  << rect.y() << ","
// 		  << rect.right() << ","
// 		  << rect.bottom() << "\n";
	Q_INT32 x;
	Q_INT32 y;
	Q_INT32 x1;
	Q_INT32 y1;
	Q_INT32 x2;
	Q_INT32 y2;
	Q_INT32 tileno;


	// Only happens if there's actually only one image. As soon as
	// a second image is created, or selected, m_currentImage is
	// set. Note that m_currentImage in KisDoc is a KisImage, while
	// m_currentImage in KisImage is a KisPaintDevice.

	if (!m_currentImage)
		m_currentImage = m_images[0];

	if (m_currentImage) {

		x1 = CLAMP(rect.x(), 0, m_currentImage -> width());
		y1 = CLAMP(rect.y(), 0, m_currentImage -> height());
		x2 = CLAMP(rect.x() + rect.width(), 0, m_currentImage -> width());
		y2 = CLAMP(rect.y() + rect.height(), 0, m_currentImage -> height());


		// Flatten the layers onto the projection layer of the current image
		for (y = y1; y <= y2; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
			for (x = x1; x <= x2; x += TILE_WIDTH - (x % TILE_WIDTH)) {
				if ((tileno = m_currentImage -> tileNum(x, y)) < 0)
					continue;

				m_currentImage -> renderToProjection(tileno);
				QImage img = m_currentImage -> projection() -> convertToQImage(x, y, TILE_WIDTH, TILE_HEIGHT);
				if (!img.isNull()) {
                                        // XXX: made obosolete by qt-copy patch 0005
					m_pixio.putImage(&m_pixmap, 0, 0, &img);
					Q_INT32 w = QMIN(x2 - x, TILE_WIDTH);
					Q_INT32 h = QMIN(y2 - y, TILE_HEIGHT);
					painter.drawPixmap(x, y, m_pixmap, 0, 0, w, h);
				}
			}
		}
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

void KisDoc::beginMacro(const QString& macroName)
{
	if (m_undo) {
		if (m_macroNestDepth == 0) {
			Q_ASSERT(m_currentMacro == 0);
			m_currentMacro = new KMacroCommand(macroName);
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

QString KisDoc::nextImageName() const
{
	QString name;

	do {
		name = m_nserver -> name();
	} while (namePresent(name));

	return name;
}

void KisDoc::slotUpdate(KisImageSP, Q_UINT32 x, Q_UINT32 y, Q_UINT32 w, Q_UINT32 h)
{
	QRect rc(x, y, w, h);

	emit docUpdated(rc);
}

void KisDoc::setCurrentImage(KisImageSP img)
{
	m_currentImage = img;
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
				KisFillPainter painter(layer.data());
				painter.fillRect(0, 0, layer -> width(), layer -> height(), KoColor::black(), OPACITY_TRANSPARENT);
				painter.end();

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
			    Q_INT32 width,
			    Q_INT32 height,
			    const QString& name,
			    CompositeOp compositeOp,
			    QUANTUM opacity,
			    QPoint pos,
			    KisStrategyColorSpaceSP colorstrategy)
{
	KisLayerSP layer;
	if (!contains(img)) return 0;
	if (img) {
		layer = new KisLayer(width, height, colorstrategy, name);
		layer -> setOpacity(opacity);
		layer -> setCompositeOp(compositeOp);
		layer -> move(pos);
		if (layer && img -> add(layer, -1)) {
			layer = img -> activate(layer);

			if (layer) {
				KisFillPainter painter(layer.data());
				painter.fillRect(0, 0, layer -> width(), layer -> height(), KoColor::black(), OPACITY_TRANSPARENT);
				painter.end();
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
	if (!contains(img) || !l)
		return 0;

	if (img -> layer(l -> name()))
		return 0;

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
	if (!contains(img))
		return;

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
				CompositeOp compositeOp,
				const QString& name)
{
	if (!contains(img))
		return;

	if (layer) {
		if (m_undo) {
			QString oldname = layer -> name();
			Q_INT32 oldopacity = layer -> opacity();
			CompositeOp oldCompositeOp = layer -> compositeOp();
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

void KisDoc::setClipboard(KisPaintDeviceSP selection)
{
	m_clipboard = selection;

	if (selection) {
		QImage qimg = selection -> convertToQImage();
		QClipboard *cb = QApplication::clipboard();

		cb -> setImage(qimg);
		m_pushedClipboard = true;
	}
}

KisPaintDeviceSP KisDoc::clipboard()
{
	return m_clipboard;
}

void KisDoc::clipboardDataChanged()
{
	if (!m_pushedClipboard) {
		QClipboard *cb = QApplication::clipboard();
		QImage qimg = cb -> image();

		if (!qimg.isNull()) {
			m_clipboard =
				new KisPaintDevice(qimg.width(), qimg.height(),
						   KisColorSpaceRegistry::instance()->colorSpace( qimg.hasAlphaBuffer() ? "RGBA" : "RGB" ),
						   "KisDoc created clipboard selection");

			m_clipboard -> convertFromImage(qimg);
		}
	}

	m_pushedClipboard = false;
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

bool KisDoc::importImage(const QString& filename)
{
	if (m_nserver == 0)
		init();

	if (!filename.isEmpty()) {
		KURL url(filename);
		KisImageMagickConverter ib(this, this);

		if (url.isEmpty())
			return false;

		if (ib.buildImage(url) == KisImageBuilder_RESULT_OK) {
			addImage(ib.image());
			return true;
		}
	}

	return false;
}

#include "kis_doc.moc"

