/*
 *
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_hessian_detector.h"

#include "APImage.h"
#include "Descriptor.h"
#include "HessianDetector.h"

#include "kis_paint_device.h"

class HessianPoint : public KisInterestPoint {
    public:
        HessianPoint(double x, double y, int scale, std::vector<double> descriptor) : KisInterestPoint(x,y)
        {
        }
        virtual double fastCompare(const KisInterestPoint* ip) const
        {
            return compare(ip);
        }
        virtual double compare(const KisInterestPoint* ip) const
        {
            const HessianPoint* hp = dynamic_cast<const HessianPoint*>(ip);
            if(not hp or hp->m_descriptor.size() != m_descriptor.size())
            {
                kError() << "not comparable";
                return 0.0;
            }
            double d = 0.0;
            for(int i = 0; i < m_descriptor.size(); i++)
            {
                double diff = m_descriptor[i] - hp->m_descriptor[i];
                d += diff * diff;
            }
            return -d;
        }
        virtual QString toString() const
        {
            return "";
        }
    private:
        int m_scale;
        std::vector<double> m_descriptor;
};

lInterestPoints KisHessianDetector::computeInterestPoints(KisPaintDeviceSP device, const QRect& area)
{
    APImage im1(device, area);
    im1.integrate();
    
    HessianDetector hd1(&im1, 4000, HD_BOX_FILTERS,1);
    if(!hd1.detect()) {
        kError() << "Detection of points failed!";
        return lInterestPoints();
    }
    Descriptor d1(&im1,&hd1);
    d1.setPoints(hd1.getPoints());
    d1.createDescriptors();
    lInterestPoints ips;
    vector<vector<int> >::iterator iter1 = d1.interestPoints->begin();
    vector<vector<double> >::iterator iterDesc = d1.descriptors.begin();
    while( iter1 != d1.interestPoints->end()) {
         vector<int > tmp2 = *iter1;
         ips.push_back(new HessianPoint( tmp2[1], tmp2[0], d1._getMaxima(tmp2[0], tmp2[1]), *iterDesc ) );
         iter1++;
         iterDesc++;
    }
    return ips;
}
