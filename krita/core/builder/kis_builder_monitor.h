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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_BUILDER_MONITOR_H_
#define KIS_BUILDER_MONITOR_H_

#include <qobject.h>
#include "builder/kis_builder_types.h"

class KisBuilderMonitor : public QObject {
	Q_OBJECT
	typedef QObject super;

public:
	KisBuilderMonitor(QObject *parent = 0, const char *name = 0);
	virtual ~KisBuilderMonitor();

public:
	void attach(KisBuilderSubject *subject);
	void detach(KisBuilderSubject *subject);

signals:
	void size(Q_INT32 nSubjects);

private slots:
	void update(KisBuilderSubject *subject, KisImageBuilder_Step step, Q_INT8 percent);
	void destroyed(QObject *o);

private:
	vKisBuilderSubject m_subjects;
};

#endif // KIS_BUILDER_MONITOR_H_

