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
	m_matrix = m_fromD50 = m_toD50 = 0;
	if (!fileName.isEmpty())
		load();
}

KisIlluminantProfile::~KisIlluminantProfile()
{
	if (m_matrix) {
		for (int i = 0; i < 3; i++) {
			delete [] m_matrix[i];
			delete [] m_fromD50[i];
			delete [] m_toD50[i];
		}
		delete [] m_matrix;
		delete [] m_fromD50;
		delete [] m_toD50;
	}
}

bool KisIlluminantProfile::load()
{
	QFile f_ill(fileName());
	QString curr;

	if (f_ill.open(QFile::ReadOnly)) {
		QTextStream in_ill(&f_ill);

		curr = in_ill.readLine();
		setName(curr);

		m_matrix = new double*[3];
		for (int i = 0; i < 3; i++) {
			m_matrix[i] = new double[WLS_NUMBER];
			for (int j = 0; j < WLS_NUMBER; j++) {
				in_ill >> curr;
				m_matrix[i][j] = curr.toFloat();
			}
		}

		m_fromD50 = new double*[3];
		for (int i = 0; i < 3; i++) {
			m_fromD50[i] = new double[3];
			for (int j = 0; j < 3; j++) {
				in_ill >> curr;
				m_fromD50[i][j] = curr.toFloat();
			}
		}

		m_toD50 = new double*[3];
		for (int i = 0; i < 3; i++) {
			m_toD50[i] = new double[3];
			for (int j = 0; j < 3; j++) {
				in_ill >> curr;
				m_toD50[i][j] = curr.toFloat();
			}
		}

	} else {
		kDebug() <<"No files found!";
		return false;
	}

	return true;
}

bool KisIlluminantProfile::save(QString /*fileName*/)
{
	// TODO Reimplement save()
	return false;
}

bool KisIlluminantProfile::valid() const
{
	if (m_matrix)
		return true;
	else
		return false;
}
