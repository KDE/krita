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
#include <kis_strategy_colorspace.h>
#include <kis_profile.h>

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

	m_page -> bnLoad -> setEnabled(false);
	m_page -> bnSaveColorRange -> setEnabled(false);
	m_page -> cmbSelectionPreview -> setEnabled(false);
        m_page -> sldrFuzziness -> setValue( 40 );
        m_page -> intFuzziness -> setValue( 40 );
	m_fuzziness = 40;

	
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
		m_mode = REPLACE;
		m_page -> cmbSelect -> setCurrentItem(0);
		m_picker -> activate();
		m_subject -> setCanvasCursor(KisCursor::pickerCursor());
	}

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
	QPixmap pix = QPixmap(m_selection -> maskImage().smoothScale(350, 350, QImage::ScaleMin));
	m_subject -> canvasController() -> updateCanvas();
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
	m_mode = ADD;
	m_picker -> activate();
	m_page -> cmbSelect -> setCurrentItem(0);
	m_subject -> setCanvasCursor(KisCursor::pickerPlusCursor());
}


void DlgColorRange::slotPickerClicked()
{
	m_mode = REPLACE;
	m_page -> cmbSelect -> setCurrentItem(0);
	m_picker -> activate();
	m_subject -> setCanvasCursor(KisCursor::pickerCursor());
}


void DlgColorRange::slotPickerMinusClicked()
{
	m_mode = SUBTRACT;
	m_page -> cmbSelect -> setCurrentItem(0);
	m_picker -> activate();
	m_subject -> setCanvasCursor(KisCursor::pickerMinusCursor());
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
	kdDebug() << "fuzziness changed\n";
	m_fuzziness = value;
	selectByColor(m_currentColor, m_fuzziness, REPLACE); // XXX: is this correct?
	updatePreview();
}

void DlgColorRange::slotSliderMoved(int value)
{
	m_page -> intFuzziness -> setValue(value);
}

void DlgColorRange::slotSelectionTypeChanged(int index)
{
	if (index != PICKER) {
		if (m_picker)
		m_picker -> clear();
		m_subject -> setCanvasCursor(m_oldCursor);
	}

	switch (index) {

	case REDS :
		m_currentColor = QColor(255, 0, 0);
		m_mode = REPLACE;
		break;
	case YELLOWS:
		m_currentColor = QColor(255, 255, 0);
		m_mode = REPLACE;
		break;
	case GREENS:
		m_currentColor = QColor(0, 255, 0);
		m_mode = REPLACE;
		break;
	case CYANS:
		m_currentColor = QColor(0, 255, 255);
		m_mode = REPLACE;
		break;
	case BLUES:
		m_currentColor = QColor(0, 0, 255);
		m_mode = REPLACE;
		break;
	case MAGENTAS:
		m_currentColor = QColor(255, 0, 255);
		m_mode = REPLACE;
		break;
	}
	
	selectByColor(m_currentColor, m_fuzziness, m_mode);
	updatePreview();
	
}

void DlgColorRange::slotPreviewTypeChanged(int /*index*/)
{
}

void DlgColorRange::slotColorChanged(const QColor & c)
{
	m_currentColor = c;
		kdDebug() << "Color: " << QString::number(c.red()) << ", "
                          << QString::number(c.green()) << ", "
                          << QString::number(c.blue()) << "\n";
 
	selectByColor(m_currentColor, m_fuzziness, m_mode);
}

/**
 * Returns the selectedness of the pixel c2 compared to the reference pixel c, taking into
 * account the specified fuzziness.
 *
 * The following algorithm is used:
 * for all channels in colortype of c:
 *    determine the absolute difference
 *    determine which is the biggest difference
 *    if the absolute difference is greater than fuzziness, return MIN_SELECTED
 *    else
 *      antialias the result of in the range using an algorithm from the gimp.
 *
 */
Q_UINT8 DlgColorRange::matchColors(QColor c, QColor c2, Q_UINT8 fuzziness)
{
// 	kdDebug() << "Color 1: " << QString::number(c.red()) << ", "
// 	                      << QString::number(c.green()) << ", "
// 	                      <<  QString::number(c.blue()) << "\n";
// 
// 
// 	kdDebug() << "Color 2: " << QString::number(c2.red()) << ", "
// 	                      << QString::number(c2.green()) << ", "
// 	                      <<  QString::number(c2.blue()) << "\n";
// 	                      
// 	kdDebug() << "Fuzziness: " << QString::number(fuzziness) << "\n";

	Q_INT32 diffMax = 0;
	Q_INT32 dr, dg, db = 0;

	dr = QABS(c.red() - c2.red());
	dg = QABS(c.green() - c2.green());
	db = QABS(c.blue() - c2.blue());

// /*	kdDebug() << " red difference: " << QString::number(dr)
// 		  << " green difference: " << QString::number(dg)
// 		  << " blue difference: " << QString::number(db) << "\n";
// 	*/
	diffMax = QMAX(dr, QMAX(dg, db));

/*	kdDebug() << "Max difference: " << QString::number(diffMax) << "\n";*/
	
	if (diffMax > fuzziness)
		return MIN_SELECTED;

	float aa = 1.5 - ((float) diffMax / fuzziness);

/*	kdDebug() << "AA value: " << QString::number(aa) << "\n";*/
	
	if (aa <= 0.0)
		return 0;
	else if (aa < 0.5)
		return Q_UINT8 (aa * 512);
	else
		return 255;

}

void DlgColorRange::selectByColor(const QColor & c, Q_UINT8 fuzziness, enumMode mode)
{
	// XXX: Multithread this!
	Q_INT32 x, y, w, h;
	m_layer -> exactBounds(x, y, w, h);
	KisStrategyColorSpaceSP cs = m_layer -> colorStrategy();
	KisProfileSP profile = m_layer -> profile();
	for (int y2 = y; y2 < h - y; ++y2) {
		KisHLineIterator hiter = m_layer -> createHLineIterator(x, y2, w, false);
		KisHLineIterator selIter = m_selection  -> createHLineIterator(x, y2, w, true);
		while (!hiter.isDone()) {
			// Clean up as we go, if necessary
			if (mode == REPLACE) memset (selIter.rawData(), 0, 1); // Selections are hard-coded one byte big.
	
			QColor c2;
			
			cs -> toQColor(hiter.rawData(), &c2, profile);

 			Q_UINT8 match = matchColors(c, c2, fuzziness);
// 			kdDebug() << "Match: " << QString::number(match) << "\n";

			if (match > 0) {
/*				kdDebug() << ">>>>>>>>>>>>>>>>>>>>>>> Match: " << QString::number(match) << "\n";*/
				if (mode == ADD || mode == REPLACE) {
					*(selIter.rawData()) =  match;
				}
				else if (mode == SUBTRACT) {
					Q_UINT8 selectedness = *(selIter.rawData());
					if (match < selectedness) {
						*(selIter.rawData()) = selectedness - match;
					}
					else {
						*(selIter.rawData()) = 0;
					}
				}
			}
			
			++hiter;
			++selIter;
		}
	}
}

void DlgColorRange::selectByValue(const enumTone tone, Q_UINT32 fuzziness, enumMode mode)
{
}

#include "dlg_colorrange.moc"
