/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *                2004 Adrian Page <adrian@pagenet.plus.com>
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
#include <qlabel.h>
#include <qvbox.h>

#include <kdebug.h>
#include <klocale.h>
#include <kprogress.h>

#include "kis_dlg_progress.h"
#include "kis_progress_subject.h"

KisDlgProgress::KisDlgProgress(KisProgressSubject *subject, QWidget *parent, const char *name) : super(parent, name, true, i18n("Importing Image"), Cancel)
{
	QVBox *page = makeVBoxMainWidget();

	Q_ASSERT(subject);
	m_lbl = new QLabel(page);
	m_lbl -> setText(i18n("Initializing..."));
	m_bar = new KProgress(100, page);
	m_subject = subject;
	connect(subject, SIGNAL(notifyProgress(KisProgressSubject*, int)), this, SLOT(update(KisProgressSubject*, int)));
	connect(subject, SIGNAL(notifyProgressStage(KisProgressSubject*, const QString&, int)), this, SLOT(updateStage(KisProgressSubject*, const QString&, int)));
	connect(subject, SIGNAL(notifyProgressDone(KisProgressSubject*)), this, SLOT(subjectDone(KisProgressSubject*)));
	connect(subject, SIGNAL(notifyProgressError(KisProgressSubject*)), this, SLOT(subjectError(KisProgressSubject*)));
}

KisDlgProgress::~KisDlgProgress()
{
}

void KisDlgProgress::updateStage(KisProgressSubject *, const QString& stage, int percent)
{
	m_lbl -> setText(stage);
	m_bar -> setValue(percent);
}

void KisDlgProgress::update(KisProgressSubject *, int percent)
{
	m_bar -> setValue(percent);
}

void KisDlgProgress::subjectDone(KisProgressSubject *)
{
	accept();
}

void KisDlgProgress::subjectError(KisProgressSubject *)
{
	reject();
}

void KisDlgProgress::slotCancel()
{
	m_subject -> cancel();
	super::slotCancel();
}

#include "kis_dlg_progress.moc"

