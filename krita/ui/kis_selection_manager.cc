
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

#include "KoCanvasController.h"
#include "KoChannelInfo.h"
#include "KoIntegerMaths.h"
#include <KoDocument.h>
#include <KoMainWindow.h>
#include <KoDocumentEntry.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoLineBorder.h>
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
#include "kis_fill_painter.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_iterator_pixel_trait.h"
#include "kis_iterators_pixel.h"
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

#include "kis_clipboard.h"
#include "kis_view2.h"

#include "kis_selection_manager_p.h"


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
        m_toNewLayer(0),
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

    m_toNewLayer  = new KAction(i18n("Copy Selection to New Layer"), this);
    collection->addAction("copy_selection_to_new_layer", m_toNewLayer);
    m_toNewLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
    connect(m_toNewLayer, SIGNAL(triggered()), this, SLOT(copySelectionToNewLayer()));

    m_cutToNewLayer  = new KAction(i18n("Cut Selection to New Layer"), this);
    collection->addAction("cut_selection_to_new_layer", m_cutToNewLayer);
    m_cutToNewLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_J));
    connect(m_cutToNewLayer, SIGNAL(triggered()), this, SLOT(cutToNewLayer()));

    m_fillForegroundColor  = new KAction(i18n("Fill with Foreground Color"), this);
    collection->addAction("fill_selection_foreground_color", m_fillForegroundColor);
    m_fillForegroundColor->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Backspace));
    connect(m_fillForegroundColor, SIGNAL(triggered()), this, SLOT(fillForegroundColor()));

    m_fillBackgroundColor  = new KAction(i18n("Fill with Background Color"), this);
    collection->addAction("fill_selection_background_color", m_fillBackgroundColor);
    m_fillBackgroundColor->setShortcut(QKeySequence(Qt::Key_Backspace));
    connect(m_fillBackgroundColor, SIGNAL(triggered()), this, SLOT(fillBackgroundColor()));

    m_fillPattern  = new KAction(i18n("Fill with Pattern"), this);
    collection->addAction("fill_selection_pattern", m_fillPattern);
    connect(m_fillPattern, SIGNAL(triggered()), this, SLOT(fillPattern()));

    m_toggleDisplaySelection  = new KToggleAction(i18n("Display Selection"), this);
    collection->addAction("toggle_display_selection", m_toggleDisplaySelection);
    m_toggleDisplaySelection->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_H));
    connect(m_toggleDisplaySelection, SIGNAL(triggered()), this, SLOT(toggleDisplaySelection()));

    m_toggleDisplaySelection->setCheckedState(KGuiItem(i18n("Hide Selection")));
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


void KisSelectionManager::updateGUI()
{
    Q_ASSERT(m_view);
    Q_ASSERT(m_clipboard);

    if (m_view == 0) {
        // "Eek, no parent!
        return;
    }

    if (m_clipboard == 0) {
        // Eek, no clipboard!
        return;
    }

    KisImageWSP image = m_view->image();
    KisLayerSP l;
    KisPaintDeviceSP dev;

    bool enable = false;

    if (image && m_view->activeDevice() && m_view->activeLayer()) {
        l = m_view->activeLayer();

        enable = l && !l->userLocked() && l->visible();
#if 0 // XXX_SELECTION (how are we going to handle deselect and
        // reselect now?
        if (l->inherits("KisAdjustmentLayer")
                if (dev && !adjLayer)
                    m_reselect->setEnabled(dev->selectionDeselected());
                    if (adjLayer) // There's no reselect for adjustment layers
                        m_reselect->setEnabled(false);
#endif
                    }

    m_clear->setEnabled(enable);
    m_cut->setEnabled(enable);
    m_fillForegroundColor->setEnabled(enable);
    m_fillBackgroundColor->setEnabled(enable);
    m_fillPattern->setEnabled(enable);

    m_cutToNewLayer->setEnabled(enable && l->selection());
    m_selectAll->setEnabled(!image.isNull());

    bool hasPixelSelection = enable && l->selection() && l->selection()->hasPixelSelection()
                             && !m_view->selection()->isDeselected();
    m_invert->setEnabled(hasPixelSelection);

    m_smooth->setEnabled(enable);
//    m_load->setEnabled(enable);
//    m_save->setEnabled(enable);


    if (m_view->selection() && !m_view->selection()->isDeselected())
        m_deselect->setEnabled(true);
    else
        m_deselect->setEnabled(false);

    if (m_view->selection() && m_view->selection()->isDeselected())
        m_reselect->setEnabled(true);
    else
        m_reselect->setEnabled(false);

    m_imageResizeToSelection->setEnabled(m_view->selection() && !m_view->selection()->isDeselected());
    if (!m_pluginActions.isEmpty()) {
        QListIterator<QAction *> i(m_pluginActions);

        while (i.hasNext()) {
            i.next()->setEnabled(!image.isNull());
        }
    }

    // You can copy from locked layers and paste the clip into a new layer, even when
    // the current layer is locked.

    enable = false;
    if (image && l) {
        enable = l->selection() && l->visible();


    }

    l = m_view->activeLayer();
    KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(l.data());
    bool shapePasteEnable = false;
    bool shapeCopyEnable = false;
    if (shapeLayer) {

        shapeCopyEnable = true;

        const QMimeData* data = QApplication::clipboard()->mimeData();
        if (data) {
            QStringList mimeTypes = m_view->canvasBase()->toolProxy()->supportedPasteMimeTypes();
            foreach(const QString & mimeType, mimeTypes) {
                if (data->hasFormat(mimeType)) {
                    shapePasteEnable = true;
                    break;
                }
            }
        }
    }

    m_copy->setEnabled(enable || shapeCopyEnable);
    m_copyMerged->setEnabled(image->rootLayer()->childCount() > 0);
    m_paste->setEnabled(!image.isNull() && (m_clipboard->hasClip() || shapePasteEnable));
    m_pasteNew->setEnabled(!image.isNull() && m_clipboard->hasClip());
    m_toNewLayer->setEnabled(enable);

    //Handle the clear action disponibility
    
    if (m_view->canvasBase()->shapeManager()->selection()->count() > 0) {
        m_clear->setEnabled(true);
    }
    else if (shapeLayer && m_view->canvasBase()->shapeManager()->shapes().empty()){
        m_clear->setEnabled(false);
    }
    else {
        m_clear->setEnabled(true);
    }
        
    updateStatusBar();

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
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    if (!m_view->selection()) return;

    copy();

    KisSelectedTransaction transaction(i18n("Cut"), layer);

    layer->paintDevice()->clearSelection(m_view->selection());
    QRect rect = m_view->selection()->selectedRect();
    deselect();

    transaction.commit(m_view->image()->undoAdapter());

    layer->setDirty(rect);
}

void KisSelectionManager::copy()
{
    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(layer.data());
    if (shapeLayer) {
        m_view->canvasBase()->toolProxy()->copy();
    } 
    else {

        KisImageWSP image = m_view->image();
        if (!image) return;

        KisPaintDeviceSP dev = m_view->activeDevice();
        if (!dev) return;
        
        copyFromDevice(dev);
    }

    selectionChanged();
}

void KisSelectionManager::copyMerged()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    image->lock();
    KisPaintDeviceSP dev = image->rootLayer()->projection();
    copyFromDevice(dev);
    image->unlock();
}

void KisSelectionManager::paste()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    //figure out where to position the clip
    // XXX: Fix this for internal points & zoom! (BSAR)
    QWidget * w = m_view->canvas();
    QPoint center = QPoint(w->width() / 2, w->height() / 2);
    QPoint bottomright = QPoint(w->width(), w->height());
    if (bottomright.x() > image->width())
        center.setX(image->width() / 2);
    if (bottomright.y() > image->height())
        center.setY(image->height() / 2);

    const KoCanvasBase* canvasBase = m_view->canvasBase();
    const KoViewConverter* viewConverter = m_view->canvasBase()->viewConverter();

    KisPaintDeviceSP clip = m_clipboard->clip(
        QPoint(
            viewConverter->viewToDocumentX(canvasBase->canvasController()->canvasOffsetX()) + center.x(),
            viewConverter->viewToDocumentY(canvasBase->canvasController()->canvasOffsetY()) + center.y()));

    if (clip) {
        KisPaintLayer *layer = new KisPaintLayer(image.data(), image->nextLayerName() + i18n("(pasted)"), OPACITY_OPAQUE_U8, clip);
        Q_CHECK_PTR(layer);

        if (m_view->activeLayer()) {
            m_adapter->addNode(layer , m_view->activeLayer()->parent(), m_view->activeLayer().data());
        } else {
            m_adapter->addNode(layer , image->rootLayer(), 0);
        }
        layer->setDirty();
        m_view->nodeManager()->activateNode(layer);

    } else
        m_view->canvasBase()->toolProxy()->paste();
}

void KisSelectionManager::pasteAt()
{
    //XXX
}

void KisSelectionManager::pasteNew()
{
    KisPaintDeviceSP clip = m_clipboard->clip(QPoint());
    if (!clip) return;

    QRect rect = clip->exactBounds();
    if (rect.isEmpty()) return;

    const QByteArray mimetype = KoDocument::readNativeFormatMimeType();
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType(mimetype);

    KisDoc2* doc = dynamic_cast<KisDoc2*>(entry.createDoc());
    if (!doc) return;

    Q_ASSERT(doc->undoAdapter());


    KisImageWSP image = new KisImage(doc->undoAdapter(),
                                     rect.width(),
                                     rect.height(),
                                     clip->colorSpace(),
                                     i18n("Pasted"));
    KisPaintLayerSP layer =
        new KisPaintLayer(image.data(), clip->objectName(),
                          OPACITY_OPAQUE_U8, clip->colorSpace());

    KisPainter p(layer->paintDevice());
    p.setCompositeOp(COMPOSITE_COPY);
    p.bitBlt(0, 0, clip, rect.x(), rect.y(), rect.width(), rect.height());
    p.end();

    image->addNode(layer.data(), image->rootLayer());
    doc->setCurrentImage(image);

    KoMainWindow *win = new KoMainWindow(doc->componentData());
    win->show();
    win->setRootDocument(doc);
}

void KisSelectionManager::selectAll()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisUndoAdapter *undoAdapter = image->undoAdapter();
    undoAdapter->beginMacro(i18n("Select All"));

    if (!image->globalSelection()) {
        QUndoCommand *cmd = new KisSetGlobalSelectionCommand(image, 0, 0);
        undoAdapter->addCommand(cmd);
    }

    KisSelectionSP selection = image->globalSelection();

    KisSelectionTransaction transaction(QString(), image, selection);
    selection->getOrCreatePixelSelection()->select(image->bounds());
    transaction.commit(undoAdapter);

    undoAdapter->endMacro();

    selectionChanged();
}

void KisSelectionManager::deselect()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    if (image->globalSelection()) {
        QUndoCommand *cmd = new KisDeselectGlobalSelectionCommand(image);
        image->undoAdapter()->addCommand(cmd);
    }
}

void KisSelectionManager::reselect()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    if (image->globalSelection()) {
        QUndoCommand *cmd = new KisReselectGlobalSelectionCommand(image);
        image->undoAdapter()->addCommand(cmd);
    }
}


void KisSelectionManager::clear()
{
    m_view->canvasBase()->toolProxy()->deleteSelection();
    updateGUI();
}

void KisSelectionManager::fill(const KoColor& color, bool fillWithPattern, const QString& transactionText)
{
    KisPaintDeviceSP device = m_view->activeDevice();
    if (!device) return;

    KisSelectionSP selection = m_view->selection();
    QRect selectedRect = selection->selectedRect();
    KisPaintDeviceSP filled = new KisPaintDevice(device->colorSpace());

    if (fillWithPattern) {
        KisFillPainter painter(filled);
        painter.fillRect(selectedRect.x(), selectedRect.y(),
                         selectedRect.width(), selectedRect.height(),
                         m_view->resourceProvider()->currentPattern());
        painter.end();
    } else {
        filled->setDefaultPixel(color.data());
    }

    KisPainter painter2(device, selection);

    painter2.beginTransaction(transactionText);
    painter2.bitBlt(selectedRect.x(), selectedRect.y(),
                    filled,
                    selectedRect.x(), selectedRect.y(),
                    selectedRect.width(), selectedRect.height());
    painter2.endTransaction(m_view->undoAdapter());

    device->setDirty(selectedRect);
}

void KisSelectionManager::fillForegroundColor()
{
    fill(m_view->resourceProvider()->fgColor(), false, i18n("Fill with Foreground Color"));
}

void KisSelectionManager::fillBackgroundColor()
{
    fill(m_view->resourceProvider()->bgColor(), false, i18n("Fill with Background Color"));
}

void KisSelectionManager::fillPattern()
{
    fill(KoColor(), true, i18n("Fill with Pattern"));
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

void KisSelectionManager::applySelectionFilter(KisSelectionFilter *filter)
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisSelectionSP selection = m_view->selection();
    if (!selection) return;

    KisSelectionTransaction transaction(filter->name(), image, selection);

    KisPixelSelectionSP mergedSelection = selection->mergedPixelSelection();
    QRect processingRect = filter->changeRect(mergedSelection->selectedExactRect());

    filter->process(mergedSelection, processingRect);

    transaction.commit(image->undoAdapter());
    selection->setDirty(processingRect);
    selectionChanged();
}

void KisSelectionManager::invert()
{
    applySelectionFilter(new KisInvertSelectionFilter());
}

void KisSelectionManager::grow(qint32 xRadius, qint32 yRadius)
{
    applySelectionFilter(new KisGrowSelectionFilter(xRadius, yRadius));
}

void KisSelectionManager::shrink(qint32 xRadius, qint32 yRadius, bool edgeLock)
{
    applySelectionFilter(new KisShrinkSelectionFilter(xRadius, yRadius, edgeLock));
}

void KisSelectionManager::smooth()
{
    applySelectionFilter(new KisSmoothSelectionFilter());
}

void KisSelectionManager::erode()
{
    applySelectionFilter(new KisErodeSelectionFilter());
}

void KisSelectionManager::dilate()
{
    applySelectionFilter(new KisDilateSelectionFilter());
}

void KisSelectionManager::border(qint32 xRadius, qint32 yRadius)
{
    applySelectionFilter(new KisBorderSelectionFilter(xRadius, yRadius));
}

void KisSelectionManager::feather(qint32 radius)
{
    applySelectionFilter(new KisFeatherSelectionFilter(radius));
}

void KisSelectionManager::shapeSelectionChanged()
{
    KoShapeManager* shapeManager = m_view->canvasBase()->globalShapeManager();

    KoSelection * selection = shapeManager->selection();
    QList<KoShape*> selectedShapes = selection->selectedShapes();

    KoLineBorder* border = new KoLineBorder(0, Qt::lightGray);
    foreach(KoShape* shape, shapeManager->shapes()) {
        if (dynamic_cast<KisShapeSelection*>(shape->parent())) {
            if (selectedShapes.contains(shape))
                shape->setBorder(border);
            else
                shape->setBorder(0);
        }
    }
}

void KisSelectionManager::imageResizeToSelection()
{
    KisSelectionSP selection = m_view->selection();
    KisImageWSP image = m_view->image();

    if (image && selection) {
        image->undoAdapter()->beginMacro(i18n("Resize Image to Size of Selection"));
        image->resize(selection->selectedExactRect(), true);
        image->undoAdapter()->endMacro();
    }
}

void KisSelectionManager::copyFromDevice(KisPaintDeviceSP device)
{
    KisImageWSP image = m_view->image();
    if (!image) return;
    
    KisSelectionSP selection = m_view->selection();

    QRect r = (selection) ? selection->selectedExactRect() : image->bounds();

    KisPaintDeviceSP clip = new KisPaintDevice(device->colorSpace());
    Q_CHECK_PTR(clip);

    const KoColorSpace * cs = clip->colorSpace();

    // TODO if the source is linked... copy from all linked layers?!?

    // Copy image data
    KisPainter gc;
    gc.begin(clip);
    gc.setCompositeOp(COMPOSITE_COPY);
    gc.bitBlt(0, 0, device, r.x(), r.y(), r.width(), r.height());
    gc.end();

    if (selection) {
        // Apply selection mask.
        KisPixelSelectionSP selectionProjection = selection->projection();
        KisHLineIteratorPixel layerIt = clip->createHLineIterator(0, 0, r.width());
        KisHLineConstIteratorPixel selectionIt = selectionProjection->createHLineIterator(r.x(), r.y(), r.width());

        for (qint32 y = 0; y < r.height(); y++) {

            while (!layerIt.isDone()) {

                cs->applyAlphaU8Mask(layerIt.rawData(), selectionIt.rawData(), 1);


                ++layerIt;
                ++selectionIt;
            }
            layerIt.nextRow();
            selectionIt.nextRow();
        }
    }

    m_clipboard->setClip(clip, r.topLeft());
}

#include "kis_selection_manager.moc"
