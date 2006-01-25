/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qobject.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qcolor.h>

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kstdaction.h>

#include <koDocument.h>
#include <koMainWindow.h>
#include <koQueryTrader.h>

#include "kis_clipboard.h"
#include "kis_types.h"
#include "kis_view.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_painter.h"
#include "kis_iterators_pixel.h"
#include "kis_iteratorpixeltrait.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device_impl.h"
#include "kis_channelinfo.h"
#include "kis_dlg_apply_profile.h"
#include "kis_config.h"
#include "kis_debug_areas.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_selected_transaction.h"
#include "kis_convolution_painter.h"
#include "kis_integer_maths.h"
#include "kis_fill_painter.h"
#include "kis_canvas.h"

KisSelectionManager::KisSelectionManager(KisView * parent, KisDoc * doc)
    : m_parent(parent),
      m_doc(doc),
      m_copy(0),
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
      m_feather(0),
      m_border(0),
      m_expand(0),
      m_smooth(0),
      m_contract(0),
      m_grow(0),
      m_similar(0),
      m_transform(0),
      m_load(0),
      m_save(0),
      m_fillForegroundColor(0),
      m_fillBackgroundColor(0),
      m_fillPattern(0)
{
    m_pluginActions.setAutoDelete(true);
    m_clipboard = KisClipboard::instance();
}

KisSelectionManager::~KisSelectionManager()
{
    m_pluginActions.clear();
}


void KisSelectionManager::setup(KActionCollection * collection)
{
    // XXX: setup shortcuts!

    m_cut = KStdAction::cut(this,
            SLOT(cut()),
            collection,
            "cut");

    m_copy = KStdAction::copy(this,
                SLOT(copy()),
                collection,
                "copy");

    m_paste = KStdAction::paste(this,
                SLOT(paste()),
                collection,
                "paste");

    m_pasteNew = new KAction(i18n("Paste into &New Image"),
                0, 0,
                this, SLOT(pasteNew()),
                collection,
                "paste_new");


    m_selectAll = KStdAction::selectAll(this,
                    SLOT(selectAll()),
                    collection,
                    "select_all");

    m_deselect = KStdAction::deselect(this,
                    SLOT(deselect()),
                    collection,
                    "deselect");


    m_clear = KStdAction::clear(this,
                SLOT(clear()),
                collection,
                "clear");

    m_reselect = new KAction(i18n("&Reselect"),
                0, "Ctrl+Shift+D",
                this, SLOT(reselect()),
                collection, "reselect");

    m_invert = new KAction(i18n("&Invert"),
                0, "Ctrl+I",
                this, SLOT(invert()),
                collection, "invert");


    m_toNewLayer = new KAction(i18n("Copy Selection to New Layer"),
                0, "Ctrl+J",
                this, SLOT(copySelectionToNewLayer()),
                collection, "copy_selection_to_new_layer");


    m_cutToNewLayer = new KAction(i18n("Cut Selection to New Layer"),
            0, "Ctrl+Shift+J",
            this, SLOT(cutToNewLayer()),
            collection, "cut_selection_to_new_layer");

    m_feather = new KAction(i18n("Feather"),
                0, "Ctrl+Alt+D",
                this, SLOT(feather()),
                collection, "feather");

    m_fillForegroundColor = new KAction(i18n("Fill with Foreground Color"), 
                                             "Alt+backspace", this, 
                                             SLOT(fillForegroundColor()), 
                                             collection, 
                                             "fill_selection_foreground_color");
    m_fillBackgroundColor = new KAction(i18n("Fill with Background Color"), 
                                             "backspace", this, 
                                             SLOT(fillBackgroundColor()), 
                                             collection,
                                             "fill_selection_background_color");
    m_fillPattern = new KAction(i18n("Fill with Pattern"), 
                                             0, this, 
                                             SLOT(fillPattern()), 
                                             collection,
                                             "fill_selection_pattern");

    m_toggleDisplaySelection = new KToggleAction(i18n("Display Selection"), "", this, SLOT(toggleDisplaySelection()), collection, "toggle_display_selection");
    m_toggleDisplaySelection->setCheckedState(KGuiItem(i18n("Hide Selection")));
    m_toggleDisplaySelection->setChecked(true);
    
#if 0 // Not implemented yet
    m_border =
        new KAction(i18n("Border..."),
                0, 0,
                this, SLOT(border()),
                collection, "border");

    m_expand =
        new KAction(i18n("Expand..."),
                0, 0,
                this, SLOT(expand()),
                collection, "expand");

    m_smooth =
        new KAction(i18n("Smooth..."),
                0, 0,
                this, SLOT(smooth()),
                collection, "smooth");


    m_contract =
        new KAction(i18n("Contract..."),
                0, 0,
                this, SLOT(contract()),
                collection, "contract");

    m_grow =
        new KAction(i18n("Grow"),
                0, 0,
                this, SLOT(grow()),
                collection, "grow");

    m_similar =
        new KAction(i18n("Similar"),
                0, 0,
                this, SLOT(similar()),
                collection, "similar");


    m_transform
        = new KAction(i18n("Transform..."),
                  0, 0,
                  this, SLOT(transform()),
                  collection, "transform_selection");


    m_load
        = new KAction(i18n("Load..."),
                  0, 0,
                  this, SLOT(load()),
                  collection, "load_selection");


    m_save
        = new KAction(i18n("Save As..."),
                  0, 0,
                  this, SLOT(save()),
                  collection, "save_selection");
#endif

        QClipboard *cb = QApplication::clipboard();
        connect(cb, SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));
}

void KisSelectionManager::clipboardDataChanged()
{
    updateGUI();
}


void KisSelectionManager::addSelectionAction(KAction * action)
{
    m_pluginActions.append(action);
}


void KisSelectionManager::updateGUI()
{
    Q_ASSERT(m_parent);
    Q_ASSERT(m_clipboard);

    if (m_parent == 0) {
        // "Eek, no parent!
        return;
    }

    if (m_clipboard == 0) {
        // Eek, no clipboard!
        return;
    }

    KisImageSP img = m_parent -> currentImg();
    KisLayerSP l = 0;
    KisPaintDeviceImplSP dev = 0;

    bool enable = false;
    if (img) {
        l = img -> activeLayer();
        dev = img -> activeDevice();

        enable = l && dev&& dev -> hasSelection() && !l -> locked() && l -> visible();

        if(dev)
            m_reselect -> setEnabled( dev -> selectionDeselected() );
    }

    m_cut -> setEnabled(enable);
    m_cutToNewLayer->setEnabled(enable);
    m_selectAll -> setEnabled(img != 0);
    m_deselect -> setEnabled(enable);
    m_clear -> setEnabled(enable);
    m_fillForegroundColor -> setEnabled(enable);
    m_fillBackgroundColor -> setEnabled(enable);
    m_fillPattern -> setEnabled(enable);
    m_invert -> setEnabled(enable);

    m_feather -> setEnabled(enable);
#if 0 // Not implemented yet
    m_border -> setEnabled(enable);
    m_expand -> setEnabled(enable);
    m_smooth -> setEnabled(enable);
    m_contract -> setEnabled(enable);
    m_grow -> setEnabled(enable);
    m_similar -> setEnabled(enable);
    m_transform -> setEnabled(enable);
    m_load -> setEnabled(enable);
    m_save -> setEnabled(enable);
#endif


    KAction * a;
    for (a = m_pluginActions.first(); a; a = m_pluginActions.next()) {
        a->setEnabled(img != 0);
    }

    // You can copy from locked layers and paste the clip into a new layer, even when
    // the current layer is locked.
    enable = false;
    if (img && l && dev) {
        enable = dev->hasSelection() && l->visible();
    }

    m_copy -> setEnabled(enable);
    m_paste -> setEnabled(img != 0 && m_clipboard -> hasClip());
    m_pasteNew -> setEnabled(img != 0 && m_clipboard -> hasClip());
    m_toNewLayer -> setEnabled(enable);

    m_parent -> updateStatusBarSelectionLabel();

}

void KisSelectionManager::imgSelectionChanged(KisImageSP img)
{
    kdDebug(DBG_AREA_CORE) << "KisSelectionManager::imgSelectionChanged\n";
        if (img == m_parent -> currentImg()) {
                updateGUI();
    }

}

void KisSelectionManager::cut()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    if (!dev -> hasSelection()) return;

    copy();

    KisSelectedTransaction *t = 0;

    if (img -> undoAdapter()) {
        t = new KisSelectedTransaction(i18n("Cut"), dev);
        Q_CHECK_PTR(t);
    }

    dev -> clearSelection();
    dev -> deselect();

    if (img -> undoAdapter()) {
        img -> undoAdapter() -> addCommand(t);
    }

    dev -> emitSelectionChanged();
}

void KisSelectionManager::copy()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    if (!dev -> hasSelection()) return;

    KisSelectionSP selection = dev -> selection();

    QRect r = selection -> selectedExactRect();

    kdDebug(DBG_AREA_CORE) << "Selection rect: "
          << r.x() << ", "
          << r.y() << ", "
          << r.width() << ", "
          << r.height() << "\n";

    KisPaintDeviceImplSP clip = new KisPaintDeviceImpl(dev -> colorSpace());
    Q_CHECK_PTR(clip);

    KisColorSpace * cs = clip->colorSpace();

    // TODO if the source is linked... copy from all linked layers?!?

    // Copy image data
    KisPainter gc;
    gc.begin(clip);
    gc.bitBlt(0, 0, COMPOSITE_COPY, dev, r.x(), r.y(), r.width(), r.height());
    gc.end();

    // Apply selection mask.

    for (Q_INT32 y = 0; y < r.height(); y++) {
        KisHLineIterator layerIt = clip -> createHLineIterator(0, y, r.width(), true);
        KisHLineIterator selectionIt = selection -> createHLineIterator(r.x(), r.y() + y, r.width(), false);

        while (!layerIt.isDone()) {

            cs->applyAlphaU8Mask( layerIt.rawData(), selectionIt.rawData(), 1 );


            ++layerIt;
            ++selectionIt;
        }
    }

    kdDebug(DBG_AREA_CORE) << "Selection copied: "
                   << r.x() << ", "
                   << r.y() << ", "
                   << r.width() << ", "
                   << r.height() << "\n";


     dev -> emitSelectionChanged();
     m_clipboard -> setClip(clip);
     imgSelectionChanged(m_parent -> currentImg());
}


KisLayerSP KisSelectionManager::paste()
{
        KisImageSP img = m_parent -> currentImg();
        if (!img) return 0;

    KisPaintDeviceImplSP clip = m_clipboard -> clip();

    if (clip) {
        KisPaintLayer *layer = new KisPaintLayer(img, img -> nextLayerName() + "(pasted)", OPACITY_OPAQUE);
        Q_CHECK_PTR(layer);

        QRect r = clip -> exactBounds();
        KisPainter gc;
        gc.begin(layer->paintDevice());
        gc.bitBlt(0, 0, COMPOSITE_COPY, clip, r.x(), r.y(), r.width(), r.height());
        gc.end();
        img->addLayer(layer, img -> rootLayer(), img -> activeLayer());

       //figure out where to position the clip
        KisCanvasController *cc = m_parent->getCanvasController();
        QPoint center = cc->viewToWindow(QPoint(cc->kiscanvas()->width()/2, cc->kiscanvas()->height()/2));
        QPoint bottomright = cc->viewToWindow(QPoint(cc->kiscanvas()->width(), cc->kiscanvas()->height()));
        if(bottomright.x() > img->width())
            center.setX(img->width()/2);
        if(bottomright.y() > img->height())
            center.setY(img->height()/2);
        center -= QPoint(r.width()/2, r.height()/2);
        layer->setX(center.x());
        layer->setY(center.y());

/*XXX CBR have an idea of asking the user if he is about to paste a clip ion another cs than that of
 the image if that is what he want rather than silently converting
        if (clip->colorSpace != img ->colorSpace())
            if (dlg -> exec() == QDialog::Accepted)
                layer -> convertTo(img -> colorSpace());
*/

        img -> notify(layer->extent());

        return layer;
    }
    return 0;
}

void KisSelectionManager::pasteNew()
{
    KisPaintDeviceImplSP clip = m_clipboard -> clip();
    if (!clip) return;

    QRect r = clip->exactBounds();
    if (r.width() < 1 && r.height() < 1) {
        // Don't paste empty clips
        return;
    }

    const QCString mimetype = KoDocument::readNativeFormatMimeType();
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType( mimetype );
    KisDoc * doc = (KisDoc*) entry.createDoc();

    Q_ASSERT(doc->undoAdapter() != 0);
    doc->undoAdapter()->setUndo(false);

    KisImageSP img = new KisImage(doc->undoAdapter(), r.width(), r.height(), clip->colorSpace(), "Pasted");
    KisPaintLayer *layer = new KisPaintLayer(img, clip->name(), OPACITY_OPAQUE, clip->colorSpace());

    KisPainter p(layer->paintDevice());
    p.bitBlt(0, 0, COMPOSITE_COPY, clip, OPACITY_OPAQUE, r.x(), r.y(), r.width(), r.height());
    p.end();

    img->addLayer(layer, img->rootLayer(), 0);
    doc->setCurrentImage(img);

    doc->undoAdapter()->setUndo(true);

    KoMainWindow *win = new KoMainWindow( doc->instance() );
    win->show();
    win->setRootDocument( doc );
}



void KisSelectionManager::selectAll()
{
        KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    KisSelectedTransaction * t = new KisSelectedTransaction(i18n("Select &All"), dev);
    Q_CHECK_PTR(t);

    dev -> selection() -> clear();
    dev -> selection() -> invert();

    if (img -> undoAdapter())
        img -> undoAdapter() -> addCommand(t);
    dev -> emitSelectionChanged();
}


void KisSelectionManager::deselect()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    KisSelectedTransaction * t = new KisSelectedTransaction(i18n("&Deselect"), dev);
    Q_CHECK_PTR(t);

    dev -> deselect();

    if (img -> undoAdapter())
        img -> undoAdapter() -> addCommand(t);

    dev -> emitSelectionChanged();
}


void KisSelectionManager::clear()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    if (!dev -> hasSelection()) return;

    KisTransaction * t = 0;

    if (img -> undoAdapter()) {
        t = new KisTransaction(i18n("Clear"), dev);
        Q_CHECK_PTR(t);
    }

    dev -> clearSelection();
    img -> notify();

    if (img -> undoAdapter()) img -> undoAdapter() -> addCommand(t);
}

void KisSelectionManager::fill(const KisColor& color, bool fillWithPattern, const QString& transactionText)
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    if (!dev -> hasSelection()) return;

    KisSelectionSP selection = dev -> selection();

    KisPaintDeviceImplSP filled = new KisPaintDeviceImpl(dev -> colorSpace());
    KisFillPainter painter(filled);

    if (fillWithPattern) {
        painter.fillRect(0, 0, img -> width(), img -> height(),
                         m_parent -> currentPattern());
    } else {
        painter.fillRect(0, 0, img -> width(), img -> height(), color); 
    }

    painter.end();

    KisPainter painter2(dev);

    painter2.beginTransaction(transactionText);
    painter2.bltSelection(0, 0, COMPOSITE_OVER, filled, OPACITY_OPAQUE,
                          0, 0, img -> width(), img -> height());
    img -> notify();

    if (img -> undoAdapter()) {
        img -> undoAdapter() -> addCommand(painter2.endTransaction());
    }
}

void KisSelectionManager::fillForegroundColor()
{
    fill(m_parent -> fgColor(), false, i18n("Fill with Foreground Color"));
}

void KisSelectionManager::fillBackgroundColor()
{
    fill(m_parent -> bgColor(), false, i18n("Fill with Background Color"));
}

void KisSelectionManager::fillPattern()
{
    fill(KisColor(), true, i18n("Fill with Pattern"));
}

void KisSelectionManager::reselect()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img ->activeDevice();
    if (!dev) return;

    KisSelectedTransaction * t = new KisSelectedTransaction(i18n("&Reselect"), dev);
    Q_CHECK_PTR(t);

    dev -> reselect(); // sets hasSelection=true

    if (img -> undoAdapter())
        img -> undoAdapter() -> addCommand(t);

    dev -> emitSelectionChanged();
}


void KisSelectionManager::invert()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    if (dev -> hasSelection()) {
        KisSelectionSP s = dev -> selection();

        KisTransaction * t = 0;
        if (img -> undoAdapter())
        {
            t = new KisTransaction(i18n("&Invert"), s.data());
            Q_CHECK_PTR(t);
        }

        s -> invert();

        if (img -> undoAdapter())
            img -> undoAdapter() -> addCommand(t);
    }

    dev -> emitSelectionChanged();
}

void KisSelectionManager::copySelectionToNewLayer()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    copy();
    paste();
}

void KisSelectionManager::cutToNewLayer()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;

    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    cut();
    paste();
}


// XXX Krita post 1.4: Make feather radius configurable
// XXX This has just become post 1.5, I'm afaid
void KisSelectionManager::feather()
{
    KisImageSP img = m_parent -> currentImg();
    if (!img) return;
    KisPaintDeviceImplSP dev = img -> activeDevice();
    if (!dev) return;

    if (!dev -> hasSelection()) {
        // activate it, but don't do anything with it
        dev -> selection();
        return;
    }

    KisSelectionSP selection = dev -> selection();

    KisSelectedTransaction * t = new KisSelectedTransaction(i18n("Feather..."), dev);
    Q_CHECK_PTR(t);


    // XXX: we should let gaussian blur & others influence alpha channels as well
    // (on demand of the caller)

    KisConvolutionPainter painter(selection.data());

    KisKernel k;
    k.width = 3;
    k.height = 3;
    k.factor = 16;
    k.offset = 0;
    k.data = new Q_INT32[9];
    k.data[0] = 1;
    k.data[1] = 2;
    k.data[2] = 1;
    k.data[3] = 2;
    k.data[4] = 4;
    k.data[5] = 2;
    k.data[6] = 1;
    k.data[7] = 2;
    k.data[8] = 1;

    QRect rect = selection -> extent();
    // Make sure we've got enough space around the edges.
    rect = QRect(rect.x() - 3, rect.y() - 3, rect.width() + 6, rect.height() + 6);
    rect &= QRect(0, 0, img -> width(), img -> height());

    painter.applyMatrix(&k, rect.x(), rect.y(), rect.width(), rect.height(), BORDER_AVOID, KisChannelInfo::FLAG_ALPHA);
    painter.end();

    if (img -> undoAdapter())
        img -> undoAdapter() -> addCommand(t);

    delete[] k.data;

    dev -> emitSelectionChanged();
}

void KisSelectionManager::toggleDisplaySelection()
{
    m_parent->canvasRefresh();
}

bool KisSelectionManager::displaySelection()
{
    return m_toggleDisplaySelection->isChecked();
}
// XXX: Maybe move these esoteric functions to plugins?
void KisSelectionManager::border() {}
void KisSelectionManager::expand() {}
void KisSelectionManager::smooth() {}
void KisSelectionManager::contract() {}
void KisSelectionManager::grow() {}
void KisSelectionManager::similar() {}
void KisSelectionManager::transform() {}
void KisSelectionManager::load() {}
void KisSelectionManager::save() {}


#include "kis_selection_manager.moc"
