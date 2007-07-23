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

#include <gmm/gmm.h>

#include <QFile>
#include <QLocale>
#include <QString>
#include <QTextCodec>
#include <QTextStream>

#include <KComponentData>
#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

#include "KoColorProfile.h"

#include "kis_illuminant_profile.h"

KisIlluminantProfile::KisIlluminantProfile(QString fileName) : KoColorProfile(fileName)
{
	gmm::resize(m_matrix, 1, 1);
	gmm::clear(m_matrix);
	if (!fileName.isEmpty())
		load();
}

bool KisIlluminantProfile::load()
{
	QFile f_ill(fileName());
	float curr;

	if (f_ill.open(QFile::ReadOnly)) {
		QTextStream in_ill(&f_ill);
// 		in_ill.setCodec(QTextCodec::codecForName("ISO 8859-1"));

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
	QString cie = KGlobal::mainComponent().dirs()->locate("kis_illuminants", "CIEXYZ10deg.txt");
	QFile f_xyz(cie);
	QString str_first_wl, str_last_wl, str_proportion, str_curr;
	int cols, first_wl, last_wl;
	float proportion, curr;

	if (f_xyz.open(QFile::ReadOnly)) {
		QTextStream in_xyz(&f_xyz);
// 		in_xyz.setCodec(QTextCodec::codecForName("ISO 8859-1"));

		in_ill >> str_first_wl >> str_last_wl >> str_proportion;
		cols = 10;
		first_wl = str_first_wl.toInt();
		last_wl = str_last_wl.toInt();
		proportion = str_proportion.toFloat();

		float cmf, k, H, Hy_integral = 0, c[3][cols];

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < cols; j++) {
				c[i][j] = 0;
			}
		}

		int i = 0, wavelen;
		for(bool leggi = false; !in_ill.atEnd(); leggi = !leggi) {
			in_ill >> str_curr;
			curr = str_curr.toFloat();
			if (leggi) {
				H = curr;

				if (wavelen >= first_wl) {
					int pos = i / ((last_wl - first_wl)/cols);
					for (int j = 0; j < 4 && !in_xyz.atEnd(); j++) {
						in_xyz >> str_curr;
						curr = str_curr.toFloat();
						if (!j)
							continue;

						cmf = curr;

						if (j == 2)
							Hy_integral += (H*cmf);

						if (pos > cols-1)
							c[j-1][cols-1] += (H*cmf);
						else
							c[j-1][pos] += (H*cmf);
					}
				}
			} else {
				wavelen = (int) curr;
				if (wavelen > last_wl)
					break;
				if (wavelen > first_wl)
					i++;
			}
		}

		k = proportion / Hy_integral;

		gmm::resize(m_matrix, 3, cols);

		for (int i = 0; i < 3; i++)
			for (int j = 0; j < cols; j++)
				m_matrix(i, j) = k*c[i][j];

		setName(fileName());

		return true;
	} else
		kDebug() << "... no XYZ10Deg file found..." << endl;

	return false;
}

bool KisIlluminantProfile::loadMatrix(QTextStream &in_ill)
{
	int cols = 10;
	gmm::resize(m_matrix, 3, cols);
	for (int i = 0; i < 3; i++)
		for (int j = 0; i < cols; j++)
			in_ill >> m_matrix(i, j);

	setName(fileName());

	return true;
}

bool KisIlluminantProfile::save(QString fileName)
{
	QFile f_ill(fileName);
	const int cols = gmm::mat_ncols(m_matrix);

	if (f_ill.open(QFile::WriteOnly | QFile::Truncate)) {
		QTextStream out_ill(&f_ill);

		out_ill << 1 << endl;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < cols - 1; j++)
				out_ill << m_matrix(i, j) << " ";
			out_ill << m_matrix(i, cols) << endl;
		}

		return true;
	}

	return false;
}

bool KisIlluminantProfile::valid() const
{
	if (!gmm::nnz(m_matrix))
		return false;

	return true;
}
