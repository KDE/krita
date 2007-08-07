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

#include <cstdlib>
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
	QString curr;

	if (f_ill.open(QFile::ReadOnly)) {
		QTextStream in_ill(&f_ill);
		m_matrix = new double*[3];
		for (int i = 0; i < 3; i++) {
			m_matrix[i] = new double[10];
			for (int j = 0; j < 10; j++) {
				in_ill >> curr;
				m_matrix[i][j] = curr.toFloat();
				kDebug() << m_matrix[i][j];
			}
		}
	} else
		kDebug() <<"No files found!";

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
