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
#if !defined KIS_DLG_BUILDER_PROGRESS_H_
#define KIS_DLG_BUILDER_PROGRESS_H_

#include <kdialogbase.h>
#include "builder/kis_builder_types.h"

class QLabel;
class KProgress;
class KisBuilderSubject;

class KisDlgBuilderProgress : public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:
	KisDlgBuilderProgress(KisBuilderSubject *subject, QWidget *parent = 0, const char *name = 0);
	virtual ~KisDlgBuilderProgress();

public slots:
	virtual void update(KisBuilderSubject *subject, KisImageBuilder_Step step, Q_INT8 percent);

protected slots:
	virtual void slotCancel();

private:
	QLabel *m_lbl;
	KProgress *m_bar;
	KisBuilderSubject *m_subject;
};

#endif // KIS_DLG_BUILDER_PROGRESS_H_

