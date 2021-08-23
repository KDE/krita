/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ToolReferenceImages.h"

#include <QDesktopServices>
#include <QFile>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QVector>
#include <QAction>
#include <QApplication>

#include <KoSelection.h>
#include <KoShapeRegistry.h>
#include <KoShapeManager.h>
#include <KoShapeController.h>
#include <KoFileDialog.h>
#include "KisMimeDatabase.h"

#include <kis_action_registry.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <KisViewManager.h>
#include <KisDocument.h>
#include <KisReferenceImagesLayer.h>
#include <kis_image.h>
#include "QClipboard"
#include "kis_action.h"

#include "ToolReferenceImagesWidget.h"
#include "KisReferenceImageCollection.h"
#include "KisReferenceImage.h"
#include "KisReferenceImageCropDecorator.h"
#include "KisReferenceImagesLayer.h"
#include "KisReferenceImageCropStrategy.h"

ToolReferenceImages::ToolReferenceImages(KoCanvasBase * canvas)
    : DefaultTool(canvas, false)
    , m_cropDecorator(new KisReferenceImageCropDecorator)
{
    setObjectName("ToolReferenceImages");

    m_sizeCursors[0] = Qt::SizeVerCursor;
    m_sizeCursors[1] = Qt::SizeBDiagCursor;
    m_sizeCursors[2] = Qt::SizeHorCursor;
    m_sizeCursors[3] = Qt::SizeFDiagCursor;
    m_sizeCursors[4] = Qt::SizeVerCursor;
    m_sizeCursors[5] = Qt::SizeBDiagCursor;
    m_sizeCursors[6] = Qt::SizeHorCursor;
    m_sizeCursors[7] = Qt::SizeFDiagCursor;
}

ToolReferenceImages::~ToolReferenceImages()
{
}

void ToolReferenceImages::activate(const QSet<KoShape*> &shapes)
{
    DefaultTool::activate(shapes);

    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    connect(kisCanvas->image(), SIGNAL(sigNodeAddedAsync(KisNodeSP)), this, SLOT(slotNodeAdded(KisNodeSP)));
    connect(kisCanvas->imageView()->document(), &KisDocument::sigReferenceImagesLayerChanged, this, &ToolReferenceImages::slotNodeAdded);

    auto referenceImageLayer = document()->referenceImagesLayer();
    if (referenceImageLayer) {
        setReferenceImageLayer(referenceImageLayer);
    }
}

void ToolReferenceImages::mousePressEvent(KoPointerEvent *event)
{
    if(m_layer) {
        QPointF newPos = event->pos();
        KoPointerEvent *newEvent = new KoPointerEvent(event, newPos);

        if (activeReferenceImage() && activeReferenceImage()->cropEnabled()) {
            KoInteractionTool::mousePressEvent(newEvent);
            updateCursor();
            return;
        }
        DefaultTool::mousePressEvent(newEvent);
        return;
    }

    DefaultTool::mousePressEvent(event);
}

void ToolReferenceImages::mouseMoveEvent(KoPointerEvent *event)
{
    KisReferenceImage *referenceImage = activeReferenceImage();

    if(m_layer) {
        QPointF newPos = event->pos();
        KoPointerEvent *newEvent = new KoPointerEvent(event, QPointF(newPos));

        if(referenceImage && referenceImage->cropEnabled()) {
            KoInteractionTool::mouseMoveEvent(newEvent);
            if(currentStrategy() == 0) {
            QRectF bounds = handlesSize();

            if(bounds.contains(newEvent->point)) {
                bool inside;
                KoFlake::SelectionHandle newDirection = handleAt(newEvent->point, &inside);

                if (inside != m_mouseWasInsideHandles || m_lastHandle != newDirection) {
                    m_lastHandle = newDirection;
                    m_mouseWasInsideHandles = inside;
                }
            }
            else {
                m_lastHandle = KoFlake::NoHandle;
                m_mouseWasInsideHandles = false;
            }
          }
            updateCursor();
            return;
        }

        DefaultTool::mouseMoveEvent(newEvent);
        return;
    }

    DefaultTool::mouseMoveEvent(event);
}

void ToolReferenceImages::deactivate()
{
    DefaultTool::deactivate();
}

void ToolReferenceImages::paint(QPainter &painter, const KoViewConverter &converter)
{
    if(m_layer) {
        painter.setTransform(QTransform());
    }
    KisReferenceImage* ref = activeReferenceImage();

    if(ref && ref->cropEnabled()) {
      m_cropDecorator->setReferenceImage(ref);
      m_cropDecorator->paint(painter, converter);
    }
    else {
        DefaultTool::paint(painter,converter);
    }
}

QRectF ToolReferenceImages::handlesSize()
{
    KisReferenceImage *referenceImage = activeReferenceImage();
    if(referenceImage && referenceImage->cropEnabled()) {
        recalcCropHandles(activeReferenceImage());
        QRectF bounds = m_cropRect.boundingRect();
        QPointF border = canvas()->viewConverter()->viewToDocument(QPointF(5, 5));// HandleSize = 5
        bounds.adjust(-border.x(), -border.y(), border.x(), border.y());
        return bounds;
    }
    return QRectF();
}

KoFlake::SelectionHandle ToolReferenceImages::handleAt(const QPointF &point, bool *innerHandleMeaning)
{
    // check for handles in this order; meaning that when handles overlap the one on top is chosen
    static const KoFlake::SelectionHandle handleOrder[] = {
        KoFlake::BottomRightHandle,
        KoFlake::TopLeftHandle,
        KoFlake::BottomLeftHandle,
        KoFlake::TopRightHandle,
        KoFlake::BottomMiddleHandle,
        KoFlake::RightMiddleHandle,
        KoFlake::LeftMiddleHandle,
        KoFlake::TopMiddleHandle,
        KoFlake::NoHandle
    };

    const KoViewConverter *converter = canvas()->viewConverter();

    if (!activeReferenceImage() || !converter) {
        return KoFlake::NoHandle;
    }

    recalcCropHandles(activeReferenceImage());
    if (innerHandleMeaning) {
        QPainterPath path;
        path.addPolygon(m_cropRect);
        *innerHandleMeaning = path.contains(point) || path.intersects(handlePaintRect(point));
    }

    const QPointF viewPoint = point;

    for (int i = 0; i < KoFlake::NoHandle; ++i) {
        KoFlake::SelectionHandle handle = handleOrder[i];

        const QPointF handlePoint = m_cropHandles[handle];
        const qreal distanceSq = kisSquareDistance(viewPoint, handlePoint);

        // if just inside the outline
        if (distanceSq < 25) {

            if (innerHandleMeaning) {
                if (distanceSq < 16) {
                    *innerHandleMeaning = true;
                }
            }

            return handle;
        }
    }
    return KoFlake::NoHandle;
}

void ToolReferenceImages::recalcCropHandles(KisReferenceImage *referenceImage)
{
    if(!referenceImage) return;

    QTransform matrix = activeReferenceImage()->absoluteTransformation();
    m_cropRect = matrix.map(QPolygonF(referenceImage->cropRect()));

    m_cropHandles[KoFlake::TopMiddleHandle] = (m_cropRect.value(0) + m_cropRect.value(1)) / 2;
    m_cropHandles[KoFlake::TopRightHandle] = m_cropRect.value(1);
    m_cropHandles[KoFlake::RightMiddleHandle] = (m_cropRect.value(1) + m_cropRect.value(2)) / 2;
    m_cropHandles[KoFlake::BottomRightHandle] = m_cropRect.value(2);
    m_cropHandles[KoFlake::BottomMiddleHandle] = (m_cropRect.value(2) + m_cropRect.value(3)) / 2;
    m_cropHandles[KoFlake::BottomLeftHandle] = m_cropRect.value(3);
    m_cropHandles[KoFlake::LeftMiddleHandle] = (m_cropRect.value(3) + m_cropRect.value(0)) / 2;
    m_cropHandles[KoFlake::TopLeftHandle] = m_cropRect.value(0);

}

void ToolReferenceImages::updateCursor()
{
    if (tryUseCustomCursor()) return;

    QCursor cursor = Qt::ArrowCursor;

    if(!activeReferenceImage() && activeReferenceImage()->cropEnabled()) return;
    if (m_mouseWasInsideHandles) {

            switch (m_lastHandle) {
            case KoFlake::TopMiddleHandle:
                cursor = m_sizeCursors[0];
                break;
            case KoFlake::TopRightHandle:
                cursor = m_sizeCursors[1];
                break;
            case KoFlake::RightMiddleHandle:
                cursor = m_sizeCursors[2];
                break;
            case KoFlake::BottomRightHandle:
                cursor = m_sizeCursors[3];
                break;
            case KoFlake::BottomMiddleHandle:
                cursor = m_sizeCursors[4];
                break;
            case KoFlake::BottomLeftHandle:
                cursor = m_sizeCursors[5];
                break;
            case KoFlake::LeftMiddleHandle:
                cursor = m_sizeCursors[6];
                break;
            case KoFlake::TopLeftHandle:
                cursor = m_sizeCursors[7];
                break;
            case KoFlake::NoHandle:
                cursor = Qt::SizeAllCursor;
                break;
            }
        }

    useCursor(cursor);

}
KoInteractionStrategy *ToolReferenceImages::createStrategy(KoPointerEvent *event)
{
    if(activeReferenceImage() && activeReferenceImage()->cropEnabled()) {
        bool insideSelection = false;
        KoFlake::SelectionHandle handle = handleAt(event->point, &insideSelection);
        if(insideSelection) {
            return new KisReferenceImageCropStrategy(this, activeReferenceImage(), event->point, handle);
        }
        else {
            return 0;
        }
     }
    return DefaultTool::createStrategy(event);
}

void ToolReferenceImages::slotNodeAdded(KisNodeSP node)
{
    auto *referenceImagesLayer = dynamic_cast<KisReferenceImagesLayer*>(node.data());

    if (referenceImagesLayer) {
        setReferenceImageLayer(referenceImagesLayer);
    }
}

void ToolReferenceImages::setReferenceImageLayer(KisSharedPtr<KisReferenceImagesLayer> layer)
{
    m_layer = layer;
    connect(layer.data(), SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
    connect(layer->shapeManager(), SIGNAL(selectionChanged()), this, SLOT(repaintDecorations()));
    connect(layer->shapeManager(), SIGNAL(selectionContentChanged()), this, SLOT(repaintDecorations()));
    connect(m_layer, SIGNAL(sigCropChanged()), this, SIGNAL(cropRectChanged()));
}

void ToolReferenceImages::addReferenceImage()
{
    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);

            KoFileDialog dialog(kisCanvas->viewManager()->mainWindow(), KoFileDialog::OpenFile, "OpenReferenceImage");
    dialog.setCaption(i18n("Select a Reference Image"));

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    if (!locations.isEmpty()) {
        dialog.setDefaultDir(locations.first());
    }

    QString filename = dialog.filename();
    if (filename.isEmpty()) return;
    if (!QFileInfo(filename).exists()) return;

    auto *reference = KisReferenceImage::fromFile(filename, *kisCanvas->coordinatesConverter(), canvas()->canvasWidget());
    if (reference) {
        if (document()->referenceImagesLayer()) {
            reference->setZIndex(document()->referenceImagesLayer()->shapes().size());
        }
        KisDocument *doc = document();
        doc->addCommand(KisReferenceImagesLayer::addReferenceImages(doc, {reference}));
    }
}

void ToolReferenceImages::pasteReferenceImage()
{
    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);

    KisReferenceImage* reference = KisReferenceImage::fromClipboard(*kisCanvas->coordinatesConverter());
    if (reference) {
        if (document()->referenceImagesLayer()) {
            reference->setZIndex(document()->referenceImagesLayer()->shapes().size());
        }
        KisDocument *doc = document();
        doc->addCommand(KisReferenceImagesLayer::addReferenceImages(doc, {reference}));
    } else {
        if (canvas()->canvasWidget()) {
            QMessageBox::critical(canvas()->canvasWidget(), i18nc("@title:window", "Krita"), i18n("Could not load reference image from clipboard"));
        }
    }
}



void ToolReferenceImages::removeAllReferenceImages()
{
    auto layer = m_layer.toStrongRef();
    if (!layer) return;

    canvas()->addCommand(layer->removeReferenceImages(document(), layer->shapes()));
}

void ToolReferenceImages::loadReferenceImages()
{
    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);

            KoFileDialog dialog(kisCanvas->viewManager()->mainWindow(), KoFileDialog::OpenFile, "OpenReferenceImageCollection");
    dialog.setMimeTypeFilters(QStringList() << "application/x-krita-reference-images");
    dialog.setCaption(i18n("Load Reference Images"));

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    if (!locations.isEmpty()) {
        dialog.setDefaultDir(locations.first());
    }

    QString filename = dialog.filename();
    if (filename.isEmpty()) return;
    if (!QFileInfo(filename).exists()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not open '%1'.", filename));
        return;
    }

    KisReferenceImageCollection collection;

    int currentZIndex = 0;
    if (document()->referenceImagesLayer()) {
        currentZIndex = document()->referenceImagesLayer()->shapes().size();
    }

    if (collection.load(&file)) {
        QList<KoShape*> shapes;
        Q_FOREACH(auto *reference, collection.referenceImages()) {
            reference->setZIndex(currentZIndex);
            shapes.append(reference);
            currentZIndex += 1;
        }

        KisDocument *doc = document();
        doc->addCommand(KisReferenceImagesLayer::addReferenceImages(doc, shapes));
    } else {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not load reference images from '%1'.", filename));
    }
    file.close();
}

void ToolReferenceImages::saveReferenceImages()
{
    auto layer = m_layer.toStrongRef();
    if (!layer || layer->shapeCount() == 0) return;

    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);

            KoFileDialog dialog(kisCanvas->viewManager()->mainWindow(), KoFileDialog::SaveFile, "SaveReferenceImageCollection");
    dialog.setMimeTypeFilters(QStringList() << "application/x-krita-reference-images");
    dialog.setCaption(i18n("Save Reference Images"));

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    if (!locations.isEmpty()) {
        dialog.setDefaultDir(locations.first());
    }

    QString filename = dialog.filename();
    if (filename.isEmpty()) return;

    QString fileMime = KisMimeDatabase::mimeTypeForFile(filename, false);
    if (fileMime != "application/x-krita-reference-images") {
        filename.append(filename.endsWith(".") ? "krf" : ".krf");
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not open '%1' for saving.", filename));
        return;
    }

    KisReferenceImageCollection collection(layer->referenceImages());
    bool ok = collection.save(&file);
    file.close();

    if (!ok) {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Failed to save reference images."));
    }
}

void ToolReferenceImages::slotSelectionChanged()
{
    auto layer = m_layer.toStrongRef();
    if (!layer) return;

    m_optionsWidget->selectionChanged(layer->shapeManager()->selection());
    updateActions();
}

QList<QPointer<QWidget>> ToolReferenceImages::createOptionWidgets()
{
    // Instead of inheriting DefaultTool's multi-tab implementation, inherit straight from KoToolBase
    return KoToolBase::createOptionWidgets();
}

QWidget *ToolReferenceImages::createOptionWidget()
{
    if (!m_optionsWidget) {
        m_optionsWidget = new ToolReferenceImagesWidget(this);
        // See https://bugs.kde.org/show_bug.cgi?id=316896
        QWidget *specialSpacer = new QWidget(m_optionsWidget);
        specialSpacer->setObjectName("SpecialSpacer");
        specialSpacer->setFixedSize(0, 0);
        m_optionsWidget->layout()->addWidget(specialSpacer);
        connect(this, SIGNAL(cropRectChanged()), m_optionsWidget, SLOT(slotCropRectChanged()));
    }
    return m_optionsWidget;
}

bool ToolReferenceImages::isValidForCurrentLayer() const
{
    return true;
}

KoShapeManager *ToolReferenceImages::shapeManager() const
{
    auto layer = m_layer.toStrongRef();
    return layer ? layer->shapeManager() : nullptr;
}

KoSelection *ToolReferenceImages::koSelection() const
{
    auto manager = shapeManager();
    return manager ? manager->selection() : nullptr;
}

void ToolReferenceImages::updateDistinctiveActions(const QList<KoShape*> &)
{
    action("object_group")->setEnabled(false);
    action("object_unite")->setEnabled(false);
    action("object_intersect")->setEnabled(false);
    action("object_subtract")->setEnabled(false);
    action("object_split")->setEnabled(false);
    action("object_ungroup")->setEnabled(false);
}

void ToolReferenceImages::deleteSelection()
{
    auto layer = m_layer.toStrongRef();
    if (!layer) return;

    QList<KoShape *> shapes = koSelection()->selectedShapes();

    if (!shapes.empty()) {
        canvas()->addCommand(layer->removeReferenceImages(document(), shapes));
    }
}

QMenu* ToolReferenceImages::popupActionsMenu()
{
    if (m_contextMenu) {
        m_contextMenu->clear();
        m_contextMenu->addSection(i18n("Reference Image Actions"));
        m_contextMenu->addSeparator();

        QMenu *transform = m_contextMenu->addMenu(i18n("Transform"));

        transform->addAction(action("object_transform_rotate_90_cw"));
        transform->addAction(action("object_transform_rotate_90_ccw"));
        transform->addAction(action("object_transform_rotate_180"));
        transform->addSeparator();
        transform->addAction(action("object_transform_mirror_horizontally"));
        transform->addAction(action("object_transform_mirror_vertically"));
        transform->addSeparator();
        transform->addAction(action("object_transform_reset"));

        m_contextMenu->addSeparator();

        KisAction* cut = new KisAction(i18n("Cut"));
        cut->setIcon(KisIconUtils::loadIcon("edit-cut"));
        KisAction* copy = new KisAction(i18n("Copy"));
        copy->setIcon(KisIconUtils::loadIcon("edit-copy"));
        KisAction* paste = new KisAction(i18n("Paste"));
        paste->setIcon(KisIconUtils::loadIcon("edit-paste"));

        connect(cut,SIGNAL(triggered()),this,SLOT(cut()));
        connect(copy,SIGNAL(triggered()),this,SLOT(copy()));
        connect(paste,SIGNAL(triggered()),this,SLOT(paste()));

        m_contextMenu->addAction(cut);
        m_contextMenu->addAction(copy);
        m_contextMenu->addAction(paste);

        m_contextMenu->addSeparator();

        m_contextMenu->addAction(action("object_order_front"));
        m_contextMenu->addAction(action("object_order_raise"));
        m_contextMenu->addAction(action("object_order_lower"));
        m_contextMenu->addAction(action("object_order_back"));
    }

    return m_contextMenu.data();
}

void ToolReferenceImages::cut()
{
    copy();
    deleteSelection();
}

void ToolReferenceImages::copy() const
{
    QList<KoShape *> shapes = koSelection()->selectedShapes();
    KoShape* shape = shapes.at(0);
    KisReferenceImage *reference = dynamic_cast<KisReferenceImage*>(shape);
    QClipboard *cb = QApplication::clipboard();
    cb->setImage(reference->image());
}

bool ToolReferenceImages::paste()
{
    pasteReferenceImage();
    return true;
}

KisReferenceImage *ToolReferenceImages::activeReferenceImage()
{
    KisReferenceImage *ref;
    if(koSelection()) {
        QList<KoShape*> shapes = koSelection()->selectedVisibleShapes();
        if(!shapes.isEmpty()) {
            ref = dynamic_cast<KisReferenceImage*>(shapes.at(0));
            return ref;
        }
    }
    return nullptr;
}

KisDocument *ToolReferenceImages::document() const
{
    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    return kisCanvas->imageView()->document();
}

QList<QAction *> ToolReferenceImagesFactory::createActionsImpl()
{
    QList<QAction *> defaultActions = DefaultToolFactory::createActionsImpl();
    QList<QAction *> actions;

    QStringList actionNames;
    actionNames << "object_order_front"
                << "object_order_raise"
                << "object_order_lower"
                << "object_order_back"
                << "object_transform_rotate_90_cw"
                << "object_transform_rotate_90_ccw"
                << "object_transform_rotate_180"
                << "object_transform_mirror_horizontally"
                << "object_transform_mirror_vertically"
                << "object_transform_reset";

    Q_FOREACH(QAction *action, defaultActions) {
        if (actionNames.contains(action->objectName())) {
            actions << action;
        }
    }
    return actions;
}
