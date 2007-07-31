/*
 * Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _HOMOGRAPHY_IMAGE_MATCH_MODEL_P_H_
#define _HOMOGRAPHY_IMAGE_MATCH_MODEL_P_H_

struct HomographyImageMatchModelStaticParams {
    inline HomographyImageMatchModelStaticParams(int w, int h)
        : width(w), height(h)
    {}
    int width, height;
};

class HomographyImageMatchModel {
    public:
        inline HomographyImageMatchModel(std::vector<KisMatch> samples, HomographyImageMatchModelStaticParams* params) : m_isValid(false), m_fitComputed(false), m_matches(samples), m_width(params->width), m_height(params->height)
        {
        }
        /**
         * @return the minimal number of points needed for a fit
         */
        inline static uint nbFit() {
            return 6;
        }
        inline bool isValid() {
            if(not m_fitComputed)
            {
                computeFitting();
            }
            return m_isValid;
        }
        inline void addData(std::vector<KisMatch>::iterator begin, std::vector<KisMatch>::iterator end)
        {
        for(std::vector<KisMatch>::iterator it = begin; it != end; it++)
        {
            m_matches.push_back(*it);
        }
        m_fitComputed = false;
        }
        inline double fittingErrorSum() {
        if(not m_fitComputed)
        {
            computeFitting();
        }
        return m_fittingErrorSum;
        }
        inline double threshold() const {
            return 20.0;
        }
        inline double fittingError(const KisMatch& m)
        {
            int indx[HomographySameDistortionFunction::SIZEINDEXES];
            for(int i = 0; i < HomographySameDistortionFunction::SIZEINDEXES; i++)
            {
                indx[i] = i;
            }
            double norm(4.0 / ( m_width * m_width + m_height * m_height ) );
            HomographySameDistortionFunction hsdf(indx, m_width * 0.5, m_height * 0.5, norm, m.ref->x(), m.ref->y(), m.match->x(), m.match->y());
            double f1, f2;
            hsdf.f(parameters(), f1, f2);
            return (f1 + f2) * 0.5;
        }
        inline const std::vector<double>& parameters() const {
            return m_parameters;
        }
        inline const std::vector<KisMatch>& matches() const {
            return m_matches;
        }
    private:
        void computeFitting()
        {
            PanoptimFunction<HomographySameDistortionFunction, HomographySameDistortionFunction::SIZEINDEXES> f( m_matches, m_width * 0.5, m_height * 0.5, m_width, m_height );
            m_parameters.resize(HomographySameDistortionFunction::SIZEINDEXES);
            m_parameters[HomographySameDistortionFunction::INDX_a] = 0.0;
            m_parameters[HomographySameDistortionFunction::INDX_b] = 0.0;
            m_parameters[HomographySameDistortionFunction::INDX_c] = 0.0;
            m_parameters[HomographySameDistortionFunction::INDX_h11] = 1.0;
            m_parameters[HomographySameDistortionFunction::INDX_h21] = 0.0;
            m_parameters[HomographySameDistortionFunction::INDX_h31] = m_matches[0].ref->x() - m_matches[0].match->x();
            m_parameters[HomographySameDistortionFunction::INDX_h12] = 0.0;
            m_parameters[HomographySameDistortionFunction::INDX_h22] = 1.0;
            m_parameters[HomographySameDistortionFunction::INDX_h32] = m_matches[0].ref->y() - m_matches[0].match->y();
            m_parameters[HomographySameDistortionFunction::INDX_h13] = 0.0;
            m_parameters[HomographySameDistortionFunction::INDX_h23] = 0.0;
            m_fittingErrorSum = Optimization::Algorithms::levenbergMarquardt(&f, m_parameters, 100, 1e-12, 0.01, 10.0);
            m_isValid = true;
            m_fitComputed = true;
            for(uint i = 0; i < m_parameters.size(); i++)
            {
                kDebug(41006) <<"m_parameters["<< i <<"]=" << m_parameters[i];
            }
        }
    private:
        bool m_isValid;
        bool m_fitComputed;
        double m_fittingErrorSum;
        std::vector<KisMatch> m_matches;
        std::vector<double> m_parameters;
        int m_width, m_height;
};

#endif
