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
#if !defined KIS_LABEL_BUILDER_PROGRESS_H_
#define KIS_LABEL_BUILDER_PROGRESS_H_

#include <qlabel.h>
#include "builder/kis_builder_types.h"

class KProgress;
class KisBuilderSubject;

class KisLabelBuilderProgress : public QLabel {
	Q_OBJECT
	typedef QLabel super;

public:
	KisLabelBuilderProgress(QWidget *parent, const char *name = 0, WFlags f = 0);
	virtual ~KisLabelBuilderProgress();

public:
	void changeSubject(KisBuilderSubject *subject);

public slots:
	virtual void update(KisBuilderSubject *subject, KisImageBuilder_Step step, Q_INT8 percent);

private slots:
	void cancelPressed();
	void subjectDestroyed();

private:
	KisBuilderSubject *m_subject;
	KProgress *m_bar;
};

#endif // KIS_LABEL_BUILDER_PROGRESS_H_

