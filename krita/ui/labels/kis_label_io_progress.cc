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
#include <qlayout.h>
#include <kprogress.h>
#include "kis_label_io_progress.h"

KisLabelIOProgress::KisLabelIOProgress(QWidget *parent, const char *name, WFlags f) : super(parent, name, f)
{
	QHBoxLayout *box = new QHBoxLayout(this);

	box -> setAutoAdd(true);
	m_bar = new KProgress(100, this);
	m_base = 0;
}

KisLabelIOProgress::~KisLabelIOProgress()
{
}

void KisLabelIOProgress::update(Q_INT8 percent)
{
	m_bar -> setValue(m_base + percent);
}

void KisLabelIOProgress::steps(Q_INT32 nsteps)
{
	if (!isShown())
		show();

	m_bar -> setTotalSteps(nsteps * 100);
	m_bar -> setValue(0);
	m_base = 0;
}

void KisLabelIOProgress::completedStep()
{
	m_base += 100;
}

void KisLabelIOProgress::done()
{
	hide();
}

#include "kis_label_io_progress.moc"

