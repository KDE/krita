/*
 *  dlg_colorrange.cc - part of KimageShop^WKrayon^WKrita
 *
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
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qcursor.h>

#include <knuminput.h>
#include <klocale.h>
#include <kdebug.h>

#include <kis_brush.h>
#include <kis_canvas_observer.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_filter.h>
#include <kis_filter_registry.h>
#include <kis_gradient.h>
#include <kis_id.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_pattern.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_tool_controller.h>
#include <kis_tool.h>
#include <kis_tool_registry.h>
#include <kis_types.h>
#include <kis_undo_adapter.h>
#include <kis_view.h>

#include "dlg_colorrange.h"
#include "wdg_colorrange.h"

void ColorRangeCanvasSubject::setBGColor(const QColor& c) 
{ 
	m_parent -> slotColorChanged(c);
}
void ColorRangeCanvasSubject::setFGColor(const QColor& c) 
{ 
	m_parent -> slotColorChanged(c); 
}


DlgColorRange::DlgColorRange( KisView * view, KisLayerSP layer, QWidget *  parent, const char * name)
	: super (parent, name, false, i18n("Color Range"), Ok | Cancel, Ok)
{
	m_layer = layer;
	m_view = view;
	
	m_subject = view -> getCanvasSubject();
	
	
	m_canvasSubject = new ColorRangeCanvasSubject(this, m_view);
	KisID id = KisID("colorpicker", "");
	m_picker = m_view -> toolRegistry() -> createTool((KisCanvasSubject*)m_canvasSubject, id);
	
	if (!m_picker) {
		m_page -> bnPickerPlus -> hide();
		m_page -> bnPicker -> hide();
		m_page -> bnPickerMinus -> hide();
		m_page -> cmbSelect -> removeItem(0);
	}

	
	m_page = new WdgColorRange(this, "color_range");
	setCaption(i18n("Color Range"));
	setMainWidget(m_page);
	resize(m_page -> sizeHint());

	connect(this, SIGNAL(okClicked()),
		this, SLOT(okClicked()));

	connect(this, SIGNAL(cancelClicked()),
		this, SLOT(cancelClicked()));
		
	connect(m_page -> bnPickerPlus, SIGNAL(clicked()),
		this, SLOT(slotPickerPlusClicked()));

	connect(m_page -> bnPicker, SIGNAL(clicked()),
		this, SLOT(slotPickerClicked()));

	connect(m_page -> bnPickerMinus, SIGNAL(clicked()),
		this, SLOT(slotPickerMinusClicked()));

	connect(m_page -> bnLoad, SIGNAL(clicked()),
		this, SLOT(slotLoad()));

	connect(m_page -> bnSaveColorRange, SIGNAL(clicked()),
		this, SLOT(slotSave()));

	connect(m_page -> chkInvert, SIGNAL(clicked()),
		this, SLOT(slotInvertClicked()));

	connect(m_page -> intFuzziness, SIGNAL(valueChanged(int)),
		this, SLOT(slotFuzzinessChanged(int)));

	connect(m_page -> sldrFuzziness, SIGNAL(sliderMoved(int)),
		this, SLOT(slotSliderMoved(int)));

	connect(m_page -> cmbSelect, SIGNAL(activated(int)),
		this, SLOT(slotSelectionTypeChanged(int)));

	connect(m_page -> cmbSelectionPreview, SIGNAL(activated(int)),
		this, SLOT(slotPreviewTypeChanged(int)));

	m_page -> bnLoad -> setEnabled(false);
	m_page -> bnSaveColorRange -> setEnabled(false);
	m_page -> cmbSelectionPreview -> setEnabled(false);
        m_page -> sldrFuzziness->setValue( 40 );

        if (m_layer)
		m_selection = m_layer -> selection();
	else {
		// Show message box? Without a layer no selections...
		hide();
		return;
	}
		
        m_transaction = new KisTransaction(i18n("Select by Color Range"), m_selection.data());
        updatePreview();


        if (m_picker) {
        	m_oldCursor = m_subject -> setCanvasCursor(KisCursor::pickerCursor());
		m_picker -> update(m_canvasSubject);
		m_pickMode = PICK;
		m_page -> cmbSelect -> setCurrentItem(0);
		m_picker -> activate();
		m_subject -> setCanvasCursor(KisCursor::pickerCursor());
	}
}

DlgColorRange::~DlgColorRange()
{
	delete m_page;
	delete m_picker;
	delete m_canvasSubject;
}


void DlgColorRange::updatePreview()
{
	if (!m_selection) return;
	
	Q_INT32 x, y, w, h;
	m_layer -> exactBounds(x, y, w, h);
	QPixmap pix = QPixmap(m_selection -> convertToQImage(0, x, y, w, h).smoothScale(250, 250, QImage::ScaleMin));

	m_page -> pixSelection -> setPixmap(pix);
}

void DlgColorRange::okClicked()
{
	hide();
	m_subject -> setCanvasCursor(m_oldCursor);
		m_subject -> undoAdapter() -> addCommand(m_transaction);
}

void DlgColorRange::cancelClicked()
{
	hide();
	m_subject -> setCanvasCursor(m_oldCursor);
	m_transaction -> unexecute();
	// Restore the old selection.
}

void DlgColorRange::slotPickerPlusClicked()
{
	m_pickMode = PICK_PLUS;
	m_picker -> activate();
	m_page -> cmbSelect -> setCurrentItem(0);
	m_canvasSubject -> setCanvasCursor(KisCursor::pickerPlusCursor());
}


void DlgColorRange::slotPickerClicked()
{
	m_pickMode = PICK;
	m_page -> cmbSelect -> setCurrentItem(0);
	m_picker -> activate();
	m_canvasSubject -> setCanvasCursor(KisCursor::pickerCursor());
}


void DlgColorRange::slotPickerMinusClicked()
{
	m_pickMode = PICK_MINUS;
	m_page -> cmbSelect -> setCurrentItem(0);
	m_picker -> activate();
	m_canvasSubject -> setCanvasCursor(KisCursor::pickerMinusCursor());
}

void DlgColorRange::slotLoad()
{
}


void DlgColorRange::slotSave()
{
}

void DlgColorRange::slotInvertClicked()
{
	m_selection -> invert(m_layer -> extent());
	updatePreview();
}

void DlgColorRange::slotFuzzinessChanged(int value)
{
	m_page -> sldrFuzziness -> setValue(value);
}

void DlgColorRange::slotSliderMoved(int value)
{
	m_page -> intFuzziness -> setValue(value);
}

void DlgColorRange::slotSelectionTypeChanged(int index)
{
	
	if (index != 0) {
		if (m_picker) m_picker -> clear();
	}
	
}

void DlgColorRange::slotPreviewTypeChanged(int /*index*/)
{
}

void DlgColorRange::slotColorChanged(const QColor & c)
{
	kdDebug() << "Color changed to " << c << "\n";
	if (m_pickMode == PICK) {
	}
	else if (m_pickMode == PICK_PLUS) {
	}
	else if (m_pickMode == PICK_MINUS) {
	}
}
#include "dlg_colorrange.moc"
