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

#include "kis_filter.h"
#include "kis_filter_factory.h"
#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_tile_command.h"
#include "kis_undo_adapter.h"

KisFilterConfiguration::KisFilterConfiguration(Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) :
	m_x(x),
	m_y(y),
	m_width(width),
	m_height(height)
{
	
}

KisFilterConfigurationWidget::KisFilterConfigurationWidget(QWidget * parent, const char * name, WFlags f ) :
	QWidget(parent, name, f)
{
}

KisFilterConfiguration* KisFilterConfigurationWidget::config()
{
	return 0;
}

KisFilter::KisFilter(const QString& name) :
	m_name(name)
{
	KisFilterFactory::singleton()->add( this );
}

KisFilterConfiguration* KisFilter::defaultConfiguration()
{
	KisLayerSP lay = KisView::activeView()->currentImg()->activeLayer();
	return new KisFilterConfiguration(0, 0, lay->width(), lay->height() );
}

void KisFilter::refreshPreview(KisFilterConfiguration* )
{
	
}

KisFilterConfigurationWidget* KisFilter::createConfigurationWidget(QWidget* )
{
	return 0;
}

void KisFilter::slotActivated()
{
	KisView* view = KisView::activeView();
	QWidget* configwidget = createConfigurationWidget( view );
	KisFilterConfiguration* config = 0;
	if( configwidget != 0)
	{
		//TODO : the dialog with previewwidget
		delete configwidget;
	}
	if( config == 0 )
	{
		config = defaultConfiguration();
	}
	KisImageSP img = view->currentImg();
	KisPaintDeviceSP device = (KisPaintDeviceSP) img->activeLayer();
	KisTileCommand* ktc = new KisTileCommand(name(), device ); // Create a command
	process(device, config, ktc);
	img->undoAdapter()->addCommand( ktc );
	img->notify();
	delete config;
}
