/*
 * SPDX-FileCopyrightText: 2017 Jouni Pentikäinen <joupent@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KoShapeCreateCommand.h>
#include <KoShapeDeleteCommand.h>
#include <KoKeepShapesSelectedCommand.h>
#include <KoSelection.h>
#include <kis_node_visitor.h>
#include <kis_processing_visitor.h>
#include <kis_shape_layer_canvas.h>

#include "KisReferenceImagesLayer.h"
#include "KisReferenceImage.h"
#include "KisDocument.h"
#include "kis_canvas2.h"
#include "KisHandlePainterHelper.h"
#include "KisFileSystemWatcherWrapper.h"
#include "KoStore.h"

struct AddReferenceImagesCommand : KoShapeCreateCommand
{
    AddReferenceImagesCommand(KisDocument *document, KisSharedPtr<KisReferenceImagesLayer> layer, const QList<KoShape*> referenceImages, KUndo2Command *parent = nullptr)
        : KoShapeCreateCommand(layer->shapeController(), referenceImages, layer.data(), parent, kundo2_i18n("Add reference image"))
        , m_document(document)
        , m_layer(layer)
    {}

    void redo() override {
        auto layer = m_document->referenceImagesLayer();
        KIS_SAFE_ASSERT_RECOVER_NOOP(!layer || layer == m_layer);

        if (!layer) {
            m_document->setReferenceImagesLayer(m_layer, true);
        }

        KoShapeCreateCommand::redo();
    }

    void undo() override {
        KoShapeCreateCommand::undo();

        if (m_layer->shapeCount() == 0) {
            m_document->setReferenceImagesLayer(nullptr, true);
        }
    }

private:
    KisDocument *m_document;
    KisSharedPtr<KisReferenceImagesLayer> m_layer;
};

struct RemoveReferenceImagesCommand : KoShapeDeleteCommand
{
    RemoveReferenceImagesCommand(KisDocument *document, KisSharedPtr<KisReferenceImagesLayer> layer, QList<KoShape *> referenceImages, KUndo2Command *parent = nullptr)
        : KoShapeDeleteCommand(layer->shapeController(), referenceImages, parent)
        , m_document(document)
        , m_layer(layer)
    {}


    void redo() override {
        KoShapeDeleteCommand::redo();

        if (m_layer->shapeCount() == 0) {
            m_document->setReferenceImagesLayer(nullptr, true);
        }
    }

    void undo() override {
        auto layer = m_document->referenceImagesLayer();
        KIS_SAFE_ASSERT_RECOVER_NOOP(!layer || layer == m_layer);

        if (!layer) {
            m_document->setReferenceImagesLayer(m_layer, true);
        }

        KoShapeDeleteCommand::undo();
    }

private:
    KisDocument *m_document;
    KisSharedPtr<KisReferenceImagesLayer> m_layer;
};

class ReferenceImagesCanvas : public KisShapeLayerCanvasBase
{
public:
    ReferenceImagesCanvas(KisReferenceImagesLayer *parent, KisImageWSP image)
        : KisShapeLayerCanvasBase(parent, image)
        , m_layer(parent)
    {}

    void updateCanvas(const QRectF &rect) override
    {
        if (!m_layer->image() || m_isDestroying) {
            return;
        }

        QRectF r = m_viewConverter->documentToView(rect);
        m_layer->signalUpdate(r);
    }

    void forceRepaint() override
    {
        m_layer->signalUpdate(m_layer->boundingImageRect());
    }

    bool hasPendingUpdates() const override
    {
        return false;
    }

    void rerenderAfterBeingInvisible() override {}
    void resetCache() override {}
    void setImage(KisImageWSP image) override
    {
        m_viewConverter->setImage(image);
    }

private:
    KisReferenceImagesLayer *m_layer;
};

Q_GLOBAL_STATIC(KisFileSystemWatcherWrapper, m_fileSystemWatcher)

KisReferenceImagesLayer::KisReferenceImagesLayer(KoShapeControllerBase* shapeController, KisImageWSP image)
    : KisShapeLayer(shapeController, image, i18n("Reference images"), OPACITY_OPAQUE_U8, new ReferenceImagesCanvas(this, image))
    , m_pinRotate(false)
    , m_pinMirror(false)
    , m_pinPosition(false)
    , m_pinZoom(false)
    , m_mirrorX(false)
    , m_mirrorY(false)
    , m_initialValues(false)
    , m_previousAngle(0)
    , m_transform(QTransform())

{
    //use signal compressor here
    connect(m_fileSystemWatcher, SIGNAL(fileChanged(QString)), SLOT(fileChanged(QString)));
}

KisReferenceImagesLayer::KisReferenceImagesLayer(const KisReferenceImagesLayer &rhs)
    : KisShapeLayer(rhs, rhs.shapeController(), new ReferenceImagesCanvas(this, rhs.image()))
    , m_pinRotate(false)
    , m_pinMirror(false)
    , m_pinPosition(false)
    , m_pinZoom(false)
    , m_mirrorX(false)
    , m_mirrorY(false)
    , m_initialValues(false)
    , m_previousAngle(0)
    , m_transform(QTransform())
{}

KUndo2Command * KisReferenceImagesLayer::addReferenceImages(KisDocument *document, const QList<KoShape*> referenceImages)
{
    KisSharedPtr<KisReferenceImagesLayer> layer = document->referenceImagesLayer();
    if (!layer) {
        layer = new KisReferenceImagesLayer(document->shapeController(), document->image());
    }

    KUndo2Command *parentCommand = new KUndo2Command();

    new KoKeepShapesSelectedCommand(layer->shapeManager()->selection()->selectedShapes(), {}, layer->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::INITIALIZING, parentCommand);
    AddReferenceImagesCommand *cmd = new AddReferenceImagesCommand(document, layer, referenceImages, parentCommand);
    parentCommand->setText(cmd->text());
    new KoKeepShapesSelectedCommand({}, referenceImages, layer->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::FINALIZING, parentCommand);

    Q_FOREACH(KoShape *shape, referenceImages) {
        KisReferenceImage *ref = dynamic_cast<KisReferenceImage*>(shape);
        m_fileSystemWatcher->addPath(ref->filename());
    }
    return parentCommand;
}

KUndo2Command * KisReferenceImagesLayer::removeReferenceImages(KisDocument *document, QList<KoShape*> referenceImages)
{
    Q_FOREACH(KoShape *shape, referenceImages) {
        KisReferenceImage *ref = dynamic_cast<KisReferenceImage*>(shape);
        m_fileSystemWatcher->removePath(ref->filename());
    }
    return new RemoveReferenceImagesCommand(document, this, referenceImages);
}

QVector<KisReferenceImage*> KisReferenceImagesLayer::referenceImages() const
{
    QVector<KisReferenceImage*> references;

    Q_FOREACH(auto shape, shapes()) {
        KisReferenceImage *referenceImage = dynamic_cast<KisReferenceImage*>(shape);
        if (referenceImage) {
            references.append(referenceImage);
        }
    }
    return references;
}

void KisReferenceImagesLayer::paintReferences(QPainter &painter)
{
    //painter.setTransform(converter()->documentToView(), true);
    shapeManager()->paint(painter, false);
}

bool KisReferenceImagesLayer::allowAsChild(KisNodeSP) const
{
    return false;
}

bool KisReferenceImagesLayer::accept(KisNodeVisitor &visitor)
{
    return visitor.visit(this);
}

void KisReferenceImagesLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    visitor.visit(this, undoAdapter);
}

bool KisReferenceImagesLayer::isFakeNode() const
{
    return true;
}

KUndo2Command *KisReferenceImagesLayer::setProfile(const KoColorProfile *profile)
{
    // references should not be converted with the image
    Q_UNUSED(profile);
    return 0;
}

KUndo2Command *KisReferenceImagesLayer::convertTo(const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    // references should not be converted with the image
    Q_UNUSED(dstColorSpace);
    Q_UNUSED(renderingIntent);
    Q_UNUSED(conversionFlags);
    return 0;
}

void KisReferenceImagesLayer::signalUpdate(const QRectF &rect)
{
    emit sigUpdateCanvas(rect);
}

QRectF KisReferenceImagesLayer::boundingImageRect() const
{
    return converter()->documentToView(boundingRect());
}

QColor KisReferenceImagesLayer::getPixel(QPointF position) const
{
    const QPointF docPoint = converter()->viewToDocument(position);

    KoShape *shape = shapeManager()->shapeAt(docPoint);

    if (shape) {
        auto *reference = dynamic_cast<KisReferenceImage*>(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(reference, QColor());

        return reference->getPixel(docPoint);
    }

    return QColor();
}

void KisReferenceImagesLayer::fileChanged(QString path)
{
    Q_FOREACH(KisReferenceImage *ref, referenceImages()) {
        if(ref->filename() == path) {
            ref->reloadImage();
        }
    }
}

bool KisReferenceImagesLayer::pinRotate()
{
    return m_pinRotate;
}

void KisReferenceImagesLayer::setPinRotate(bool value)
{
    m_pinRotate = value;
}

bool KisReferenceImagesLayer::pinMirror()
{
    return m_pinMirror;
}

void KisReferenceImagesLayer::setPinMirror(bool value)
{
    m_pinMirror = value;
}

bool KisReferenceImagesLayer::pinPosition()
{
    return m_pinPosition;
}

void KisReferenceImagesLayer::setPinPosition(bool value)
{
    m_pinPosition = value;
}

bool KisReferenceImagesLayer::pinZoom()
{
    return m_pinZoom;
}

void KisReferenceImagesLayer::setPinZoom(bool value)
{
    m_pinZoom = value;
}

QTransform KisReferenceImagesLayer::transform(KisCanvas2 *kisCanvas)
 {
    if(!m_initialValues) {
        m_docOffset = kisCanvas->documentOffset();
        m_zoom = kisCanvas->viewConverter()->zoom();
        m_transform *= QTransform::fromScale(m_zoom,m_zoom);
        m_initialValues = true;
    }

    if(!qFuzzyCompare(kisCanvas->viewConverter()->zoom(), m_zoom)) {
        if(!m_pinZoom) {
            qreal diff = kisCanvas->viewConverter()->zoom()- m_zoom;
            m_transform *= QTransform::fromScale((m_zoom + diff)/m_zoom,(m_zoom + diff)/m_zoom);
            m_zoom = kisCanvas->viewConverter()->zoom();

        }
        m_docOffset = kisCanvas->documentOffset();
    }

    if(m_mirrorX != kisCanvas->coordinatesConverter()->xAxisMirrored()
                || m_mirrorY != kisCanvas->coordinatesConverter()->yAxisMirrored()) {
        if(!m_pinMirror) {
            qreal scaleX = m_mirrorX != kisCanvas->coordinatesConverter()->xAxisMirrored() ? -1 : 1;
            qreal scaleY = m_mirrorY != kisCanvas->coordinatesConverter()->yAxisMirrored() ? -1 : 1;
            QPointF center = kisCanvas->coordinatesConverter()->widgetCenterPoint();

            m_transform *= QTransform::fromTranslate(-center.x(),-center.y()) *
                    QTransform::fromScale(scaleX, scaleY) * QTransform::fromTranslate(center.x(),center.y());
        }
        m_docOffset = kisCanvas->documentOffset();
        m_previousAngle = kisCanvas->rotationAngle();
    }

    if(m_previousAngle != kisCanvas->rotationAngle())
    {
        if(!m_pinRotate) {
            QPointF center = kisCanvas->coordinatesConverter()->widgetCenterPoint();
            qreal diff = kisCanvas->rotationAngle() - m_previousAngle;
            QTransform rot;
            rot.rotate(diff);
            m_transform *= QTransform::fromTranslate(-center.x(),-center.y()) * rot
                    * QTransform::fromTranslate(center.x(),center.y());
        }
        m_docOffset = kisCanvas->documentOffset();
    }

    if(kisCanvas->documentOffset() != m_docOffset && !m_pinPosition) {
      QPointF diff = kisCanvas->documentOffset() - m_docOffset;
        m_transform *= QTransform::fromTranslate(-diff.x(), -diff.y());
    }

    m_zoom = kisCanvas->viewConverter()->zoom();
    m_docOffset = kisCanvas->documentOffset();
    m_mirrorX = kisCanvas->coordinatesConverter()->xAxisMirrored();
    m_mirrorY = kisCanvas->coordinatesConverter()->yAxisMirrored();
    m_previousAngle = kisCanvas->rotationAngle();
    return m_transform ;
}
