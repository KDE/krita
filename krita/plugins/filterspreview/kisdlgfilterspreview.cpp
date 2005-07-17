//
// C++ Implementation: kisdlgfilterspreview
//
// Description: 
//
//
// Author: Cyrille Berger <cberger@cberger.net>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "kisdlgfilterspreview.h"

#include <qlayout.h>

#include "kisfilterslistview.h"
#include "kis_filter.h"
#include "kis_view.h"


namespace Krita {
namespace Plugins {
namespace FiltersPreview {


KisDlgFiltersPreview::KisDlgFiltersPreview(KisView* view, QWidget* parent,const char *name)
	: KDialogBase(parent,name, true,"Preview filters action", Ok | Cancel), m_view(view),m_currentConfigWidget(0)
{
	QFrame* frame = makeMainWidget();
	QHBoxLayout *layout = new QHBoxLayout(frame);

	
	m_kflw = new KisFiltersListView(m_view, frame  );
	layout->add(m_kflw);
	m_vlayout = new QVBoxLayout(frame);
	layout->addLayout(m_vlayout);

	connect(m_kflw, SIGNAL(selectionChanged(QIconViewItem*)), this, SLOT(selectionHasChanged(QIconViewItem* )));
	
}


KisDlgFiltersPreview::~KisDlgFiltersPreview()
{
}


void KisDlgFiltersPreview::selectionHasChanged ( QIconViewItem * item )
{
	KisFiltersIconViewItem* kisitem = (KisFiltersIconViewItem*) item;
	m_currentFilter = kisitem->filter();
	if(m_currentConfigWidget != 0)
	{
		m_vlayout->remove(m_currentConfigWidget);
		delete m_currentConfigWidget;
		m_currentConfigWidget = 0;
	}
	KisImageSP img = m_view->currentImg();
	KisLayerSP activeLayer = img->activeLayer();
	m_currentConfigWidget = m_currentFilter->createConfigurationWidget(this,(KisPaintDeviceSP)activeLayer);
	if(m_currentConfigWidget != 0)
	{
		m_vlayout->add(m_currentConfigWidget);
	}
}



};
};
};
