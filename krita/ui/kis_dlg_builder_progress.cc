/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <qlabel.h>
#include <qvbox.h>
#include <klocale.h>
#include <kprogress.h>
#include "kis_dlg_builder_progress.h"
#include "builder/kis_builder_subject.h"

KisDlgBuilderProgress::KisDlgBuilderProgress(KisBuilderSubject *subject, QWidget *parent, const char *name) : super(parent, name, true, i18n("Importing Image"), Cancel)
{
	QVBox *page = makeVBoxMainWidget();

	Q_ASSERT(subject);
	m_lbl = new QLabel(page);
	m_lbl -> setText(i18n("Initializing..."));
	m_bar = new KProgress(100, page);
	m_subject = subject;
	connect(subject, SIGNAL(notify(KisBuilderSubject*, KisImageBuilder_Step, Q_INT8)), this, SLOT(update(KisBuilderSubject*, KisImageBuilder_Step, Q_INT8)));
}

KisDlgBuilderProgress::~KisDlgBuilderProgress()
{
}

void KisDlgBuilderProgress::update(KisBuilderSubject *subject, KisImageBuilder_Step step, Q_INT8 percent)
{
	switch (step) {
	case KisImageBuilder_STEP_PREP:
		m_lbl -> setText(i18n("Initializing..."));
		break;
	case KisImageBuilder_STEP_LOADING:
		m_lbl -> setText(i18n("Loading..."));
		break;
	case KisImageBuilder_STEP_DECODING:
		m_lbl -> setText(i18n("Decoding..."));
		break;
	case KisImageBuilder_STEP_TILING:
		m_lbl -> setText(i18n("Importing..."));

		if (percent == 100) {
			m_bar -> setValue(0);
			accept();
		}

		break;
	case KisImageBuilder_STEP_DONE:
		accept();
		break;
	case KisImageBuilder_STEP_ERROR:
		if (subject == m_subject)
			subject -> intr();

		reject();
		break;
	default:
		break;
	}

	m_bar -> setValue(percent);
}

void KisDlgBuilderProgress::slotCancel()
{
	m_subject -> intr();
	super::slotCancel();
}

#include "kis_dlg_builder_progress.moc"

