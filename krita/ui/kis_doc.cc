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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include <kmessagebox.h>

// KOffice
#include <koFilterManager.h>
#include <koMainWindow.h>
#include <koQueryTrader.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <koTemplateChooseDia.h>
#include <koApplication.h>
#include <kocommandhistory.h>

// Local
#include <kis_meta_registry.h>
#include "kis_annotation.h"
#include "kis_types.h"
#include "kis_config.h"
#include "kis_debug_areas.h"
#include "kis_doc.h"
#include "kis_factory.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_nameserver.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_command.h"
#include "kis_view.h"
#include "kis_colorspace.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_profile.h"
#include "kis_id.h"
#include "kis_part_layer.h"
#include "kis_doc_iface.h"
#include "kis_paint_device_action.h"
#include "kis_custom_image_widget.h"

static const char *CURRENT_DTD_VERSION = "1.3";

/**
 * Mime type for this app - not same as file type, but file types
 * can be associated with a mime type and are opened with applications
 * associated with the same mime type
 */
#define APP_MIMETYPE "application/x-krita"

/**
 * Mime type for native file format
 */
#define NATIVE_MIMETYPE "application/x-kra"

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

    setInstance( KisFactory::instance(), false );
    setTemplateType( "krita_template" );

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
        m_dcop = new KisDocIface(this);
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
/*
                if ((ok = slotNewImage()))
                        emit imageListUpdated();
                setModified(false);
                KoDocument::setEmpty();
                setUndo(true);
                return ok;
*/        }

    QString file;
    KoTemplateChooseDia::DialogType dlgtype;

     if (flags != KoDocument::InitDocFileNew) {
        dlgtype = KoTemplateChooseDia::Everything;
    } else {
         dlgtype = KoTemplateChooseDia::OnlyTemplates;
    }

    KoTemplateChooseDia::ReturnType ret =
        KoTemplateChooseDia::choose(KisFactory::instance(),
                        file,
                        dlgtype,
                        "krita_template",
                        parentWidget);
    setUndo(false);

    if (ret == KoTemplateChooseDia::Template) {
        resetURL();
        ok = loadNativeFormat( file );
        emit imageListUpdated();
        setEmpty();
        ok = true;

    } else if (ret == KoTemplateChooseDia::File) {

        KURL url( file );
        ok = openURL(url);

    } else if (ret == KoTemplateChooseDia::Empty) {
/*
        if ((ok = slotNewImage())) {
            emit imageListUpdated();
            setEmpty();
        }
*/
    }

    setModified(false);
    setUndo(true);

    return ok;
}

void KisDoc::openExistingFile(const QString& file)
{
  setUndo(false);

  KoDocument::openExistingFile(file);

  setUndo(true);
}

void KisDoc::openTemplate(const QString& file)
{
  setUndo(false);

  KoDocument::openTemplate(file);

  setUndo(true);
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

   if (!KisMetaRegistry::instance()->csRegistry()->exists(KisID("RGBA",""))) {
        KMessageBox::sorry(0, i18n("No colorspace modules loaded: cannot run Krita"));
        return false;
    }

    return true;
}

QDomDocument KisDoc::saveXML()
{
    QDomDocument doc = createDomDocument("DOC", CURRENT_DTD_VERSION);
    QDomElement root = doc.documentElement();

    root.setAttribute("editor", "Krita");
    root.setAttribute("depth", sizeof(Q_UINT8));
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
        return false; // XXX used to be: return slotNewImage();
    }

    for (node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        kdDebug(DBG_AREA_FILE) << "Node: " << node.nodeName() << ", element: " << node.isElement() << "\n";
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
    image.setAttribute("colorspacename", img -> colorSpace() -> id().id());
    image.setAttribute("description", img -> description());
    // XXX: Save profile as blob inside the image, instead of the product name.
    if (img -> getProfile() && img -> getProfile()-> valid())
        image.setAttribute("profile", img -> getProfile() -> productName());
    image.setAttribute("x-res", img -> xRes());
    image.setAttribute("y-res", img -> yRes());

    layers = img -> layers();

    if (layers.size() > 0) {
        QDomElement elem = doc.createElement("LAYERS");

        image.appendChild(elem);

        for (vKisLayerSP_it it = layers.begin(); it != layers.end(); ++it)
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
    KisColorSpace * cs;


    if ((attr = element.attribute("mime")) == NATIVE_MIMETYPE) {
        if ((name = element.attribute("name")).isNull())
            return 0;
        if ((attr = element.attribute("width")).isNull())
            return 0;
        if ((attr = element.attribute("height")).isNull())
            return 0;
        description = element.attribute("description");

        if ((attr = element.attribute("x-res")).isNull())
            xres = 100.0;

        if ((attr = element.attribute("y-res")).isNull())
            yres = 100.0;

        if ((colorspacename = element.attribute("colorspacename")).isNull())
        {
            // An old file: take a reasonable default.
            // Krita didn't support anything else in those
            // days anyway.
            colorspacename = "RGBA";
        }

        // A hack for an old colorspacename
        if (colorspacename  == "Grayscale + Alpha")
            colorspacename  = "GRAYA";

        if ((profileProductName = element.attribute("profile")).isNull()) {
            // no mention of profile so get default profile
            cs = KisMetaRegistry::instance()->csRegistry() -> getColorSpace(colorspacename,"");
        }
        else {
            cs = KisMetaRegistry::instance()->csRegistry() -> getColorSpace(colorspacename, profileProductName);
        }

        if (cs == 0) {
            kdDebug(DBG_AREA_FILE) << "Could not open colorspace\n";
            return 0;
        }

        img = new KisImage(this, width, height, cs, name);
        Q_CHECK_PTR(img);
        connect( img, SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));
        img -> setDescription(description);
        img -> setResolution(xres, yres);

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
    layerElement.setAttribute("colorspacename", layer -> colorSpace() -> id().id());
    // XXX: Save profile as blob inside the layer, instead of the product name.
    if (layer -> colorSpace() -> getProfile() && layer -> colorSpace() -> getProfile() -> valid()) {
        layerElement.setAttribute("profile", layer -> colorSpace() -> getProfile() -> productName());
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
    KisColorSpace * cs;

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

    if ((opacity = attr.toInt()) < 0 || opacity > Q_UINT8_MAX)
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

    QString colorspacename;
    QString profileProductName;

    if ((colorspacename = element.attribute("colorspacename")).isNull())
    {
        cs = img -> colorSpace();
    }
    else
    {
        if ((profileProductName = element.attribute("profile")).isNull()) {
            // no mention of profile so get default profile
            cs = KisMetaRegistry::instance()->csRegistry() -> getColorSpace(colorspacename,"");
        }
        else {
            cs = KisMetaRegistry::instance()->csRegistry() -> getColorSpace(colorspacename, profileProductName);
        }

        if (cs == 0) {
            kdDebug(DBG_AREA_FILE) << "Could not open ColorSpace of layer " << colorspacename << "\n";
            return 0;

        kdDebug(DBG_AREA_FILE) << "ColorSpace of layer: " << colorspacename << "\n";
        }
    }

    layer = new KisLayer(img, name, opacity, cs);
    Q_CHECK_PTR(layer);

    layer -> setCompositeOp(compositeOp);
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
    connect( img, SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));
    img -> setName((m_currentImage) -> name());

    setIOSteps(totalSteps + 1);

    vKisLayerSP layers = (img) -> layers();

    for (vKisLayerSP_it it2 = layers.begin(); it2 != layers.end(); ++it2) {
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

        if ((*it2) -> colorSpace() -> getProfile()) {
            KisAnnotationSP annotation = (*it2) -> colorSpace() -> getProfile() -> annotation();

            if (annotation) {
                // save layer profile
                location = external ? QString::null : uri;
                location += (img) -> name() + "/layers/" + (*it2) -> name() + ".icc";

                if (store -> open(location)) {
                    store -> write(annotation -> annotation());
                    store -> close();
                }
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
    if (img -> getProfile()) {
        annotation = img -> getProfile() -> annotation();

        if (annotation) {
            location = external ? QString::null : uri;
            location += img -> name() + "/annotations/icc";
            if (store -> open(location)) {
                store -> write(annotation -> annotation());
                store -> close();
            }
        }
    }

    // Composite rendition of the entire image for easier kimgio loading
    // and to speed up loading the image into Krita: show the composite png first,
    // then load the layers.
    if (store -> open("composite.png")) {

        QPixmap * pix = new QPixmap(m_currentImage -> width(), m_currentImage -> height());
        QPainter gc(pix);
        m_currentImage -> renderToPainter(0, 0, m_currentImage -> width(), m_currentImage -> height(), gc, m_currentImage -> getProfile(),
                                          KisImage::PAINT_IMAGE_ONLY);
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

    for (vKisLayerSP_it it2 = layers.begin(); it2 != layers.end(); ++it2) {
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
/*PROFILEMERGE            (*it2) -> setProfile(new KisProfile(data,
                (*it2) -> colorSpace() -> colorSpaceType()));
*/            kdDebug(DBG_AREA_FILE) << "Opened icc information, size is " << data.size() << endl;
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
        (m_currentImage) -> setProfile(new KisProfile(data));
        kdDebug(DBG_AREA_FILE) << "Opened icc information, size is " << data.size() << endl;
    }

    IODone();

    setModified( false );
    return true;
}

QWidget* KisDoc::createCustomDocumentWidget(QWidget *parent)
{
    KisConfig cfg;

    return new KisCustomImageWidget(parent, this,cfg.defImgWidth(), cfg.defImgHeight(), cfg.defImgResolution(), cfg.workingColorSpace(),"foobar");
}

void KisDoc::renameImage(const QString& oldName, const QString& newName)
{
    (m_currentImage) -> setName(newName);

    if (m_undo)
        addCommand(new KisCommandImageMv(this, this, newName, oldName));

    emit imageListUpdated();
}


KisImageSP KisDoc::newImage(const QString& name, Q_INT32 width, Q_INT32 height, KisColorSpace * colorstrategy)
{
    KisImageSP img = new KisImage(this, width, height, colorstrategy, name);
    Q_CHECK_PTR(img);
    connect( img, SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));

    KisLayerSP layer = new KisLayer(img, img -> nextLayerName(), OPACITY_OPAQUE);
    Q_CHECK_PTR(layer);

    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    KisFillPainter painter;

    painter.begin(layer.data());
    painter.fillRect(0, 0, width, height, KisColor(Qt::white, cs), OPACITY_OPAQUE);
    painter.end();

    img -> add(layer, -1);
    img->notify();

    m_currentImage = img;
    return img;
}

bool KisDoc::newImage(const QString& name, Q_INT32 width, Q_INT32 height, KisColorSpace * cs, const KisColor &bgColor, const QString &imgDescription, const double imgResolution)
{
    KisConfig cfg;

    Q_UINT8 opacity = OPACITY_OPAQUE;//bgColor.getAlpha();
    KisImageSP img;
    KisLayerSP layer;

    if (!cs) return false;

    img = new KisImage(this, width, height, cs, name);
    Q_CHECK_PTR(img);
    connect( img, SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));
    img -> setResolution(imgResolution, imgResolution);
    img -> setDescription(imgDescription);
    img -> setProfile(cs->getProfile());

    layer = new KisLayer(img, img -> nextLayerName(), OPACITY_OPAQUE);
    Q_CHECK_PTR(layer);

    KisFillPainter painter;
    painter.begin(layer.data());
    painter.fillRect(0, 0, width, height, bgColor, opacity);
    painter.end();

    QValueVector<KisPaintDeviceAction *> actions = KisMetaRegistry::instance() ->
                csRegistry() -> paintDeviceActionsFor(cs);
    for (uint i = 0; i < actions.count(); i++)
        actions.at(i) -> act(layer.data(), img -> width(), img -> height());

    img -> setBackgroundColor(bgColor);
    img -> add(layer, -1);
    img -> notify();

    m_currentImage = img;

    cfg.defImgWidth(width);
    cfg.defImgHeight(height);
    cfg.defImgResolution(imgResolution);

    return true;
}

KoView* KisDoc::createViewInstance(QWidget* parent, const char *name)
{
    kdDebug() << "Going to create the view\n";
    KisView * v = new KisView(this, this, parent, name);
    Q_CHECK_PTR(v);

    return v;
}

void KisDoc::paintContent(QPainter& painter, const QRect& rc, bool transparent, double zoomX, double zoomY)
{
    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();
    KisProfile *  profile = KisMetaRegistry::instance()->csRegistry() -> getProfileByName(monitorProfileName);
    painter.scale(zoomX, zoomY);
    QRect rect = rc & m_currentImage -> bounds();
    KisImage::PaintFlags paintFlags;
    if (transparent) {
        paintFlags = KisImage::PAINT_SELECTION;
    } else {
        paintFlags = (KisImage::PaintFlags)(KisImage::PAINT_BACKGROUND|KisImage::PAINT_SELECTION);
    }
    m_currentImage -> renderToPainter(rect.left(), rect.top(), rect.right(), rect.bottom(), painter, profile, paintFlags);
}

void KisDoc::slotImageUpdated()
{
    emit docUpdated();
    setModified( true );
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

KisChildDoc * KisDoc::createChildDoc( const QRect & rect, KoDocument* childDoc )
{
    KisChildDoc * ch = new KisChildDoc( this, rect, childDoc );
    insertChild( ch );
    return ch;

}

void KisDoc::prepareForImport()
{
    if (m_nserver == 0)
        init();

}

KisImageSP KisDoc::currentImage()
{
    return m_currentImage;
}

void KisDoc::setCurrentImage(KisImageSP image)
{
    m_currentImage = image;
}

#include "kis_doc.moc"

