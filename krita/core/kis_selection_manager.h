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
#ifndef KIS_SELECTION_MANAGER_
#define KIS_SELECTION_MANAGER_

#include "qobject.h"

#include "kis_image.h"
#include "kis_selection.h"
#include "kis_floatingselection.h"

class KAction;
class KisView;
class KisDoc;

/**
 * The selection manager is responsible selections
 * and the clipboard.
 */
class KisSelectionManager : public QObject {

	Q_OBJECT;

public:

	KisSelectionManager(KisView * parent, KisDoc * doc);
	virtual ~KisSelectionManager();

	void setup(KActionCollection * collection);

public slots:

	void clipboardDataChanged();
	void updateGUI(bool enable);
	void imgSelectionChanged(KisImageSP img);

	void cut();
	void copy();
	void paste();
	void selectAll();
	void unSelectAll();
	void clear();
	void reselect();
	void invert();
	void paste_into();
	void copySelectionToNewLayer();
	void feather();
	void border();
	void expand();
	void smooth();
	void contract();
	void grow();
	void similar();
	void transform();
	void load();
	void save();

private:

	KisView * m_parent;
	KisDoc * m_doc;

	bool m_clipboardHasImage;

	KAction *m_copy;
	KAction *m_cut;
	KAction *m_paste;
	KAction *m_selectAll;
	KAction *m_deselect;
	KAction *m_clear;
	KAction *m_reselect;
	KAction *m_invert;
	KAction *m_pasteInto;
	KAction *m_toNewLayer;
	KAction *m_feather;
	KAction *m_border;
	KAction *m_expand;
	KAction *m_smooth;
	KAction *m_contract;
	KAction *m_grow;
	KAction *m_similar;
	KAction *m_transform;
	KAction *m_load;
	KAction *m_save;
};

#endif // KIS_SELECTION_MANAGER_
