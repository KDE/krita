/*
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

#include "kis_custom_convolution_filter_configuration_widget.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

#include <klocale.h>

#include "kis_filter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_view.h"
#include "kis_types.h"
#include "kis_strategy_colorspace.h"
#include "kis_custom_convolution_filter_configuration_base_widget.h"
#include "kis_matrix_widget.h"

KisCustomConvolutionFilterConfigurationWidget::KisCustomConvolutionFilterConfigurationWidget( KisFilter* nfilter, QWidget * parent, const char * name)
	: KisFilterConfigurationWidget( nfilter, parent, name )
{
	QGridLayout *widgetLayout = new QGridLayout(this, 2, 1);

	QPushButton *bnRefresh = new QPushButton(i18n("Refresh preview"), this, "bnrefresh");
	QSpacerItem *spacer = new QSpacerItem(100, 30, QSizePolicy::Expanding, QSizePolicy::Minimum);

	widgetLayout -> addWidget(bnRefresh, 0, 0);
	widgetLayout -> addItem(spacer, 0, 1);

	QTabWidget* tabWidget = new QTabWidget(this, "tabWidget");
	widgetLayout -> addMultiCellWidget(tabWidget, 1, 1, 0, 1);

	KisImageSP img = filter() -> view() -> currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	KisStrategyColorSpaceSP cs = layer->colorStrategy();
	// Create the form
	vKisChannelInfoSP cis = cs->channels();
	Q_INT32 depth = cs->nColorChannels();
	m_ccfcws = new KisCustomConvolutionFilterConfigurationBaseWidget*[depth];
	m_pos = new int[depth];
	for(Q_INT32 i = 0; i < depth; i++)
	{
		m_pos[i] = cis[i] -> pos();
		m_ccfcws[i] = new KisCustomConvolutionFilterConfigurationBaseWidget((QWidget*)this);
		tabWidget->addTab(m_ccfcws[i], cis[ i ] -> name() );
	}

	connect( bnRefresh, SIGNAL(clicked()), filter(), SLOT(refreshPreview()));
	
}

#include "kis_custom_convolution_filter_configuration_widget.moc"
