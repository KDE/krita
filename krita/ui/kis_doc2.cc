/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2000 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
 *  Copyright (c) 2002, 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "modeltest.h"

#include <QApplication>
#include <qdom.h>
#include <QImage>
#include <QPainter>
#include <q3tl.h>
#include <QStringList>
#include <QWidget>
#include <QList>

// KDE
#include <kis_debug.h>
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
#include <colorprofiles/KoIccColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoFilterManager.h>
#include <KoID.h>
#include <KoMainWindow.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KoQueryTrader.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoSelection.h>
#include <KoDocumentInfo.h>
#include <KoShape.h>
#include <KoToolManager.h>

// Krita Image
#include <kis_adjustment_layer.h>
#include <kis_annotation.h>
#include <kis_debug.h>
#include <kis_external_layer_iface.h>
#include <kis_fill_painter.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>

#include <kis_name_server.h>
#include <kis_paint_device_action.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_shape_layer.h>

// Local
#include "kis_factory2.h"
#include "kis_view2.h"
#include "kis_clipboard.h"
#include "kis_config.h"
#include "widgets/kis_custom_image_widget.h"
#include "kra/kis_kra_save_visitor.h"
#include "kra/kis_savexml_visitor.h"
#include "canvas/kis_canvas2.h"
#include "kis_undo_adapter.h"
#include "kis_shape_controller.h"
#include "kis_node_model.h"
#include "kra/kis_kra_loader.h"


static const char *CURRENT_DTD_VERSION = "1.3";

/**
 * Mime type for this app - not same as file type, but file types
 * can be associated with a mime type and are opened with applications
 * associated with the same mime type
 */
#define APP_MIMETYPE "application/x-krita"


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
        , kraLoader( 0 )
        {
        }

    ~KisDocPrivate()
        {
            // Don't delete m_d->shapeController or m_d->nodeModel because it's in a QObject hierarchy.
            //delete undoAdapter;
            //delete nserver;
        }

    KisUndoAdapter *undoAdapter;
    KisNameServer *nserver;
    qint32 macroNestDepth;
    qint32 conversionDepth;
    int ioProgressTotalSteps;
    int ioProgressBase;

    KisImageSP image;
    KisShapeController * shapeController;
    KisNodeModel * nodeModel;

    KisKraLoader * kraLoader;
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

    m_d->shapeController = new KisShapeController( this, m_d->nserver );
    m_d->nodeModel = new KisNodeModel( this );
//    new ModelTest(m_d->nodeModel, this);


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

bool KisDoc2::loadOdf( KoOdfReadStore & odfStore )
{
    Q_UNUSED(odfStore);
    return false;
}


bool KisDoc2::saveOdf( SavingContext &documentContext )
{
    Q_UNUSED(documentContext);
    return false;
}

bool KisDoc2::loadXML(QIODevice *, const KoXmlDocument& doc)
{
    KoXmlElement root;
    QString attr;
    KoXmlNode node;
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
        return false;
    }

    setUndo(false);
    m_d->kraLoader = new KisKraLoader(this);

    // XXX: This still handles multi-image .kra files?
    for (node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement()) {
            if (node.nodeName() == "IMAGE") {
                KoXmlElement elem = node.toElement();
                if (!(img = m_d->kraLoader->loadXML(elem)))
                    return false;

            } else {
                return false;
            }
        }
    }
    setCurrentImage( img );

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
    image.setAttribute("name", documentInfo()->aboutInfo("title"));
    image.setAttribute("mime", "application/x-kra");
    image.setAttribute("width", img->width());
    image.setAttribute("height", img->height());
    image.setAttribute("colorspacename", img->colorSpace()->id());
    image.setAttribute("description", documentInfo()->aboutInfo("comment"));
    // XXX: Save profile as blob inside the image, instead of the product name.
    if (img->profile() && img->profile()-> valid())
        image.setAttribute("profile", img->profile()->name());
    image.setAttribute("x-res", img->xRes());
    image.setAttribute("y-res", img->yRes());

    quint32 count=0;
    KisSaveXmlVisitor visitor(doc, image, count, true);

    m_d->image->rootLayer()->accept(visitor);

    return image;
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
    KisKraSaveVisitor visitor( img, store, count, documentInfo()->aboutInfo("title"));

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
        location += documentInfo()->aboutInfo("title") + "/annotations/exif";
        if (store->open(location)) {
            store->write(annotation->annotation());
            store->close();
        }
    }
    if (img->profile()) {
        const KoColorProfile *profile = img->profile();
        KisAnnotationSP annotation;
        if (profile)
        {
            const KoIccColorProfile* iccprofile = dynamic_cast<const KoIccColorProfile*>(profile);
            if (iccprofile && !iccprofile->rawData().isEmpty())
                annotation = new  KisAnnotation("icc", iccprofile->name(), iccprofile->rawData());
        }

        if (annotation) {
            location = external ? QString::null : uri;
            location += documentInfo()->aboutInfo("title") + "/annotations/icc";
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
    if ( !m_d->image ) return false;

    setIOSteps(m_d->image->nlayers());

    m_d->kraLoader->loadBinaryData(store, m_d->image, url().url(), isStoredExtern());

    IODone();

    setModified( false );
    setUndo(true);

    delete m_d->kraLoader;
    m_d->kraLoader = 0;

    connect( m_d->image.data(), SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));
    emit sigLoadingFinished();

    return true;
}

QList<KoDocument::CustomDocumentWidgetItem> KisDoc2::createCustomDocumentWidgets(QWidget *parent)
{
    KisConfig cfg;

    int w = cfg.defImgWidth();
    int h = cfg.defImgHeight();
    bool clipAvailable = false;

    QSize sz = KisClipboard::instance()->clipSize();
    if (sz.isValid() && sz.width() != 0 && sz.height() != 0) {
        w = sz.width();
        h = sz.height();
        clipAvailable = true;
    }

    QList<KoDocument::CustomDocumentWidgetItem> widgetList;
    KoDocument::CustomDocumentWidgetItem item;
    item.widget = new KisCustomImageWidget(parent, this, w, h, clipAvailable, cfg.defImgResolution(), cfg.workingColorSpace(),"unnamed");
    widgetList << item;

    return widgetList;
}


KoDocument* KisDoc2::hitTest(const QPoint &pos, KoView* view, const QMatrix& matrix) {
    KoDocument* doc = KoDocument::hitTest(pos, view, matrix);
    return doc;
}


KisImageSP KisDoc2::newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * colorspace)
{
    if (!init())
        return KisImageSP(0);

    setUndo(false);

    KisImageSP img = KisImageSP(new KisImage(m_d->undoAdapter, width, height, colorspace, name));
    img->lock();

    Q_CHECK_PTR(img);
    connect( img.data(), SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));

    KisPaintLayerSP layer = new KisPaintLayer(img.data(), img->nextLayerName(), OPACITY_OPAQUE,colorspace);
    Q_CHECK_PTR(layer);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFillPainter painter;

    layer->paintDevice()->fill( 0, 0, width, height, KoColor(Qt::white, cs).data() );

    img->addNode(layer.data(), img->rootLayer().data() );

    setCurrentImage(img );
    layer->setDirty();
    setUndo(true);
    img->unlock();

    return img;
}

bool KisDoc2::newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * cs, const KoColor &bgColor, const QString &description, const double imgResolution)
{
    if (!init())
        return false;

    KisConfig cfg;

    quint8 opacity = OPACITY_OPAQUE;//bgColor.alpha();
    KisImageSP img;
    KisPaintLayerSP layer;

    if (!cs) return false;

    setUndo(false);

    img = new KisImage(m_d->undoAdapter, width, height, cs, name);

    Q_CHECK_PTR(img);
    img->lock();

    connect( img.data(), SIGNAL( sigImageModified() ), this, SLOT( slotImageUpdated() ));
    img->setResolution(imgResolution, imgResolution);
    img->setProfile(cs->profile());
    documentInfo()->setAboutInfo("title", name);
    documentInfo()->setAboutInfo("comments", description);


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
    img->addNode(layer.data(), img->rootLayer().data() );

    setCurrentImage( img );

    cfg.defImgWidth(width);
    cfg.defImgHeight(height);
    cfg.defImgResolution(imgResolution);

    setUndo(true);
    layer->setDirty();
    img->unlock();

    return true;
}

KoView* KisDoc2::createViewInstance(QWidget* parent)
{
    KisView2 * v = new KisView2(this, parent);
    Q_CHECK_PTR(v);

    m_d->shapeController->setInitialShapeForView( v );
    KoToolManager::instance()->switchToolRequested("KritaShape/KisToolBrush");
    return v;
}

void KisDoc2::paintContent(QPainter& painter, const QRect& rc)
{
    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();
    const KoColorProfile *  profile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
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

KoShapeControllerBase * KisDoc2::shapeController() const
{
    return m_d->shapeController;
}

KoShape * KisDoc2::shapeForNode( KisNodeSP layer ) const
{
    return m_d->shapeController->shapeForNode( layer );
}


KoShape * KisDoc2::addShape(const KisNodeSP node)
{
    m_d->shapeController->slotNodeAdded( const_cast<KisNode*>(node.data()), 0 );
    return m_d->shapeController->shapeForNode( node );
}

KisNodeModel * KisDoc2::nodeModel() const
{
    return m_d->nodeModel;
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

KisImageSP KisDoc2::image() const
{
    return m_d->image;
}


void KisDoc2::setCurrentImage(KisImageSP image)
{

    if ( m_d->image ) {
        // Disconnect existing sig/slot connections
        m_d->image->disconnect( this );
    }
    m_d->image = image;
    m_d->image->setUndoAdapter( m_d->undoAdapter );
    m_d->shapeController->setImage( image );
    m_d->nodeModel->setImage( image );
    setUndo(true);

    emit sigLoadingFinished();
}

void KisDoc2::initEmpty()
{
    KisConfig cfg;
    const KoColorSpace * rgb = KoColorSpaceRegistry::instance()->rgb8();
    newImage("", cfg.defImgWidth(), cfg.defImgHeight(), rgb);
}

KisUndoAdapter* KisDoc2::undoAdapter() const
{
    return m_d->undoAdapter;
}
#include "kis_doc2.moc"
