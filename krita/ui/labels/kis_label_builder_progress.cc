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
#include <qlayout.h>
#include <qtooltip.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kprogress.h>
#include "builder/kis_builder_subject.h"
#include "kis_label_builder_progress.h"

KisLabelBuilderProgress::KisLabelBuilderProgress(QWidget *parent, const char *name, WFlags f) : super(parent, name, f)
{
	QHBoxLayout *box = new QHBoxLayout(this);
	KPushButton *btn;

	m_subject = 0;
	box -> setAutoAdd(true);
	m_bar = new KProgress(100, this);
	btn = new KPushButton("X", this);
	QToolTip::add(btn, i18n("Interrupt"));
	connect(btn, SIGNAL(pressed()), this, SLOT(cancelPressed()));
}

KisLabelBuilderProgress::~KisLabelBuilderProgress()
{
}

void KisLabelBuilderProgress::changeSubject(KisBuilderSubject *subject)
{
	if (!subject)
		hide();

	if (m_subject)
		m_subject -> disconnect(this);

	m_subject = subject;

	if (m_subject && subject) {
		connect(subject, SIGNAL(notify(KisBuilderSubject*, KisImageBuilder_Step, Q_INT8)), this, SLOT(update(KisBuilderSubject*, KisImageBuilder_Step, Q_INT8)));
		connect(subject, SIGNAL(destroyed()), this, SLOT(subjectDestroyed()));
		show();
	}

	m_bar -> setValue(0);
}

void KisLabelBuilderProgress::update(KisBuilderSubject *, KisImageBuilder_Step step, Q_INT8 percent)
{
	if (step == KisImageBuilder_STEP_DONE || step == KisImageBuilder_STEP_ERROR)
		changeSubject(0);

	if (percent == 100 && (step == KisImageBuilder_STEP_SAVING || step == KisImageBuilder_STEP_TILING))
		changeSubject(0);

	m_bar -> setValue(percent);
}

void KisLabelBuilderProgress::cancelPressed()
{
	if (m_subject) {
		m_subject -> intr();
		changeSubject(0);
	}
}

void KisLabelBuilderProgress::subjectDestroyed()
{
	changeSubject(0);
}

#include "kis_label_builder_progress.moc"

