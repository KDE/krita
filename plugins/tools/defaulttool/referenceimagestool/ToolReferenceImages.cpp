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

#include <kis_action_registry.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <KisViewManager.h>
#include <KisDocument.h>
#include <KisReferenceImagesLayer.h>
#include <kis_image.h>

#include "ToolReferenceImagesWidget.h"
#include "KisReferenceImageCollection.h"

ToolReferenceImages::ToolReferenceImages(KoCanvasBase * canvas)
    : DefaultTool(canvas, false)
{
    setObjectName("ToolReferenceImages");
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

void ToolReferenceImages::deactivate()
{
    DefaultTool::deactivate();
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
