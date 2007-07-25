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

#ifndef KIS_ILLUMINANT_PROFILE_H_
#define KIS_ILLUMINANT_PROFILE_H_

#include <gmm/gmm.h>

#include <QString>

#include "KoColorProfile.h"

#define SAMPLE_NUMBER 55

class QTextStream;

class KisIlluminantProfile : public KoColorProfile {
	public:
		KisIlluminantProfile(QString fileName = "");
		~KisIlluminantProfile() {return;}

		bool load();

		bool save(QString fileName);

		bool valid() const;

		bool isSuitableForOutput() const {return false;}

		bool isSuitableForPrinting() const {return false;}

		bool isSuitableForDisplay() const {return false;}

		const gmm::dense_matrix<float> &matrix() const {return m_matrix;}

	private:
		bool loadCurve(QTextStream &in_ill);
		bool loadMatrix(QTextStream &in_ill);

	private:

		gmm::dense_matrix<float> m_matrix;
};


#endif // KIS_ILLUMINANT_PROFILE_H_
