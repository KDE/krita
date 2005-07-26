/*
 *  kis_previewwidget.cc - part of Krita
 *
 *  Copyright (c) 2001 John Califf  <jwcaliff@compuzone.net>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#include <qpainter.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcolor.h>

#include <kdebug.h>

#include "kis_undo_adapter.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "color_strategy/kis_strategy_colorspace.h"

#include "kis_previewwidgetbase.h"
#include "kis_previewwidget.h"
#include "kis_previewview.h"

KisPreviewWidget::KisPreviewWidget( QWidget* parent, const char* name )
	: PreviewWidgetBase( parent, name )
{
	m_autoupdate = true;
	connect(m_preview, SIGNAL(updated()), this, SLOT(redirectUpdated()));
}

void KisPreviewWidget::redirectUpdated() {
	if (m_autoupdate)
		emit updated();
}

void KisPreviewWidget::slotSetLayer(KisLayerSP lay)
{
	m_original->setSourceLayer(lay);
	m_preview->setSourceLayer(lay);
}

void KisPreviewWidget::slotRenewLayer() {
	m_preview->updateView();
}

KisLayerSP KisPreviewWidget::getLayer()
{
	return m_preview->getPreviewLayer();
}

void KisPreviewWidget::slotUpdate()
{
	m_preview->updatedPreview();
}

void KisPreviewWidget::slotSetAutoUpdate(bool set) {
	m_autoupdate = set;
}

double KisPreviewWidget::getZoom()
{
	return m_preview->getZoom();
}

QPoint KisPreviewWidget::getPos()
{
	return m_preview->getPos();
}

bool KisPreviewWidget::getAutoUpdate() {
	return m_autoupdate;
}
#include "kis_previewwidget.moc"
