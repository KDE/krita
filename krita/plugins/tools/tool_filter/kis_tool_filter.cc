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

#include "kis_tool_filter.h"

#include <QBitmap>
#include <QPainter>
#include <QComboBox>
#include <QLayout>
#include <QLabel>
#include <QGridLayout>

#include <kactioncollection.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "KoPointerEvent.h"
#include "KoCanvasBase.h"

#include "kis_filter_config_widget.h"
#include "kis_brush.h"
#include "kis_cmb_idlist.h"
#include "kis_cursor.h"
#include "kis_filter.h"
#include "kis_filter_registry.h"
#include "kis_filterop.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_vec.h"

KisToolFilter::KisToolFilter(KoCanvasBase * canvas)
    : super(canvas, KisCursor::load("tool_filter_cursor.png", 5, 5), i18n("Filter Brush")),
    m_filterConfigurationWidget(0)
{
    setObjectName("tool_filter");
    m_optionWidget = 0;
}

KisToolFilter::~KisToolFilter()
{
}

void KisToolFilter::initPaint(KoPointerEvent *e)
{
    // Some filters want to paint directly on the current state of
    // the canvas, others cannot handle that and need a temporary layer
    // so they can work on the old data before painting started.
    m_paintIncremental = m_filter->supportsIncrementalPainting();

    super::initPaint(e);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("filter", 0, m_painter, currentImage());
    op->setSource ( m_source );
    m_painter->setPaintOp(op); // And now the painter owns the op and will destroy it.
    m_painter->setFilter( m_filter );

    // XXX: Isn't there a better way to set the config? The filter config widget needs to
    // to go into the tool options widget, and just the data carried over to the filter.
    // I've got a bit of a problem with core classes having too much GUI about them.
    // BSAR.
    if (m_filterConfigurationWidget) {
        dynamic_cast<KisFilterOp *>(op)->setFilterConfiguration( m_filterConfigurationWidget->configuration() );
    }
}

QWidget* KisToolFilter::createOptionWidget()
{
    m_optionWidget = super::createOptionWidget();

    m_cbFilter = new KisCmbIDList(m_optionWidget);
    Q_CHECK_PTR(m_cbFilter);

    QLabel* lbFilter = new QLabel(i18n("Filter:"), m_optionWidget);
    Q_CHECK_PTR(lbFilter);

    // Check which filters support painting
    QList<KoID> l = KisFilterRegistry::instance()->listKeys();
    QList<KoID> l2;
    QList<KoID>::iterator it;
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->value((*it).id());
        if (f->supportsPainting()) {
            l2.push_back(*it);
        }
    }
    m_cbFilter ->setIDList( l2 );

    addOptionWidgetOption(m_cbFilter, lbFilter);

    m_optionLayout = new QGridLayout(m_optionWidget);
    Q_CHECK_PTR(m_optionLayout);
    m_optionLayout->setMargin(0);
    m_optionLayout->setSpacing(6);
    super::addOptionWidgetLayout(m_optionLayout);

    connect(m_cbFilter, SIGNAL(activated ( const KoID& )), this, SLOT( changeFilter( const KoID& ) ) );
    changeFilter( m_cbFilter->currentItem () );

    return m_optionWidget;
}

QWidget* KisToolFilter::optionWidget()
{
    return m_optionWidget;
}

void KisToolFilter::changeFilter( const KoID & id)
{
    m_filter =  KisFilterRegistry::instance()->value( id.id() );
    Q_ASSERT(!m_filter.isNull());
    if( m_filterConfigurationWidget != 0 )
    {
        m_optionLayout->removeWidget( m_filterConfigurationWidget );
        delete m_filterConfigurationWidget;
    }

    m_source = currentLayer()->paintDevice();
    if (!m_source) return;

    m_filterConfigurationWidget = m_filter->createConfigurationWidget( m_optionWidget, m_source );
    if( m_filterConfigurationWidget != 0 )
    {
        m_optionLayout->addWidget ( m_filterConfigurationWidget, 2, 0, 1, 2 );
        m_filterConfigurationWidget->show();
    }
}

#include "kis_tool_filter.moc"
