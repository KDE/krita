/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QFile>
#include <QString>
#include <QTextStream>

#include <KDebug>

#include "KoColorProfile.h"

#include "kis_illuminant_profile.h"

KisIlluminantProfile::KisIlluminantProfile(QString fileName) : KoColorProfile(fileName)
{
	if (!fileName.isEmpty())
		load();
}

bool KisIlluminantProfile::load()
{
	QFile f_ill(fileName());
	float curr;

	if (f_ill.open(QFile::ReadOnly)) {
		QTextStream in_ill(&f_ill);

		in_ill >> curr;

		switch ((int)curr) {
			case 0:
				return loadCurve(in_ill);
			case 1:
				return loadMatrix(in_ill);
		}
	} else
		kDebug() << "No files found!" << endl;

	return false;
}

bool KisIlluminantProfile::loadCurve(QTextStream &in_ill)
{
	// TODO Reimplement loadCurve() using gaussian quadrature
	return false;
}


bool KisIlluminantProfile::loadMatrix(QTextStream &in_ill)
{
	// TODO Reimplement loadMatrix()
	return false;
}

bool KisIlluminantProfile::save(QString fileName)
{
	// TODO Reimplement save()
	return false;
}

bool KisIlluminantProfile::valid() const
{
	// TODO Reimplement valid()
	return false;
}
