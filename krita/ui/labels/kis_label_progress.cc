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
#include <qlayout.h>
#include <qtooltip.h>

#include <klocale.h>
#include <kpushbutton.h>
#include <kprogress.h>

#include "kis_progress_subject.h"
#include "kis_label_progress.h"

KisLabelProgress::KisLabelProgress(QWidget *parent, const char *name, WFlags f) : super(parent, name, f)
{
	QHBoxLayout *box = new QHBoxLayout(this);
	KPushButton *btn;

	m_subject = 0;
	box -> setAutoAdd(true);
	m_bar = new KProgress(100, this);
	btn = new KPushButton("X", this);
	QToolTip::add(btn, i18n("Cancel"));
	connect(btn, SIGNAL(pressed()), this, SLOT(cancelPressed()));
}

KisLabelProgress::~KisLabelProgress()
{
}

void KisLabelProgress::changeSubject(KisProgressSubject *subject)
{
	if (!subject)
		hide();

	if (m_subject)
		m_subject -> disconnect(this);

	m_subject = subject;

	if (m_subject && subject) {
		connect(subject, SIGNAL(notifyProgress(KisProgressSubject*, int)), this, SLOT(update(KisProgressSubject*, int)));
		connect(subject, SIGNAL(notifyProgressStage(KisProgressSubject*, const QString&, int)), this, SLOT(updateStage(KisProgressSubject*, const QString&, int)));
		connect(subject, SIGNAL(notifyProgressDone(KisProgressSubject*)), this, SLOT(done(KisProgressSubject*)));
		connect(subject, SIGNAL(notifyProgressError(KisProgressSubject*)), this, SLOT(error(KisProgressSubject*)));
		connect(subject, SIGNAL(destroyed()), this, SLOT(subjectDestroyed()));
		show();
	}

	m_bar -> setValue(0);
}

void KisLabelProgress::update(KisProgressSubject *, int percent)
{
	m_bar -> setValue(percent);
}

void KisLabelProgress::updateStage(KisProgressSubject *, const QString&, int percent)
{
	m_bar -> setValue(percent);
}

void KisLabelProgress::cancelPressed()
{
	if (m_subject) {
		m_subject -> cancel();
		changeSubject(0);
	}
}

void KisLabelProgress::subjectDestroyed()
{
	changeSubject(0);
}

void KisLabelProgress::done(KisProgressSubject *)
{
	changeSubject(0);
}

void KisLabelProgress::error(KisProgressSubject *)
{
	changeSubject(0);
}

#include "kis_label_progress.moc"

