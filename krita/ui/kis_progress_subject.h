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
#if !defined KIS_PROGRESS_SUBJECT_H_
#define KIS_PROGRESS_SUBJECT_H_

#include <qobject.h>

class KisProgressSubject : public QObject {
	Q_OBJECT

protected:
	virtual ~KisProgressSubject();

public:
	virtual void cancel() = 0;

signals:
	void notifyProgress(KisProgressSubject *subject, int percent);
	void notifyProgressStage(KisProgressSubject *subject, const QString& stage, int percent);
	void notifyProgressDone(KisProgressSubject *subject);
	void notifyProgressError(KisProgressSubject *subject);
};

#endif // KIS_PROGRESS_SUBJECT_H_

