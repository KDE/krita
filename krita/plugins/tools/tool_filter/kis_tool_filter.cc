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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qbitmap.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_filter_config_widget.h"
#include "kis_tool_filter.h"
#include <kis_brush.h>
#include <kis_button_press_event.h>
#include <kis_button_release_event.h>
#include <kis_canvas_subject.h>
#include <kis_cmb_idlist.h>
#include <kis_cursor.h>
#include <kis_doc.h>
#include <kis_filter.h>
#include <kis_filterop.h>
#include <kis_id.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_move_event.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_paintop_registry.h>
#include <kis_vec.h>

KisToolFilter::KisToolFilter()
    : super(i18n("Filter Brush")), m_filterConfigurationWidget(0)
{
    setName("tool_filter");
    m_subject = 0;
    setCursor(KisCursor::load("tool_filter_cursor.png", 5, 5));
}

KisToolFilter::~KisToolFilter()
{
}

void KisToolFilter::setup(KActionCollection *collection)
{
    m_action = collection->action(name());

    if (m_action == 0) {
        m_action = new KAction(i18n("&Filter Brush"),
                        "tool_filter", 0, this,
                        SLOT(activate()), collection,
                        name());
        Q_CHECK_PTR(m_action);
        m_action->setToolTip(i18n("Paint with filters"));
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}

void KisToolFilter::initPaint(KisEvent *e)
{
    // Some filters want to paint directly on the current state of
    // the canvas, others cannot handle that and need a temporary layer
    // so they can work on the old data before painting started.
    m_paintIncremental = m_filter->supportsIncrementalPainting();
    
    super::initPaint(e);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("filter", 0, painter());
    op->setSource ( m_source );
    painter()->setPaintOp(op); // And now the painter owns the op and will destroy it.
    painter()->setFilter( m_filter );

    // XXX: Isn't there a better way to set the config? The filter config widget needs to
    // to go into the tool options widget, and just the data carried over to the filter.
    // I've got a bit of a problem with core classes having too much GUI about them.
    // BSAR.
    dynamic_cast<KisFilterOp *>(op)->setFilterConfiguration( m_filter->configuration( m_filterConfigurationWidget) );
}

QWidget* KisToolFilter::createOptionWidget(QWidget* parent)
{
    QWidget *widget = super::createOptionWidget(parent);

    m_cbFilter = new KisCmbIDList(widget);
    Q_CHECK_PTR(m_cbFilter);

    QLabel* lbFilter = new QLabel(i18n("Filter:"), widget);
    Q_CHECK_PTR(lbFilter);

    // Check which filters support painting
    KisIDList l = KisFilterRegistry::instance()->listKeys();
    KisIDList l2;
    KisIDList::iterator it;
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->get(*it);
        if (f->supportsPainting()) {
            l2.push_back(*it);
        }
    }
    m_cbFilter ->setIDList( l2 );

    addOptionWidgetOption(m_cbFilter, lbFilter);

    m_optionLayout = new Q3GridLayout(widget, 1, 1, 0, 6);
    Q_CHECK_PTR(m_optionLayout);
    super::addOptionWidgetLayout(m_optionLayout);

    connect(m_cbFilter, SIGNAL(activated ( const KisID& )), this, SLOT( changeFilter( const KisID& ) ) );
    changeFilter( m_cbFilter->currentItem () );

    return widget;
}

void KisToolFilter::changeFilter( const KisID & id)
{
    m_filter =  KisFilterRegistry::instance()->get( id );
    Q_ASSERT(!m_filter.isNull());
    if( m_filterConfigurationWidget != 0 )
    {
        m_optionLayout->remove ( m_filterConfigurationWidget );
        delete m_filterConfigurationWidget;
    }

    m_source = m_currentImage->activeDevice();
    if (!m_source) return;

    m_filterConfigurationWidget = m_filter->createConfigurationWidget( optionWidget(), m_source );
    if( m_filterConfigurationWidget != 0 )
    {
        m_optionLayout->addMultiCellWidget ( m_filterConfigurationWidget, 2, 2, 0, 1 );
        m_filterConfigurationWidget->show();
    }
}

#include "kis_tool_filter.moc"
