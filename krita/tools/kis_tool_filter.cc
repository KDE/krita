/*
 *  kis_tool_filter.cc - part of Krita
 *
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include <qbitmap.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_filter.h"
#include "kis_filter_configuration_widget.h"
#include "kis_filter_registry.h"
#include "kis_brush.h"
#include "kis_view.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_tool_filter.h"
#include "kis_painter.h"
#include "kis_vec.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_filterop.h"

KisToolFilter::KisToolFilter(KisView* view) 
	: super(i18n("Filter tool")), m_view(view), m_filterConfigurationWidget(0)
{
	setName("tool_filter");
	m_subject = 0;
	setCursor(KisCursor::penCursor());
}

KisToolFilter::~KisToolFilter()
{
}

void KisToolFilter::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Filter"),
					    "filter", 0, this,
					    SLOT(activate()), collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

void KisToolFilter::initPaint(KisEvent *e) 
{
	super::initPaint(e);

	KisFilterOp* op = new KisFilterOp(painter());
	painter() -> setPaintOp(op); // And now the painter owns the op and will destroy it.
	painter() -> setFilter( m_filter );
	op -> setFilterConfiguration( m_filter -> configuration( m_filterConfigurationWidget ) );
}

QWidget* KisToolFilter::createOptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Filter"));
	QWidget* optionFreeHandWidget = KisToolFreeHand::createOptionWidget(m_optWidget);
	m_cbFilter = new QComboBox(m_optWidget);
	QLabel* lbFilter = new QLabel(i18n("Filter : "), m_optWidget);

	m_cbFilter ->insertStringList(m_view->filterRegistry()->listKeys() );
	
	m_optionLayout = new QGridLayout(m_optWidget, 3, 2);

	m_optionLayout -> addMultiCellWidget(optionFreeHandWidget, 0, 0, 0 , 1 );
 	m_optionLayout -> addWidget(lbFilter, 1, 0);
 	m_optionLayout -> addWidget(m_cbFilter, 1, 1);

	connect(m_cbFilter, SIGNAL(activated ( const QString& )), this, SLOT( changeFilter( const QString& ) ) );
	changeFilter( m_cbFilter->currentText () );
	
	return m_optWidget;
}

QWidget* KisToolFilter::optionWidget()
{
	return m_optWidget;
}

void KisToolFilter::changeFilter( const QString & string )
{
	kdDebug() << "KisToolFilter::changeFilter : change to " << string << endl;
	m_filter = m_view -> filterRegistry() -> get( string );
	Q_ASSERT(m_filter != 0);
	if( m_filterConfigurationWidget != 0 )
	{
		m_optionLayout -> remove ( m_filterConfigurationWidget );
		delete m_filterConfigurationWidget;
	}
	m_filterConfigurationWidget = m_filter -> createConfigurationWidget( m_optWidget );
	kdDebug() << "KisToolFilter::changeFilter m_filterConfigurationWidget = " << m_filterConfigurationWidget << endl;
	if( m_filterConfigurationWidget != 0 )
	{
		kdDebug() << "KisToolFilter::changeFilter add to layout manager" << endl;
		m_optionLayout -> addMultiCellWidget ( m_filterConfigurationWidget, 2, 2, 0, 1 );
	}
}

#include "kis_tool_filter.moc"
