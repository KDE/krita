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

#ifndef _IMAGE_MATCH_MODEL_P_H_
#define _IMAGE_MATCH_MODEL_P_H_

class ImageMatchModel {
  public:
    inline ImageMatchModel(std::vector<KisMatch> samples, void*) : m_isValid(false), m_fitComputed(false), m_matches(samples)
    {
    }
    inline static uint nbFit() {
        return 2;
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
      Eigen::Vector3d v;
      v[0] = m.match->x();
      v[1] = m.match->y();
      v[2] = 1.0;
      Eigen::Vector3d expectedMatchPosition = transfo() * v;
      return sqrtf (powf (m.ref->x() - expectedMatchPosition[0], 2.0) + powf(m.ref->y() - expectedMatchPosition[1], 2.0));
    }
    inline const Eigen::Matrix3d& transfo() const {
      return m_transfo;
    }
    inline const std::vector<KisMatch>& matches() const {
      return m_matches;
    }
  private:
    void computeFitting()
    {
      const KisMatch& m1 = m_matches[0];
      const KisMatch& m2 = m_matches[1];

      double diff_ri = m2.ref->x() - m1.ref->x();
      double diff_rj = m2.ref->y() - m1.ref->y();
      double diff_mi = m2.match->x() - m1.match->x();
      double diff_mj = m2.match->y() - m1.match->y();
      m_angle = atan2 (diff_rj, diff_ri) - atan2 (diff_mj, diff_mi);
      double s = sin (m_angle);
      double c = cos (m_angle);

      double n = sqrt(diff_mi * diff_mi + diff_mj * diff_mj);
      if( n != 0.0)
      {
        n = sqrt(diff_ri * diff_ri + diff_rj * diff_rj) / n;
        m_transfo(0, 0) = n * c;
        m_transfo(0, 1) = n * -s;
        m_transfo(0, 2) = n * (c * (-m1.match->x()) - s * (-m1.match->y())) + m1.ref->x();
        m_transfo(1, 0) = n * s;
        m_transfo(1, 1) = n * c;
        m_transfo(1, 2) = n * (s * (-m1.match->x()) + c * (-m1.match->y())) + m1.ref->y();
        m_transfo(2, 0) = 0.0;
        m_transfo(2, 1) = 0.0;
        m_transfo(2, 2) = 1.0;
        m_fittingErrorSum = 0.0;
        if( m_matches.size() > 2 )
        {
          for(std::vector<KisMatch>::iterator it = m_matches.begin(); it != m_matches.end(); it++)
          {
            m_fittingErrorSum += fittingError(*it) /* * (1.0 - it->strength )*/;
          }
          m_fittingErrorSum /= m_matches.size();
//           m_fittingErrorSum *= (1.0 - m1.strength) * (1.0 - m2.strength);
        }
        m_isValid = true;
      } else {
        m_isValid = false;
      }
      m_fitComputed = true;
//       kdDebug() << m_angle << " " << m_transfo(0,2) << " " << m_transfo(1,2) << " " << fittingError(m1) << " " << fittingError(m2) << endl;
    }
  private:
    bool m_isValid;
    bool m_fitComputed;
    double m_fittingErrorSum;
    double m_angle;
    std::vector<KisMatch> m_matches;
    Eigen::Matrix3d m_transfo;
};

#endif
