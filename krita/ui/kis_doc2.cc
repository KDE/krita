/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2000 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
 *  Copyright (c) 2002, 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_doc2.h"

// Qt
#include <QApplication>
#include <qdom.h>
#include <QImage>
#include <QPainter>
#include <q3tl.h>
#include <QStringList>
#include <QWidget>
#include <QList>

// KDE
#include <kdebug.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

// KOffice
#include <KoApplication.h>
#include <KoCanvasBase.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoFilterManager.h>
#include <KoID.h>
#include <KoMainWindow.h>
#include <KoOasisStore.h>
#include <KoQueryTrader.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoSelection.h>

// Krita Image
#include "kis_types.h"
#include "kis_annotation.h"
#include "kis_debug_areas.h"
#include "kis_fill_painter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device_action.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_meta_registry.h"
#include "kis_nameserver.h"
#include "kis_external_layer_iface.h"
#include "kis_shape_layer.h"

// Local
#include "kis_factory2.h"
#include "kis_view2.h"
#include "kis_clipboard.h"
#include "kis_config.h"
#include "kis_custom_image_widget.h"
#include "kis_load_visitor.h"
#include "kis_save_visitor.h"
#include "kis_savexml_visitor.h"
#include "kis_oasis_load_visitor.h"
#include "kis_oasis_save_data_visitor.h"
#include "kis_oasis_save_visitor.h"
#include "kis_canvas2.h"
#include "kis_undo_adapter.h"
#include "kis_shape_controller.h"
#include "kis_layer_model.h"

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

class KisDoc2::KisDocPrivate
{

public:

    KisDocPrivate()
        : undoAdapter( 0 )
        , nserver( 0 )
        , macroNestDepth( 0 )
        , conversionDepth( 0 )
        , ioProgressTotalSteps( 0 )
        , ioProgressBase( 0 )
        {
        }

    ~KisDocPrivate()
        {
            // Don't delete m_d->shapeController or m_d->layerModel because it's in a QObject hierarchy.
            //delete undoAdapter;
            //delete nserver;
        }

    KisUndoAdapter *undoAdapter;
    KisNameServer *nserver;
    qint32 macroNestDepth;
    qint32 conversionDepth;
    int ioProgressTotalSteps;
    int ioProgressBase;
    QMap<KisLayer *, QString> layerFilenames; // temp storage during
                                              // load

    KisImageSP image;
    KisShapeController * shapeController;
    KisLayerModel * layerModel;
};


KisDoc2::KisDoc2(QWidget *parentWidget, QObject *parent, bool singleViewMode)
    : KoDocument(parentWidget, parent, singleViewMode)
    , m_d( new KisDocPrivate() )
{

    setComponentData( KisFactory2::componentData(), false );
    setTemplateType( "krita_template" );
    init();

}

KisDoc2::~KisDoc2()
{
    delete m_d;
}

QByteArray KisDoc2::mimeType() const
{
    return APP_MIMETYPE;
}

void KisDoc2::openExistingFile(const KUrl& url)
{
  setUndo(false);

  KoDocument::openExistingFile(url);

  setUndo(true);
}

void KisDoc2::openTemplate(const KUrl& url)
{
  setUndo(false);

  KoDocument::openTemplate(url);

  setUndo(true);
}

bool KisDoc2::init()
{
    if (m_d->undoAdapter) {
        delete m_d->undoAdapter;
        m_d->undoAdapter = 0;
    }

    if (m_d->nserver) {
        delete m_d->nserver;
        m_d->nserver = 0;
    }

    m_d->undoAdapter = new KisUndoAdapter(this);
    Q_CHECK_PTR(m_d->undoAdapter);

    setUndo(true);

    m_d->nserver = new KisNameServer(1);
    Q_CHECK_PTR(m_d->nserver);

    if (!KoColorSpaceRegistry::instance()->contains("RGBA")) {
        KMessageBox::sorry(0, i18n("No colorspace modules loaded: cannot run Krita"));
        return false;
    }

    m_d->shapeController = new KisShapeController( this, m_d->nserver );
    m_d->layerModel = new KisLayerModel( this );

    return true;
}

QDomDocument KisDoc2::saveXML()
{
    QDomDocument doc = createDomDocument("DOC", CURRENT_DTD_VERSION);
    QDomElement root = doc.documentElement();

    root.setAttribute("editor", "Krita");
    root.setAttribute("depth", (uint)sizeof(quint8));
    root.setAttribute("syntaxVersion", "1");

    root.appendChild(saveImage(doc, m_d->image));

    return doc;
}

bool KisDoc2::loadOasis( const QDomDocument& doc, KoOasisStyles&, const QDomDocument&, KoStore* store)
{
    kDebug(41008) << "loading with OpenRaster" << endl;
    QDomNode root = doc.documentElement();
    for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement() && node.nodeName() == "office:body") {
            QDomElement elem = node.toElement();
            KoOasisStore* oasisStore =  new KoOasisStore( store );
            KisOasisLoadVisitor olv(this,oasisStore);
            olv.loadImage(elem);
            if (!olv.image() )
                return false;

            setCurrentImage( olv.image() );

            return true;
        }
    }
    return false;
}


bool KisDoc2::saveOasis( KoStore* store, KoXmlWriter* manifestWriter)
{
    kDebug(41008) << "saving with OpenRaster" << endl;
    manifestWriter->addManifestEntry( "content.xml", "text/xml" );
    KoOasisStore* oasisStore =  new KoOasisStore( store );
    KoXmlWriter* contentWriter = oasisStore->contentWriter();
    if ( !contentWriter ) {
        delete oasisStore;
        return false;
    }

    manifestWriter->addManifestEntry( "data/", "" );
    KoXmlWriter* bodyWriter = oasisStore->bodyWriter();
    // Save the structure
    KisOasisSaveVisitor osv(oasisStore);
    bodyWriter->startElement("office:body");
    m_d->image->rootLayer()->accept(osv);
    bodyWriter->endElement();
    oasisStore->closeContentWriter();
    // Sqve the data
    KisOasisSaveDataVisitor osdv(oasisStore, manifestWriter);
    m_d->image->rootLayer()->accept(osdv);
    delete oasisStore;
    return true;
}

bool KisDoc2::loadXML(QIODevice *, const QDomDocument& doc)
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
    m_d->conversionDepth = attr.toInt();

    if (!root.hasChildNodes()) {
        return false; // XXX used to be: return slotNewImage();
    }

    setUndo(false);

    // XXX: This still handles multi-image .kra files?
    for (node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement()) {
            if (node.nodeName() == "IMAGE") {
                QDomElement elem = node.toElement();
                if (!(img = loadImage(elem)))
                    return false;

            } else {
                return false;
            }
        }
    }
    setCurrentImage( img );

    emit sigLoadingFinished();
    return true;
}

bool KisDoc2::loadChildren(KoStore* store) {
    Q3PtrListIterator<KoDocumentChild> it(children());
    for( ; it.current(); ++it ) {
        if (!it.current()->loadDocument(store)) {
            return false;
        }
    }
    return true;
}

QDomElement KisDoc2::saveImage(QDomDocument& doc, KisImageSP img)
{
    QDomElement image = doc.createElement("IMAGE");

    Q_ASSERT(img);
    image.setAttribute("name", img->name());
    image.setAttribute("mime", "application/x-kra");
    image.setAttribute("width", img->width());
    image.setAttribute("height", img->height());
    image.setAttribute("colorspacename", img->colorSpace()->id());
    image.setAttribute("description", img->description());
    // XXX: Save profile as blob inside the image, instead of the product name.
    if (img->profile() && img->profile()-> valid())
        image.setAttribute("profile", img->profile()->productName());
    image.setAttribute("x-res", img->xRes());
    image.setAttribute("y-res", img->yRes());

    quint32 count=0;
    KisSaveXmlVisitor visitor(doc, image, count, true);

    m_d->image->rootLayer()->accept(visitor);

    return image;
}

KisImageSP KisDoc2::loadImage(const QDomElement& element)
{

    KisConfig cfg;
    QString attr;
    QDomNode node;
    QDomNode child;
    KisImageSP img;
    QString name;
    qint32 width;
    qint32 height;
    QString description;
    QString profileProductName;
    double xres;
    double yres;
    QString colorspacename;
    KoColorSpace * cs;

    if ((attr = element.attribute("mime")) == NATIVE_MIMETYPE) {
        if ((name = element.attribute("name")).isNull())
            return KisImageSP(0);
        if ((attr = element.attribute("width")).isNull())
            return KisImageSP(0);
        width = attr.toInt();
        if ((attr = element.attribute("height")).isNull())
            return KisImageSP(0);
        height = attr.toInt();

        description = element.attribute("description");

        if ((attr = element.attribute("x-res")).isNull())
            xres = 100.0 / 72.0;
        else
            xres = attr.toDouble() / 72.0;

        if ((attr = element.attribute("y-res")).isNull())
            yres = 100.0 / 72.0;
        else
            yres = attr.toDouble() / 72.0;

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
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspacename,"");
        }
        else {
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspacename, profileProductName);
        }

        if (cs == 0) {
            kWarning(DBG_AREA_FILE) << "Could not open colorspace\n";
            return KisImageSP(0);
        }

        img = new KisImage(m_d->undoAdapter, width, height, cs, name);
        Q_CHECK_PTR(img);
        connect( img.data(), SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));
        img->setDescription(description);
        img->setResolution(xres, yres);

        loadLayers(element, img, img->rootLayer());

    }

    return img;
}

void KisDoc2::loadLayers(const QDomElement& element, KisImageSP img, KisGroupLayerSP parent)
{
    QDomNode node = element.firstChild();
    QDomNode child;

    if(!node.isNull())
    {
        if (node.isElement()) {
            if (node.nodeName() == "LAYERS") {
                for (child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
                    KisLayerSP layer = loadLayer(child.toElement(), img);

                    if (!layer) {
                        kWarning(DBG_AREA_FILE) << "Could not load layer\n";
                    }
                    else {
                        img->nextLayerName(); // Make sure the nameserver is current with the number of layers.
                        img->addLayer(layer, parent, KisLayerSP(0));
                    }
                }
            }
        }
    }
}

KisLayerSP KisDoc2::loadLayer(const QDomElement& element, KisImageSP img)
{
    // Nota bene: If you add new properties to layers, you should
    // ALWAYS define a default value in case the property is not
    // present in the layer definition: this helps a LOT with backward
    // compatibility.
    QString attr;
    QString name;
    qint32 x;
    qint32 y;
    qint32 opacity;
    bool visible;
    bool locked;

    if ((name = element.attribute("name")).isNull())
        return KisLayerSP(0);

    if ((attr = element.attribute("x")).isNull())
        return KisLayerSP(0);
    x = attr.toInt();

    if ((attr = element.attribute("y")).isNull())
        return KisLayerSP(0);

    y = attr.toInt();

    if ((attr = element.attribute("opacity")).isNull())
        return KisLayerSP(0);

    if ((opacity = attr.toInt()) < 0 || opacity > quint8_MAX)
        opacity = OPACITY_OPAQUE;


    QString compositeOpName = element.attribute("compositeop");

    if ((attr = element.attribute("visible")).isNull())
        attr = "1";

    visible = attr == "0" ? false : true;

    if ((attr = element.attribute("locked")).isNull())
        attr = "0";

    locked = attr == "0" ? false : true;

    // Now find out the layer type and do specific handling
    if ((attr = element.attribute("layertype")).isNull())
        return loadPaintLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName);

    if(attr == "paintlayer")
        return loadPaintLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName);

    if(attr == "grouplayer")
        return KisLayerSP(loadGroupLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName).data());

    if(attr == "adjustmentlayer")
        return KisLayerSP(loadAdjustmentLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName).data());

    kWarning(DBG_AREA_FILE) << "Specified layertype is not recognised\n";
    return KisLayerSP(0);
}


KisLayerSP KisDoc2::loadPaintLayer(const QDomElement& element, KisImageSP img,
                                  const QString & name, qint32 x, qint32 y,
                                  qint32 opacity, bool visible, bool locked, const QString & compositeOp)
{
    QString attr;
    KisPaintLayerSP layer;
    KoColorSpace * cs;

    QString colorspacename;
    QString profileProductName;

    if ((colorspacename = element.attribute("colorspacename")).isNull())
        cs = img->colorSpace();
    else
        // use default profile - it will be replaced later in completLoading
        cs = KoColorSpaceRegistry::instance()->colorSpace(colorspacename,"");

    const KoCompositeOp * op = cs->compositeOp(compositeOp);

    layer = new KisPaintLayer(img.data(), name, opacity, cs);
    Q_CHECK_PTR(layer);

    layer->setCompositeOp(op);
    layer->setVisible(visible);
    layer->setLocked(locked);
    layer->setX(x);
    layer->setY(y);

    if ((element.attribute("filename")).isNull())
        m_d->layerFilenames[layer.data()] = name;
    else
        m_d->layerFilenames[layer.data()] = QString(element.attribute("filename"));

    // Load exif info
    for( QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        QDomElement e = node.toElement();
        if ( !e.isNull() && e.tagName() == "ExifInfo" )
        {
            layer->paintDevice()->exifInfo()->load(e);
        }
    }
    return KisLayerSP(layer.data());
}

KisGroupLayerSP KisDoc2::loadGroupLayer(const QDomElement& element, KisImageSP img,
                                       const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked,
                                       const QString & compositeOp)
{
    QString attr;
    KisGroupLayerSP layer;

    layer = new KisGroupLayer(img.data(), name, opacity);
    Q_CHECK_PTR(layer);
    const KoCompositeOp * op = img->colorSpace()->compositeOp(compositeOp);
    layer->setCompositeOp(op);
    layer->setVisible(visible);
    layer->setLocked(locked);
    layer->setX(x);
    layer->setY(y);

    loadLayers(element, img, layer);

    return layer;
}

KisAdjustmentLayerSP KisDoc2::loadAdjustmentLayer(const QDomElement& element, KisImageSP img,
                                             const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked,
                                             const QString & compositeOp)
{
    QString attr;
    KisAdjustmentLayerSP layer;
    QString filtername;

    if ((filtername = element.attribute("filtername")).isNull()) {
        // XXX: Invalid adjustmentlayer! We should warn about it!
        kWarning(DBG_AREA_FILE) << "No filter in adjustment layer" << endl;
        return KisAdjustmentLayerSP(0);
    }

    KisFilterSP f = KisFilterRegistry::instance()->value(filtername);
    if (!f) {
        kWarning(DBG_AREA_FILE) << "No filter for filtername " << filtername << "\n";
        return KisAdjustmentLayerSP(0); // XXX: We don't have this filter. We should warn about it!
    }

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);

    // We'll load the configuration and the selection later.
    layer = KisAdjustmentLayerSP(new KisAdjustmentLayer(img, name, kfc, KisSelectionSP(0)));
    Q_CHECK_PTR(layer);

    const KoCompositeOp * op = img->colorSpace()->compositeOp(compositeOp);
    layer->setCompositeOp(op);
    layer->setVisible(visible);
    layer->setLocked(locked);
    layer->setX(x);
    layer->setY(y);
    layer->setOpacity(opacity);

    if ((element.attribute("filename")).isNull())
        m_d->layerFilenames[layer.data()] = name;
    else
        m_d->layerFilenames[layer.data()] = QString(element.attribute("filename"));

    return layer;
}


KisShapeLayerSP KisDoc2::loadShapeLayer(const QDomElement& elem, KisImageSP img, const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked, const QString &compositeOp)
{
#ifdef __GNUC__
#warning "Implement loading of shape layers!"
#endif
    Q_UNUSED(elem);
    Q_UNUSED(img);
    Q_UNUSED(name);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(opacity);
    Q_UNUSED(visible);
    Q_UNUSED(locked);
    Q_UNUSED(compositeOp);

    return KisShapeLayerSP();
}


bool KisDoc2::completeSaving(KoStore *store)
{
    QString uri = url().url();
    QString location;
    bool external = isStoredExtern();
    qint32 totalSteps = 0;

    KisImageSP img = m_d->image;

    if (!img) return false;

    totalSteps = img->nlayers();


    setIOSteps(totalSteps + 1);

    // Save the layers data
    quint32 count=0;
    KisSaveVisitor visitor( img, store, count);

    if(external)
        visitor.setExternalUri(uri);

    img->rootLayer()->accept(visitor);
    // saving annotations
    // XXX this only saves EXIF and ICC info. This would probably need
    // a redesign of the dtd of the krita file to do this more generally correct
    // e.g. have <ANNOTATION> tags or so.
    KisAnnotationSP annotation = img->annotation("exif");
    if (annotation) {
        location = external ? QString::null : uri;
        location += (m_d->image)->name() + "/annotations/exif";
        if (store->open(location)) {
            store->write(annotation->annotation());
            store->close();
        }
    }
    if (img->profile()) {
        KoColorProfile *profile = img->profile();
        KisAnnotationSP annotation;
        if (profile)
        {
            // XXX we hardcode icc, this is correct for lcms?
            // XXX productName(), or just "ICC Profile"?
            if (!profile->rawData().isEmpty())
                annotation = new  KisAnnotation("icc", profile->productName(), profile->rawData());
        }

        if (annotation) {
            location = external ? QString::null : uri;
            location += img->name() + "/annotations/icc";
            if (store->open(location)) {
                store->write(annotation->annotation());
                store->close();
            }
        }
    }
    IODone();
    return true;
}

bool KisDoc2::completeLoading(KoStore *store)
{
    QString uri = url().url();
    QString location;
    bool external = isStoredExtern();
    qint32 totalSteps = 0;

    KisImageSP img = m_d->image;
    if ( !img ) return false;

    totalSteps = img->nlayers();

    setIOSteps(totalSteps);

    // Load the layers data
    KisLoadVisitor visitor(img, store, m_d->layerFilenames);

    if(external)
        visitor.setExternalUri(uri);

    img->rootLayer()->accept(visitor);
    // annotations
    // exif
    location = external ? QString::null : uri;
    location += (img)->name() + "/annotations/exif";
    if (store->hasFile(location)) {
        QByteArray data;
        store->open(location);
        data = store->read(store->size());
        store->close();
        img->addAnnotation(KisAnnotationSP(new KisAnnotation("exif", "", data)));
    }
    // icc profile
    location = external ? QString::null : uri;
    location += (img)->name() + "/annotations/icc";
    if (store->hasFile(location)) {
        QByteArray data;
        store->open(location);
        data = store->read(store->size());
        store->close();
        img->setProfile(new KoColorProfile(data));
    }

    IODone();

    setModified( false );
    setUndo(true);
    return true;
}

QWidget* KisDoc2::createCustomDocumentWidget(QWidget *parent)
{
    KisConfig cfg;

    int w = cfg.defImgWidth();
    int h = cfg.defImgHeight();

    QSize sz = KisClipboard::instance()->clipSize();
    if (sz.isValid() && sz.width() != 0 && sz.height() != 0) {
        w = sz.width();
        h = sz.height();
    }
    return new KisCustomImageWidget(parent, this, w, h, cfg.defImgResolution(), cfg.workingColorSpace(),"unnamed");
    return 0;
}


KoDocument* KisDoc2::hitTest(const QPoint &pos, KoView* view, const QMatrix& matrix) {
    KoDocument* doc = KoDocument::hitTest(pos, view, matrix);
    return doc;
}


KisImageSP KisDoc2::newImage(const QString& name, qint32 width, qint32 height, KoColorSpace * colorspace)
{
    if (!init())
        return KisImageSP(0);

    setUndo(false);

    KisImageSP img = KisImageSP(new KisImage(m_d->undoAdapter, width, height, colorspace, name));
    Q_CHECK_PTR(img);
    connect( img.data(), SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));

    KisPaintLayer *layer = new KisPaintLayer(img.data(), img->nextLayerName(), OPACITY_OPAQUE,colorspace);
    Q_CHECK_PTR(layer);

    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFillPainter painter;

    painter.begin(layer->paintDevice());
    painter.fillRect(0, 0, width, height, KoColor(Qt::white, cs), OPACITY_OPAQUE);
    painter.end();

    img->addLayer(KisLayerSP(layer), img->rootLayer(), KisLayerSP(0));
    img->activateLayer(KisLayerSP(layer));

    setCurrentImage(img );

    setUndo(true);

    return img;
}

bool KisDoc2::newImage(const QString& name, qint32 width, qint32 height, KoColorSpace * cs, const KoColor &bgColor, const QString &imgDescription, const double imgResolution)
{
    if (!init())
        return false;

    KisConfig cfg;

    quint8 opacity = OPACITY_OPAQUE;//bgColor.alpha();
    KisImageSP img;
    KisPaintLayer *layer;

    if (!cs) return false;

    setUndo(false);

    img = new KisImage(m_d->undoAdapter, width, height, cs, name);
    Q_CHECK_PTR(img);
    connect( img.data(), SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));
    img->setResolution(imgResolution, imgResolution);
    img->setDescription(imgDescription);
    img->setProfile(cs->profile());

    layer = new KisPaintLayer(img.data(), img->nextLayerName(), OPACITY_OPAQUE, cs);
    Q_CHECK_PTR(layer);

    KisFillPainter painter;
    painter.begin(layer->paintDevice());
    painter.fillRect(0, 0, width, height, bgColor, opacity);
    painter.end();

    QList<KisPaintDeviceAction *> actions =
        KoColorSpaceRegistry::instance()->paintDeviceActionsFor(cs);
    for (int i = 0; i < actions.count(); i++)
        actions.at(i)->act(layer->paintDevice(), img->width(), img->height());

    img->setBackgroundColor(bgColor);
    img->addLayer(KisLayerSP(layer), img->rootLayer(), KisLayerSP(0));
    img->activateLayer(KisLayerSP(layer));

    setCurrentImage( img );

    cfg.defImgWidth(width);
    cfg.defImgHeight(height);
    cfg.defImgResolution(imgResolution);

    setUndo(true);

    return true;
}

KoView* KisDoc2::createViewInstance(QWidget* parent)
{
    KisView2 * v = new KisView2(this, parent);
    Q_CHECK_PTR(v);

    m_d->shapeController->setInitialShapeForView( v );

    return v;
}

void KisDoc2::paintContent(QPainter& painter, const QRect& rc)
{
    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();
    KoColorProfile *  profile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
    QRect rect = rc & m_d->image->bounds();
    m_d->image->renderToPainter(rect.left(), rect.left(), rect.top(), rect.height(), rect.width(), rect.height(), painter, profile);
}

void KisDoc2::slotImageUpdated()
{
    emit sigDocUpdated();
    setModified(true);
}

void KisDoc2::slotImageUpdated(const QRect& rect)
{
    emit sigDocUpdated(rect);
}

void KisDoc2::setUndo(bool undo)
{
    m_d->undoAdapter->setUndo(undo);
}

void KisDoc2::slotDocumentRestored()
{
    setModified(false);
}

void KisDoc2::slotUpdate(KisImageSP, quint32 x, quint32 y, quint32 w, quint32 h)
{
    QRect rc(x, y, w, h);

    emit sigDocUpdated(rc);
}

bool KisDoc2::undo() const
{
    return m_d->undoAdapter->undo();
}

KoShapeControllerBase * KisDoc2::shapeController()
{
    return m_d->shapeController;
}

KisLayerModel * KisDoc2::layerModel()
{
    return m_d->layerModel;
}

void KisDoc2::setIOSteps(qint32 nsteps)
{
    m_d->ioProgressTotalSteps = nsteps * 100;
    m_d->ioProgressBase = 0;
    emitProgress(0);
}

void KisDoc2::IOCompletedStep()
{
    m_d->ioProgressBase += 100;
}

void KisDoc2::IODone()
{
    emitProgress(-1);
}

void KisDoc2::slotIOProgress(qint8 percentage)
{
    Q_ASSERT(qApp);

    if (qApp->hasPendingEvents())
        qApp->processEvents();

    int totalPercentage = ((m_d->ioProgressBase + percentage) * 100) / m_d->ioProgressTotalSteps;

    emitProgress(totalPercentage);
}

KisChildDoc * KisDoc2::createChildDoc( const QRect & rect, KoDocument* childDoc )
{
    Q_UNUSED(rect);
    Q_UNUSED(childDoc);
#if 0
    KisChildDoc * ch = new KisChildDoc( this, rect, childDoc );
    insertChild( ch );
    ch->document()->setStoreInternal(true);
    return ch;
#endif
return 0;
}

void KisDoc2::prepareForImport()
{
    if (m_d->nserver == 0)
        init();
    setUndo(false);
}

KisImageSP KisDoc2::image()
{
    return m_d->image;
}


void KisDoc2::setCurrentImage(KisImageSP image)
{

    kDebug(41007) << "KisDoc2::setCurrentImage: " << image->name() << ", active layer: " << image->activeLayer() << endl;

    if ( m_d->image ) {
        // Disconnect existing sig/slot connections
        m_d->image->disconnect( this );
    }
    m_d->image = image;
    m_d->shapeController->setImage( image );
    m_d->layerModel->setImage( image );
    setUndo(true);

    emit sigLoadingFinished();
}

void KisDoc2::initEmpty()
{
    KisConfig cfg;
    KoColorSpace * rgb = KoColorSpaceRegistry::instance()->rgb8();
    newImage("", cfg.defImgWidth(), cfg.defImgHeight(), rgb);
}

KisUndoAdapter* KisDoc2::undoAdapter()
{
    return m_d->undoAdapter;
}
#include "kis_doc2.moc"

