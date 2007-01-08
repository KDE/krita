/*
 *  Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_layer_manager.h"

#include <QRect>
#include <QApplication>
#include <QCursor>
#include <QString>
#include <QDialog>

#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kcommand.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kurl.h>

#include <KoFilterManager.h>
#include <KoDocument.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoPointerEvent.h>
#include <KoPartSelectAction.h>
#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_filter.h>
#include <kis_selection.h>
#include <kis_transaction.h>
#include <kis_filter_strategy.h>
#include <kis_shear_visitor.h>
#include <kis_transform_worker.h>
#include <kis_meta_registry.h>
#include <kis_undo_adapter.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_zoom_manager.h"
#include "kis_doc2.h"
#include "kis_view2.h"
#include "kis_part_layer.h"
#include "kis_dlg_adj_layer_props.h"
#include "kis_dlg_layer_properties.h"
#include "kis_dlg_adjustment_layer.h"
#include "kis_dlg_new_layer.h"
#include "kis_selection_manager.h"
#include "kis_filter_manager.h"
#include "kis_resource_provider.h"
#include "kis_statusbar.h"
#include "kis_label_progress.h"

KisLayerManager::KisLayerManager( KisView2 * view, KisDoc2 * doc )
    : m_view( view )
    , m_doc( doc )
    , m_imgFlatten( 0 )
    , m_imgMergeLayer( 0 )
    , m_actionPartLayer( 0 )
    , m_actionAdjustmentLayer( 0 )
    , m_layerAdd( 0 )
    , m_layerBottom( 0 )
    , m_layerDup( 0 )
    , m_layerHide( 0 )
    , m_layerLower( 0 )
    , m_layerProperties( 0 )
    , m_layerRaise( 0 )
    , m_layerRm( 0 )
    , m_layerSaveAs( 0 )
    , m_layerTop( 0 )
    , m_actLayerVis( false )
    , m_imgResizeToLayer( 0 )
{
}

KisLayerManager::~KisLayerManager()
{
}

void KisLayerManager::setup(KActionCollection * actionCollection)
{
    m_imgFlatten  = new KAction(i18n("&Flatten image"), this);
    actionCollection->addAction("flatten_image", m_imgFlatten );
    m_imgFlatten->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_E));
    connect(m_imgFlatten, SIGNAL(triggered()), this, SLOT(flattenImage()));

    m_imgMergeLayer  = new KAction(i18n("&Merge with Layer Below"), this);
    actionCollection->addAction("merge_layer", m_imgMergeLayer );
    m_imgMergeLayer->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_E));
    connect(m_imgMergeLayer, SIGNAL(triggered()), this, SLOT(mergeLayer()));

    m_layerAdd  = new KAction(i18n("&Add..."), this);
    actionCollection->addAction("insert_layer", m_layerAdd );
    m_layerAdd->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_N));
    connect(m_layerAdd, SIGNAL(triggered()), this, SLOT(layerAdd()));

    m_actionPartLayer = new KoPartSelectAction( i18n( "&Object Layer" ), "frame_query",
                                                    this, SLOT( addPartLayer() ),
                                                    actionCollection, "insert_part_layer" );

    m_actionAdjustmentLayer  = new KAction(i18n("&Adjustment Layer"), this);
    actionCollection->addAction("insert_adjustment_layer", m_actionAdjustmentLayer );
    connect(m_actionAdjustmentLayer, SIGNAL(triggered()), this, SLOT(addAdjustmentLayer()));

    m_layerRm  = new KAction(i18n("&Remove"), this);
    actionCollection->addAction("remove_layer", m_layerRm );
    connect(m_layerRm, SIGNAL(triggered()), this, SLOT(layerRemove()));

    m_layerDup  = new KAction(i18n("Duplicate"), this);
    actionCollection->addAction("duplicate_layer", m_layerDup );
    connect(m_layerDup, SIGNAL(triggered()), this, SLOT(layerDuplicate()));

    m_layerHide  = new KToggleAction(i18n("&Hide"), this);
    actionCollection->addAction("hide_layer", m_layerHide );
    connect(m_layerHide, SIGNAL(triggered()), this,  SLOT(layerToggleVisible()));

    m_layerHide->setCheckedState(KGuiItem(i18n("&Show")));
    m_layerHide->setChecked(false);

    m_layerRaise  = new KAction(KIcon("raise"), i18n("Raise"), this);
    actionCollection->addAction("raiselayer", m_layerRaise );
    m_layerRaise->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_BracketRight));
    connect(m_layerRaise, SIGNAL(triggered()), this, SLOT(layerRaise()));

    m_layerLower  = new KAction(KIcon("lower"), i18n("Lower"), this);
    actionCollection->addAction("lowerlayer", m_layerLower );
    m_layerLower->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_BracketLeft));
    connect(m_layerLower, SIGNAL(triggered()), this, SLOT(layerLower()));

    m_layerTop  = new KAction(KIcon("bring_forward"), i18n("To Top"), this);
    actionCollection->addAction("toplayer", m_layerTop );
    m_layerTop->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_BracketRight));
    connect(m_layerTop, SIGNAL(triggered()), this, SLOT(layerFront()));

    m_layerBottom  = new KAction(KIcon("send_backward"), i18n("To Bottom"), this);
    actionCollection->addAction("bottomlayer", m_layerBottom );
    m_layerBottom->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_BracketLeft));
    connect(m_layerBottom, SIGNAL(triggered()), this, SLOT(layerBack()));

    m_layerProperties  = new KAction(i18n("Properties..."), this);
    actionCollection->addAction("layer_properties", m_layerProperties );
    connect(m_layerProperties, SIGNAL(triggered()), this, SLOT(layerProperties()));

    m_layerSaveAs  = new KAction(KIcon("filesave"), i18n("Save Layer as Image..."), this);
    actionCollection->addAction("save_layer_as_image", m_layerSaveAs );
    connect(m_layerSaveAs, SIGNAL(triggered()), this, SLOT(saveLayerAsImage()));

    KAction * action  = new KAction(KIcon("view_left_right"), i18n("Flip on &X Axis"), this);
    actionCollection->addAction("mirrorLayerX", action );
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorLayerX()));

    action  = new KAction(KIcon("view_top_bottom"), i18n("Flip on &Y Axis"), this);
    actionCollection->addAction("mirrorLayerY", action );
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorLayerY()));

    m_imgResizeToLayer  = new KAction(i18n("Resize Image to Size of Current Layer"), this);
    actionCollection->addAction("resizeimgtolayer", m_imgResizeToLayer );
    connect(m_imgResizeToLayer, SIGNAL(triggered()), this, SLOT(imgResizeToActiveLayer()));
}

void KisLayerManager::addAction(QAction * action)
{
    m_pluginActions.append(action);
}

void KisLayerManager::updateGUI()
{

    KisImageSP img = m_view->image();

    KisLayerSP layer;
    qint32 nlayers = 0;
    qint32 nvisible = 0;


    if (img) {
        layer = img->activeLayer();
        nlayers = img->nlayers();
        nvisible = nlayers - img->nHiddenLayers();
    }

#if 0

    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(layer.data());

    if (pl && ( m_currentColorChooserDisplay != KoID("BLA") ||
                pl->paintDevice()->colorSpace()->id() != m_currentColorChooserDisplay)) {
        m_currentColorChooserDisplay = pl->paintDevice()->colorSpace()->id();
    }
#endif

    bool enable = img && layer && layer->visible() && !layer->locked();

    m_layerDup->setEnabled(enable);
    m_layerRm->setEnabled(enable);
    m_layerHide->setEnabled(img && layer);
    m_layerProperties->setEnabled(enable);
    m_layerSaveAs->setEnabled(enable);
    m_layerRaise->setEnabled(enable && layer->prevSibling());
    m_layerLower->setEnabled(enable && layer->nextSibling());
    m_layerTop->setEnabled(enable && nlayers > 1 && layer != img->rootLayer()->firstChild());
    m_layerBottom->setEnabled(enable && nlayers > 1 && layer != img->rootLayer()->lastChild());

    // XXX these should be named layer instead of img
    m_imgFlatten->setEnabled(nlayers > 1);
    m_imgMergeLayer->setEnabled(nlayers > 1 && layer && layer->nextSibling());


#if 0 //Port if we want to keep embedded parts
    KisPartLayer * partLayer = dynamic_cast<KisPartLayer*>(layer.data());
    if (partLayer) {
        setCanvasCursor( KisCursor::arrowCursor() );
    }
#endif
    if (img && img->activeDevice())
        emit currentColorSpaceChanged(img->activeDevice()->colorSpace());

    m_imgResizeToLayer->setEnabled(img && img->activeLayer());

    m_view->statusBar()->setProfile(img);

}

void KisLayerManager::imgResizeToActiveLayer()
{
    KisImageSP img = m_view->image();
    KisLayerSP layer;
    KisUndoAdapter * undoAdapter = m_view->undoAdapter();

    if (img && (layer = img->activeLayer())) {

        if (undoAdapter && undoAdapter->undo()) {
            undoAdapter->beginMacro(i18n("Resize Image to Size of Current Layer"));
        }

        img->lock();

        QRect r = layer->exactBounds();
        img->resize(r.width(), r.height(), r.x(), r.y(), true);

        img->unlock();

        if (undoAdapter && undoAdapter->undo()) {
            undoAdapter->endMacro();
        }
    }
}

void KisLayerManager::layerCompositeOp(const KoCompositeOp* compositeOp)
{
    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    if (img->undo()) {
        KNamedCommand *cmd = layer->setCompositeOpCommand(compositeOp);
        cmd->execute();
        m_view->undoAdapter()->addCommand(cmd);
    }
}

// range: 0 - 100
void KisLayerManager::layerOpacity(int opacity, bool dontundo)
{
    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    opacity = int(float(opacity * 255) / 100 + 0.5);
    if (opacity > 255)
        opacity = 255;

    if (opacity == layer->opacity()) return;

    if (dontundo)
        layer->setOpacity( opacity );
    else
    {
        if (img->undo()) {
            KNamedCommand *cmd = layer->setOpacityCommand(opacity);
            cmd->execute();
            m_view->undoAdapter()->addCommand(cmd);
        }
    }
}

void KisLayerManager::layerOpacityFinishedChanging( int previous, int opacity )
{
    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    opacity = int(float(opacity * 255) / 100 + 0.5);
    if (opacity > 255)
        opacity = 255;

    previous = int(float(previous * 255) / 100 + 0.5);
    if (previous > 255)
        previous = 255;

    if (previous == opacity) return;

    if (img->undo()) {
        KNamedCommand *cmd = layer->setOpacityCommand(previous, opacity);
        m_view->undoAdapter()->addCommand(cmd);
    }
}

void KisLayerManager::layerToggleVisible()
{
    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    layer->setVisible(!layer->visible());
}

void KisLayerManager::layerToggleLocked()
{
    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    layer->setLocked(!layer->locked());
}

void KisLayerManager::actLayerVisChanged(int show)
{
    m_actLayerVis = (show != 0);
}

void KisLayerManager::layerProperties()
{
    if (m_view->image() && m_view->image()->activeLayer())
        showLayerProperties(m_view->image()->activeLayer());
}

namespace {
        class KisChangeFilterCmd : public KNamedCommand {
            typedef KNamedCommand super;
        public:
            // The QStrings are the _serialized_ configs
            KisChangeFilterCmd(KisAdjustmentLayerSP layer,
                               KisFilterConfiguration* config,
                               const QString& before,
                               const QString& after) : super(i18n("Change Filter"))
            {
                m_layer = layer;
                m_config = config;
                m_before = before;
                m_after = after;
            }
        public:
            virtual void execute()
            {
                QApplication::setOverrideCursor(KisCursor::waitCursor());
                m_config->fromXML(m_after);
                //Q_ASSERT(m_after == m_config->toString());
                m_layer->setFilter(m_config);
                m_layer->setDirty();
                QApplication::restoreOverrideCursor();
            }

            virtual void unexecute()
            {
                QApplication::setOverrideCursor(KisCursor::waitCursor());
                m_config->fromXML(m_before);
                //Q_ASSERT(m_before == m_config->toString());
                m_layer->setFilter(m_config);
                m_layer->setDirty();
                QApplication::restoreOverrideCursor();
            }
        private:
            KisAdjustmentLayerSP m_layer;
            KisFilterConfiguration* m_config;
            QString m_before;
            QString m_after;
        };
}



void KisLayerManager::showLayerProperties(KisLayerSP layer)
{
    Q_ASSERT( layer );
    if ( !layer ) return;

    KoColorSpace * cs = 0;
    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>( layer.data() );
    if ( pl ) {
        cs = pl->paintDevice()->colorSpace();
    }
    else {
        cs = layer->image()->colorSpace();
    }


    if (KisAdjustmentLayerSP alayer = KisAdjustmentLayerSP(dynamic_cast<KisAdjustmentLayer*>(layer.data())))
    {
        KisDlgAdjLayerProps dlg(alayer, alayer->name(), i18n("Adjustment Layer Properties"), m_view, "dlgadjlayerprops");
        QString before = dlg.filterConfiguration()->toString();
        if (dlg.exec() == QDialog::Accepted)
        {
            KisChangeFilterCmd * cmd = new KisChangeFilterCmd(alayer,
                    dlg.filterConfiguration(),
                    before,
                    dlg.filterConfiguration()->toString());
            cmd->execute();
            m_view->undoAdapter()->addCommand(cmd);
            m_doc->setModified( true );
        }
    }
    else
    {
        KisDlgLayerProperties dlg(layer->name(),
                                  layer->opacity(),
                                  layer->compositeOp(),
                                  cs);
        if (dlg.exec() == QDialog::Accepted)
        {
            if (layer->name() != dlg.getName() ||
                layer->opacity() != dlg.getOpacity() ||
                layer->compositeOp() != dlg.getCompositeOp())
            {
                QApplication::setOverrideCursor(KisCursor::waitCursor());
                m_view->undoAdapter()->beginMacro(i18n("Property Changes"));
                layer->image()->setLayerProperties(layer, dlg.getOpacity(), dlg.getCompositeOp(), dlg.getName());
                layer->setDirty();
                m_view->undoAdapter()->endMacro();
                QApplication::restoreOverrideCursor();
                m_doc->setModified( true );
            }
        }
    }
}

void KisLayerManager::layerAdd()
{
    KisImageSP img = m_view->image();
    if (img && img->activeLayer()) {
        addLayer(img->activeLayer()->parent(), img->activeLayer());
    }
    else if (img)
        addLayer(img->rootLayer(), KisLayerSP(0));
}

void KisLayerManager::addLayer(KisGroupLayerSP parent, KisLayerSP above)
{
    KisImageSP img = m_view->image();
    if (img) {
        KisConfig cfg;
        QString profilename;
        if(img->colorSpace()->profile())
            profilename = img->colorSpace()->profile()->productName();
        NewLayerDialog dlg(KoID(img->colorSpace()->id()), profilename, img->nextLayerName(), m_view);

        if (dlg.exec() == QDialog::Accepted) {
            KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(dlg.colorSpaceID(),dlg.profileName());
            KisLayerSP layer = KisLayerSP(new KisPaintLayer(img.data(), dlg.layerName(), dlg.opacity(), cs));
            if (layer) {
                layer->setCompositeOp(dlg.compositeOp());
                img->addLayer(layer, parent, above);

                m_view->canvas()->update();
            } else {
                KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
            }
        }
        else {
            img->rollBackLayerName();
        }
    }
}
void KisLayerManager::addGroupLayer(KisGroupLayerSP parent, KisLayerSP above)
{
    KisImageSP img = m_view->image();
    if (img) {
        QString profilename;
        if(img->colorSpace()->profile())
            profilename = img->colorSpace()->profile()->productName();
        KisConfig cfg;
        NewLayerDialog dlg(KoID(img->colorSpace()->id()), profilename, img->nextLayerName(), m_view);
        dlg.setColorSpaceEnabled(false);

        if (dlg.exec() == QDialog::Accepted) {
            KisLayerSP layer = KisLayerSP(new KisGroupLayer(img.data(), dlg.layerName(), dlg.opacity()));
            if (layer) {
                layer->setCompositeOp(dlg.compositeOp());
                img->addLayer(layer, parent, above);
                m_view->canvas()->update();
            } else {
                KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
            }
        }
    }
}

void KisLayerManager::insertPart(const QRect& viewRect, const KoDocumentEntry& entry,
                                 KisGroupLayerSP parent, KisLayerSP above)
{
#warning "Port or remove the part layers!"
    Q_UNUSED(viewRect);
    Q_UNUSED(entry);
    Q_UNUSED(parent);
    Q_UNUSED(above);
#if 0 // XXX: What shall we do with parts?
    KisImageSP img = m_view->image();
    if (!img) return;

    KoDocument* doc = entry.createDoc(0, m_doc);
    if ( !doc )
        return;

    if ( !doc->showEmbedInitDialog(m_view) )
        return;

    QRect rect = viewToWindow(viewRect);

    KisChildDoc * childDoc = m_doc->createChildDoc(rect, doc);

    KisPartLayerImpl* partLayer = new KisPartLayerImpl(img, childDoc);
    partLayer->setDocType(entry.service()->genericName());
    img->addLayer(KisLayerSP(partLayer), parent, above);
    m_doc->setModified(true);

    reconnectAfterPartInsert();
#endif
}


void KisLayerManager::addPartLayer()
{
    KisImageSP img = m_view->image();
    if (!img) return;

    addPartLayer(img->rootLayer(), img->rootLayer()->firstChild(), m_actionPartLayer->documentEntry());
}

void KisLayerManager::addPartLayer(KisGroupLayerSP parent, KisLayerSP above, const KoDocumentEntry& entry)
{
#warning "Port addPartLayer or remove it!"
    Q_UNUSED(parent);
    Q_UNUSED(above);
    Q_UNUSED(entry);
#if 0 // XXX: commented out because the canvas works differently and

    delete m_partHandler; // Only one at a time
    m_partHandler = new KisPartLayerHandler(m_view, entry, parent, above);
      // will we use embeddded parts in krita 2.0 at all?
    disconnect(m_canvas, SIGNAL(sigGotButtonPressEvent(KoPointerEvent*)), this, 0);
    disconnect(m_canvas, SIGNAL(sigGotButtonReleaseEvent(KoPointerEvent*)), this, 0);
    disconnect(m_canvas, SIGNAL(sigGotMoveEvent(KoPointerEvent*)), this, 0);
    disconnect(m_canvas, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)), this, 0);

    connect(m_canvas, SIGNAL(sigGotButtonPressEvent(KoPointerEvent*)),
            m_partHandler, SLOT(gotButtonPressEvent(KoPointerEvent*)));
    connect(m_canvas, SIGNAL(sigGotButtonReleaseEvent(KoPointerEvent*)),
            m_partHandler, SLOT(gotButtonReleaseEvent(KoPointerEvent*)));
    connect(m_canvas, SIGNAL(sigGotMoveEvent(KoPointerEvent*)),
            m_partHandler, SLOT(gotMoveEvent(KoPointerEvent*)));
    connect(m_canvas, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)),
            m_partHandler, SLOT(gotKeyPressEvent(QKeyEvent*)));

    connect(m_partHandler, SIGNAL(sigGotMoveEvent(KoPointerEvent*)),
            this, SLOT(canvasGotMoveEvent(KoPointerEvent*)));
    connect(m_partHandler, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)),
            this, SLOT(canvasGotKeyPressEvent(QKeyEvent*)));
    connect(m_partHandler, SIGNAL(handlerDone()),
            this, SLOT(reconnectAfterPartInsert()));
#endif
}

void KisLayerManager::reconnectAfterPartInsert() {
#if 0
    connect(m_canvas, SIGNAL(sigGotButtonPressEvent(KoPointerEvent*)),
            this, SLOT(canvasGotButtonPressEvent(KoPointerEvent*)));
    connect(m_canvas, SIGNAL(sigGotButtonReleaseEvent(KoPointerEvent*)),
            this, SLOT(canvasGotButtonReleaseEvent(KoPointerEvent*)));
    connect(m_canvas, SIGNAL(sigGotMoveEvent(KoPointerEvent*)),
            this, SLOT(canvasGotMoveEvent(KoPointerEvent*)));
    connect(m_canvas, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)),
            this, SLOT(canvasGotKeyPressEvent(QKeyEvent*)));

    delete m_partHandler;
    m_partHandler = 0;
#endif
}

void KisLayerManager::addAdjustmentLayer()
{
    KisImageSP img = m_view->image();
    if (!img) return;

    addAdjustmentLayer( img->activeLayer()->parent(), img->activeLayer() );
}

void KisLayerManager::addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above)
{
    Q_ASSERT(parent);
    Q_ASSERT(above);

    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP l = img->activeLayer();

    KisPaintDeviceSP dev;

    //  Argh! I hate having to cast, cast and cast again to see what kind of a layer I've got!
    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(l.data());
    if (pl) {
        dev = pl->paintDevice();
    }
    else {
        KisGroupLayer * gl = dynamic_cast<KisGroupLayer*>(l.data());
        if (gl) {
            dev = gl->projection();
        }
        else {
            KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(l.data());
            if (al) {
                dev = al->cachedPaintDevice();
            }
            else {
                return;
            }
        }
    }

    KisDlgAdjustmentLayer dlg(img.data(), img->nextLayerName(), i18n("New Adjustment Layer"), m_view, "dlgadjustmentlayer");
    if (dlg.exec() == QDialog::Accepted) {
        KisSelectionSP selection = KisSelectionSP(0);
        if (dev->hasSelection()) {
            selection = dev->selection();
        }
        KisFilterConfiguration * filter = dlg.filterConfiguration();
        QString name = dlg.layerName();

        addAdjustmentLayer( parent, above, name, filter, selection);

    }
}

void KisLayerManager::addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above, const QString & name,
                                 KisFilterConfiguration * filter, KisSelectionSP selection)
{
    Q_ASSERT(parent);
    Q_ASSERT(above);
    Q_ASSERT(filter);

    KisImageSP img = m_view->image();
    if (!img) return;

    KisAdjustmentLayer * l = new KisAdjustmentLayer(img, name, filter, selection);
    img->addLayer(KisLayerSP(l), parent, above);
}


void KisLayerManager::layerRemove()
{
    KisImageSP img = m_view->image();

    if (img) {
        KisLayerSP layer = img->activeLayer();

        if (layer) {


            img->removeLayer(layer);

            if (layer->parent())
                layer->parent()->setDirty(layer->extent());

            m_view->canvas()->update();
            m_view->updateGUI();
        }
    }
}

void KisLayerManager::layerDuplicate()
{
    KisImageSP img = m_view->image();

    if (!img)
        return;

    KisLayerSP active = img->activeLayer();

    if (!active)
        return;

    KisLayerSP dup = active->clone();
    dup->setName(i18n("Duplicate of '%1'",active->name()));
    img->addLayer(dup, active->parent(), active);
    if (dup) {
        img->activateLayer( dup );
        m_view->canvas()->update();
    } else {
        KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
    }
}

void KisLayerManager::layerRaise()
{
    KisImageSP img = m_view->image();
    KisLayerSP layer;

    if (!img)
        return;

    layer = img->activeLayer();

    img->raiseLayer(layer);
}

void KisLayerManager::layerLower()
{
    KisImageSP img = m_view->image();
    KisLayerSP layer;

    if (!img)
        return;

    layer = img->activeLayer();

    img->lowerLayer(layer);
}

void KisLayerManager::layerFront()
{
    KisImageSP img = m_view->image();
    KisLayerSP layer;

    if (!img)
        return;

    layer = img->activeLayer();
    img->toTop(layer);
}

void KisLayerManager::layerBack()
{
    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer;

    layer = img->activeLayer();
    img->toBottom(layer);
}

void KisLayerManager::rotateLayer180()
{
    rotateLayer( 180 );
}

void KisLayerManager::rotateLayerLeft90()
{
    rotateLayer( 270 );
}

void KisLayerManager::rotateLayerRight90()
{
    rotateLayer( 90 );
}

void KisLayerManager::mirrorLayerX()
{
    if (!m_view->image()) return;
    KisPaintDeviceSP dev = m_view->image()->activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Mirror Layer X"), dev);
        Q_CHECK_PTR(t);
    }

    dev->mirrorX();

    if (t) m_view->undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();
}

void KisLayerManager::mirrorLayerY()
{
    if (!m_view->image()) return;
    KisPaintDeviceSP dev = m_view->image()->activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Mirror Layer Y"), dev);
        Q_CHECK_PTR(t);
    }

    dev->mirrorY();

    if (t) m_view->undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();
}

void KisLayerManager::scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    if (!m_view->image()) return;

    KisPaintDeviceSP dev = m_view->image()->activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Scale Layer"), dev);
        Q_CHECK_PTR(t);
    }

    KisTransformWorker worker(dev, sx, sy, 0, 0, 0.0, 0, 0, m_view->statusBar()->progress(), filterStrategy);
    worker.run();

    if (t) m_view->undoAdapter()->addCommand(t);
    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();
}

void KisLayerManager::rotateLayer(double angle)
{
    if (!m_view->image()) return;

    KisPaintDeviceSP dev = m_view->image()->activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Rotate Layer"), dev);
        Q_CHECK_PTR(t);
    }

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->get(KoID("Triangle"));
    angle *= M_PI/180;
    qint32 w = m_view->image()->width();
    qint32 h = m_view->image()->height();
    qint32 tx = qint32((w*cos(angle) - h*sin(angle) - w) / 2 + 0.5);
    qint32 ty = qint32((h*cos(angle) + w*sin(angle) - h) / 2 + 0.5);

    KisTransformWorker tw(dev, 1.0, 1.0, 0, 0, angle, -tx, -ty, m_view->statusBar()->progress(), filter);
    tw.run();

    if (t) m_view->undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();
}

void KisLayerManager::shearLayer(double angleX, double angleY)
{
    if (!m_view->image()) return;

    KisLayerSP layer = m_view->image()->activeLayer();
    if (!layer) return;

    KisUndoAdapter * undo = 0;
    if ((undo = m_view->image()->undoAdapter())) {
        undo->beginMacro(i18n("Shear layer"));
    }

    KisShearVisitor v(angleX, angleY, m_view->statusBar()->progress());
    v.setUndoAdapter(undo);
    layer->accept(v);

    if (undo) undo->endMacro();

    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();
}

void KisLayerManager::flattenImage()
{
    KisImageSP img = m_view->image();

    if (img) {
        bool doIt = true;

        if (img->nHiddenLayers() > 0) {
            int answer = KMessageBox::warningYesNo(m_view,
                                                   i18n("The image contains hidden layers that will be lost."),
                                                   i18n("Flatten Image"),
                                                   KGuiItem(i18n("&Flatten Image")),
                                                   KStandardGuiItem::cancel());

            if (answer != KMessageBox::Yes) {
                doIt = false;
            }
        }

        if (doIt) {
            img->flatten();
        }
    }
}

void KisLayerManager::mergeLayer()
{
    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    img->mergeLayer(layer);
    m_view->updateGUI();

}

void KisLayerManager::layersUpdated()
{
    kDebug(41007) << "layersUpdated called\n";

    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    m_view->updateGUI();
    m_view->resourceProvider()->slotLayerActivated( layer );

}

void KisLayerManager::saveLayerAsImage()
{
    QStringList listMimeFilter = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Export);
    QString mimelist = listMimeFilter.join(" ");

    KFileDialog fd (KUrl(QString::null), mimelist, m_view);
    fd.setObjectName("Export Layer");
    fd.setCaption(i18n("Export Layer"));
    fd.setMimeFilter(listMimeFilter);
    fd.setOperationMode(KFileDialog::Saving);

    if (!fd.exec()) return;

    KUrl url = fd.selectedUrl();
    QString mimefilter = fd.currentMimeFilter();

    if (url.isEmpty())
        return;


    KisImageSP img = m_view->image();
    if (!img) return;

    KisLayerSP l = img->activeLayer();
    if (!l) return;

    QRect r = l->exactBounds();

    KisDoc2 d;
    d.prepareForImport();

    KisImageSP dst = KisImageSP(new KisImage(d.undoAdapter(), r.width(), r.height(), img->colorSpace(), l->name()));
    d.setCurrentImage( dst );
    dst->addLayer(l->clone(),dst->rootLayer(),KisLayerSP(0));

    d.setOutputMimeType(mimefilter.toLatin1());
    d.exp0rt(url);
}



bool KisLayerManager::activeLayerHasSelection()
{
    return m_view->image() && m_view->image()->activeDevice() && m_view->image()->activeDevice()->hasSelection();
}

void KisLayerManager::handlePartLayerAdded(KisLayerSP layer)
{
    KisPartLayer* l = dynamic_cast<KisPartLayer*>(layer.data());
    if (!l)
        return;
#if 0 // XXX: What shall we do with part layers?
    connect(m_view, SIGNAL(childActivated(KoDocumentChild*)),
            layer.data(), SLOT(childActivated(KoDocumentChild*)));
#endif
}


#include "kis_layer_manager.moc"

