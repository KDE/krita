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
#include <qtl.h>
#include <kapplication.h>
#include <kdebug.h>
#include "kis_global.h"
#include "kis_builder_monitor.h"
#include "kis_builder_subject.h"

KisBuilderMonitor::KisBuilderMonitor(QObject *parent, const char *name) : super(parent, name)
{
}

KisBuilderMonitor::~KisBuilderMonitor()
{
}

void KisBuilderMonitor::attach(KisBuilderSubject *subject)
{
	vKisBuilderSubject_it it = m_subjects.find(subject);

	if (it == m_subjects.end() && subject) {
		m_subjects.push_back(subject);
		connect(subject, SIGNAL(notify(KisBuilderSubject*, KisImageBuilder_Step, Q_INT8)), this, SLOT(update(KisBuilderSubject*, KisImageBuilder_Step, Q_INT8)));
		connect(subject, SIGNAL(destroyed(QObject*)), this, SLOT(destroyed(QObject*)));
		emit size(m_subjects.size());
	} else {
		kdDebug(DBG_AREA_CORE) << "Attaching already attached subject\n";
	}
}

void KisBuilderMonitor::detach(KisBuilderSubject *subject)
{
	vKisBuilderSubject_it it = m_subjects.find(subject);

	if (it != m_subjects.end()) {
		subject -> disconnect();
		m_subjects.erase(it);
		emit size(m_subjects.size());
	} else {
		kdDebug(DBG_AREA_CORE) << "Detaching unattached subject.\n";
	}
}

void KisBuilderMonitor::update(KisBuilderSubject *subject, KisImageBuilder_Step step, Q_INT8)
{
	KApplication *app = KApplication::kApplication();

	Q_ASSERT(app);

	if (app -> hasPendingEvents())
		app -> processEvents();

	if (step == KisImageBuilder_STEP_DONE || step == KisImageBuilder_STEP_ERROR)
		detach(subject);
}

void KisBuilderMonitor::destroyed(QObject *o)
{
	KisBuilderSubject *subject = static_cast<KisBuilderSubject*>(o);

	m_subjects.remove(subject);
	emit size(m_subjects.size());
}

#include "kis_builder_monitor.moc"

