/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ToolReferenceImages.h"

#include <QDesktopServices>
#include <QFile>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QVector>

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
    : DefaultTool(canvas)
{
    setObjectName("ToolReferenceImages");
}

ToolReferenceImages::~ToolReferenceImages()
{
}

void ToolReferenceImages::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    DefaultTool::activate(toolActivation, shapes);

    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    connect(kisCanvas->image(), SIGNAL(sigNodeAddedAsync(KisNodeSP)), this, SLOT(slotNodeAdded(KisNodeSP)));

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
}

void ToolReferenceImages::addReferenceImage()
{
    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas)

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
        KisDocument *doc = document();
        doc->addCommand(KisReferenceImagesLayer::addReferenceImages(doc, {reference}));
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
    KIS_ASSERT_RECOVER_RETURN(kisCanvas)

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
        QMessageBox::critical(nullptr, i18nc("@title:window", "Krita"), i18n("Could not open '%1'.", filename));
        return;
    }

    KisReferenceImageCollection collection;
    if (collection.load(&file)) {
        QList<KoShape*> shapes;
        Q_FOREACH(auto *reference, collection.referenceImages()) {
            shapes.append(reference);
        }

        KisDocument *doc = document();
        doc->addCommand(KisReferenceImagesLayer::addReferenceImages(doc, shapes));
    } else {
        QMessageBox::critical(nullptr, i18nc("@title:window", "Krita"), i18n("Could not load reference images from '%1'.", filename));
    }
    file.close();
}

void ToolReferenceImages::saveReferenceImages()
{
    auto layer = m_layer.toStrongRef();
    if (!layer || layer->shapeCount() == 0) return;

    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas)

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
        QMessageBox::critical(nullptr, i18nc("@title:window", "Krita"), i18n("Could not open '%1' for saving.", filename));
        return;
    }

    KisReferenceImageCollection collection(layer->referenceImages());
    bool ok = collection.save(&file);
    file.close();

    if (!ok) {
        QMessageBox::critical(nullptr, i18nc("@title:window", "Krita"), i18n("Failed to save reference images."));
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
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions = DefaultToolFactory::createActionsImpl();

    actions << actionRegistry->makeQAction("object_order_front");
    actions << actionRegistry->makeQAction("object_order_raise");
    actions << actionRegistry->makeQAction("object_order_lower");
    actions << actionRegistry->makeQAction("object_order_back");
    actions << actionRegistry->makeQAction("object_group");
    actions << actionRegistry->makeQAction("object_ungroup");
    actions << actionRegistry->makeQAction("object_transform_rotate_90_cw");
    actions << actionRegistry->makeQAction("object_transform_rotate_90_ccw");
    actions << actionRegistry->makeQAction("object_transform_rotate_180");
    actions << actionRegistry->makeQAction("object_transform_mirror_horizontally");
    actions << actionRegistry->makeQAction("object_transform_mirror_vertically");
    actions << actionRegistry->makeQAction("object_transform_reset");
    actions << actionRegistry->makeQAction("object_unite");
    actions << actionRegistry->makeQAction("object_intersect");
    actions << actionRegistry->makeQAction("object_subtract");
    actions << actionRegistry->makeQAction("object_split");
    return actions;
}
