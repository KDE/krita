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
#if !defined KIS_LABEL_IO_PROGRESS_H_
#define KIS_LABEL_IO_PROGRESS_H_

#include <qlabel.h>

class KProgress;

class KisLabelIOProgress : public QLabel {
	Q_OBJECT
	typedef QLabel super;

public:
	KisLabelIOProgress(QWidget *parent, const char *name = 0, WFlags f = 0);
	virtual ~KisLabelIOProgress();

public slots:
	void update(Q_INT8 percent);
	void steps(Q_INT32 nsteps);
	void completedStep();
	void done();

private:
	KProgress *m_bar;
	Q_INT32 m_base;
};

#endif // KIS_LABEL_IO_PROGRESS_H_

