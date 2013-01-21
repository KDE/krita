/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  The outline algorith uses the limn algorithm of fontutils by
 *  Karl Berry <karl@cs.umb.edu> and Kathryn Hargreaves <letters@cs.umb.edu>
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

#include "kis_selection_manager.h"
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QTimer>

#include <kaction.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kactioncollection.h>

#include <KoServiceProvider.h>
#include "KoCanvasController.h"
#include "KoChannelInfo.h"
#include "KoIntegerMaths.h"
#include <KoDocument.h>
#include <KoMainWindow.h>
#include <KoDocumentEntry.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShapeStroke.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoToolProxy.h>

#include "kis_adjustment_layer.h"
#include "kis_node_manager.h"
#include "canvas/kis_canvas2.h"
#include "kis_config.h"
#include "kis_convolution_painter.h"
#include "kis_convolution_kernel.h"
#include "kis_debug.h"
#include "kis_doc2.h"
#include "kis_part2.h"
#include "kis_fill_painter.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_statusbar.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_transaction.h"
#include "kis_selection.h"
#include "kis_types.h"
#include "kis_canvas_resource_provider.h"
#include "kis_undo_adapter.h"
#include "kis_pixel_selection.h"
#include "flake/kis_shape_selection.h"
#include "commands/kis_selection_commands.h"
#include "kis_selection_mask.h"
#include "flake/kis_shape_layer.h"
#include "kis_selection_decoration.h"
#include "canvas/kis_canvas_decoration.h"
#include "kis_node_commands_adapter.h"
#include "kis_iterator_ng.h"
#include "kis_clipboard.h"
#include "kis_view2.h"
#include "kis_selection_manager_p.h"
#include "kis_figure_painting_tool_helper.h"

#include "actions/kis_selection_action_factories.h"


KisSelectionManager::KisSelectionManager(KisView2 * view, KisDoc2 * doc)
        : m_view(view),
        m_doc(doc),
        m_adapter(new KisNodeCommandsAdapter(view)),
        m_copy(0),
        m_copyMerged(0),
        m_cut(0),
        m_paste(0),
        m_pasteNew(0),
        m_cutToNewLayer(0),
        m_selectAll(0),
        m_deselect(0),
        m_clear(0),
        m_reselect(0),
        m_invert(0),
        m_copyToNewLayer(0),
        m_smooth(0),
        m_load(0),
        m_save(0),
        m_fillForegroundColor(0),
        m_fillBackgroundColor(0),
        m_fillPattern(0),
        m_imageResizeToSelection(0)
{
    m_clipboard = KisClipboard::instance();

    KoSelection * selection = m_view->canvasBase()->globalShapeManager()->selection();
    Q_ASSERT(selection);
    connect(selection, SIGNAL(selectionChanged()), this, SLOT(shapeSelectionChanged()));

    KisSelectionDecoration* decoration = new KisSelectionDecoration(m_view);
    connect(this, SIGNAL(currentSelectionChanged()), decoration, SLOT(selectionChanged()));
    decoration->setVisible(true);
    m_view->canvasBase()->addDecoration(decoration);
}

KisSelectionManager::~KisSelectionManager()
{
    qDeleteAll(m_pluginActions);
}

void KisSelectionManager::setup(KActionCollection * collection)
{
    // XXX: setup shortcuts!

    m_cut = KStandardAction::cut(this, SLOT(cut()), collection);
    m_copy = KStandardAction::copy(this, SLOT(copy()), collection);
    m_paste = KStandardAction::paste(this, SLOT(paste()), collection);

    m_pasteNew  = new KAction(i18n("Paste into &New Image"), this);
    collection->addAction("paste_new", m_pasteNew);
    connect(m_pasteNew, SIGNAL(triggered()), this, SLOT(pasteNew()));

    m_pasteAt = new KAction(i18n("Paste at cursor"), this);
    collection->addAction("paste_at", m_pasteAt);
    connect(m_pasteAt, SIGNAL(triggered()), this, SLOT(pasteAt()));

    m_copyMerged = new KAction(i18n("Copy merged"), this);
    collection->addAction("copy_merged", m_copyMerged);
    m_copyMerged->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
    connect(m_copyMerged, SIGNAL(triggered()), this, SLOT(copyMerged()));

    m_selectAll = collection->addAction(KStandardAction::SelectAll,  "select_all", this, SLOT(selectAll()));

    m_deselect = collection->addAction(KStandardAction::Deselect,  "deselect", this, SLOT(deselect()));

    m_clear = collection->addAction(KStandardAction::Clear,  "clear", this, SLOT(clear()));
    m_clear->setShortcut(QKeySequence((Qt::Key_Delete)));

    m_reselect  = new KAction(i18n("&Reselect"), this);
    collection->addAction("reselect", m_reselect);
    m_reselect->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connect(m_reselect, SIGNAL(triggered()), this, SLOT(reselect()));

    m_invert  = new KAction(i18n("&Invert Selection"), this);
    collection->addAction("invert", m_invert);
    m_invert->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    connect(m_invert, SIGNAL(triggered()), this, SLOT(invert()));

    m_copyToNewLayer  = new KAction(i18n("Copy Selection to New Layer"), this);
    collection->addAction("copy_selection_to_new_layer", m_copyToNewLayer);
    m_copyToNewLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_J));
    connect(m_copyToNewLayer, SIGNAL(triggered()), this, SLOT(copySelectionToNewLayer()));

    m_cutToNewLayer  = new KAction(i18n("Cut Selection to New Layer"), this);
    collection->addAction("cut_selection_to_new_layer", m_cutToNewLayer);
    m_cutToNewLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_J));
    connect(m_cutToNewLayer, SIGNAL(triggered()), this, SLOT(cutToNewLayer()));

    m_fillForegroundColor  = new KAction(i18n("Fill with Foreground Color"), this);
    collection->addAction("fill_selection_foreground_color", m_fillForegroundColor);
    m_fillForegroundColor->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Backspace));
    connect(m_fillForegroundColor, SIGNAL(triggered()), this, SLOT(fillForegroundColor()));

    m_fillBackgroundColor  = new KAction(i18n("Fill with Background Color"), this);
    collection->addAction("fill_selection_background_color", m_fillBackgroundColor);
    m_fillBackgroundColor->setShortcut(QKeySequence(Qt::Key_Backspace));
    connect(m_fillBackgroundColor, SIGNAL(triggered()), this, SLOT(fillBackgroundColor()));

    m_fillPattern  = new KAction(i18n("Fill with Pattern"), this);
    collection->addAction("fill_selection_pattern", m_fillPattern);
    connect(m_fillPattern, SIGNAL(triggered()), this, SLOT(fillPattern()));

    m_strokeShapes  = new KAction(i18n("Stroke selected shapes"), this);
    collection->addAction("stroke_shapes", m_strokeShapes);
    connect(m_strokeShapes, SIGNAL(triggered()), this, SLOT(paintSelectedShapes()));

    m_toggleDisplaySelection  = new KToggleAction(i18n("Display Selection"), this);
    collection->addAction("toggle_display_selection", m_toggleDisplaySelection);
    m_toggleDisplaySelection->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    connect(m_toggleDisplaySelection, SIGNAL(triggered()), this, SLOT(toggleDisplaySelection()));

    m_toggleDisplaySelection->setChecked(true);

    m_smooth  = new KAction(i18n("Smooth..."), this);
    collection->addAction("smooth", m_smooth);
    connect(m_smooth, SIGNAL(triggered()), this, SLOT(smooth()));

    m_imageResizeToSelection  = new KAction(i18n("Size Canvas to Size of Selection"), this);
    collection->addAction("resizeimagetoselection", m_imageResizeToSelection);
    connect(m_imageResizeToSelection, SIGNAL(triggered()), this, SLOT(imageResizeToSelection()));

//     m_load
//         = new KAction(i18n("Load..."),
//                   0, 0,
//                   this, SLOT(load()),
//                   collection, "load_selection");
//
//
//     m_save
//         = new KAction(i18n("Save As..."),
//                   0, 0,
//                   this, SLOT(save()),
//                   collection, "save_selection");

    QClipboard *cb = QApplication::clipboard();
    connect(cb, SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));
    connect(m_view->canvasBase()->toolProxy(), SIGNAL(toolChanged(const QString&)), SLOT(clipboardDataChanged()));

}

void KisSelectionManager::clipboardDataChanged()
{
    updateGUI();
}


void KisSelectionManager::addSelectionAction(QAction * action)
{
    m_pluginActions.append(action);
}


bool KisSelectionManager::havePixelsSelected()
{
    KisSelectionSP activeSelection = m_view->selection();
    return activeSelection && !activeSelection->selectedRect().isEmpty();
}

bool KisSelectionManager::havePixelsInClipboard()
{
    return m_clipboard->hasClip();
}

bool KisSelectionManager::haveShapesSelected()
{
    return m_view->canvasBase()->shapeManager()->selection()->count() > 0;
}

bool KisSelectionManager::haveShapesInClipboard()
{
    KisShapeLayer *shapeLayer =
        dynamic_cast<KisShapeLayer*>(m_view->activeLayer().data());

    if (shapeLayer) {
        const QMimeData* data = QApplication::clipboard()->mimeData();
        if (data) {
            QStringList mimeTypes = m_view->canvasBase()->toolProxy()->supportedPasteMimeTypes();
            foreach(const QString & mimeType, mimeTypes) {
                if (data->hasFormat(mimeType)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool KisSelectionManager::haveEditablePixelSelectionWithPixels()
{
    if (!m_view->selectionEditable()) {
        return false;
    }
    KisSelectionSP selection = m_view->selection();
    if (selection && selection->hasPixelSelection()) {
        return !selection->pixelSelection()->selectedRect().isEmpty();
    }
    return false;
}

void KisSelectionManager::updateGUI()
{
    Q_ASSERT(m_view);
    Q_ASSERT(m_clipboard);
    if (!m_view || !m_clipboard) return;

    bool havePixelsSelected = this->havePixelsSelected();
    bool havePixelsInClipboard = this->havePixelsInClipboard();
    bool haveShapesSelected = this->haveShapesSelected();
    bool haveShapesInClipboard = this->haveShapesInClipboard();
    bool haveEditablePixelSelectionWithPixels = this->haveEditablePixelSelectionWithPixels();
    bool haveDevice = m_view->activeDevice();

    KisLayerSP activeLayer = m_view->activeLayer();
    KisImageWSP image = activeLayer ? activeLayer->image() : 0;
    bool canReselect = image && image->canReselectGlobalSelection();
    bool canDeselect =  image && image->globalSelection();

    m_clear->setEnabled(haveDevice || havePixelsSelected || haveShapesSelected);
    m_cut->setEnabled(havePixelsSelected || haveShapesSelected);
    m_copy->setEnabled(havePixelsSelected || haveShapesSelected);
    m_paste->setEnabled(havePixelsInClipboard || haveShapesInClipboard);
    m_pasteAt->setEnabled(havePixelsInClipboard || haveShapesInClipboard);
    // FIXME: how about pasting shapes?
    m_pasteNew->setEnabled(havePixelsInClipboard);

    m_copyMerged->setEnabled(havePixelsSelected);
    m_cutToNewLayer->setEnabled(havePixelsSelected);
    m_copyToNewLayer->setEnabled(havePixelsSelected);
    m_invert->setEnabled(haveEditablePixelSelectionWithPixels);
    m_smooth->setEnabled(haveEditablePixelSelectionWithPixels);
    m_imageResizeToSelection->setEnabled(havePixelsSelected);

    m_fillForegroundColor->setEnabled(haveDevice);
    m_fillBackgroundColor->setEnabled(haveDevice);
    m_fillPattern->setEnabled(haveDevice);
    m_strokeShapes->setEnabled(haveShapesSelected);

    m_selectAll->setEnabled(true);
    m_deselect->setEnabled(canDeselect);
    m_reselect->setEnabled(canReselect);


    if (!m_pluginActions.isEmpty()) {
        QListIterator<QAction *> i(m_pluginActions);
        while (i.hasNext()) {
            i.next()->setEnabled(true);
        }
    }

//    m_load->setEnabled(true);
//    m_save->setEnabled(havePixelsSelected);

    updateStatusBar();
    emit signalUpdateGUI();
}

void KisSelectionManager::updateStatusBar()
{
    if (m_view && m_view->statusBar()) {
        m_view->statusBar()->setSelection(m_view->image());
    }
}

void KisSelectionManager::selectionChanged()
{
    updateGUI();
    emit currentSelectionChanged();
}

void KisSelectionManager::cut()
{
    KisCutCopyActionFactory factory;
    factory.run(true, m_view);
}

void KisSelectionManager::copy()
{
    KisCutCopyActionFactory factory;
    factory.run(false, m_view);
}

void KisSelectionManager::copyMerged()
{
    KisCopyMergedActionFactory factory;
    factory.run(m_view);
}

void KisSelectionManager::paste()
{
    KisPasteActionFactory factory;
    factory.run(m_view);
}

void KisSelectionManager::pasteAt()
{
    //XXX
}

void KisSelectionManager::pasteNew()
{
    KisPasteNewActionFactory factory;
    factory.run(m_view);
}

void KisSelectionManager::selectAll()
{
    KisSelectAllActionFactory factory;
    factory.run(m_view);
}

void KisSelectionManager::deselect()
{
    KisDeselectActionFactory factory;
    factory.run(m_view);
}

void KisSelectionManager::reselect()
{
    KisReselectActionFactory factory;
    factory.run(m_view);
}


void KisSelectionManager::clear()
{
    KisClearActionFactory factory;
    factory.run(m_view);
}

void KisSelectionManager::fillForegroundColor()
{
    KisFillActionFactory  factory;
    factory.run("fg", m_view);
}

void KisSelectionManager::fillBackgroundColor()
{
    KisFillActionFactory factory;
    factory.run("bg", m_view);
}

void KisSelectionManager::fillPattern()
{
    KisFillActionFactory factory;
    factory.run("pattern", m_view);
}

void KisSelectionManager::copySelectionToNewLayer()
{
    copy();
    paste();
}

void KisSelectionManager::cutToNewLayer()
{
    cut();
    paste();
}

void KisSelectionManager::toggleDisplaySelection()
{
    KisCanvasDecoration* decoration = m_view->canvasBase()->decoration("selection");
    if (decoration) decoration->toggleVisibility();
}

bool KisSelectionManager::displaySelection()
{
    return m_toggleDisplaySelection->isChecked();
}

void KisSelectionManager::invert()
{
    KisApplySelectionFilterActionFactory factory;
    KisUiActionConfiguration config(factory.id());
    config.setProperty("filter-name", "invert");
    factory.runFromXML(m_view, config);
}

void KisSelectionManager::grow(qint32 xRadius, qint32 yRadius)
{
    KisApplySelectionFilterActionFactory factory;
    KisUiActionConfiguration config(factory.id());
    config.setProperty("filter-name", "grow");
    config.setProperty("x-radius", xRadius);
    config.setProperty("y-radius", yRadius);
    factory.runFromXML(m_view, config);
}

void KisSelectionManager::shrink(qint32 xRadius, qint32 yRadius, bool edgeLock)
{
    KisApplySelectionFilterActionFactory factory;
    KisUiActionConfiguration config(factory.id());
    config.setProperty("filter-name", "shrink");
    config.setProperty("x-radius", xRadius);
    config.setProperty("y-radius", yRadius);
    config.setProperty("edge-lock", edgeLock);
    factory.runFromXML(m_view, config);
}

void KisSelectionManager::smooth()
{
    KisApplySelectionFilterActionFactory factory;
    KisUiActionConfiguration config(factory.id());
    config.setProperty("filter-name", "smooth");
    factory.runFromXML(m_view, config);
}

void KisSelectionManager::erode()
{
    KisApplySelectionFilterActionFactory factory;
    KisUiActionConfiguration config(factory.id());
    config.setProperty("filter-name", "erode");
    factory.runFromXML(m_view, config);
}

void KisSelectionManager::dilate()
{
    KisApplySelectionFilterActionFactory factory;
    KisUiActionConfiguration config(factory.id());
    config.setProperty("filter-name", "dilate");
    factory.runFromXML(m_view, config);
}

void KisSelectionManager::border(qint32 xRadius, qint32 yRadius)
{
    KisApplySelectionFilterActionFactory factory;
    KisUiActionConfiguration config(factory.id());
    config.setProperty("filter-name", "border");
    config.setProperty("x-radius", xRadius);
    config.setProperty("y-radius", yRadius);
    factory.runFromXML(m_view, config);
}

void KisSelectionManager::feather(qint32 radius)
{
    KisApplySelectionFilterActionFactory factory;
    KisUiActionConfiguration config(factory.id());
    config.setProperty("filter-name", "feather");
    config.setProperty("radius", radius);
    factory.runFromXML(m_view, config);
}

void KisSelectionManager::shapeSelectionChanged()
{
    KoShapeManager* shapeManager = m_view->canvasBase()->globalShapeManager();

    KoSelection * selection = shapeManager->selection();
    QList<KoShape*> selectedShapes = selection->selectedShapes();

    KoShapeStroke* border = new KoShapeStroke(0, Qt::lightGray);
    foreach(KoShape* shape, shapeManager->shapes()) {
        if (dynamic_cast<KisShapeSelection*>(shape->parent())) {
            if (selectedShapes.contains(shape))
                shape->setStroke(border);
            else
                shape->setStroke(0);
        }
    }
    updateGUI();
}

void KisSelectionManager::imageResizeToSelection()
{
    KisImageResizeToSelectionActionFactory factory;
    factory.run(m_view);
}

void KisSelectionManager::paintSelectedShapes()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    QList<KoShape*> shapes = m_view->canvasBase()->shapeManager()->selection()->selectedShapes();

    KisPaintLayerSP paintLayer = new KisPaintLayer(image, i18n("Stroked Shapes"), OPACITY_OPAQUE_U8);

    m_adapter->beginMacro(i18n("Stroke Shapes"));
    m_adapter->addNode(paintLayer.data(), layer->parent().data(), layer.data());

    KisFigurePaintingToolHelper helper(i18n("Stroke Shapes"),
                                        image,
                                        m_view->canvasBase()->resourceManager(),
                                        KisPainter::StrokeStyleBrush,
                                        KisPainter::FillStyleNone);

    foreach(KoShape* shape, shapes) {    
        QTransform matrix = shape->absoluteTransformation(0) * QTransform::fromScale(image->xRes(), image->yRes());
        QPainterPath mapedOutline = matrix.map(shape->outline());
        helper.paintPainterPath(mapedOutline);
    }
    m_adapter->endMacro();
}


#include "kis_selection_manager.moc"
