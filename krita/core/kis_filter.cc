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

#include <qlayout.h>

#include "kis_filter_registry.h"
#include "kis_tile_command.h"
#include "kis_undo_adapter.h"
#include "kis_filter_configuration_widget.h"
#include "kis_preview_dialog.h"
#include "kis_previewwidget.h"

KisFilter::KisFilter(const QString& name) :
	m_name(name)
{
	KisFilterRegistry::singleton()->add( this );
}

KisFilterConfiguration* KisFilter::configuration()
{
	return 0;
}

void KisFilter::refreshPreview( )
{
	m_dialog->previewWidget->slotRenewLayer();
	KisLayerSP layer = m_dialog->previewWidget->getLayer();
	KisFilterConfiguration* config = configuration();
	QRect rect(0, 0, layer->width(), layer->height());
	process((KisPaintDeviceSP) layer, config, rect, 0 );
	m_dialog->previewWidget->slotUpdate();
}

KisFilterConfigurationWidget* KisFilter::createConfigurationWidget(QWidget* )
{
	return 0;
}

void KisFilter::slotActivated()
{
	KisView* view = KisView::activeView();
	KisImageSP img = view->currentImg();
	KisLayerSP layer = img->activeLayer();
	// Create the config dialog
	m_dialog = new KisPreviewDialog( (QWidget*)view, name().ascii(), true);
	m_widget = createConfigurationWidget( (QWidget*)m_dialog->container );
	if( m_widget != 0)
	{
		m_dialog->previewWidget->slotSetPreview( layer );
		QVBoxLayout* layout = new QVBoxLayout((QWidget *)m_dialog->container);
		layout->addWidget( m_widget );
		refreshPreview();
		if(m_dialog->exec() == QDialog::Rejected )
		{
			delete m_dialog;
			return;
		}
	}
	//Apply the filter
	KisFilterConfiguration* config = configuration();
	KisTileCommand* ktc = new KisTileCommand(name(), (KisPaintDeviceSP)layer ); // Create a command
	QRect rect(0, 0, layer->width(), layer->height());
	process((KisPaintDeviceSP)layer, config, rect, ktc);
	img->undoAdapter()->addCommand( ktc );
	img->notify();
/*	delete m_dialog;
	delete config;*/
}
#include "kis_filter.moc"
