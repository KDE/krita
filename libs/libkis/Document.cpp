/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Document.h"
#include <QPointer>
#include <QUrl>
#include <QDomDocument>

#include <KoColorSpaceConstants.h>
#include <KoXmlReader.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <KisPart.h>
#include <kis_paint_device.h>
#include <KisMainWindow.h>
#include <kis_node_manager.h>
#include <kis_node_selection_adapter.h>
#include <KisViewManager.h>
#include <kis_file_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_mask.h>
#include <kis_clone_layer.h>
#include <kis_group_layer.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include <kis_transparency_mask.h>
#include <kis_selection_mask.h>
#include <kis_effect_mask.h>
#include <kis_paint_layer.h>
#include <kis_generator_layer.h>
#include <kis_generator_registry.h>
#include <kis_shape_layer.h>
#include <kis_filter_configuration.h>
#include <kis_filter_registry.h>
#include <kis_selection.h>
#include <KisMimeDatabase.h>
#include <kis_filter_strategy.h>
#include <kis_guides_config.h>
#include <kis_coordinates_converter.h>
#include <kis_time_span.h>
#include <KisImportExportErrorCode.h>
#include <kis_types.h>
#include <kis_annotation.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorConversionTransformation.h>
#include <KoDocumentInfo.h>
#include <KisGlobalResourcesInterface.h>

#include <InfoObject.h>
#include <Node.h>
#include <Selection.h>
#include <LibKisUtils.h>

#include "kis_animation_importer.h"
#include <kis_canvas2.h>
#include <KoUpdater.h>
#include <QMessageBox>

#include <kis_image_animation_interface.h>

struct Document::Private {
    Private() {}
    QPointer<KisDocument> document;
    bool ownsDocument {false};
};

Document::Document(KisDocument *document, bool ownsDocument, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->document = document;
    d->ownsDocument = ownsDocument;
}

Document::~Document()
{
    if (d->ownsDocument && d->document) {
        KisPart::instance()->removeDocument(d->document);
        delete d->document;
    }
    delete d;
}

bool Document::operator==(const Document &other) const
{
    return (d->document == other.d->document);
}

bool Document::operator!=(const Document &other) const
{
    return !(operator==(other));
}

bool Document::batchmode() const
{
    if (!d->document) return false;
    return d->document->fileBatchMode();
}

void Document::setBatchmode(bool value)
{
    if (!d->document) return;
    d->document->setFileBatchMode(value);
}

Node *Document::activeNode() const
{
    QList<KisNodeSP> activeNodes;
    Q_FOREACH(QPointer<KisView> view, KisPart::instance()->views()) {
        if (view && view->document() == d->document) {
            activeNodes << view->currentNode();

        }
    }
    if (activeNodes.size() > 0) {
        QList<Node*> nodes = LibKisUtils::createNodeList(activeNodes, d->document->image());
        return nodes.first();
    }

    return 0;
}

void Document::setActiveNode(Node* value)
{
    if (!value->node()) return;
    KisMainWindow *mainWin = KisPart::instance()->currentMainwindow();
    if (!mainWin) return;
    KisViewManager *viewManager = mainWin->viewManager();
    if (!viewManager) return;
    if (viewManager->document() != d->document) return;
    KisNodeManager *nodeManager = viewManager->nodeManager();
    if (!nodeManager) return;
    KisNodeSelectionAdapter *selectionAdapter = nodeManager->nodeSelectionAdapter();
    if (!selectionAdapter) return;
    selectionAdapter->setActiveNode(value->node());

}

QList<Node *> Document::topLevelNodes() const
{
    if (!d->document) return QList<Node *>();
    Node n(d->document->image(), d->document->image()->rootLayer());
    return n.childNodes();
}


Node *Document::nodeByName(const QString &name) const
{
    if (!d->document) return 0;
    KisNodeSP node = d->document->image()->rootLayer()->findChildByName(name);
    if (node.isNull()) return 0;
    return Node::createNode(d->document->image(), node);
}

Node *Document::nodeByUniqueID(const QUuid &id) const
{
    if (!d->document) return 0;
    KisNodeSP node = d->document->image()->rootLayer()->findChildByUniqueID(id);
    if (node.isNull()) return 0;
    return Node::createNode(d->document->image(), node);
}


QString Document::colorDepth() const
{
    if (!d->document) return "";
    return d->document->image()->colorSpace()->colorDepthId().id();
}

QString Document::colorModel() const
{
    if (!d->document) return "";
    return d->document->image()->colorSpace()->colorModelId().id();
}

QString Document::colorProfile() const
{
    if (!d->document) return "";
    return d->document->image()->colorSpace()->profile()->name();
}

bool Document::setColorProfile(const QString &value)
{
    if (!d->document) return false;
    if (!d->document->image()) return false;
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileByName(value);
    if (!profile) return false;
    bool retval = d->document->image()->assignImageProfile(profile);
    d->document->image()->waitForDone();
    return retval;
}

bool Document::setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile)
{
    if (!d->document) return false;
    if (!d->document->image()) return false;
    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, colorProfile);
    if (!colorSpace) return false;

    d->document->image()->convertImageColorSpace(colorSpace,
                                                 KoColorConversionTransformation::IntentPerceptual,
                                                 KoColorConversionTransformation::HighQuality | KoColorConversionTransformation::NoOptimization);

    d->document->image()->waitForDone();
    return true;
}

QColor Document::backgroundColor()
{
    if (!d->document) return QColor();
    if (!d->document->image()) return QColor();

    const KoColor color = d->document->image()->defaultProjectionColor();
    return color.toQColor();
}

bool Document::setBackgroundColor(const QColor &color)
{
    if (!d->document) return false;
    if (!d->document->image()) return false;

    KoColor background = KoColor(color, d->document->image()->colorSpace());
    d->document->image()->setDefaultProjectionColor(background);

    d->document->image()->setModifiedWithoutUndo();
    d->document->image()->initialRefreshGraph();

    return true;
}

QString Document::documentInfo() const
{
    QDomDocument doc = KisDocument::createDomDocument("document-info"
                                                      /*DTD name*/, "document-info" /*tag name*/, "1.1");
    doc = d->document->documentInfo()->save(doc);
    return doc.toString();
}

void Document::setDocumentInfo(const QString &document)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    doc.setContent(document, &errorMsg, &errorLine, &errorColumn);
    d->document->documentInfo()->load(doc);
}


QString Document::fileName() const
{
    if (!d->document) return QString();
    return d->document->path();
}

void Document::setFileName(QString value)
{
    if (!d->document) return;
    QString mimeType = KisMimeDatabase::mimeTypeForFile(value, false);
    d->document->setMimeType(mimeType.toLatin1());
    d->document->setPath(value);
}

int Document::height() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;
    return image->height();
}

void Document::setHeight(int value)
{
    if (!d->document) return;
    if (!d->document->image()) return;
    resizeImage(d->document->image()->bounds().x(),
                d->document->image()->bounds().y(),
                d->document->image()->width(),
                value);
}


QString Document::name() const
{
    if (!d->document) return "";
    return d->document->documentInfo()->aboutInfo("title");
}

void Document::setName(QString value)
{
    if (!d->document) return;
    d->document->documentInfo()->setAboutInfo("title", value);
}


int Document::resolution() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;

    return qRound(d->document->image()->xRes() * 72);
}

void Document::setResolution(int value)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;

    KisFilterStrategy *strategy = KisFilterStrategyRegistry::instance()->get("Bicubic");
    KIS_SAFE_ASSERT_RECOVER_RETURN(strategy);

    image->scaleImage(image->size(), value / 72.0, value / 72.0, strategy);
    image->waitForDone();
}


Node *Document::rootNode() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;

    return Node::createNode(image, image->root());
}

Selection *Document::selection() const
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    if (!d->document->image()->globalSelection()) return 0;
    return new Selection(d->document->image()->globalSelection());
}

void Document::setSelection(Selection* value)
{
    if (!d->document) return;
    if (!d->document->image()) return;
    if (value) {
        d->document->image()->setGlobalSelection(value->selection());
    }
    else {
        d->document->image()->setGlobalSelection(0);
    }
}


int Document::width() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;
    return image->width();
}

void Document::setWidth(int value)
{
    if (!d->document) return;
    if (!d->document->image()) return;
    resizeImage(d->document->image()->bounds().x(),
                d->document->image()->bounds().y(),
                value,
                d->document->image()->height());
}


int Document::xOffset() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;
    return image->bounds().x();
}

void Document::setXOffset(int x)
{
    if (!d->document) return;
    if (!d->document->image()) return;
    resizeImage(x,
                d->document->image()->bounds().y(),
                d->document->image()->width(),
                d->document->image()->height());
}


int Document::yOffset() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;
    return image->bounds().y();
}

void Document::setYOffset(int y)
{
    if (!d->document) return;
    if (!d->document->image()) return;
    resizeImage(d->document->image()->bounds().x(),
                y,
                d->document->image()->width(),
                d->document->image()->height());
}


double Document::xRes() const
{
    if (!d->document) return 0.0;
    if (!d->document->image()) return 0.0;
    return d->document->image()->xRes()*72.0;
}

void Document::setXRes(double xRes) const
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;

    KisFilterStrategy *strategy = KisFilterStrategyRegistry::instance()->get("Bicubic");
    KIS_SAFE_ASSERT_RECOVER_RETURN(strategy);

    image->scaleImage(image->size(), xRes / 72.0, image->yRes(), strategy);
    image->waitForDone();
}

double Document::yRes() const
{
    if (!d->document) return 0.0;
    if (!d->document->image()) return 0.0;
    return d->document->image()->yRes()*72.0;
}

void Document::setYRes(double yRes) const
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;

    KisFilterStrategy *strategy = KisFilterStrategyRegistry::instance()->get("Bicubic");
    KIS_SAFE_ASSERT_RECOVER_RETURN(strategy);

    image->scaleImage(image->size(), image->xRes(), yRes / 72.0, strategy);
    image->waitForDone();
}


QByteArray Document::pixelData(int x, int y, int w, int h) const
{
    QByteArray ba;

    if (!d->document) return ba;
    KisImageSP image = d->document->image();
    if (!image) return ba;

    KisPaintDeviceSP dev = image->projection();
    ba.resize(w * h * dev->pixelSize());
    dev->readBytes(reinterpret_cast<quint8*>(ba.data()), x, y, w, h);
    return ba;
}

bool Document::close()
{
    bool retval = d->document->closePath(false);

    Q_FOREACH(KisView *view, KisPart::instance()->views()) {
        if (view->document() == d->document) {
            view->close();
            view->closeView();
            view->deleteLater();
        }
    }

    KisPart::instance()->removeDocument(d->document, !d->ownsDocument);

    if (d->ownsDocument) {

        delete d->document;
    }

    d->document = 0;
    return retval;
}

void Document::crop(int x, int y, int w, int h)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    QRect rc(x, y, w, h);
    image->cropImage(rc);
    image->waitForDone();
}

bool Document::exportImage(const QString &filename, const InfoObject &exportConfiguration)
{
    if (!d->document) return false;

    const QString outputFormatString = KisMimeDatabase::mimeTypeForFile(filename, false);
    const QByteArray outputFormat = outputFormatString.toLatin1();

    return d->document->exportDocumentSync(filename, outputFormat, exportConfiguration.configuration());
}

void Document::flatten()
{
    if (!d->document) return;
    if (!d->document->image()) return;
    d->document->image()->flatten(0);
    d->document->image()->waitForDone();
}

void Document::resizeImage(int x, int y, int w, int h)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    QRect rc;
    rc.setX(x);
    rc.setY(y);
    rc.setWidth(w);
    rc.setHeight(h);

    image->resizeImage(rc);
    image->waitForDone();
}

void Document::scaleImage(int w, int h, int xres, int yres, QString strategy)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    QRect rc = image->bounds();
    rc.setWidth(w);
    rc.setHeight(h);

    KisFilterStrategy *actualStrategy = KisFilterStrategyRegistry::instance()->get(strategy);
    if (!actualStrategy) actualStrategy = KisFilterStrategyRegistry::instance()->get("Bicubic");

    image->scaleImage(rc.size(), xres/72, yres/72, actualStrategy);
    image->waitForDone();
}

void Document::rotateImage(double radians)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    image->rotateImage(radians);
    image->waitForDone();
}

void Document::shearImage(double angleX, double angleY)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    image->shear(angleX, angleY);
    image->waitForDone();
}

bool Document::save()
{
    if (!d->document) return false;
    if (d->document->path().isEmpty()) return false;

    bool retval = d->document->save(true, 0);
    d->document->waitForSavingToComplete();

    return retval;
}

bool Document::saveAs(const QString &filename)
{
    if (!d->document) return false;

    const QString outputFormatString = KisMimeDatabase::mimeTypeForFile(filename, false);
    const QByteArray outputFormat = outputFormatString.toLatin1();
    QString oldPath = d->document->path();
    d->document->setPath(filename);
    bool retval = d->document->saveAs(filename, outputFormat, true);
    d->document->waitForSavingToComplete();
    d->document->setPath(oldPath);

    return retval;
}

Node* Document::createNode(const QString &name, const QString &nodeType)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();

    Node *node = 0;

    if (nodeType.toLower()== "paintlayer") {
        node = new Node(image, new KisPaintLayer(image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType.toLower()  == "grouplayer") {
        node = new Node(image, new KisGroupLayer(image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType.toLower()  == "filelayer") {
        node = new Node(image, new KisFileLayer(image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType.toLower()  == "filterlayer") {
        node = new Node(image, new KisAdjustmentLayer(image, name, 0, 0));
    }
    else if (nodeType.toLower()  == "filllayer") {
        node = new Node(image, new KisGeneratorLayer(image, name, 0, 0));
    }
    else if (nodeType.toLower()  == "clonelayer") {
        node = new Node(image, new KisCloneLayer(0, image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType.toLower()  == "vectorlayer") {
        node = new Node(image, new KisShapeLayer(d->document->shapeController(), image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType.toLower()  == "transparencymask") {
        node = new Node(image, new KisTransparencyMask(image, name));
    }
    else if (nodeType.toLower()  == "filtermask") {
        node = new Node(image, new KisFilterMask(image, name));
    }
    else if (nodeType.toLower()  == "transformmask") {
        node = new Node(image, new KisTransformMask(image, name));
    }
    else if (nodeType.toLower()  == "selectionmask") {
        node = new Node(image, new KisSelectionMask(image, name));
    }

    return node;
}

GroupLayer *Document::createGroupLayer(const QString &name)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();

    return new GroupLayer(image, name);
}

FileLayer *Document::createFileLayer(const QString &name, const QString fileName, const QString scalingMethod)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();

    return new FileLayer(image, name, this->fileName(), fileName, scalingMethod);
}

FilterLayer *Document::createFilterLayer(const QString &name, Filter &filter, Selection &selection)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();

    return new FilterLayer(image, name, filter, selection);
}

FillLayer *Document::createFillLayer(const QString &name, const QString generatorName, InfoObject &configuration, Selection &selection)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();

    KisGeneratorSP generator = KisGeneratorRegistry::instance()->value(generatorName);
    if (generator) {

        KisFilterConfigurationSP config = generator->factoryConfiguration(KisGlobalResourcesInterface::instance());
        Q_FOREACH(const QString property, configuration.properties().keys()) {
            config->setProperty(property, configuration.property(property));
        }

        return new FillLayer(image, name, config, selection);
    }
    return 0;
}

CloneLayer *Document::createCloneLayer(const QString &name, const Node *source)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();
    KisLayerSP layer = qobject_cast<KisLayer*>(source->node().data());

    return new CloneLayer(image, name, layer);
}

VectorLayer *Document::createVectorLayer(const QString &name)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();

    return new VectorLayer(d->document->shapeController(), image, name);
}

FilterMask *Document::createFilterMask(const QString &name, Filter &filter, const Node *selection_source)
{
    if (!d->document)
        return 0;

    if (!d->document->image())
        return 0;

    if(!selection_source)
        return 0;

    KisLayerSP layer = qobject_cast<KisLayer*>(selection_source->node().data());
    if(layer.isNull())
        return 0;

    KisImageSP image = d->document->image();
    FilterMask* mask = new FilterMask(image, name, filter);
    qobject_cast<KisMask*>(mask->node().data())->initSelection(layer);

    return mask;
}

FilterMask *Document::createFilterMask(const QString &name, Filter &filter, Selection &selection)
{
    if (!d->document)
        return 0;

    if (!d->document->image())
        return 0;

    KisImageSP image = d->document->image();
    FilterMask* mask = new FilterMask(image, name, filter);
    qobject_cast<KisMask*>(mask->node().data())->setSelection(selection.selection());

    return mask;
}

SelectionMask *Document::createSelectionMask(const QString &name)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();

    return new SelectionMask(image, name);
}

QImage Document::projection(int x, int y, int w, int h) const
{
    if (!d->document || !d->document->image()) return QImage();
    return d->document->image()->convertToQImage(x, y, w, h, 0);
}

QImage Document::thumbnail(int w, int h) const
{
    if (!d->document || !d->document->image()) return QImage();
    return d->document->generatePreview(QSize(w, h)).toImage();
}


void Document::lock()
{
    if (!d->document || !d->document->image()) return;
    d->document->image()->barrierLock();
}

void Document::unlock()
{
    if (!d->document || !d->document->image()) return;
    d->document->image()->unlock();
}

void Document::waitForDone()
{
    if (!d->document || !d->document->image()) return;
    d->document->image()->waitForDone();
}

bool Document::tryBarrierLock()
{
    if (!d->document || !d->document->image()) return false;
    return d->document->image()->tryBarrierLock();
}

void Document::refreshProjection()
{
    if (!d->document || !d->document->image()) return;
    d->document->image()->refreshGraph();
}

QList<qreal> Document::horizontalGuides() const
{
    QList<qreal> lines;
    if (!d->document || !d->document->image()) return lines;
    KisCoordinatesConverter converter;
    converter.setImage(d->document->image());
    QTransform transform = converter.imageToDocumentTransform().inverted();
    QList<qreal> untransformedLines = d->document->guidesConfig().horizontalGuideLines();
    for (int i = 0; i< untransformedLines.size(); i++) {
        qreal line = untransformedLines[i];
        lines.append(transform.map(QPointF(line, line)).x());
    }
    return lines;
}

QList<qreal> Document::verticalGuides() const
{
    QList<qreal> lines;
    if (!d->document || !d->document->image()) return lines;
    KisCoordinatesConverter converter;
    converter.setImage(d->document->image());
    QTransform transform = converter.imageToDocumentTransform().inverted();
    QList<qreal> untransformedLines = d->document->guidesConfig().verticalGuideLines();
    for (int i = 0; i< untransformedLines.size(); i++) {
        qreal line = untransformedLines[i];
        lines.append(transform.map(QPointF(line, line)).y());
    }
    return lines;
}

bool Document::guidesVisible() const
{
    return d->document->guidesConfig().showGuides();
}

bool Document::guidesLocked() const
{
    return d->document->guidesConfig().lockGuides();
}

Document *Document::clone() const
{
    if (!d->document) return 0;
    QPointer<KisDocument> clone = d->document->clone();
    Document * newDocument = new Document(clone, d->ownsDocument);
    clone->setParent(newDocument); // It's owned by the document, not KisPart
    return newDocument;
}

void Document::setHorizontalGuides(const QList<qreal> &lines)
{
    if (!d->document) return;
    KisGuidesConfig config = d->document->guidesConfig();
    KisCoordinatesConverter converter;
    converter.setImage(d->document->image());
    QTransform transform = converter.imageToDocumentTransform();
    QList<qreal> transformedLines;
    for (int i = 0; i< lines.size(); i++) {
        qreal line = lines[i];
        transformedLines.append(transform.map(QPointF(line, line)).x());
    }
    config.setHorizontalGuideLines(transformedLines);
    d->document->setGuidesConfig(config);
}

void Document::setVerticalGuides(const QList<qreal> &lines)
{
    if (!d->document) return;
    KisGuidesConfig config = d->document->guidesConfig();
    KisCoordinatesConverter converter;
    converter.setImage(d->document->image());
    QTransform transform = converter.imageToDocumentTransform();
    QList<qreal> transformedLines;
    for (int i = 0; i< lines.size(); i++) {
        qreal line = lines[i];
        transformedLines.append(transform.map(QPointF(line, line)).y());
    }
    config.setVerticalGuideLines(transformedLines);
    d->document->setGuidesConfig(config);
}

void Document::setGuidesVisible(bool visible)
{
    if (!d->document) return;
    KisGuidesConfig config = d->document->guidesConfig();
    config.setShowGuides(visible);
    d->document->setGuidesConfig(config);
}

void Document::setGuidesLocked(bool locked)
{
    if (!d->document) return;
    KisGuidesConfig config = d->document->guidesConfig();
    config.setLockGuides(locked);
    d->document->setGuidesConfig(config);
}

bool Document::modified() const
{
    if (!d->document) return false;
    return d->document->isModified();
}

QRect Document::bounds() const
{
    if (!d->document) return QRect();
    return d->document->image()->bounds();
}

QPointer<KisDocument> Document::document() const
{
    return d->document;
}

void Document::setOwnsDocument(bool ownsDocument)
{
    d->ownsDocument = ownsDocument;
}

/* Animation related function */

bool Document::importAnimation(const QList<QString> &files, int firstFrame, int step)
{
    KisView *activeView = KisPart::instance()->currentMainwindow()->activeView();

    KoUpdaterPtr updater = 0;
    if (activeView && d->document->fileBatchMode()) {
         updater = activeView->viewManager()->createUnthreadedUpdater(i18n("Import frames"));
    }

    KisAnimationImporter importer(d->document->image(), updater);
    KisImportExportErrorCode status = importer.import(files, firstFrame, step);

    return status.isOk();
}

int Document::framesPerSecond()
{
    if (!d->document) return false;
    if (!d->document->image()) return false;

    return d->document->image()->animationInterface()->framerate();
}

void Document::setFramesPerSecond(int fps)
{
    if (!d->document) return;
    if (!d->document->image()) return;

    d->document->image()->animationInterface()->setFramerate(fps);
}

void Document::setFullClipRangeStartTime(int startTime)
{
    if (!d->document) return;
    if (!d->document->image()) return;

    d->document->image()->animationInterface()->setFullClipRangeStartTime(startTime);
}


int Document::fullClipRangeStartTime()
{
    if (!d->document) return false;
    if (!d->document->image()) return false;

    return d->document->image()->animationInterface()->fullClipRange().start();
}


void Document::setFullClipRangeEndTime(int endTime)
{
    if (!d->document) return;
    if (!d->document->image()) return;

    d->document->image()->animationInterface()->setFullClipRangeEndTime(endTime);
}


int Document::fullClipRangeEndTime()
{
    if (!d->document) return false;
    if (!d->document->image()) return false;

    return d->document->image()->animationInterface()->fullClipRange().end();
}

int Document::animationLength()
{
    if (!d->document) return false;
    if (!d->document->image()) return false;

    return d->document->image()->animationInterface()->totalLength();
}

void Document::setPlayBackRange(int start, int stop)
{
    if (!d->document) return;
    if (!d->document->image()) return;

    const KisTimeSpan newTimeRange = KisTimeSpan::fromTimeWithDuration(start, (stop-start));
    d->document->image()->animationInterface()->setPlaybackRange(newTimeRange);
}

int Document::playBackStartTime()
{
    if (!d->document) return false;
    if (!d->document->image()) return false;

    return d->document->image()->animationInterface()->playbackRange().start();
}

int Document::playBackEndTime()
{
    if (!d->document) return false;
    if (!d->document->image()) return false;

    return d->document->image()->animationInterface()->playbackRange().end();
}

int Document::currentTime()
{
    if (!d->document) return false;
    if (!d->document->image()) return false;

    return d->document->image()->animationInterface()->currentTime();
}

void Document::setCurrentTime(int time)
{
    if (!d->document) return;
    if (!d->document->image()) return;

    return d->document->image()->animationInterface()->requestTimeSwitchWithUndo(time);
}

QStringList Document::annotationTypes() const
{
    if (!d->document) return QStringList();

    QStringList types;

    KisImageSP image = d->document->image().toStrongRef();

    if (!image) return QStringList();

    vKisAnnotationSP_it beginIt = image->beginAnnotations();
    vKisAnnotationSP_it endIt = image->endAnnotations();

    vKisAnnotationSP_it it = beginIt;
    while (it != endIt) {
        if (!(*it) || (*it)->type().isEmpty()) {
            qWarning() << "Warning: empty annotation";
            it++;
            continue;
        }
        types << (*it)->type();

        it++;
    }
    return types;
}

QString Document::annotationDescription(const QString &type) const
{
    KisImageSP image = d->document->image().toStrongRef();
    KisAnnotationSP annotation = image->annotation(type);
    return annotation->description();
}

QByteArray Document::annotation(const QString &type)
{
    KisImageSP image = d->document->image().toStrongRef();
    KisAnnotationSP annotation = image->annotation(type);
    if (annotation) {
        return annotation->annotation();
    }
    else {
        return QByteArray();
    }
}

void Document::setAnnotation(const QString &key, const QString &description, const QByteArray &annotation)
{
    KisAnnotation *a = new KisAnnotation(key, description, annotation);
    KisImageSP image = d->document->image().toStrongRef();
    image->addAnnotation(a);

}

void Document::removeAnnotation(const QString &type)
{
    KisImageSP image = d->document->image().toStrongRef();
    image->removeAnnotation(type);
}
