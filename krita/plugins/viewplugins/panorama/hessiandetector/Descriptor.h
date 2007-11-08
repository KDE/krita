/***************************************************************************
 *   Copyright (C) 2007 by Zoran Mesec   *
 *   zoran.mesec@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#ifndef DESCRIPTOR_H_INCLUDED
#define DESCRIPTOR_H_INCLUDED
#include <string>
#include "APImage.h"
#include "HessianDetector.h"

#define NR_ANGLE_BINS 8
#define DESCRIPTOR_SIZE 96

class Descriptor
 {
    public:
    Descriptor(APImage* i, HessianDetector* hd);
    double _gaussWeighting(int x, int y, double stdev);



    void setPoints(vector<vector<int> >* pts);
    void orientate();
    void createDescriptors();
    vector<vector<double> >* getDescriptors();
    vector<vector<int> >* interestPoints;
    vector<vector<double> > descriptors;

    double _getMaxima(int x,int y);

 	private:
    APImage* image;
    HessianDetector* hd;

    double _euclidianDistance(int x1, int y1, int x2, int y2);
    //double _initDescriptor(double* desciptor);
    void _GaborResponse(int x,int y,double maxima,double* descriptor);

    vector<vector<int> >* maximas;
    int orientations[];
 };
#endif // DESCRIPTOR_H_INCLUDED
