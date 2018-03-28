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
#include <QLayout>

#include <KoShapeRegistry.h>
#include <KoShapeManager.h>
#include <KoShapeController.h>
#include <KoFileDialog.h>

#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <KisViewManager.h>
#include <KisDocument.h>
#include <KisReferenceImagesLayer.h>

#include "ToolReferenceImagesWidget.h"

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
    // Add code here to initialize your tool when it got activated
    DefaultTool::activate(toolActivation, shapes);

    KisReferenceImagesLayer *layer = getOrCreteReferenceImagesLayer();
    connect(layer, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
}

void ToolReferenceImages::deactivate()
{
    DefaultTool::deactivate();
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

    auto *reference = KisReferenceImage::fromFile(filename, *kisCanvas->coordinatesConverter());
    KisReferenceImagesLayer *layer = getOrCreteReferenceImagesLayer();
    kisCanvas->imageView()->document()->addCommand(layer->addReferenceImage(reference));
}

void ToolReferenceImages::removeAllReferenceImages()
{
    KisReferenceImagesLayer *layer = getOrCreteReferenceImagesLayer();
    canvas()->addCommand(canvas()->shapeController()->removeShapes(layer->shapes()));
}

void ToolReferenceImages::loadReferenceImages()
{
}

void ToolReferenceImages::saveReferenceImages()
{
}

void ToolReferenceImages::slotSelectionChanged()
{
    KisReferenceImagesLayer *layer = getOrCreteReferenceImagesLayer();
    m_optionsWidget->selectionChanged(layer->shapeManager()->selection());
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
    auto layer = referenceImagesLayer();
    return layer ? referenceImagesLayer()->shapeManager() : nullptr;
}

KisReferenceImagesLayer *ToolReferenceImages::referenceImagesLayer() const
{
    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KisDocument *document = kisCanvas->imageView()->document();

    return document->referenceImagesLayer();
}

KisReferenceImagesLayer *ToolReferenceImages::getOrCreteReferenceImagesLayer()
{
    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KisDocument *document = kisCanvas->imageView()->document();

    return document->createReferenceImagesLayer().data();
}

KoSelection *ToolReferenceImages::koSelection() const
{
    auto manager = shapeManager();
    return manager ? manager->selection() : nullptr;
}
