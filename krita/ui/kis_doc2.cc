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
#include "kis_doc2_p.h"

#include <QDesktopServices>
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
#include <QTimer>
#include <QScopedPointer>

// KDE
#include <krun.h>
#include <kimageio.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#include <kundo2stack.h>
#include <kstandarddirs.h>

// Calligra
#include <KoApplication.h>
#include <KoCanvasBase.h>
#include <KoColorProfile.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoID.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KoStore.h>
#include <KoXmlWriter.h>
#include <KoSelection.h>
#include <KoDocumentInfo.h>
#include <KoShape.h>
#include <KoToolManager.h>
#include <KoPart.h>

// Krita Image
#include <kis_config.h>
#include <flake/kis_shape_layer.h>
#include <kis_debug.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_name_server.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_fill_painter.h>
#include <kis_document_undo_store.h>
#include <kis_painting_assistants_decoration.h>

// Local
#include "kis_factory2.h"
#include "kis_view2.h"
#include "kis_clipboard.h"
#include "widgets/kis_custom_image_widget.h"
#include "canvas/kis_canvas2.h"
#include "flake/kis_shape_controller.h"
#include "kra/kis_kra_loader.h"
#include "kra/kis_kra_saver.h"
#include "kis_statusbar.h"
#include "widgets/kis_progress_widget.h"
#include "kis_canvas_resource_provider.h"
#include "kis_resource_server_provider.h"
#include "kis_node_manager.h"
#include "kis_part2.h"

static const char CURRENT_DTD_VERSION[] = "2.0";

class KisDoc2::KisDocPrivate
{

public:

    KisDocPrivate()
        : nserver(0)
        , macroNestDepth(0)
        , kraLoader(0)
    {
    }

    ~KisDocPrivate() {
        // Don't delete m_d->shapeController because it's in a QObject hierarchy.
        delete nserver;
    }

    KisNameServer *nserver;
    qint32 macroNestDepth;

    KisImageSP image;
    KisNodeSP preActivatedNode;
    KisShapeController* shapeController;

    KisKraLoader* kraLoader;
    KisKraSaver* kraSaver;

    QList<KisPaintingAssistant*> assistants;

    QString flipbook;
    QString animation;
};


KisDoc2::KisDoc2()
    : KoDocument(new KisPart2, new UndoStack(this))
    , m_d(new KisDocPrivate())
{
    qobject_cast<KisPart2*>(documentPart())->setDocument(this);
    // preload the krita resources
    KisResourceServerProvider::instance();

    init();
    connect(this, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    undoStack()->setUndoLimit(KisConfig().undoStackLimit());
    setBackupFile(KisConfig().backupFile());

}

KisDoc2::KisDoc2(KisPart2 *part)
    : KoDocument(part, new UndoStack(this))
    , m_d(new KisDocPrivate())
{
    Q_ASSERT(part);
    qobject_cast<KisPart2*>(documentPart())->setDocument(this);
    // preload the krita resources
    KisResourceServerProvider::instance();

    init();
    connect(this, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    undoStack()->setUndoLimit(KisConfig().undoStackLimit());
    setBackupFile(KisConfig().backupFile());

}



KisDoc2::~KisDoc2()
{
    // Despite being QObject they needs to be deleted before the image
    delete m_d->shapeController;

    if (m_d->image) {
        m_d->image->notifyAboutToBeDeleted();
    }
    // The following line trigger the deletion of the image
    m_d->image.clear();

    delete m_d;
}

QByteArray KisDoc2::mimeType() const
{
    return KIS_MIME_TYPE;
}

void KisDoc2::slotLoadingFinished() {
    if (m_d->image) {
        m_d->image->initialRefreshGraph();
    }
    setAutoSave(KisConfig().autoSaveInterval());
}

void KisDoc2::init()
{
    delete m_d->nserver;
    m_d->nserver = 0;

    m_d->nserver = new KisNameServer(1);
    Q_CHECK_PTR(m_d->nserver);

    m_d->shapeController = new KisShapeController(this, m_d->nserver);

    m_d->kraSaver = 0;
    m_d->kraLoader = 0;
}

bool KisDoc2::saveNativeFormat(const QString &file)
{
    const int realAutoSaveInterval = KisConfig().autoSaveInterval();
    const int emergencyAutoSaveInterval = 10; // sec

    if (!m_d->image->tryBarrierLock()) {
        if (isAutosaving()) {
            setDisregardAutosaveFailure(true);
            if (realAutoSaveInterval) {
                setAutoSave(emergencyAutoSaveInterval);
            }
            return false;
        } else {
            m_d->image->requestStrokeEnd();
            QApplication::processEvents();
            if (!m_d->image->tryBarrierLock()) {
                return false;
            }
        }
    }
    setDisregardAutosaveFailure(false);
    bool retval = KoDocument::saveNativeFormat(file);
    m_d->image->unlock();
    setAutoSave(realAutoSaveInterval);

    return retval;
}

QDomDocument KisDoc2::saveXML()
{
    dbgFile << url();
    QDomDocument doc = createDomDocument("DOC", CURRENT_DTD_VERSION);
    QDomElement root = doc.documentElement();

    root.setAttribute("editor", "Krita");
    root.setAttribute("syntaxVersion", "2");

    Q_ASSERT(m_d->kraSaver == 0);
    m_d->kraSaver = new KisKraSaver(this);

    root.appendChild(m_d->kraSaver->saveXML(doc, m_d->image));
    if (!m_d->kraSaver->errorMessages().isEmpty()) {
        setErrorMessage(m_d->kraSaver->errorMessages().join(".\n"));
    }

    return doc;
}

bool KisDoc2::loadOdf(KoOdfReadStore & odfStore)
{
    Q_UNUSED(odfStore);
    setErrorMessage(i18n("Krita does not support the OpenDocument file format."));
    return false;
}


bool KisDoc2::saveOdf(SavingContext &documentContext)
{
    Q_UNUSED(documentContext);
    setErrorMessage(i18n("Krita does not support the OpenDocument file format."));
    return false;
}

bool KisDoc2::loadXML(const KoXmlDocument& doc, KoStore */*store*/)
{
    if (m_d->image) {
        m_d->shapeController->setImage(0);
        m_d->image = 0;
    }

    KoXmlElement root;
    KoXmlNode node;
    KisImageWSP image;

    init();

    if (doc.doctype().name() != "DOC") {
        setErrorMessage(i18n("The format is not supported or the file is corrupted"));
        return false;
    }
    root = doc.documentElement();
    int syntaxVersion = root.attribute("syntaxVersion", "3").toInt();
    if (syntaxVersion > 2) {
         setErrorMessage(i18n("The file is too new for this version of Krita (%1).", syntaxVersion));
        return false;
    }

    if (!root.hasChildNodes()) {
        setErrorMessage(i18n("The file has no layers."));
        return false;
    }

    Q_ASSERT(m_d->kraLoader == 0);
    m_d->kraLoader = new KisKraLoader(this, syntaxVersion);

    // Legacy from the multi-image .kra file period.
    for (node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement()) {
            if (node.nodeName() == "IMAGE") {
                KoXmlElement elem = node.toElement();
                if (!(image = m_d->kraLoader->loadXML(elem))) {
                    if (m_d->kraLoader->errorMessages().isEmpty()) {
                        setErrorMessage(i18n("Unknown error."));
                    }
                    else {
                        setErrorMessage(m_d->kraLoader->errorMessages().join(".\n"));
                    }
                    return false;
                }

            }
            else {
                if (m_d->kraLoader->errorMessages().isEmpty()) {
                    setErrorMessage(i18n("The file does not contain an image."));
                }
                return false;
            }
        }
    }

    if (m_d->image) {
        // Disconnect existing sig/slot connections
        m_d->image->disconnect(this);
    }
    m_d->image = image;

    return true;
}

bool KisDoc2::completeSaving(KoStore *store)
{
    QString uri = url().url();

    m_d->kraSaver->saveBinaryData(store, m_d->image, url().url(), isStoredExtern(), isAutosaving());

    if (!m_d->kraSaver->errorMessages().isEmpty()) {
        setErrorMessage(m_d->kraSaver->errorMessages().join(".\n"));
        return false;
    }

    delete m_d->kraSaver;
    m_d->kraSaver = 0;

    return true;
}

int KisDoc2::supportedSpecialFormats() const
{
    return 0; // we don't support encryption.
}


bool KisDoc2::completeLoading(KoStore *store)
{
    if (!m_d->image) {
        if (m_d->kraLoader->errorMessages().isEmpty()) {
            setErrorMessage(i18n("Unknown error."));
        }
        else {
            setErrorMessage(m_d->kraLoader->errorMessages().join(".\n"));
        }
        return false;
    }

    m_d->kraLoader->loadBinaryData(store, m_d->image, url().url(), isStoredExtern());

    if (!m_d->kraLoader->errorMessages().isEmpty()) {
        setErrorMessage(m_d->kraLoader->errorMessages().join(".\n"));
        return false;
    }

    vKisNodeSP preselectedNodes = m_d->kraLoader->selectedNodes();
    if (preselectedNodes.size() > 0) {
        m_d->preActivatedNode = preselectedNodes.first();
    }

    // before deleting the kraloader, get the list with preloaded assistants and save it
    m_d->assistants = m_d->kraLoader->assistants();

    delete m_d->kraLoader;
    m_d->kraLoader = 0;

    m_d->shapeController->setImage(m_d->image);

    connect(m_d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));

    emit sigLoadingFinished();

    return true;
}

KisImageWSP KisDoc2::newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace* colorspace)
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

bool KisDoc2::newImage(const QString& name,
                       qint32 width, qint32 height,
                       const KoColorSpace* cs, const KoColor &bgColor,
                       const QString &description, const double imageResolution)
{
    Q_ASSERT(cs);

    init();

    KisConfig cfg;

    KisImageWSP image;
    KisPaintLayerSP layer;

    if (!cs) return false;

    qApp->setOverrideCursor(Qt::BusyCursor);

    image = new KisImage(createUndoStore(), width, height, cs, name);
    Q_CHECK_PTR(image);

    connect(image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    image->setResolution(imageResolution, imageResolution);

    image->assignImageProfile(cs->profile());
    documentInfo()->setAboutInfo("title", name);
    if (name != i18n("unnamed") && !name.isEmpty()) {
        setUrl(KUrl(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) + '/' + name + ".kra"));
    }
    documentInfo()->setAboutInfo("comments", description);

    layer = new KisPaintLayer(image.data(), image->nextLayerName(), bgColor.opacityU8(), cs);
    Q_CHECK_PTR(layer);

    layer->paintDevice()->setDefaultPixel(bgColor.data());
    image->addNode(layer.data(), image->rootLayer().data());
    setCurrentImage(image);

    cfg.defImageWidth(width);
    cfg.defImageHeight(height);
    cfg.defImageResolution(imageResolution);
    cfg.defColorModel(image->colorSpace()->colorModelId().id());
    cfg.setDefaultColorDepth(image->colorSpace()->colorDepthId().id());
    cfg.defColorProfile(image->colorSpace()->profile()->name());

    qApp->restoreOverrideCursor();
    return true;
}

void KisDoc2::paintContent(QPainter& painter, const QRect& rc)
{
    if (!m_d->image) return;
    KisConfig cfg;
    const KoColorProfile *profile = cfg.displayProfile();
    QRect rect = rc & m_d->image->bounds();
    m_d->image->renderToPainter(rect.left(), rect.left(), rect.top(), rect.height(), rect.width(), rect.height(), painter, profile);
}

QPixmap KisDoc2::generatePreview(const QSize& size)
{
    if (m_d->image) {
        QRect bounds = m_d->image->bounds();
        QSize newSize = bounds.size();
        newSize.scale(size, Qt::KeepAspectRatio);

        QImage image;
        if (bounds.width() < 10000 && bounds.height() < 10000) {
            image = m_d->image->convertToQImage(m_d->image->bounds(), 0);
        }
        else {
            image = m_d->image->convertToQImage(QRect(0, 0, newSize.width(), newSize.height()), newSize, 0);
        }
        image = image.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        return QPixmap::fromImage(image);
    }
    return QPixmap(size);
}

KoShapeBasedDocumentBase *KisDoc2::shapeController() const
{
    return m_d->shapeController;
}

KoShapeLayer* KisDoc2::shapeForNode(KisNodeSP layer) const
{
    return m_d->shapeController->shapeForNode(layer);
}

vKisNodeSP KisDoc2::activeNodes() const
{
    vKisNodeSP nodes;
    foreach(KoView *v, documentPart()->views()) {
        KisView2 *view = qobject_cast<KisView2*>(v);
        if (view) {
            KisNodeSP activeNode = view->activeNode();
            if (activeNode && !nodes.contains(activeNode)) {
                if (activeNode->inherits("KisMask")) {
                    activeNode = activeNode->parent();
                }
                nodes.append(activeNode);
            }
        }
    }
    return nodes;
}

QList<KisPaintingAssistant*> KisDoc2::assistants()
{
    QList<KisPaintingAssistant*> assistants;
    foreach(KoView *v, documentPart()->views()) {
        KisView2 *view = qobject_cast<KisView2*>(v);
        if (view) {
            KisPaintingAssistantsDecoration* assistantsDecoration = view->paintingAssistantsDecoration();
            assistants.append(assistantsDecoration->assistants());
        }
    }
    return assistants;
}

QList<KisPaintingAssistant *> KisDoc2::preLoadedAssistants()
{
    return m_d->assistants;
}

void KisDoc2::setPreActivatedNode(KisNodeSP activatedNode)
{
    m_d->preActivatedNode = activatedNode;
}

KisNodeSP KisDoc2::preActivatedNode() const
{
    return m_d->preActivatedNode;
}

void KisDoc2::prepareForImport()
{
    if (m_d->nserver == 0) {
        init();
    }
}

KisImageWSP KisDoc2::image() const
{
    return m_d->image;
}


void KisDoc2::setCurrentImage(KisImageWSP image)
{
    //if (!image.isValid()) return;

    if (m_d->image) {
        // Disconnect existing sig/slot connections
        m_d->image->disconnect(this);
        m_d->shapeController->setImage(0);
    }
    m_d->image = image;
    m_d->shapeController->setImage(image);

    setModified(false);

    connect(m_d->image, SIGNAL(sigImageModified()), this, SLOT(setImageModified()));

    emit sigLoadingFinished();
}

void KisDoc2::initEmpty()
{
    KisConfig cfg;
    const KoColorSpace * rgb = KoColorSpaceRegistry::instance()->rgb8();
    newImage("", cfg.defImageWidth(), cfg.defImageHeight(), rgb);
}

void KisDoc2::setImageModified()
{
    setModified(true);
}


KisUndoStore* KisDoc2::createUndoStore()
{
    return new KisDocumentUndoStore(this);
}

#include "kis_doc2.moc"

