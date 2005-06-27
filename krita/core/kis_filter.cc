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
#include <qframe.h>
#include <qcursor.h>

#include "kis_cursor.h"
#include "kis_filter_registry.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_previewdialog.h"
#include "kis_previewwidget.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_id.h"
#include "kis_canvas_subject.h"
#include "kis_doc.h"
#include <kis_progress_display_interface.h>

KisFilter::KisFilter(const KisID& id, KisView * view) :
	m_id(id),
	m_view(view),
	m_dialog(0)
{
}

KisFilterConfiguration* KisFilter::configuration(QWidget*)
{
	return 0;
}

void KisFilter::refreshPreview( )
{
	if( m_dialog == 0 )
		return;
	m_dialog -> previewWidget() -> slotRenewLayer();
	
	KisLayerSP layer = m_dialog -> previewWidget() -> getLayer();
	KisFilterConfiguration* config = configuration(m_widget);
	QRect rect = layer -> extent();
	process((KisPaintDeviceSP) layer, (KisPaintDeviceSP) layer, config, rect);
	m_dialog->previewWidget() -> slotUpdate();
}

QWidget* KisFilter::createConfigurationWidget(QWidget* )
{
	return 0;
}

void KisFilter::slotActivated()
{
	kdDebug(DBG_AREA_FILTERS) << "Filter activated: " << m_id.name() << "\n";
	KisImageSP img = m_view -> currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	disableProgress();

	// Create the config dialog
	m_dialog = new KisPreviewDialog( (QWidget*) m_view, id().name().ascii(), true, id().name());
	Q_CHECK_PTR(m_dialog);

	m_widget = createConfigurationWidget( (QWidget*)m_dialog->container() );

	if( m_widget != 0)
	{
		m_dialog->previewWidget()->slotSetLayer( layer );
		connect(m_dialog->previewWidget(), SIGNAL(updated()), this, SLOT(refreshPreview()));
		QGridLayout *widgetLayout = new QGridLayout((QWidget *)m_dialog->container(), 1, 1);
		widgetLayout -> addWidget(m_widget, 0 , 0);
		m_dialog->container()->setMinimumSize(m_widget->minimumSize());
		refreshPreview();
		if(m_dialog->exec() == QDialog::Rejected )
		{
			delete m_dialog;
			return;
		}
	}
	else
	{
		delete m_dialog;
		m_dialog = 0;
	}

	QCursor oldCursor = m_view -> cursor();
	m_view -> setCursor(KisCursor::waitCursor());

	//Apply the filter
	KisFilterConfiguration* config = configuration(m_widget);

	QRect r1 = layer -> extent();
	QRect r2 = img -> bounds();

	// Filters should work only on the visible part of an image.
	QRect rect = r1.intersect(r2);

	if (layer->hasSelection()) {
		QRect r3 = layer -> selection() -> selectedRect();
		rect = rect.intersect(r3);
	}

	enableProgress();

	KisTransaction * cmd = new KisTransaction(id().name(), layer.data());
	Q_CHECK_PTR(cmd);
	process((KisPaintDeviceSP)layer, (KisPaintDeviceSP)layer, config, rect);

	if (m_cancelRequested) {
		cmd -> unexecute();
		delete cmd;
	} else {
		img -> undoAdapter() -> addCommand(cmd);
		// Yuck, filters should work against the canvassubject interface, not the view object.
		// code against interfaces, not implementations!
		dynamic_cast<KisCanvasSubject*>(m_view) -> document() -> setModified(true);
		img->notify();
	}

	disableProgress();
	
	m_view -> setCursor(oldCursor);

	delete m_dialog;
	m_dialog = 0;
	delete config;
}

void KisFilter::enableProgress() {
	m_progressEnabled = true;
	m_cancelRequested = false;
}

void KisFilter::disableProgress() {
	m_progressEnabled = false;
	m_cancelRequested = false;
}

void KisFilter::setProgressTotalSteps(Q_INT32 totalSteps)
{
	if (m_progressEnabled) {

		m_progressTotalSteps = totalSteps;
		m_lastProgressPerCent = 0;

		KisProgressDisplayInterface *progress = view() -> progressDisplay();

		progress -> setSubject(this, true, true);
		emit notifyProgress(this, 0);
	}
}

void KisFilter::setProgress(Q_INT32 progress)
{
	if (m_progressEnabled) {

		Q_INT32 progressPerCent = (progress * 100) / m_progressTotalSteps;

		if (progressPerCent != m_lastProgressPerCent) {

			m_lastProgressPerCent = progressPerCent;
			emit notifyProgress(this, progressPerCent);
		}
	}
}

void KisFilter::setProgressStage(const QString& stage, Q_INT32 progress)
{
	if (m_progressEnabled) {

		Q_INT32 progressPerCent = (progress * 100) / m_progressTotalSteps;

		m_lastProgressPerCent = progressPerCent;
		emit notifyProgressStage(this, stage, progressPerCent);
	}
}

void KisFilter::setProgressDone()
{
	if (m_progressEnabled) {
		emit notifyProgressDone(this);
	}
}

void KisFilter::setAutoUpdate(bool set) {
	m_dialog -> previewWidget() -> slotSetAutoUpdate(set);
}


#include "kis_filter.moc"
