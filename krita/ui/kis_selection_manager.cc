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
#include <QColor>

#include <kdebug.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kstdaction.h>

#include <KoDocument.h>
#include <KoMainWindow.h>
#include <KoQueryTrader.h>

#include "kis_part_layer.h"
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
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
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
                collection,
                "paste_new");
    connect(m_pasteNew, SIGNAL(triggered()), this, SLOT(pasteNew()));

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
                             collection,
                             "reselect");
    m_reselect->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_D);
    connect(m_reselect, SIGNAL(triggered()), this, SLOT(reselect()));

    m_invert = new KAction(i18n("&Invert"),
                           collection,
                           "invert");
    m_invert->setShortcut(Qt::CTRL+Qt::Key_I);
    connect(m_invert, SIGNAL(triggered()), this, SLOT(invert()));

    m_toNewLayer = new KAction(i18n("Copy Selection to New Layer"),
                               collection,
                               "copy_selection_to_new_layer");
    m_toNewLayer->setShortcut(Qt::CTRL+Qt::Key_J);
    connect(m_toNewLayer, SIGNAL(triggered()), this, SLOT(copySelectionToNewLayer()));

    m_cutToNewLayer = new KAction(i18n("Cut Selection to New Layer"),
                                  collection,
                                  "cut_selection_to_new_layer");
    m_cutToNewLayer->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_J);
    connect(m_cutToNewLayer, SIGNAL(triggered()), this, SLOT(cutToNewLayer()));

    m_feather = new KAction(i18n("Feather"),
                            collection,
                            "feather");
    m_feather->setShortcut(Qt::CTRL+Qt::ALT+Qt::Key_D);
    connect(m_feather, SIGNAL(triggered()), this, SLOT(feather()));

    m_fillForegroundColor = new KAction(i18n("Fill with Foreground Color"),
                                             collection,
                                             "fill_selection_foreground_color");
    m_fillForegroundColor->setShortcut(Qt::ALT+Qt::Key_Backspace);
    connect(m_fillForegroundColor, SIGNAL(triggered()), this, SLOT(fillForegroundColor()));

    m_fillBackgroundColor = new KAction(i18n("Fill with Background Color"),
                                             collection,
                                             "fill_selection_background_color");
    m_fillBackgroundColor->setShortcut(Qt::Key_Backspace);
    connect(m_fillBackgroundColor, SIGNAL(triggered()), this, SLOT(fillBackgroundColor()));

    m_fillPattern = new KAction(i18n("Fill with Pattern"),
                                collection,
                                "fill_selection_pattern");
    connect(m_fillPattern, SIGNAL(triggered()), this, SLOT(fillPattern()));

    m_toggleDisplaySelection = new KToggleAction(i18n("Display Selection"),
                                                 collection,
                                                 "toggle_display_selection");
    m_toggleDisplaySelection->setShortcut(Qt::CTRL+Qt::Key_H);
    connect(m_toggleDisplaySelection, SIGNAL(triggered()), this, SLOT(toggleDisplaySelection()));

    m_toggleDisplaySelection->setCheckedState(KGuiItem(i18n("Hide Selection")));
    m_toggleDisplaySelection->setChecked(true);

    m_border = new KAction(i18n("Border..."),
                           collection,
                           "border");
    connect(m_border, SIGNAL(triggered()), this, SLOT(border()));

    m_expand = new KAction(i18n("Expand..."),
                           collection,
                           "expand");
    connect(m_expand, SIGNAL(triggered()), this, SLOT(expand()));

    m_smooth = new KAction(i18n("Smooth..."),
                           collection,
                           "smooth");
    connect(m_smooth, SIGNAL(triggered()), this, SLOT(smooth()));

    m_contract = new KAction(i18n("Contract..."),
                             collection,
                             "contract");
    connect(m_contract, SIGNAL(triggered()), this, SLOT(contract()));

    m_similar = new KAction(i18n("Similar"),
                            collection,
                            "similar");
    connect(m_similar, SIGNAL(triggered()), this, SLOT(similar()));

    m_transform = new KAction(i18n("Transform..."),
                              collection,
                              "transform_selection");
    connect(m_transform, SIGNAL(triggered()), this, SLOT(transform()));


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

    KisImageSP img = m_parent->currentImg();
    KisLayerSP l;
    KisPaintDeviceSP dev;

    bool enable = false;
    if (img && img->activeDevice() && img->activeLayer()) {
        l = img->activeLayer();
        dev = img->activeDevice();


        KisPartLayer * partLayer = dynamic_cast<KisPartLayer*>(l.data());

        enable = l && dev&& dev->hasSelection() && !l->locked() && l->visible() && (partLayer==0);

        if(dev)
            m_reselect->setEnabled( dev->selectionDeselected() );
    }

    m_cut->setEnabled(enable);
    m_cutToNewLayer->setEnabled(enable);
    m_selectAll->setEnabled(!img.isNull());
    m_deselect->setEnabled(enable);
    m_clear->setEnabled(enable);
    m_fillForegroundColor->setEnabled(enable);
    m_fillBackgroundColor->setEnabled(enable);
    m_fillPattern->setEnabled(enable);
    m_invert->setEnabled(enable);

    m_feather->setEnabled(enable);

    m_border->setEnabled(enable);
    m_expand->setEnabled(enable);
    m_smooth->setEnabled(enable);
    m_contract->setEnabled(enable);
    m_similar->setEnabled(enable);
    m_transform->setEnabled(enable);
//    m_load->setEnabled(enable);
//    m_save->setEnabled(enable);


    KAction * a;
    for (a = m_pluginActions.first(); a; a = m_pluginActions.next()) {
        a->setEnabled(!img.isNull());
    }

    // You can copy from locked layers and paste the clip into a new layer, even when
    // the current layer is locked.
    enable = false;
    if (img && l && dev) {
        enable = dev->hasSelection() && l->visible();
    }

    m_copy->setEnabled(enable);
    m_paste->setEnabled(!img.isNull() && m_clipboard->hasClip());
    m_pasteNew->setEnabled(!img.isNull() && m_clipboard->hasClip());
    m_toNewLayer->setEnabled(enable);

    m_parent->updateStatusBarSelectionLabel();

}

void KisSelectionManager::imgSelectionChanged(KisImageSP img)
{
    if (img == m_parent->currentImg()) {
        updateGUI();
    }
}

void KisSelectionManager::cut()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;

    copy();

    KisSelectedTransaction *t = 0;

    if (img->undo()) {
        t = new KisSelectedTransaction(i18n("Cut"), dev);
        Q_CHECK_PTR(t);
    }

    dev->clearSelection();
    dev->deselect();
    dev->emitSelectionChanged();

    if (img->undo()) {
        img->undoAdapter()->addCommand(t);
    }
}

void KisSelectionManager::copy()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;

    KisSelectionSP selection = dev->selection();

    QRect r = selection->selectedExactRect();

    KisPaintDeviceSP clip = KisPaintDeviceSP(new KisPaintDevice(dev->colorSpace(), "clip"));
    Q_CHECK_PTR(clip);

    KisColorSpace * cs = clip->colorSpace();

    // TODO if the source is linked... copy from all linked layers?!?

    // Copy image data
    KisPainter gc;
    gc.begin(clip);
    gc.bitBlt(0, 0, COMPOSITE_COPY, dev, r.x(), r.y(), r.width(), r.height());
    gc.end();

    // Apply selection mask.

    for (qint32 y = 0; y < r.height(); y++) {
        KisHLineIterator layerIt = clip->createHLineIterator(0, y, r.width(), true);
        KisHLineIterator selectionIt = selection->createHLineIterator(r.x(), r.y() + y, r.width(), false);

        while (!layerIt.isDone()) {

            cs->applyAlphaU8Mask( layerIt.rawData(), selectionIt.rawData(), 1 );


            ++layerIt;
            ++selectionIt;
        }
    }

    m_clipboard->setClip(clip);
    imgSelectionChanged(m_parent->currentImg());
}


KisLayerSP KisSelectionManager::paste()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return KisLayerSP(0);

    KisPaintDeviceSP clip = m_clipboard->clip();

    if (clip) {
        KisPaintLayer *layer = new KisPaintLayer(img.data(), img->nextLayerName() + "(pasted)", OPACITY_OPAQUE);
        Q_CHECK_PTR(layer);

        QRect r = clip->exactBounds();
        KisPainter gc;
        gc.begin(layer->paintDevice());
        gc.bitBlt(0, 0, COMPOSITE_COPY, clip, r.x(), r.y(), r.width(), r.height());
        gc.end();

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
            if (dlg->exec() == QDialog::Accepted)
                layer->convertTo(img->colorSpace());
*/
        img->addLayer(KisLayerSP(layer), img->rootLayer(), img->activeLayer());

        return KisLayerSP(layer);
    }
    return KisLayerSP(0);
}

void KisSelectionManager::pasteNew()
{
    KisPaintDeviceSP clip = m_clipboard->clip();
    if (!clip) return;

    QRect r = clip->exactBounds();
    if (r.width() < 1 && r.height() < 1) {
        // Don't paste empty clips
        return;
    }

    const QByteArray mimetype = KoDocument::readNativeFormatMimeType();
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType( mimetype );
    KisDoc * doc = (KisDoc*) entry.createDoc();

    Q_ASSERT(doc->undoAdapter() != 0);
    doc->undoAdapter()->setUndo(false);

    KisImageSP img = KisImageSP(new KisImage(doc->undoAdapter(), r.width(), r.height(), clip->colorSpace(), "Pasted"));
    KisPaintLayer *layer = new KisPaintLayer(img.data(), clip->objectName(), OPACITY_OPAQUE, clip->colorSpace());

    KisPainter p(layer->paintDevice());
    p.bitBlt(0, 0, COMPOSITE_COPY, clip, OPACITY_OPAQUE, r.x(), r.y(), r.width(), r.height());
    p.end();

    img->addLayer(KisLayerSP(layer), img->rootLayer(), KisLayerSP(0));
    doc->setCurrentImage(img);

    doc->undoAdapter()->setUndo(true);

    KoMainWindow *win = new KoMainWindow( doc->instance() );
    win->show();
    win->setRootDocument( doc );
}

void KisSelectionManager::selectAll()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    KisSelectedTransaction * t = 0;
    if (img->undo()) t = new KisSelectedTransaction(i18n("Select &All"), dev);
    Q_CHECK_PTR(t);

    dev->selection()->clear();
    dev->selection()->invert();
    dev->emitSelectionChanged();

    if (img->undo())
        img->undoAdapter()->addCommand(t);
}

void KisSelectionManager::deselect()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;
    KisSelectedTransaction * t = 0;
    if (img->undo()) t = new KisSelectedTransaction(i18n("Deselect"), dev);
    Q_CHECK_PTR(t);

    dev->deselect();
    dev->emitSelectionChanged();

    if (img->undo())
        img->undoAdapter()->addCommand(t);
}


void KisSelectionManager::clear()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;

    KisTransaction * t = 0;

    if (img->undo()) {
        t = new KisTransaction(i18n("Clear"), dev);
    }

    dev->clearSelection();
    dev->emitSelectionChanged();

    if (img->undo()) img->undoAdapter()->addCommand(t);
}

void KisSelectionManager::fill(const KisColor& color, bool fillWithPattern, const QString& transactionText)
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;

    KisSelectionSP selection = dev->selection();

    KisPaintDeviceSP filled = KisPaintDeviceSP(new KisPaintDevice(dev->colorSpace()));
    KisFillPainter painter(filled);

    if (fillWithPattern) {
        painter.fillRect(0, 0, img->width(), img->height(),
                         m_parent->currentPattern());
    } else {
        painter.fillRect(0, 0, img->width(), img->height(), color);
    }

    painter.end();

    KisPainter painter2(dev);

    if (img->undo()) painter2.beginTransaction(transactionText);
    painter2.bltSelection(0, 0, COMPOSITE_OVER, filled, OPACITY_OPAQUE,
                          0, 0, img->width(), img->height());

    dev->emitSelectionChanged();

    if (img->undo()) {
        img->undoAdapter()->addCommand(painter2.endTransaction());
    }
}

void KisSelectionManager::fillForegroundColor()
{
    fill(m_parent->fgColor(), false, i18n("Fill with Foreground Color"));
}

void KisSelectionManager::fillBackgroundColor()
{
    fill(m_parent->bgColor(), false, i18n("Fill with Background Color"));
}

void KisSelectionManager::fillPattern()
{
    fill(KisColor(), true, i18n("Fill with Pattern"));
}

void KisSelectionManager::reselect()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img ->activeDevice();
    if (!dev) return;

    KisSelectedTransaction * t = 0;
    if (img->undo()) t = new KisSelectedTransaction(i18n("&Reselect"), dev);
    Q_CHECK_PTR(t);

    dev->reselect(); // sets hasSelection=true
    dev->emitSelectionChanged();

    if (img->undo())
        img->undoAdapter()->addCommand(t);
}


void KisSelectionManager::invert()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (dev->hasSelection()) {
        KisSelectionSP s = dev->selection();

        KisSelectedTransaction * t = 0;
        if (img->undo())
        {
            t = new KisSelectedTransaction(i18n("&Invert"), dev);
            Q_CHECK_PTR(t);
        }

        s->invert();
        dev->emitSelectionChanged();

        if (t) {
            img->undoAdapter()->addCommand(t);
        }
    }
}

void KisSelectionManager::copySelectionToNewLayer()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    copy();
    paste();
}

void KisSelectionManager::cutToNewLayer()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    cut();
    paste();
}


void KisSelectionManager::feather()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;
    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) {
        // activate it, but don't do anything with it
        dev->selection();
        return;
    }

    KisSelectionSP selection = dev->selection();
    KisSelectedTransaction * t = 0;
    if (img->undo()) t = new KisSelectedTransaction(i18n("Feather..."), dev);
    Q_CHECK_PTR(t);


    // XXX: we should let gaussian blur & others influence alpha channels as well
    // (on demand of the caller)

    KisConvolutionPainter painter(KisPaintDeviceSP(selection.data()));

    KisKernelSP k = KisKernelSP(new KisKernel());
    k->width = 3;
    k->height = 3;
    k->factor = 16;
    k->offset = 0;
    k->data = new qint32[9];
    k->data[0] = 1;
    k->data[1] = 2;
    k->data[2] = 1;
    k->data[3] = 2;
    k->data[4] = 4;
    k->data[5] = 2;
    k->data[6] = 1;
    k->data[7] = 2;
    k->data[8] = 1;

    QRect rect = selection->extent();
    // Make sure we've got enough space around the edges.
    rect = QRect(rect.x() - 3, rect.y() - 3, rect.width() + 6, rect.height() + 6);
    rect &= QRect(0, 0, img->width(), img->height());

    painter.applyMatrix(k, rect.x(), rect.y(), rect.width(), rect.height(), BORDER_AVOID, KisChannelInfo::FLAG_ALPHA);
    painter.end();

    dev->emitSelectionChanged();

    if (img->undo())
        img->undoAdapter()->addCommand(t);

}

void KisSelectionManager::toggleDisplaySelection()
{
    m_parent->selectionDisplayToggled(displaySelection());
}

bool KisSelectionManager::displaySelection()
{
    return m_toggleDisplaySelection->isChecked();
}
// XXX: Maybe move these esoteric functions to plugins?
void KisSelectionManager::border() {}
void KisSelectionManager::expand() {}
void KisSelectionManager::contract() {}
void KisSelectionManager::similar() {}
void KisSelectionManager::transform() {}
void KisSelectionManager::load() {}
void KisSelectionManager::save() {}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

void KisSelectionManager::grow (qint32 xradius, qint32 yradius)
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;
    KisSelectionSP selection = dev->selection();

    //determine the layerSize
    QRect layerSize = dev->exactBounds();
    /*
        Any bugs in this fuction are probably also in thin_region
        Blame all bugs in this function on jaycox@gimp.org
    */

    quint8  **buf;  // caches the region's pixel data
    quint8  **max;  // caches the largest values for each column

    if (xradius <= 0 || yradius <= 0)
        return;

    KisSelectedTransaction *t = 0;

    if (img->undo()) {
        t = new KisSelectedTransaction(i18n("Grow"), dev);
        Q_CHECK_PTR(t);
    }

    max = new quint8* [layerSize.width() + 2 * xradius];
    buf = new quint8* [yradius + 1];
    for (qint32 i = 0; i < yradius + 1; i++)
    {
        buf[i] = new quint8[layerSize.width()];
    }
    quint8* buffer = new quint8[ ( layerSize.width() + 2 * xradius ) * ( yradius + 1 ) ];
    for (qint32 i = 0; i < layerSize.width() + 2 * xradius; i++)
    {
        if (i < xradius)
            max[i] = buffer;
        else if (i < layerSize.width() + xradius)
            max[i] = &buffer[(yradius + 1) * (i - xradius)];
        else
            max[i] = &buffer[(yradius + 1) * (layerSize.width() + xradius - 1)];

        for (qint32 j = 0; j < xradius + 1; j++)
            max[i][j] = 0;
    }
    /* offset the max pointer by xradius so the range of the array
        is [-xradius] to [region->w + xradius] */
    max += xradius;

    quint8* out = new quint8[ layerSize.width() ]; // holds the new scan line we are computing

    qint32* circ = new qint32[ 2 * xradius + 1 ]; // holds the y coords of the filter's mask
    computeBorder (circ, xradius, yradius);

    /* offset the circ pointer by xradius so the range of the array
        is [-xradius] to [xradius] */
    circ += xradius;

    memset (buf[0], 0, layerSize.width());
    for (qint32 i = 0; i < yradius && i < layerSize.height(); i++) // load top of image
    {
        selection->readBytes(buf[i + 1], layerSize.x(), layerSize.y() + i, layerSize.width(), 1);
    }

    for (qint32 x = 0; x < layerSize.width() ; x++) // set up max for top of image
    {
            max[x][0] = 0;         // buf[0][x] is always 0
            max[x][1] = buf[1][x]; // MAX (buf[1][x], max[x][0]) always = buf[1][x]
            for (qint32 j = 2; j < yradius + 1; j++)
            {
                max[x][j] = MAX(buf[j][x], max[x][j-1]);
            }
    }

    for (qint32 y = 0; y < layerSize.height(); y++)
    {
        rotatePointers (buf, yradius + 1);
        if (y < layerSize.height() - (yradius))
            selection->readBytes(buf[yradius], layerSize.x(), layerSize.y() + y + yradius, layerSize.width(), 1);
        else
            memset (buf[yradius], 0, layerSize.width());
    for (qint32 x = 0; x < layerSize.width(); x++) /* update max array */
    {
        for (qint32 i = yradius; i > 0; i--)
        {
            max[x][i] = MAX (MAX (max[x][i - 1], buf[i - 1][x]), buf[i][x]);
        }
        max[x][0] = buf[0][x];
    }
    qint32 last_max = max[0][circ[-1]];
    qint32 last_index = 1;
    for (qint32 x = 0; x < layerSize.width(); x++) /* render scan line */
    {
        last_index--;
        if (last_index >= 0)
        {
            if (last_max == 255)
                out[x] = 255;
            else
            {
                last_max = 0;
                for (qint32 i = xradius; i >= 0; i--)
                    if (last_max < max[x + i][circ[i]])
                    {
                        last_max = max[x + i][circ[i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
        }
        else
        {
            last_index = xradius;
            last_max = max[x + xradius][circ[xradius]];
            for (qint32 i = xradius - 1; i >= -xradius; i--)
                if (last_max < max[x + i][circ[i]])
                {
                    last_max = max[x + i][circ[i]];
                    last_index = i;
                }
            out[x] = last_max;
        }
    }
    selection->writeBytes(out, layerSize.x(), layerSize.y() + y, layerSize.width(), 1);
    }
    /* undo the offsets to the pointers so we can free the malloced memmory */
    circ -= xradius;
    max -= xradius;
    //XXXX: replace delete by delete[] where it is necessary to avoid memory leaks!
    delete[] circ;
    delete[] buffer;
    delete[] max;
    for (qint32 i = 0; i < yradius + 1; i++)
        delete[] buf[i];
    delete[] buf;
    delete[] out;

    dev->emitSelectionChanged();

    if (t) {
        img->undoAdapter()->addCommand(t);
    }
}

void KisSelectionManager::shrink (qint32 xradius, qint32 yradius, bool edge_lock)
{

    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;
    KisSelectionSP selection = dev->selection();

    //determine the layerSize
    QRect layerSize = dev->exactBounds();
  /*
     pretty much the same as fatten_region only different
     blame all bugs in this function on jaycox@gimp.org
  */
  /* If edge_lock is true  we assume that pixels outside the region
     we are passed are identical to the edge pixels.
     If edge_lock is false, we assume that pixels outside the region are 0
  */
    quint8  **buf;  // caches the the region's pixels
    quint8  **max;  // caches the smallest values for each column
    qint32    last_max, last_index;

    if (xradius <= 0 || yradius <= 0)
        return;

    max = new quint8* [layerSize.width() + 2 * xradius];
    buf = new quint8* [yradius + 1];
    for (qint32 i = 0; i < yradius + 1; i++)
    {
        buf[i] = new quint8[layerSize.width()];
    }

    qint32 buffer_size = (layerSize.width() + 2 * xradius + 1) * (yradius + 1);
    quint8* buffer = new quint8[buffer_size];

    if (edge_lock)
        memset(buffer, 255, buffer_size);
    else
        memset(buffer, 0, buffer_size);

    for (qint32 i = 0; i < layerSize.width() + 2 * xradius; i++)
    {
        if (i < xradius)
            if (edge_lock)
                max[i] = buffer;
            else
                max[i] = &buffer[(yradius + 1) * (layerSize.width() + xradius)];
        else if (i < layerSize.width() + xradius)
            max[i] = &buffer[(yradius + 1) * (i - xradius)];
        else
            if (edge_lock)
                max[i] = &buffer[(yradius + 1) * (layerSize.width() + xradius - 1)];
            else
                max[i] = &buffer[(yradius + 1) * (layerSize.width() + xradius)];
    }
    if (!edge_lock)
        for (qint32 j = 0 ; j < xradius + 1; j++) max[0][j] = 0;

    // offset the max pointer by xradius so the range of the array is [-xradius] to [region->w + xradius]
    max += xradius;

    quint8* out = new quint8[layerSize.width()]; // holds the new scan line we are computing

    qint32* circ = new qint32[2 * xradius + 1]; // holds the y coords of the filter's mask

    computeBorder (circ, xradius, yradius);

    // offset the circ pointer by xradius so the range of the array is [-xradius] to [xradius]
    circ += xradius;

    for (qint32 i = 0; i < yradius && i < layerSize.height(); i++) // load top of image
        selection->readBytes(buf[i + 1], layerSize.x(), layerSize.y() + i, layerSize.width(), 1);

    if (edge_lock)
        memcpy (buf[0], buf[1], layerSize.width());
    else
        memset (buf[0], 0, layerSize.width());


    for (qint32 x = 0; x < layerSize.width(); x++) // set up max for top of image
    {
        max[x][0] = buf[0][x];
        for (qint32 j = 1; j < yradius + 1; j++)
            max[x][j] = MIN(buf[j][x], max[x][j-1]);
    }

    for (qint32 y = 0; y < layerSize.height(); y++)
    {
        rotatePointers (buf, yradius + 1);
        if (y < layerSize.height() - yradius)
            selection->readBytes(buf[yradius], layerSize.x(), layerSize.y() + y + yradius, layerSize.width(), 1);
        else if (edge_lock)
            memcpy (buf[yradius], buf[yradius - 1], layerSize.width());
        else
            memset (buf[yradius], 0, layerSize.width());

        for (qint32 x = 0 ; x < layerSize.width(); x++) // update max array
        {
            for (qint32 i = yradius; i > 0; i--)
            {
                max[x][i] = MIN (MIN (max[x][i - 1], buf[i - 1][x]), buf[i][x]);
            }
            max[x][0] = buf[0][x];
        }
        last_max =  max[0][circ[-1]];
        last_index = 0;

        for (qint32 x = 0 ; x < layerSize.width(); x++) // render scan line
        {
            last_index--;
            if (last_index >= 0)
            {
                if (last_max == 0)
                out[x] = 0;
                else
                {
                    last_max = 255;
                    for (qint32 i = xradius; i >= 0; i--)
                        if (last_max > max[x + i][circ[i]])
                        {
                            last_max = max[x + i][circ[i]];
                            last_index = i;
                        }
                    out[x] = last_max;
                }
            }
            else
            {
                last_index = xradius;
                last_max = max[x + xradius][circ[xradius]];
                for (qint32 i = xradius - 1; i >= -xradius; i--)
                if (last_max > max[x + i][circ[i]])
                    {
                    last_max = max[x + i][circ[i]];
                    last_index = i;
                    }
                out[x] = last_max;
            }
        }
        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, layerSize.width(), 1);
    }

    // undo the offsets to the pointers so we can free the malloced memmory
    circ -= xradius;
    max -= xradius;
    //free the memmory
    //XXXX: replace delete by delete[] where it is necessary to avoid memory leaks!
    delete[] circ;
    delete[] buffer;
    delete[] max;
    for (qint32 i = 0; i < yradius + 1; i++)
            delete buf[i];
    delete[] buf;
    delete[] out;

    dev->emitSelectionChanged();
}

//Simple convolution filter to smooth a mask (1bpp)

void KisSelectionManager::smooth()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;
    KisSelectionSP selection = dev->selection();

    //determine the layerSize
    QRect layerSize = dev->exactBounds();

    quint8      *buf[3];

    qint32 width = layerSize.width();

    for (qint32 i = 0; i < 3; i++) buf[i] = new quint8[width + 2];

    quint8* out = new quint8[width];

    // load top of image
    selection->readBytes(buf[0] + 1, layerSize.x(), layerSize.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy (buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < layerSize.height(); y++)
    {
        if (y + 1 < layerSize.height())
        {
            selection->readBytes(buf[2] + 1, layerSize.x(), layerSize.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        }
        else
        {
            memcpy (buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++)
        {
            qint32 value = (buf[0][x] + buf[0][x+1] + buf[0][x+2] +
                             buf[1][x] + buf[2][x+1] + buf[1][x+2] +
                             buf[2][x] + buf[1][x+1] + buf[2][x+2]);

            out[x] = value / 9;
        }

        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, width, 1);

        rotatePointers (buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];

    delete[] out;

    dev->emitSelectionChanged();
}

// Erode (radius 1 pixel) a mask (1bpp)

void KisSelectionManager::erode()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;
    KisSelectionSP selection = dev->selection();

    //determine the layerSize
    QRect layerSize = dev->exactBounds();

    quint8* buf[3];


    qint32 width = layerSize.width();

    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];

    quint8* out = new quint8[width];

    // load top of image
    selection->readBytes(buf[0] + 1, layerSize.x(), layerSize.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy (buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < layerSize.height(); y++)
    {
        if (y + 1 < layerSize.height())
        {
            selection->readBytes(buf[2] + 1, layerSize.x(), layerSize.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        }
        else
        {
            memcpy (buf[2], buf[1], width + 2);
        }

      for (qint32 x = 0 ; x < width; x++)
        {
          qint32 min = 255;

          if (buf[0][x+1] < min) min = buf[0][x+1];
          if (buf[1][x]   < min) min = buf[1][x];
          if (buf[1][x+1] < min) min = buf[1][x+1];
          if (buf[1][x+2] < min) min = buf[1][x+2];
          if (buf[2][x+1] < min) min = buf[2][x+1];

          out[x] = min;
        }

        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, width, 1);

        rotatePointers (buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];

    delete[] out;

    dev->emitSelectionChanged();
}

// dilate (radius 1 pixel) a mask (1bpp)

void KisSelectionManager::dilate()
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;
    KisSelectionSP selection = dev->selection();

    //determine the layerSize
    QRect layerSize = dev->exactBounds();

    quint8* buf[3];

    qint32 width = layerSize.width();

    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];

    quint8* out = new quint8[width];

    // load top of image
    selection->readBytes(buf[0] + 1, layerSize.x(), layerSize.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy (buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < layerSize.height(); y++)
    {
        if (y + 1 < layerSize.height())
        {
            selection->readBytes(buf[2] + 1, layerSize.x(), layerSize.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        }
        else
        {
            memcpy (buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++)
        {
            qint32 max = 0;

            if (buf[0][x+1] > max) max = buf[0][x+1];
            if (buf[1][x]   > max) max = buf[1][x];
            if (buf[1][x+1] > max) max = buf[1][x+1];
            if (buf[1][x+2] > max) max = buf[1][x+2];
            if (buf[2][x+1] > max) max = buf[2][x+1];

            out[x] = max;
        }

        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, width, 1);

        rotatePointers (buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];

    delete[] out;

    dev->emitSelectionChanged();
}

void KisSelectionManager::border(qint32 xradius, qint32 yradius)
{
    KisImageSP img = m_parent->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (!dev->hasSelection()) return;
    KisSelectionSP selection = dev->selection();

    //determine the layerSize
    QRect layerSize = dev->exactBounds();

  /*
     This function has no bugs, but if you imagine some you can
     blame them on jaycox@gimp.org
  */
    quint8  *buf[3];
    quint8 **density;
    quint8 **transition;

    if (xradius == 1 && yradius == 1) // optimize this case specifically
    {
        quint8* source[3];

        for (qint32 i = 0; i < 3; i++)
            source[i] = new quint8[layerSize.width()];

        quint8* transition = new quint8[layerSize.width()];

        selection->readBytes(source[0], layerSize.x(), layerSize.y(), layerSize.width(), 1);
        memcpy (source[1], source[0], layerSize.width());
        if (layerSize.height() > 1)
            selection->readBytes(source[2], layerSize.x(), layerSize.y() + 1, layerSize.width(), 1);
        else
            memcpy (source[2], source[1], layerSize.width());

        computeTransition (transition, source, layerSize.width());
        selection->writeBytes(transition, layerSize.x(), layerSize.y(), layerSize.width(), 1);

        for (qint32 y = 1; y < layerSize.height(); y++)
        {
            rotatePointers (source, 3);
            if (y + 1 < layerSize.height())
                selection->readBytes(source[2], layerSize.x(), layerSize.y() + y + 1, layerSize.width(), 1);
            else
                memcpy(source[2], source[1], layerSize.width());
            computeTransition (transition, source, layerSize.width());
            selection->writeBytes(transition, layerSize.x(), layerSize.y() + y, layerSize.width(), 1);
        }

        for (qint32 i = 0; i < 3; i++)
            delete[] source[i];
        delete[] transition;
        return;
    }

    qint32* max = new qint32[layerSize.width() + 2 * xradius];
    for (qint32 i = 0; i < (layerSize.width() + 2 * xradius); i++)
        max[i] = yradius + 2;
    max += xradius;

    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[layerSize.width()];

    transition = new quint8*[yradius + 1];
    for (qint32 i = 0; i < yradius + 1; i++)
    {
        transition[i] = new quint8[layerSize.width() + 2 * xradius];
        memset(transition[i], 0, layerSize.width() + 2 * xradius);
        transition[i] += xradius;
    }
    quint8* out = new quint8[layerSize.width()];
    density = new quint8*[2 * xradius + 1];
    density += xradius;

    for (qint32 x = 0; x < (xradius + 1); x++) // allocate density[][]
    {
        density[ x]  = new quint8[2 * yradius + 1];
        density[ x] += yradius;
        density[-x]  = density[x];
    }
    for (qint32 x = 0; x < (xradius + 1); x++) // compute density[][]
    {
        double tmpx, tmpy, dist;
        quint8 a;

        if (x > 0)
            tmpx = x - 0.5;
        else if (x < 0)
            tmpx = x + 0.5;
        else
            tmpx = 0.0;

        for (qint32 y = 0; y < (yradius + 1); y++)
        {
            if (y > 0)
                tmpy = y - 0.5;
            else if (y < 0)
                tmpy = y + 0.5;
            else
                tmpy = 0.0;
            dist = ((tmpy * tmpy) / (yradius * yradius) +
                    (tmpx * tmpx) / (xradius * xradius));
            if (dist < 1.0)
                a = 255 * (quint8)(1.0 - sqrt (dist));
            else
                a = 0;
            density[ x][ y] = a;
            density[ x][-y] = a;
            density[-x][ y] = a;
            density[-x][-y] = a;
        }
    }
    selection->readBytes(buf[0], layerSize.x(), layerSize.y(), layerSize.width(), 1);
    memcpy (buf[1], buf[0], layerSize.width());
    if (layerSize.height() > 1)
        selection->readBytes(buf[2], layerSize.x(), layerSize.y() + 1, layerSize.width(), 1);
    else
        memcpy (buf[2], buf[1], layerSize.width());
    computeTransition (transition[1], buf, layerSize.width());

    for (qint32 y = 1; y < yradius && y + 1 < layerSize.height(); y++) // set up top of image
    {
        rotatePointers (buf, 3);
        selection->readBytes(buf[2], layerSize.x(), layerSize.y() + y + 1, layerSize.width(), 1);
        computeTransition (transition[y + 1], buf, layerSize.width());
    }
    for (qint32 x = 0; x < layerSize.width(); x++) // set up max[] for top of image
    {
        max[x] = -(yradius + 7);
        for (qint32 j = 1; j < yradius + 1; j++)
            if (transition[j][x])
            {
                max[x] = j;
                break;
            }
    }
    for (qint32 y = 0; y < layerSize.height(); y++) // main calculation loop
    {
        rotatePointers (buf, 3);
        rotatePointers (transition, yradius + 1);
        if (y < layerSize.height() - (yradius + 1))
        {
            selection->readBytes(buf[2], layerSize.x(), layerSize.y() + y + yradius + 1, layerSize.width(), 1);
            computeTransition (transition[yradius], buf, layerSize.width());
        }
        else
            memcpy (transition[yradius], transition[yradius - 1], layerSize.width());

        for (qint32 x = 0; x < layerSize.width(); x++) // update max array
        {
            if (max[x] < 1)
            {
                if (max[x] <= -yradius)
                {
                    if (transition[yradius][x])
                        max[x] = yradius;
                    else
                        max[x]--;
                }
                else
                    if (transition[-max[x]][x])
                        max[x] = -max[x];
                    else if (transition[-max[x] + 1][x])
                        max[x] = -max[x] + 1;
                else
                  max[x]--;
            }
            else
                max[x]--;
            if (max[x] < -yradius - 1)
                max[x] = -yradius - 1;
        }
        quint8 last_max =  max[0][density[-1]];
        qint32 last_index = 1;
        for (qint32 x = 0 ; x < layerSize.width(); x++) // render scan line
        {
            last_index--;
            if (last_index >= 0)
            {
                last_max = 0;
                for (qint32 i = xradius; i >= 0; i--)
                    if (max[x + i] <= yradius && max[x + i] >= -yradius && density[i][max[x+i]] > last_max)
                    {
                        last_max = density[i][max[x + i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
            else
            {
                last_max = 0;
                for (qint32 i = xradius; i >= -xradius; i--)
                    if (max[x + i] <= yradius && max[x + i] >= -yradius && density[i][max[x + i]] > last_max)
                    {
                        last_max = density[i][max[x + i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
            if (last_max == 0)
            {
                qint32 i;
                for (i = x + 1; i < layerSize.width(); i++)
                {
                    if (max[i] >= -yradius)
                        break;
                }
                if (i - x > xradius)
                {
                    for (; x < i - xradius; x++)
                        out[x] = 0;
                    x--;
                }
                last_index = xradius;
            }
        }
        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, layerSize.width(), 1);
    }
    delete out;

    for (qint32 i = 0; i < 3; i++)
        delete buf[i];

    max -= xradius;
    delete[] max;

    for (qint32 i = 0; i < yradius + 1; i++)
    {
        transition[i] -= xradius;
        delete transition[i];
    }
    delete[] transition;

    for (qint32 i = 0; i < xradius + 1 ; i++)
    {
        density[i] -= yradius;
        delete density[i];
    }
    density -= xradius;
    delete[] density;

    dev->emitSelectionChanged();
}

#define RINT(x) floor ((x) + 0.5)

void KisSelectionManager::computeBorder (qint32  *circ, qint32  xradius, qint32  yradius)
{
  qint32 i;
  qint32 diameter = xradius * 2 + 1;
  double tmp;

    for (i = 0; i < diameter; i++)
    {
        if (i > xradius)
        tmp = (i - xradius) - 0.5;
        else if (i < xradius)
        tmp = (xradius - i) - 0.5;
        else
        tmp = 0.0;

        circ[i] = (qint32) RINT (yradius / (double) xradius * sqrt (xradius * xradius - tmp * tmp));
    }
}

void KisSelectionManager::rotatePointers (quint8  **p, quint32 n)
{
    quint32  i;
    quint8  *tmp;

    tmp = p[0];

    for (i = 0; i < n - 1; i++) p[i] = p[i + 1];

    p[i] = tmp;
}

void KisSelectionManager::computeTransition (quint8* transition, quint8** buf, qint32 width)
{
    qint32 x = 0;

    if (width == 1)
    {
        if (buf[1][x] > 127 && (buf[0][x] < 128 || buf[2][x] < 128))
            transition[x] = 255;
        else
            transition[x] = 0;
    return;
    }
    if (buf[1][x] > 127)
    {
        if ( buf[0][x] < 128 || buf[0][x + 1] < 128 ||
            buf[1][x + 1] < 128 ||
            buf[2][x] < 128 || buf[2][x + 1] < 128 )
            transition[x] = 255;
        else
            transition[x] = 0;
    }
    else
        transition[x] = 0;
    for (qint32 x = 1; x < width - 1; x++)
    {
        if (buf[1][x] >= 128)
        {
            if (buf[0][x - 1] < 128 || buf[0][x] < 128 || buf[0][x + 1] < 128 ||
                buf[1][x - 1] < 128           ||          buf[1][x + 1] < 128 ||
                buf[2][x - 1] < 128 || buf[2][x] < 128 || buf[2][x + 1] < 128)
                transition[x] = 255;
            else
                transition[x] = 0;
        }
        else
            transition[x] = 0;
    }
    if (buf[1][x] >= 128)
    {
        if (buf[0][x - 1] < 128 || buf[0][x] < 128 ||
            buf[1][x - 1] < 128 ||
            buf[2][x - 1] < 128 || buf[2][x] < 128)
            transition[x] = 255;
        else
            transition[x] = 0;
    }
    else
        transition[x] = 0;
}

#include "kis_selection_manager.moc"
