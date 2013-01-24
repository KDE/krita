/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_selection_action_factories.h"

#include <klocale.h>
#include <kundo2command.h>
#include <KoMainWindow.h>
#include <KoDocumentEntry.h>
#include <KoServiceProvider.h>
#include "kis_view2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_clipboard.h"
#include "kis_pixel_selection.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_fill_painter.h"
#include "kis_transaction.h"
#include "kis_iterator_ng.h"
#include "kis_processing_applicator.h"
#include "commands/kis_selection_commands.h"
#include "commands/kis_image_layer_add_command.h"
#include "kis_tool_proxy.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_selection_manager.h"
#include "kis_selection_manager_p.h"


namespace ActionHelper {
    KisProcessingApplicator* beginAction(KisView2 *view, const QString &actionName) {
        KisImageSP image = view->image();
        Q_ASSERT(image);

        KisImageSignalVector emitSignals;
        emitSignals << ModifiedSignal;

        return new KisProcessingApplicator(image, 0,
                                           KisProcessingApplicator::NONE,
                                           emitSignals, actionName);
    }

    void endAction(KisProcessingApplicator *applicator, const QString &xmlData) {
        Q_UNUSED(xmlData);
        applicator->end();
        delete applicator;
    }

    void copyFromDevice(KisView2 *view, KisPaintDeviceSP device) {
        KisImageWSP image = view->image();
        KisSelectionSP selection = view->selection();

        QRect rc = (selection) ? selection->selectedExactRect() : image->bounds();

        KisPaintDeviceSP clip = new KisPaintDevice(device->colorSpace());
        Q_CHECK_PTR(clip);

        const KoColorSpace *cs = clip->colorSpace();

        // TODO if the source is linked... copy from all linked layers?!?

        // Copy image data
        KisPainter gc;
        gc.begin(clip);
        gc.setCompositeOp(COMPOSITE_COPY);
        gc.bitBlt(0, 0, device, rc.x(), rc.y(), rc.width(), rc.height());
        gc.end();

        if (selection) {
            // Apply selection mask.
            KisPaintDeviceSP selectionProjection = selection->projection();
            KisHLineIteratorSP layerIt = clip->createHLineIteratorNG(0, 0, rc.width());
            KisHLineConstIteratorSP selectionIt = selectionProjection->createHLineIteratorNG(rc.x(), rc.y(), rc.width());

            for (qint32 y = 0; y < rc.height(); y++) {

                for (qint32 x = 0; x < rc.width(); x++) {

                    cs->applyAlphaU8Mask(layerIt->rawData(), selectionIt->oldRawData(), 1);

                    layerIt->nextPixel();
                    selectionIt->nextPixel();
                }
                layerIt->nextRow();
                selectionIt->nextRow();
            }
        }

        KisClipboard::instance()->setClip(clip, rc.topLeft());
    }

    class KisTransactionBasedCommand : public KUndo2Command
    {
    public:
        KisTransactionBasedCommand(const QString &text = "",
                                   KUndo2Command *parent = 0)
            : KUndo2Command(text, parent),
              m_firstRedo(true),
              m_transactionData(0) {}

        ~KisTransactionBasedCommand() {
            delete m_transactionData;
        }

        void redo() {
            if (m_firstRedo) {
                m_transactionData = paint();
            }
            m_transactionData->redo();
        }
        void undo() {
            m_transactionData->undo();
        }

    protected:
        virtual KUndo2Command* paint() = 0;
    private:
        bool m_firstRedo;
        KUndo2Command *m_transactionData;
    };

}

void KisSelectAllActionFactory::run(KisView2 *view)
{
    KisProcessingApplicator *ap = ActionHelper::beginAction(view, i18n("Select All"));

    KisImageWSP image = view->image();
    if (!image->globalSelection()) {
        ap->applyCommand(new KisSetEmptyGlobalSelectionCommand(image),
                         KisStrokeJobData::SEQUENTIAL,
                         KisStrokeJobData::EXCLUSIVE);
    }

    struct SelectAll : public ActionHelper::KisTransactionBasedCommand {
        SelectAll(KisImageSP image) : m_image(image) {}
        KisImageSP m_image;
        KUndo2Command* paint() {
            KisSelectionSP selection = m_image->globalSelection();
            KisSelectionTransaction transaction(QString(), m_image->undoAdapter(), selection);
            selection->getOrCreatePixelSelection()->select(m_image->bounds());
            return transaction.endAndTake();
        }
    };

    ap->applyCommand(new SelectAll(image),
                     KisStrokeJobData::SEQUENTIAL,
                     KisStrokeJobData::EXCLUSIVE);

    ActionHelper::endAction(ap, KisUiActionConfiguration(id()).toXML());
}

void KisDeselectActionFactory::run(KisView2 *view)
{
    KUndo2Command *cmd = new KisDeselectGlobalSelectionCommand(view->image());

    KisProcessingApplicator *ap = ActionHelper::beginAction(view, cmd->text());
    ap->applyCommand(cmd, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    ActionHelper::endAction(ap, KisUiActionConfiguration(id()).toXML());
}

void KisReselectActionFactory::run(KisView2 *view)
{
    KUndo2Command *cmd = new KisReselectGlobalSelectionCommand(view->image());

    KisProcessingApplicator *ap = ActionHelper::beginAction(view, cmd->text());
    ap->applyCommand(cmd, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    ActionHelper::endAction(ap, KisUiActionConfiguration(id()).toXML());
}

void KisFillActionFactory::run(const QString &fillSource, KisView2 *view)
{
    KisNodeSP node = view->activeNode();
    if (!node || !node->isEditable()) return;

    KisSelectionSP selection = view->selection();
    QRect selectedRect = selection ?
        selection->selectedRect() : view->image()->bounds();
    KisPaintDeviceSP filled = new KisPaintDevice(node->paintDevice()->colorSpace());

    QString actionName;

    if (fillSource == "pattern") {
        KisFillPainter painter(filled);
        painter.fillRect(selectedRect.x(), selectedRect.y(),
                         selectedRect.width(), selectedRect.height(),
                         view->resourceProvider()->currentPattern());
        painter.end();
        actionName = i18n("Fill with Pattern");
    } else if (fillSource == "bg") {
        filled->setDefaultPixel(view->resourceProvider()->bgColor().data());
        actionName = i18n("Fill with Background Color");
    } else if (fillSource == "fg") {
        filled->setDefaultPixel(view->resourceProvider()->fgColor().data());
        actionName = i18n("Fill with Foreground Color");
    }

    struct BitBlt : public ActionHelper::KisTransactionBasedCommand {
        BitBlt(KisPaintDeviceSP src, KisPaintDeviceSP dst,
               KisSelectionSP sel, const QRect &rc)
            : m_src(src), m_dst(dst), m_sel(sel), m_rc(rc){}
        KisPaintDeviceSP m_src;
        KisPaintDeviceSP m_dst;
        KisSelectionSP m_sel;
        QRect m_rc;

        KUndo2Command* paint() {
            KisPainter gc(m_dst, m_sel);
            gc.beginTransaction("");
            gc.bitBlt(m_rc.x(), m_rc.y(),
                      m_src,
                      m_rc.x(), m_rc.y(),
                      m_rc.width(), m_rc.height());
            m_dst->setDirty(m_rc);
            return gc.endAndTakeTransaction();
        }
    };

    KisProcessingApplicator *ap = ActionHelper::beginAction(view, actionName);
    ap->applyCommand(new BitBlt(filled, view->activeDevice()/*node->paintDevice()*/, selection, selectedRect),
                     KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);

    KisUiActionConfiguration config(id());
    config.setProperty("fill-source", fillSource);

    ActionHelper::endAction(ap, config.toXML());
}

void KisClearActionFactory::run(KisView2 *view)
{
#ifdef __GNUC__
#warning "Add saving of XML data for Clear action"
#endif

    KisNodeSP node = view->activeNode();
    if (!node || !node->isEditable()) return;

    view->canvasBase()->toolProxy()->deleteSelection();
}

void KisApplySelectionFilterActionFactory::runFromXML(KisView2 *view, const KisUiActionConfiguration &config)
{
    QString filterName = config.getString("filter-name", "no-filter");

    KisSelectionFilter *filter = 0;

    int radius = config.getInt("radius", 1);
    int xRadius = config.getInt("x-radius", 1);
    int yRadius = config.getInt("y-radius", 1);
    bool edgeLock = config.getBool("edge-lock", false);

    if (filterName == "invert") {
        filter = new KisInvertSelectionFilter();
    } else if (filterName == "grow") {
        filter = new KisGrowSelectionFilter(xRadius, yRadius);
    } else if (filterName == "shrink") {
        filter = new KisShrinkSelectionFilter(xRadius, yRadius, edgeLock);
    } else if (filterName == "smooth") {
        filter = new KisSmoothSelectionFilter();
    } else if (filterName == "erode") {
        filter = new KisErodeSelectionFilter();
    } else if (filterName == "dilate") {
        filter = new KisDilateSelectionFilter();
    } else if (filterName == "border") {
        filter = new KisBorderSelectionFilter(xRadius, yRadius);
    } else if (filterName == "feather") {
        filter = new KisFeatherSelectionFilter(radius);
    }

    if (!filter) return;

    KisSelectionSP selection = view->selection();
    if (!selection) return;

    struct FilterSelection : public ActionHelper::KisTransactionBasedCommand {
        FilterSelection(KisImageSP image, KisSelectionSP sel, KisSelectionFilter *filter)
            : m_image(image), m_sel(sel), m_filter(filter) {}
        ~FilterSelection() { delete m_filter;}
        KisImageSP m_image;
        KisSelectionSP m_sel;
        KisSelectionFilter *m_filter;

        KUndo2Command* paint() {
            KisSelectionTransaction transaction("", m_image->undoAdapter(), m_sel);
            KisPixelSelectionSP mergedSelection = m_sel->getOrCreatePixelSelection();
            QRect processingRect = m_filter->changeRect(mergedSelection->selectedExactRect());
            m_filter->process(mergedSelection, processingRect);
            m_sel->setDirty(processingRect); // check if really needed
            return transaction.endAndTake();
        }
    };

    KisProcessingApplicator *ap = ActionHelper::beginAction(view, filter->name());
    ap->applyCommand(new FilterSelection(view->image(), selection, filter),
                     KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);
    ActionHelper::endAction(ap, config.toXML());
}

void KisImageResizeToSelectionActionFactory::run(KisView2 *view)
{
#ifdef __GNUC__
#warning "Add saving of XML data for Image Resize To Selection action"
#endif

    KisSelectionSP selection = view->selection();
    if (!selection) return;

    view->image()->cropImage(selection->selectedExactRect());
}

void KisCutCopyActionFactory::run(bool willCut, KisView2 *view)
{
    KisImageSP image = view->image();
    bool haveShapesSelected = view->selectionManager()->haveShapesSelected();

    if (haveShapesSelected) {
#ifdef __GNUC__
#warning "Add saving of XML data for Cut/Copy of shapes"
#endif

        image->barrierLock();
        if (willCut) {
            view->canvasBase()->toolProxy()->cut();
        } else {
            view->canvasBase()->toolProxy()->copy();
        }
        image->unlock();
    } else {
        KisNodeSP node = view->activeNode();
        if (!node) return;

        image->barrierLock();
        ActionHelper::copyFromDevice(view, node->paintDevice());
        image->unlock();

        KUndo2Command *command = 0;

        if (willCut && node->isEditable()) {
            struct ClearSelection : public ActionHelper::KisTransactionBasedCommand {
                ClearSelection(KisNodeSP node, KisSelectionSP sel)
                    : m_node(node), m_sel(sel) {}
                KisNodeSP m_node;
                KisSelectionSP m_sel;

                KUndo2Command* paint() {
                    KisSelectedTransaction transaction("", m_node);
                    m_node->paintDevice()->clearSelection(m_sel);
                    m_node->setDirty(m_sel->selectedRect());
                    return transaction.endAndTake();
                }
            };

            command = new ClearSelection(node, view->selection());
        }

        QString actionName = willCut ? i18n("Cut") : i18n("Copy");
        KisProcessingApplicator *ap = ActionHelper::beginAction(view, actionName);

        if (command) {
            ap->applyCommand(command,
                             KisStrokeJobData::SEQUENTIAL,
                             KisStrokeJobData::NORMAL);
        }

        KisUiActionConfiguration config(id());
        config.setProperty("will-cut", willCut);
        ActionHelper::endAction(ap, config.toXML());
    }
}

void KisCopyMergedActionFactory::run(KisView2 *view)
{
    KisImageWSP image = view->image();

    image->barrierLock();
    KisPaintDeviceSP dev = image->root()->projection();
    ActionHelper::copyFromDevice(view, dev);
    image->unlock();

    KisProcessingApplicator *ap = ActionHelper::beginAction(view, i18n("Copy Merged"));
    ActionHelper::endAction(ap, KisUiActionConfiguration(id()).toXML());
}

void KisPasteActionFactory::run(KisView2 *view)
{
    KisImageWSP image = view->image();

    //figure out where to position the clip
    // XXX: Fix this for internal points & zoom! (BSAR)
    QWidget * w = view->canvas();
    QPoint center = QPoint(w->width() / 2, w->height() / 2);
    QPoint bottomright = QPoint(w->width(), w->height());
    if (bottomright.x() > image->width())
        center.setX(image->width() / 2);
    if (bottomright.y() > image->height())
        center.setY(image->height() / 2);

    const KoCanvasBase* canvasBase = view->canvasBase();
    const KoViewConverter* viewConverter = view->canvasBase()->viewConverter();

    KisPaintDeviceSP clip = KisClipboard::instance()->clip(
        QPoint(
            viewConverter->viewToDocumentX(canvasBase->canvasController()->canvasOffsetX()) + center.x(),
            viewConverter->viewToDocumentY(canvasBase->canvasController()->canvasOffsetY()) + center.y()));

    if (clip) {
        // Pasted layer content could be outside image bounds and invisible, if that is the case move content into the bounds
        QRect exactBounds = clip->exactBounds();
        if (!exactBounds.isEmpty() && !exactBounds.intersects(image->bounds())) {
            clip->setX(clip->x() - exactBounds.x());
            clip->setY(clip->y() - exactBounds.y());
        }

        KisPaintLayer *newLayer = new KisPaintLayer(image.data(), image->nextLayerName() + i18n("(pasted)"), OPACITY_OPAQUE_U8, clip);
        KisNodeSP aboveNode = view->activeLayer();
        KisNodeSP parentNode = aboveNode ? aboveNode->parent() : image->root();

        KUndo2Command *cmd = new KisImageLayerAddCommand(image, newLayer, parentNode, aboveNode);
        KisProcessingApplicator *ap = ActionHelper::beginAction(view, cmd->text());
        ap->applyCommand(cmd, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);
        ActionHelper::endAction(ap, KisUiActionConfiguration(id()).toXML());
    } else {
#ifdef __GNUC__
#warning "Add saving of XML data for Paste of shapes"
#endif

        view->canvasBase()->toolProxy()->paste();
    }
}

void KisPasteNewActionFactory::run(KisView2 *view)
{
    Q_UNUSED(view);

    KisPaintDeviceSP clip = KisClipboard::instance()->clip(QPoint());
    if (!clip) return;

    QRect rect = clip->exactBounds();
    if (rect.isEmpty()) return;

    const QByteArray mimetype = KoServiceProvider::readNativeFormatMimeType();
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType(mimetype);

    QString error;
    KisPart2* part = dynamic_cast<KisPart2*>(entry.createKoPart(&error));
    if (!part) return;
    KisDoc2 *doc = new KisDoc2(part);
    if (!doc) return;
    part->setDocument(doc);

    KisImageSP image = new KisImage(doc->createUndoStore(),
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

    KoMainWindow *win = new KoMainWindow(part->componentData());
    win->show();
    win->setRootDocument(doc);
}
