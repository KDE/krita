/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2000 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2001 Toshitaka Fujioka  <fujioka@kde.org>
 *  Copyright (c) 2002, 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2009 Boudewijn Rempt <boud@valdyas.org>
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


#include <QApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QRect>
#include <QSize>
#include <QPainter>
#include <QStringList>
#include <QWidget>
#include <QList>

// KDE
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
#include <KoID.h>
#include <KoMainWindow.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoSelection.h>
#include <KoDocumentInfo.h>
#include <KoShape.h>
#include <KoToolManager.h>
#include <KoUndoStack.h>

// Krita Image
#include <flake/kis_shape_layer.h>
#include <kis_debug.h>
#include <kis_fill_painter.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_name_server.h>
#include <kis_paint_device_action.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>

// Local
#include "kis_factory2.h"
#include "kis_view2.h"
#include "kis_clipboard.h"
#include "kis_config.h"
#include "widgets/kis_custom_image_widget.h"
#include "canvas/kis_canvas2.h"
#include "kis_undo_adapter.h"
#include "flake/kis_shape_controller.h"
#include "kis_node_model.h"
#include "kra/kis_kra_loader.h"
#include "kra/kis_kra_saver.h"
#include "kis_statusbar.h"
#include "widgets/kis_progress_widget.h"


static const char *CURRENT_DTD_VERSION = "2.0";

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
            : undoAdapter(0)
            , nserver(0)
            , macroNestDepth(0)
            , ioProgressTotalSteps(0)
            , ioProgressBase(0)
            , kraLoader(0) {
    }

    ~KisDocPrivate() {
        // Don't delete m_d->shapeController or m_d->nodeModel because it's in a QObject hierarchy.
        //delete undoAdapter;
        delete nserver;
    }

    KisUndoAdapter *undoAdapter;
    KisNameServer *nserver;
    qint32 macroNestDepth;
    int ioProgressTotalSteps;
    int ioProgressBase;

    KisImageSP image;
    KisShapeController* shapeController;
    KisNodeModel* nodeModel;

    KisKraLoader* kraLoader;
    KisKraSaver* kraSaver;

};


KisDoc2::KisDoc2(QWidget *parentWidget, QObject *parent, bool singleViewMode)
        : KoDocument(parentWidget, parent, singleViewMode)
        , m_d(new KisDocPrivate())
{

    setComponentData(KisFactory2::componentData(), false);
    setTemplateType("krita_template");
    init();

}

KisDoc2::~KisDoc2()
{
    m_d->image.clear();

    delete m_d;
}

QByteArray KisDoc2::mimeType() const
{
    return APP_MIMETYPE;
}

void KisDoc2::openExistingFile(const KUrl& url)
{
    setUndo(false);
    qApp->setOverrideCursor(Qt::BusyCursor);
    KoDocument::openExistingFile(url);
    qApp->restoreOverrideCursor();
    setUndo(true);
}

void KisDoc2::openTemplate(const KUrl& url)
{
    setUndo(false);
    qApp->setOverrideCursor(Qt::BusyCursor);
    KoDocument::openTemplate(url);
    qApp->restoreOverrideCursor();
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
    connect(undoStack(), SIGNAL(indexChanged(int)), SLOT(undoIndexChanged(int)));
    Q_CHECK_PTR(m_d->undoAdapter);

    setUndo(true);

    m_d->nserver = new KisNameServer(1);
    Q_CHECK_PTR(m_d->nserver);

    m_d->shapeController = new KisShapeController(this, m_d->nserver);
    m_d->nodeModel = new KisNodeModel(this);

    m_d->kraSaver = 0;
    m_d->kraLoader = 0;

    return true;
}

QDomDocument KisDoc2::saveXML()
{
    QDomDocument doc = createDomDocument("DOC", CURRENT_DTD_VERSION);
    QDomElement root = doc.documentElement();

    root.setAttribute("editor", "Krita");
    root.setAttribute("syntaxVersion", "2");

    Q_ASSERT(m_d->kraSaver == 0);
    m_d->kraSaver = new KisKraSaver(this);

    root.appendChild(m_d->kraSaver->saveXML(doc, m_d->image));

    return doc;
}

bool KisDoc2::loadOdf(KoOdfReadStore & odfStore)
{
    Q_UNUSED(odfStore);
    return false;
}


bool KisDoc2::saveOdf(SavingContext &documentContext)
{
    Q_UNUSED(documentContext);
    return false;
}

bool KisDoc2::loadXML(const KoXmlDocument& doc, KoStore *)
{
    KoXmlElement root;
    QString attr;
    KoXmlNode node;
    KisImageWSP img;

    if (!init())
        return false;
    if (doc.doctype().name() != "DOC")
        return false;
    root = doc.documentElement();
    int syntaxVersion = root.attribute("syntaxVersion", "3").toInt();
    if (syntaxVersion > 2)
        return false;

    if (!root.hasChildNodes()) {
        return false;
    }

    setUndo(false);
    Q_ASSERT(m_d->kraLoader == 0);
    m_d->kraLoader = new KisKraLoader(this, syntaxVersion);

    // Legacy from the multi-image .kra file period.
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

    if (m_d->image) {
        // Disconnect existing sig/slot connections
        m_d->image->disconnect(this);
    }
    m_d->image = img;

    return true;
}

bool KisDoc2::completeSaving(KoStore *store)
{
    QString uri = url().url();

    setIOSteps(m_d->image->nlayers() + 1);

    m_d->kraSaver->saveBinaryData(store, m_d->image, url().url(), isStoredExtern());

    delete m_d->kraSaver;
    m_d->kraSaver = 0;

    IODone();
    return true;
}


bool KisDoc2::completeLoading(KoStore *store)
{
    if (!m_d->image) return false;

    setIOSteps(m_d->image->nlayers());

    m_d->kraLoader->loadBinaryData(store, m_d->image, url().url(), isStoredExtern());

    IODone();

    setModified(false);
    setUndo(true);

    delete m_d->kraLoader;
    m_d->kraLoader = 0;

    m_d->image->setUndoAdapter(m_d->undoAdapter);
    m_d->shapeController->setImage(m_d->image);
    m_d->nodeModel->setImage(m_d->image);
    setUndo(true);

    connect(m_d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setModified()));



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
    item.widget = new KisCustomImageWidget(parent, this, w, h, clipAvailable, cfg.defImgResolution(), cfg.workingColorSpace(), "unnamed");
    widgetList << item;

    return widgetList;
}



KisImageWSP KisDoc2::newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * colorspace)
{
    KoColor backgroundColor(Qt::white, colorspace);

    /**
     * FIXME: check whether this is a good value
     */
    double defaultResolution=1.;

    newImage(name, width, height, colorspace, backgroundColor, "",
             defaultResolution);
    return image();
}

bool KisDoc2::newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * cs, const KoColor &bgColor, const QString &description, const double imgResolution)
{
    if (!init())
        return false;

    KisConfig cfg;

    quint8 opacity = OPACITY_OPAQUE;//bgColor.alpha();
    KisImageWSP img;
    KisPaintLayerSP layer;

    if (!cs) return false;

    qApp->setOverrideCursor(Qt::BusyCursor);

    setUndo(false);

    img = new KisImage(m_d->undoAdapter, width, height, cs, name);

    Q_CHECK_PTR(img);
    img->lock();

    connect(img.data(), SIGNAL(sigImageModified()), this, SLOT(setModified()));
    img->setResolution(imgResolution, imgResolution);
    img->setProfile(cs->profile());
    documentInfo()->setAboutInfo("title", name);
    documentInfo()->setAboutInfo("comments", description);

    layer = new KisPaintLayer(img.data(), img->nextLayerName(), OPACITY_OPAQUE, cs);
    Q_CHECK_PTR(layer);

    KisFillPainter painter;
    painter.begin(layer->paintDevice());
    painter.beginTransaction("");
    painter.fillRect(0, 0, width, height, bgColor, opacity);
    painter.end();

    img->addNode(layer.data(), img->rootLayer().data());

    setCurrentImage(img);

    cfg.defImgWidth(width);
    cfg.defImgHeight(height);
    cfg.defImgResolution(imgResolution);

    setUndo(true);
    img->unlock();

    qApp->restoreOverrideCursor();
    return true;
}

KoView* KisDoc2::createViewInstance(QWidget* parent)
{
    qApp->setOverrideCursor(Qt::WaitCursor);
    KisView2 * v = new KisView2(this, parent);
    Q_CHECK_PTR(v);

    m_d->shapeController->setInitialShapeForView(v);
    KoToolManager::instance()->switchToolRequested("KritaShape/KisToolBrush");

    // XXX: this prevents a crash when opening a new document after opening a
    // a document that has not been touched! I have no clue why, though.
    // see: https://bugs.kde.org/show_bug.cgi?id=208239.
    setModified(true);
    setModified(false);
    qApp->restoreOverrideCursor();
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

QPixmap KisDoc2::generatePreview(const QSize& size)
{
    if (m_d->image) {
        QSize newSize = m_d->image->bounds().size();
        newSize.scale(size, Qt::KeepAspectRatio);

        QImage img = m_d->image->convertToQImage(QRect(0, 0, newSize.width(), newSize.height()), newSize, 0);
        img.save("thumb.png");
        return QPixmap::fromImage(img);
    }
    return QPixmap(size);
}

void KisDoc2::setUndo(bool undo)
{
    m_d->undoAdapter->setUndo(undo);
}

bool KisDoc2::undo() const
{
    return m_d->undoAdapter->undo();
}

KoShapeControllerBase * KisDoc2::shapeController() const
{
    return m_d->shapeController;
}

KoShape * KisDoc2::shapeForNode(KisNodeSP layer) const
{
    return m_d->shapeController->shapeForNode(layer);
}

KoShape * KisDoc2::addShape(const KisNodeSP node)
{
    KisNodeSP parent = node->parent();
    m_d->shapeController->slotNodeAdded(parent.data(), parent->index(node));
    return m_d->shapeController->shapeForNode(node);
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

void KisDoc2::prepareForImport()
{
    if (m_d->nserver == 0)
        init();
    setUndo(false);
}

KisImageWSP KisDoc2::image() const
{
    return m_d->image;
}


void KisDoc2::setCurrentImage(KisImageWSP image)
{
    if (m_d->image) {
        // Disconnect existing sig/slot connections
        m_d->image->disconnect(this);
    }
    m_d->image = image;
    m_d->image->setUndoAdapter(m_d->undoAdapter);
    m_d->shapeController->setImage(image);
    m_d->nodeModel->setImage(image);
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

void KisDoc2::undoIndexChanged(int idx)
{
    const QUndoCommand* command = undoStack()->command(idx);
    if (command) {
        m_d->undoAdapter->notifyCommandExecuted(undoStack()->command(idx));
    } else {
        kWarning() << "trying to clear undo stack to index" << idx << ": no command at that index";
    }
}


#include "kis_doc2.moc"

