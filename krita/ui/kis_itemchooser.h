/*
 *  Copyright (c) 2002 Patrick Julein <freak@codepimps.org>
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
#if !defined KIS_BRUSH_CHOOSER_H_
#define KIS_BRUSH_CHOOSER_H_

#include <qptrlist.h>
#include <qwidget.h>
#include <qframe.h>

#include <koIconChooser.h>
#include "kfloatingdialog.h"

class QHBox;
class QLabel;

class KoIconChooser;
class KoIconItem;

class IntegerWidget;

class KisCmbComposite;

typedef QPtrList<KoIconItem> vKoIconItem;

class KisItemChooser : public KFloatingDialog {
	typedef KFloatingDialog super;
	Q_OBJECT

public:
	KisItemChooser(const vKoIconItem& items, 
		       bool spacing, 
		       QWidget *parent = 0, 
		       const char *name = 0);
	KisItemChooser(bool spacing, 
		       QWidget *parent = 0, 
		       const char *name = 0);
	virtual ~KisItemChooser();

	KoIconItem *currentItem();
	void setCurrent(KoIconItem *item);

public slots:
	void addItem(KoIconItem *item);
	void addItem(const vKoIconItem& items);

signals:
	void selected(KoIconItem *item);

private:
	void initGUI(bool spacing);
	void init(bool spacing);

private slots:
	void slotItemSelected(KoIconItem *item);
	void slotSetItemSpacing(int spacing);
	void slotSetItemOpacity(int opacity);
	void slotSetItemCompositeMode(int compositeOp);

private:
	QHBox *m_frame;
	QWidget *m_container;
	QLabel *m_lbSpacing;
	IntegerWidget *m_slSpacing;
	QLabel *m_lbOpacity;
	IntegerWidget *m_slOpacity;
	QLabel *m_lbComposite;
	KisCmbComposite *m_cmbComposite;
	KoIconChooser *m_chooser;
	bool m_doSpacing;
};

#endif // KIS_BRUSH_CHOOSER_H_

