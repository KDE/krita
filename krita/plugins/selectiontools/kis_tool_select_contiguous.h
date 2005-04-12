/*
 *  kis_tool_select_contiguous.h - part of KImageShop^WKrayon^Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __KIS_TOOL_SELECT_CONTIGUOUS_H__
#define __KIS_TOOL_SELECT_CONTIGUOUS_H__

#include <kis_tool.h>
#include <kis_tool_non_paint.h>
#include <kis_tool_factory.h>
#include <kis_selection.h>

class KisCanvasSubject;
class QWidget;
class QVBoxLayout;
class QCheckBox;
class KisSelectionOptions;

/**
 * The 'magic wand' selection tool -- in fact just
 * a floodfill that only creates a selection.
 */
class KisToolSelectContiguous : public KisToolNonPaint {

	typedef KisToolNonPaint super;
	Q_OBJECT

public:
	KisToolSelectContiguous();
	virtual ~KisToolSelectContiguous();

public:
	virtual void update(KisCanvasSubject *subject);

	virtual void setup(KActionCollection *collection);
	virtual QWidget* createOptionWidget(QWidget* parent);
        virtual QWidget* optionWidget();

	virtual void buttonPress(KisButtonPressEvent *event);

public slots:
	virtual void slotSetFuzziness(int);
	virtual void slotSetAction(int);

private:
	// Floodfill a selection -- factor this out once it works
	// to KisSelection.
	void fillSelection(KisPaintDeviceSP device, enumSelectionMode mode, int startX, int startY);
	/**
	 * calculates the difference between 2 pixel values. Returns a value between 0 and
	 * 255 (actually should be MIN_SELECTED to MAX_SELECTED?). Only 0 and 255 are
	 * returned when anti-aliasing is off. This is RGB based, not HSV like in the selection
	 * picker.
	 * XXX: factor this out when done.
	 **/
	QUANTUM difference(const QUANTUM* src, KisPixel dst);

	typedef enum { Left, Right } Direction;

	void floodLine(int x, int y, enumSelectionMode mode);
	int floodSegment(int x, int y, int most, KisHLineIteratorPixel& it, int lastPixel, Direction d, enumSelectionMode mode);

private:
	KisCanvasSubject *m_subject;
        QWidget * m_optWidget;
	KisSelectionOptions * m_options; // Default options widget

	int m_fuzziness;
	enumSelectionMode m_selectAction;

	// Scratch variables for floodfilling the selection
	KisSelectionSP m_selection;
	KisPaintDeviceSP m_device;
	int m_size;
	int m_depth;
	int m_colorChannels;
	int m_width, m_height;
	QRect m_rect;
	bool* m_map;
	QUANTUM* m_oldColor, *m_color;
};

class KisToolSelectContiguousFactory : public KisToolFactory {
	typedef KisToolFactory super;
public:
	KisToolSelectContiguousFactory(KActionCollection * ac) : super(ac) {};
	virtual ~KisToolSelectContiguousFactory(){};

	virtual KisTool * createTool() { 
		KisTool * t =  new KisToolSelectContiguous(); 
		Q_CHECK_PTR(t);
		t -> setup(m_ac); 
		return t; 
	}
	virtual KisID id() { return KisID("contiguousselect", i18n("Contiguous select tool")); }
};


#endif //__KIS_TOOL_SELECT_CONTIGUOUS_H__

