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
#if !defined KIS_DLG_PROGRESS_H_
#define KIS_DLG_PROGRESS_H_

#include <kdialogbase.h>

class QLabel;
class KProgress;
class KisProgressSubject;

class KisDlgProgress : public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:
	KisDlgProgress(KisProgressSubject *subject, QWidget *parent = 0, const char *name = 0);
	virtual ~KisDlgProgress();

public slots:
	virtual void update(KisProgressSubject *subject, int percent);
	virtual void updateStage(KisProgressSubject *subject, const QString& stage, int percent);
	virtual void subjectDone(KisProgressSubject *subject);
	virtual void subjectError(KisProgressSubject *);

protected slots:
	virtual void slotCancel();

private:
	QLabel *m_lbl;
	KProgress *m_bar;
	KisProgressSubject *m_subject;
};

#endif // KIS_DLG_PROGRESS_H_

