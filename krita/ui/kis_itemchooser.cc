/*
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
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <kinstance.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <koIconChooser.h>

#include "integerwidget.h"
#include "kis_itemchooser.h"
#include "kis_global.h"
#include "kis_icon_item.h"
#include "kis_brush.h"

KisItemChooser::KisItemChooser(const vKoIconItem& items, bool spacing, QWidget *parent, const char *name) : super(parent, name)
{
	init(spacing);

	QPtrListIterator<KoIconItem> itr(items);

	for (itr.toFirst(); itr.current(); ++itr)
		m_chooser -> addItem(itr.current());
}

KisItemChooser::KisItemChooser(bool spacing, QWidget *parent, const char *name) : super(parent, name)
{
	init(spacing);
}

KisItemChooser::~KisItemChooser()
{
}

void KisItemChooser::initGUI(bool spacing)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this, 2, -1, "main layout");

	mainLayout -> addWidget(m_frame, 10);

	if (spacing) {
		QGridLayout *spacingLayout = new QGridLayout( 2, 2);

		mainLayout -> addLayout(spacingLayout, 1);

		spacingLayout -> addWidget(m_lbSpacing, 0, 0);
		spacingLayout -> addWidget(m_slSpacing, 0, 1);

		spacingLayout -> addMultiCellWidget(m_chkColorMask, 1, 1, 0, 1);
		
	}
}

void KisItemChooser::setCurrent(KoIconItem *item)
{
	m_chooser -> setCurrentItem(item);

	if (m_doSpacing) {
		KisIconItem *kisItem = dynamic_cast<KisIconItem*>(item);
		m_chkColorMask -> setChecked(kisItem -> useColorAsMask());
		m_chkColorMask -> setEnabled(kisItem -> hasColor());
	}
}

KoIconItem* KisItemChooser::currentItem()
{
	return m_chooser -> currentItem();
}

void KisItemChooser::slotItemSelected(KoIconItem *item)
{
	if (m_doSpacing && item) {
		KisIconItem *kisItem = dynamic_cast<KisIconItem*>(item);
		m_slSpacing -> setValue(kisItem -> spacing() / 10 );
		m_chkColorMask -> setChecked(kisItem -> useColorAsMask());
		m_chkColorMask -> setEnabled(kisItem -> hasColor());
	}
	emit selected(item);
}

void KisItemChooser::slotSetItemSpacing(int spacingValue)
{
	KoIconItem *item = currentItem();

	if (m_doSpacing && item)
		(dynamic_cast<KisIconItem*>(item)) -> setSpacing(spacingValue * 10 );
}

void KisItemChooser::slotSetItemUseColorAsMask(bool useColorAsMask)
{
	KoIconItem *item = currentItem();

 	if (item) {
 		(dynamic_cast<KisIconItem*>(item)) -> setUseColorAsMask(useColorAsMask);
		// The item's pixmaps may have changed so get observers to update
		// their display.
		emit selected(item);
	}
}


void KisItemChooser::addItem(KoIconItem *item)
{
	m_chooser -> addItem(item);
}

void KisItemChooser::addItem(const vKoIconItem& items)
{
	QPtrListIterator<KoIconItem> itr(items);

	for (itr.toFirst(); itr.current(); ++itr)
		m_chooser -> addItem(itr.current());
}

void KisItemChooser::init(bool spacing)
{
	m_doSpacing = spacing;

	if (spacing) {
		m_lbSpacing = new QLabel(i18n("Spacing: "), this);
		m_slSpacing = new IntegerWidget( 1, 100, this, "int_widget" );
		m_slSpacing -> setTickmarks(QSlider::Below);
		m_slSpacing -> setTickInterval(10);
		QObject::connect(m_slSpacing, SIGNAL(valueChanged(int)), this, SLOT(slotSetItemSpacing(int)));

		m_chkColorMask = new QCheckBox(i18n("Use color as mask"), this);
		QObject::connect(m_chkColorMask, SIGNAL(toggled(bool)), this, SLOT(slotSetItemUseColorAsMask(bool)));

	} else {
		m_lbSpacing = 0;
		m_slSpacing = 0;
		m_chkColorMask = 0;
	}

	m_frame = new QHBox(this);
	m_frame -> setFrameStyle(QFrame::Panel | QFrame::Sunken);
	m_chooser = new KoIconChooser(QSize(30,30), m_frame, "icon_chooser", true);
	QObject::connect(m_chooser, SIGNAL(selected(KoIconItem*)), this, SLOT(slotItemSelected(KoIconItem*)));
	initGUI(spacing);
}



#include "kis_itemchooser.moc"

