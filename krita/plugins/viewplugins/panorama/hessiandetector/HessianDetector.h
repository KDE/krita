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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef HESSIANDETECTOR_H_
#define HESSIANDETECTOR_H_
#include "APImage.h"

using namespace std;

#define CONVOLUTION_TYPE int
#define HD_SLIDING_WINDOW 2
#define HD_BOX_FILTERS 1
#define HD_MAX_OCTAVES 3
#define HD_INITIAL_SCALE 1.2

//initial kernel size, representing scale 1.2
#define HD_INIT_KERNEL_SIZE 9

#define PI 3.14159265

class HessianDetector
 {
    public:
    HessianDetector(APImage* i, int nrPoints=1000, CONVOLUTION_TYPE type=HD_BOX_FILTERS, int nrOctaves=1);

    bool detect();

    //clears memory of data structures that are not required anymore(before the description process)
    void dump();
    vector<vector<int> >* getPoints();
    double getMaxima(int x, int y);
    int getNrPoints();

/* private slots:
*/

 	private:
    APImage* image;
    int nrPoints;
    int nrOctaves;
    CONVOLUTION_TYPE convolutionType;
	std::vector<vector<int> > determinants;	//holds the values of the hessian determinant for each pixel
	std::vector<vector<int> > orderedList;    //first n pixels with the largest determinant values
	std::vector<vector<double> > maximas;	//holds the scales where scale-space representation for each pixel attends maximum

	int _getHessianDeterminant(int* pixelSumXX, int* pixelSumXY, int *pixelSumYY);
	void _calculateMaxDet(int i, int j);  //calculates scale-space maxima for pixel at coord i,j
	double _getScale(int kernelSize);
	int _convolutePixel(int* coordX, int* coordY, int* kernelSize);

	bool _boxFilterDetect();
	bool _slidingWDetect();
    void _insertToList(int* x, int* y);
 };

#endif /*HESSIANDETECTOR_H_*/
