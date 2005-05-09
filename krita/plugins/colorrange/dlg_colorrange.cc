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
#include <qradiobutton.h>

#include <knuminput.h>
#include <klocale.h>
#include <kdebug.h>

#include <kis_brush.h>
#include <kis_canvas_observer.h>
#include <kis_canvas_subject.h>
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
#include <kis_types.h>
#include <kis_undo_adapter.h>
#include <kis_view.h>
#include <kis_strategy_colorspace.h>
#include <kis_profile.h>
#include "kis_color_conversions.h"
#include "kis_color_utilities.h"

#include "dlg_colorrange.h"
#include "wdg_colorrange.h"



Q_UINT32 matchColors(const QColor & c, enumAction action)
{
	int r = c.red();
	int g = c.green();
	int b = c.blue();

	int h, s, v;
	rgb_to_hsv(r, g, b, &h, &s, &v);



	// XXX: Map the degree in which the colors conform to the requirement
	//      to a range of selectedness between 0 and 255

	// XXX: Implement out-of-gamut using lcms

	switch(action) {

		case REDS:
			if (isReddish(h))
				return MAX_SELECTED;
			else
				return MIN_SELECTED;
		case YELLOWS:
			if (isYellowish(h)) {
				return MAX_SELECTED;
			}
			else
				return MIN_SELECTED;
		case GREENS:
			if (isGreenish(h))
				return MAX_SELECTED;
			else
				return MIN_SELECTED;
		case CYANS:
			if (isCyanish(h))
				return MAX_SELECTED;
			else
				return MIN_SELECTED;
		case BLUES:
			if (isBlueish(h))
				return MAX_SELECTED;
			else
				return MIN_SELECTED;
		case MAGENTAS:
			if (isMagentaish(h))
				return MAX_SELECTED;
			else
				return MIN_SELECTED;
		case HIGHLIGHTS:
			if (isHighlight(v))
				return MAX_SELECTED;
			else
				return MIN_SELECTED;
		case MIDTONES:
			if (isMidTone(v))
				return MAX_SELECTED;
			else
				return MIN_SELECTED;
		case SHADOWS:
			if (isShadow(v))
				return MAX_SELECTED;
			else
				return MIN_SELECTED;
	};

	return MIN_SELECTED;
}



DlgColorRange::DlgColorRange( KisView * view, KisLayerSP layer, QWidget *  parent, const char * name)
	: super (parent, name, true, i18n("Color Range"), Ok | Cancel, Ok)
{
	m_layer = layer;
	m_view = view;

	m_subject = view -> getCanvasSubject();

	m_page = new WdgColorRange(this, "color_range");
	Q_CHECK_PTR(m_page);

	setCaption(i18n("Color Range"));
	setMainWidget(m_page);
	resize(m_page -> sizeHint());

        if (m_layer) {
		m_hadSelectionToStartWith = m_layer -> hasSelection();
		m_selection = m_layer -> selection();
	}
	else {
		// Show message box? Without a layer no selections...
		return;
	}

        m_transaction = new KisTransaction(i18n("Select by Color Range"), m_selection.data());
	Q_CHECK_PTR(m_transaction);

        updatePreview();

	m_invert = false;
	m_mode = SELECTION_REPLACE;
	m_currentAction = REDS;

	connect(this, SIGNAL(okClicked()),
		this, SLOT(okClicked()));

	connect(this, SIGNAL(cancelClicked()),
		this, SLOT(cancelClicked()));

	connect(m_page -> chkInvert, SIGNAL(clicked()),
		this, SLOT(slotInvertClicked()));

	connect(m_page -> cmbSelect, SIGNAL(activated(int)),
		this, SLOT(slotSelectionTypeChanged(int)));

	connect (m_page -> radioAdd, SIGNAL(toggled(bool)),
		 this, SLOT(slotAdd(bool)));

	connect (m_page -> radioReplace, SIGNAL(toggled(bool)),
		 this, SLOT(slotReplace(bool)));

	connect (m_page -> radioSubtract, SIGNAL(toggled(bool)),
		 this, SLOT(slotSubtract(bool)));

	connect (m_page -> bnSelect, SIGNAL(clicked()),
		this, SLOT(slotSelectClicked()));

}

DlgColorRange::~DlgColorRange()
{
	delete m_page;
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
	m_subject -> undoAdapter() -> addCommand(m_transaction);
	accept();
}

void DlgColorRange::cancelClicked()
{
	if (!m_hadSelectionToStartWith)
		m_layer -> deselect();

	m_transaction -> unexecute();

	m_subject -> canvasController() -> updateCanvas();
	reject();
}

void DlgColorRange::slotInvertClicked()
{
	m_invert = m_page -> chkInvert -> isChecked();
}

void DlgColorRange::slotSelectionTypeChanged(int index)
{
	m_currentAction = (enumAction)index;
}

void DlgColorRange::slotSubtract(bool on)
{
	if (on)
		m_mode = SELECTION_SUBTRACT;
}
void DlgColorRange::slotAdd(bool on)
{
	if (on)
		m_mode = SELECTION_ADD;
}

void DlgColorRange::slotReplace(bool on)
{
	if (on)
		m_mode = SELECTION_REPLACE;
}

void DlgColorRange::slotSelectClicked()
{
	// XXX: Multithread this!
	Q_INT32 x, y, w, h;
	m_layer -> exactBounds(x, y, w, h);
	KisStrategyColorSpaceSP cs = m_layer -> colorStrategy();
	KisProfileSP profile = m_layer -> profile();
	QUANTUM opacity;
	for (int y2 = y; y2 < h - y; ++y2) {
		KisHLineIterator hiter = m_layer -> createHLineIterator(x, y2, w, false);
		KisHLineIterator selIter = m_selection  -> createHLineIterator(x, y2, w, true);
		while (!hiter.isDone()) {
			// Clean up as we go, if necessary
			if (m_mode == SELECTION_REPLACE) memset (selIter.rawData(), 0, 1); // Selections are hard-coded one byte big.

			QColor c;

			cs -> toQColor(hiter.rawData(), &c, &opacity, profile);
			// Don't try to select transparent pixels.
			if (opacity > OPACITY_TRANSPARENT) {
				Q_UINT8 match = matchColors(c, m_currentAction);

				if (match) {
					// Personally, I think the invert option a bit silly. But it's possible I don't quite understand it. BSAR.
					if (!m_invert) {
						if (m_mode == SELECTION_ADD || m_mode == SELECTION_REPLACE) {
							*(selIter.rawData()) =  match;
						}
						else if (m_mode == SELECTION_SUBTRACT) {
							Q_UINT8 selectedness = *(selIter.rawData());
							if (match < selectedness) {
								*(selIter.rawData()) = selectedness - match;
							}
							else {
								*(selIter.rawData()) = 0;
							}
						}
					}
					else {
						if (m_mode == SELECTION_ADD || m_mode == SELECTION_REPLACE) {
							Q_UINT8 selectedness = *(selIter.rawData());
							if (match < selectedness) {
								*(selIter.rawData()) = selectedness - match;
							}
							else {
								*(selIter.rawData()) = 0;
							}
						}
						else if (m_mode == SELECTION_SUBTRACT) {
							*(selIter.rawData()) =  match;
						}
					}
				}
			}
			++hiter;
			++selIter;
		}
	}
	updatePreview();
}

#include "dlg_colorrange.moc"
