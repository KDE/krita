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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qobject.h>
#include <qclipboard.h>
#include <qapplication.h>

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kstdaction.h>

#include "kis_types.h"
#include "kis_view.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_floatingselection.h"
#include "kis_selection_manager.h"


KisSelectionManager::KisSelectionManager(KisView * parent, KisDoc * doc)
	: m_parent(parent),
	  m_doc(doc),
	  m_previousSelection(0),
	  m_clipboardHasImage(false),
	  m_copy(0),
	  m_cut(0),
	  m_paste(0),
	  m_selectAll(0),
	  m_deselect(0),
	  m_clear(0),
	  m_reselect(0),
	  m_invert(0),
	  m_pasteInto(0),
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
	  m_save(0)
	  
{
}

KisSelectionManager::~KisSelectionManager()
{
}


void KisSelectionManager::setup(KActionCollection * collection)
{
	// XXX: setup shortcuts!

        m_cut = 
		KStdAction::cut(this, 
				SLOT(cut()), 
				collection, 
				"cut");
	
        m_copy = 
		KStdAction::copy(this, 
				 SLOT(copy()), 
				 collection, 
				 "copy");
	
        m_paste = 
		KStdAction::paste(this, 
				  SLOT(paste()), 
				  collection, 
				  "paste");
	

        m_pasteInto = 
		new KAction(i18n("Paste Into"), 
			    "paste_into", 0, 
			    this, SLOT(paste_into()), 
			    collection, "paste_into");
	

        m_selectAll = 
		KStdAction::selectAll(this, 
				      SLOT(selectAll()), 
				      collection, 
				      "select_all");

        m_deselect = 
		KStdAction::deselect(this, 
				     SLOT(unSelectAll()), 
				     collection, 
				     "deselect");


        m_clear = 
		KStdAction::clear(this,
				  SLOT(clear()), 
				  collection, 
				  "clear");

	m_reselect =
		new KAction(i18n("&Reselect"),
			    0, 0,
			    this, SLOT(reselect()),
			    collection, "reselect");


	m_invert =
		new KAction(i18n("&Invert"),
			    0, 0,
			    this, SLOT(invert()),
			    collection, "invert");

	
        m_toNewLayer = 
		new KAction(i18n("Copy Selection to New Layer"), 
			    0, 0,
			    this, SLOT(copySelectionToNewLayer()), 
			    collection, "copy_selection_to_new_layer");
	
	m_feather =
		new KAction(i18n("Feather..."),
			    0, 0,
			    this, SLOT(feather()),
			    collection, "feather");

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
		= new KAction(i18n("Save..."),
			      0, 0,
			      this, SLOT(save()),
			      collection, "save_selection");

        QClipboard *cb = QApplication::clipboard();
        connect(cb, SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));

}


void KisSelectionManager::clipboardDataChanged()
{
        m_clipboardHasImage = !QApplication::clipboard() -> image().isNull();
}


void KisSelectionManager::updateGUI(bool enable)
{
// 	kdDebug() << "KisSelectionManager::updateGUI(" << enable << ")\n";

        KisImageSP img = m_parent -> currentImg();

        enable = enable && img && img -> activeLayer() -> hasSelection(); // XXX: don't see the idea behind this: && img -> activeSelection() -> parent();

// 	kdDebug() << "Becomes: " << enable << "\n";

	m_copy -> setEnabled(enable);
	m_cut -> setEnabled(enable);
	m_paste -> setEnabled(img != 0 && m_clipboardHasImage);
	m_selectAll -> setEnabled(img != 0);
	m_deselect -> setEnabled(enable);
	m_clear -> setEnabled(enable);
	m_reselect -> setEnabled(m_previousSelection != 0);
	m_invert -> setEnabled(enable);
	m_pasteInto -> setEnabled(img != 0 && m_clipboardHasImage);
	m_toNewLayer -> setEnabled(enable);
	m_feather -> setEnabled(enable);
	m_border -> setEnabled(enable);
	m_expand -> setEnabled(enable);
	m_smooth -> setEnabled(enable);
	m_contract -> setEnabled(enable);
	m_grow -> setEnabled(enable);
	m_similar -> setEnabled(enable);
	m_transform -> setEnabled(enable);
	m_load -> setEnabled(false); // XXX: not implemented yet
	m_save -> setEnabled(false); // XXX: not implemented yet


}

void KisSelectionManager::imgSelectionChanged(KisImageSP img)
{
        if (img == m_parent -> currentImg()) {
                updateGUI(img -> activeLayer() -> hasSelection());
		m_parent -> updateCanvas();
	}
	
}

void KisSelectionManager::cut()
{
        copy();
        clear();
}

void KisSelectionManager::copy()
{
        KisImageSP img = m_parent -> currentImg();
        if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	if (!layer -> hasSelection()) return;

	KisSelectionSP selection = layer -> selection();

//                 KisSelectionSP selection = img -> activeLayer() -> selection();

//                 if (selection) {
// 			// create a floating selection from selection

//                         selection -> clearParentOnMove(false);
//                         m_doc -> setClipboardSelection(selection);
//                         imgSelectionChanged(currentImg());
 

}


void KisSelectionManager::paste()
{
//      Q_ASSERT(!QApplication::clipboard() -> image().isNull());
//      activateTool(m_paste);
}


void KisSelectionManager::selectAll()
{
        KisImageSP img = m_parent -> currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;
	

	KisSelectionSP s = new KisSelection(layer, "layer selection for: " + layer -> name());
	s -> select(QRect(0, 0, layer -> width(), layer -> height()));
	s -> setVisible(true);

	layer -> setSelection(s);

}

void KisSelectionManager::unSelectAll()
{
        KisImageSP img = m_parent -> currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	layer -> removeSelection(); // XXX save selection for reselect
	
	m_parent -> updateCanvas();
        
}

void KisSelectionManager::clear()
{
        KisImageSP img = m_parent -> currentImg();

        if (img) {
		// XXX: for every selected pixel, make pixel in parent layer transparent.

//                 KisFloatingSelectionSP selection = img -> selection();

//                 if (selection) {
//                         KisPaintDeviceSP parent = selection -> parent();

//                         if (parent) {
//                                 QRect rc = selection -> bounds();
//                                 QRect ur;
//                                 KisPainter gc(parent);

//                                 m_adapter -> beginMacro(i18n("Remove Selection"));
// 		img -> removeActiveSelection(); // commit=true
//                                 ur = rc;

//                                 if (parent -> x() || parent -> y())
//                                         rc.moveBy(-parent -> x(), -parent -> y());

//                                 gc.beginTransaction("remove selection on parent");
//                                 gc.eraseRect(rc);
//                                 m_adapter -> addCommand(gc.end());
//                                 m_adapter -> endMacro();
//                                 m_doc -> setModified(true);
//                                 updateCanvas(ur);
//                         }
//                 }
	}
}



void KisSelectionManager::reselect()
{
	// Restore the previous selection
}


void KisSelectionManager::invert()
{
        KisImageSP img = m_parent -> currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;
	
	if (layer -> hasSelection()) {
		KisSelectionSP s = layer -> selection();
		s -> invert(QRect(0, 0, layer -> width(), layer -> height()));
	}
	else {
		selectAll();
	}
	m_parent -> updateCanvas(layer -> bounds());

}

void KisSelectionManager::paste_into()
{
        KisImageSP img = m_parent -> currentImg();

        Q_ASSERT(!QApplication::clipboard() -> image().isNull());

        if (img) {
                KisFloatingSelectionSP fsel = m_doc -> clipboardFloatingSelection();
                KisLayerSP layer = m_doc -> layerAdd(img, img -> nextLayerName(), fsel);

                img -> unsetFloatingSelection(false);

                if (layer) {
                        layer -> setX(0);
                        layer -> setY(0);
                        m_parent -> updateCanvas(layer -> bounds());
                }
        }
}



void KisSelectionManager::copySelectionToNewLayer()
{
        KisImageSP img = m_parent -> currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	KisSelectionSP selection = layer -> selection();
	//XXX: Create new layer, transfer all selected pixels to new layer
	
// 		img -> removeActiveSelection(); // XXX: commit==false
	
//                 if (selection && m_doc -> layerAdd(img, img -> nextLayerName(), selection))
//                         layersUpdated();
//                 else
//                         img -> setSelection(selection);
        
}


void KisSelectionManager::feather() {}
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
