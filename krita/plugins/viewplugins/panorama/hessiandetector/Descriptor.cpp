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

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Descriptor.h"

using namespace std;

Descriptor::Descriptor(APImage* i,HessianDetector* hessianDetector) {
    this->image=i;
    this->hd=hessianDetector;
}

void Descriptor::setPoints(vector<vector<int> >* pts) {
    this->interestPoints=pts;
}
void Descriptor::orientate() {
    vector<vector<int> >::iterator iter1 = this->interestPoints->begin();
    double regionSize;
    int kernelSize;

    //int angle;
    int max;
    int orientation;
    double step;

    //coordinates of the sample
    int pointX;
    int pointY;

/*
S0: if dx>0 and dy>0 and |dy|>|dx|
S1: if dx>0 and dy>0 and |dy|<=|dx|
S2: if dx>0 and dy<=0 and |dy|<=|dx|
S3: if dx>0 and dy<=0 and |dy|>|dx|
S4: if dx<=0 and dy<=0 and |dy|>|dx|
S5: if dx<=0 and dy<=0 and |dy|<=|dx|
S6: if dx<=0 and dy>0 and |dy|<=|dx|
S7: if dx<=0 and dy>0 and |dy|>|dx|
 */
    double Sx[NR_ANGLE_BINS];
    double Sy[NR_ANGLE_BINS];



    double dx;
    double dy;

    int pointCount=0;

     while( iter1 != this->interestPoints->end()) { //loop over every interest point
         vector<int > interestPoint=*iter1;
        //cout << interestPoint[0]<<","<<interestPoint[1]<<":";
        regionSize=_getMaxima(interestPoint[0], interestPoint[1])*3;
        kernelSize=round(_getMaxima(interestPoint[0], interestPoint[1])*2);
        step=_getMaxima(interestPoint[0], interestPoint[1]);

        for(int i=0; i<NR_ANGLE_BINS;i++) {
            Sx[i]=0;
            Sy[i]=0;
        }

        /*for(int i=interestPoint[0]-regionSize;i<=interestPoint[0]+regionSize;i++) {
            for(int j=interestPoint[1]-regionSize; j<=interestPoint[1]+regionSize;j++) {
                cout << "(" <<this->image->getPixel(i,j)<< "," << this->image->getIntegralPixel(i,j) <<") ";
            }
            cout << "\n";
        }*/

        for(double i=(interestPoint[0]-regionSize);i<=(interestPoint[0]+regionSize);i+=step) {
            for(double j=(interestPoint[1]-regionSize); j<=(interestPoint[1]+regionSize);j+=step) {

                //pixels have integer values
                pointX=round(i);
                pointY=round(j);
                //pointX and pointY are the coordinates of the sample point

                double dist=_euclidianDistance(interestPoint[0],interestPoint[1], pointX,pointY);
                if(dist<=regionSize) {   //if point is in the circle

                    double weight=_gaussWeighting(interestPoint[0]-pointX,interestPoint[1]-pointY,2.5*_getMaxima(pointX,pointY));

                    //x directon
                    dx=this->image->getRegionSum(pointX-kernelSize, pointY,pointX+kernelSize,pointY+kernelSize);
                    dx-=this->image->getRegionSum(pointX-kernelSize, pointY-kernelSize,pointX+kernelSize,pointY);
                    dx*=weight;

                    //y direction
                    dy=this->image->getRegionSum(pointX,pointY-kernelSize,pointX+kernelSize, pointY+kernelSize);
                    dy-=this->image->getRegionSum(pointX-kernelSize,pointY-kernelSize,pointX, pointY+kernelSize);
                    dy*=weight;

                    if(dx>0) {
                        if(dy>0) {
                            if(fabs(dy)>fabs(dx)) {
                                Sx[0]+=dx;
                                Sy[0]+=dy;
                            } else {
                                Sx[1]+=dx;
                                Sy[1]+=dy;
                            }
                        } else {
                            if(fabs(dy)<=fabs(dx)) {
                                Sx[2]+=dx;
                                Sy[2]+=dy;
                            } else {
                                Sx[3]+=dx;
                                Sy[3]+=dy;
                            }
                        }
                    } else {
                        if(dy<=0) {
                            if(fabs(dy)>fabs(dx)) {
                                Sx[4]+=dx;
                                Sy[4]+=dy;
                            } else {
                                Sx[5]+=dx;
                                Sy[5]+=dy;
                            }
                        } else {
                            if(fabs(dy)<=fabs(dx)) {
                                Sx[6]+=dx;
                                Sy[6]+=dy;
                            } else {
                                Sx[7]+=dx;
                                Sy[7]+=dy;
                            }
                        }
                    }

                } //if point in circle
            } //for j
        } //for i
        max=0;
        orientation=0;

        for(int i=0; i<NR_ANGLE_BINS;i++) {
            double tmp = _euclidianDistance(0,0,Sx[i],Sy[i]);
            if(tmp>max) {
                    max = tmp;
                    orientation=i;
            }
        }

        orientation=round(atan2(Sy[orientation],Sx[orientation]) * 180 / PI);
        //if(orientation<0) orientation=360+orientation;

        this->orientations[pointCount]=orientation;

        this->image->drawCircle(interestPoint[1],interestPoint[0],1);
        this->image->drawLine(interestPoint[1],interestPoint[0],interestPoint[1]+round(sin(orientation)*10),interestPoint[0]+round(cos(orientation)*10));
        //cout << orientation << "\n";
       iter1++;
       //pointCount++;
     }
}

double Descriptor::_gaussWeighting(int x, int y, double stdev) {
return (1/(2*PI*pow(stdev,2.0)))*exp(-(pow((double)x,2.0)+pow((double)y,2.0))/(2*stdev));
}
double Descriptor::_getMaxima(int x,int y) {
    return this->hd->getMaxima(x,y);
}
double Descriptor::_euclidianDistance(int x1, int y1, int x2, int y2) {
 return sqrt(pow((double)(x1-x2),2.0)+pow((double)(y1-y2),2.0));
}
void Descriptor::_GaborResponse(int x,int y, double maxima, double* descriptor) {
    //
    double factor = maxima/HD_INITIAL_SCALE;
   /*vector<int> d=*descriptor;
    d[0]++;*/

/**
 * Gabor filter bank consists of 8 kernels. There are 4 orientations and 2 frequencies.
 * Orientations: 0, 45, 90 and 135 degrees
 * First frequency: bandwith 2, phase PI/2, aspect ratio 0.1, wavelength 5
 * Second frequency: bandwith 4, phase PI/2, aspect ratio 0.1, wavelength 10
 * Third frequency: bandwith 6, phase PI/2, aspect ratio 0.1, wavelength 8
 */
    //relative coordinates(from the center of the kernel) and weights
    //for points of local maxima for first gabor filter(x,y,weight)
    double gabor11[6][3]={
    {-1,0,0.839304},
    {1,0,-0.839304},
    {-3,0,-0.190826},
    {3,0,0.190826},
    {-6,0,0.0105653},
    {6,0,-0.0105653}
    };
    //second...orientation=0, wavelength=10
    double gabor12[6][3]={
    {-2,0,0.839304},
    {2,0,-0.839304},
    {-7,0,-0.20568},
    {7,0,0.20568},
    {-11,0,0.0133981},
    {11,0,-0.0133981}
    };

    double gabor13[10][3]={
    {-2,0,0.945959},
    {2,0,-0.945959},
    {-6,0,-0.606531},
    {6,0,0.606531},
    {-10,0,0.249352},
    {10,0,-0.249352},
    {-13,0,0.0676238},
    {13,0,-0.0676238},
    {-17,0,0.0127725},
    {17,0,-0.0127725}
    };

    //orientation=PI/4, wavelength=5
    double gabor21[6][3]={
    {-1,-1,0.762278},
    {-3,-2,-0.201919},
    {2,3,0.201919},
    {1,1,0.762278},
    {3,5,0.0134254},
    {-5,-3,-0.0134254}
    };

    //orientation=PI/4, wavelength=10
    double gabor22[6][3]={
    {-2,-1,0.844207},
    {2,1,-0.844207},
    {-5,-4,-0.213173},
    {5,4,0.213173},
    {-9,-7,0.013459},
    {9,7,-0.013459}
    };

    double gabor23[10][3]={
    {-2,-1,0.935087},
    {2,1,-0.935087},
    {-4,-4,-0.618035},
    {4,4,0.618035},
    {-7,-7,0.255577},
    {7,7,-0.255577},
    {-9,-10,-0.0736175},
    {10,9,0.0736175},
    {-12,-12,0.0126482},
    {12,12,-0.0126482}
    };

    double gabor31[6][3]={
    {0,-1,0.839304},
    {0,1,-0.839304},
    {0,-3,-0.190826},
    {0,3,0.190826},
    {0,6,-0.0105653},
    {0,-6,0.0105653}
    };
    double gabor32[6][3]={
    {0,2,-0.839304},
    {0,7,0.20568},
    {0,11,-0.0133981},
    {0,-2,0.839304},
    {0,-7,-0.20568},
    {0,-11,0.0133981}
    };
    double gabor33[10][3]={
    {0,-2,0.945959},
    {0,2,-0.945959},
    {0,-6,-0.606531},
    {0,6,0.606531},
    {0,-10,0.249352},
    {0,10,-0.249352},
    {0,-13,-0.0676238},
    {0,13,0.0676238},
    {0,-17,-0.0127725},
    {0,17,0.0127725}
    };

    double gabor41[6][3]={
    {1,-1,0.762278},
    {-2,+3,0.201919},
    {2,-3,-0.201919},
    {-1,1,-0.762278},
    {3,-5,0.0134254},
    {-3,5,-0.0134254}
    };
    double gabor42[6][3]={
    {-1,2,-0.844207},
    {2,-1,0.844207},
    {4,-5,-0.213173},
    {-5,4,0.213173},
    {8,-8,0.013459},
    {-8,8,-0.013459}
    };
    double gabor43[10][3]={
    {2,-1,0.935087},
    {-1,2,-0.935087},
    {4,-4,-0.618035},
    {-4,4,0.618035},
    {7,-7,0.255577},
    {-7,7,-0.255577},
    {9,-10,-0.0736175},
    {-10,9,0.0736175},
    {12,-12,0.0126482},
    {-12,12,-0.0126482}
    };

    //orientation PI/8
    double gabor53[10][3]={
    {-2,0,0.946801},
    {2,0,-0.946801},
    {-5,-3,-0.619484},
    {3,5,0.619484},
    {-7,-8,0.263348},
    {8,7,-0.263348},
    {-12,-6,0.0735327},
    {6,12,-0.0735327},
    {-15,-9,0.0133379},
    {9,15,-0.0133379}
    };

    //orientation 3*(PI/8)
    double gabor63[10][3]={
    {0,-2,0.946801},
    {0,2,-0.946801},
    {-3,-5,-0.619484},
    {5,3,0.619484},
    {-8,-7,0.263348},
    {7,8,-0.263348},
    {-6,-12,0.0735327},
    {12,6,-0.0735327},
    {-9,-15,0.0133379},
    {15,9,-0.0133379}
    };

    //orientation 5*PI/8
    double gabor73[10][3]={
    {0,-2,0.946801},
    {0,2,-0.946801},
    {3,-5,-0.619484},
    {-3,5,0.619484},
    {8,-7,0.263348},
    {-8,8,-0.263348},
    {6,-12,0.0735327},
    {-6,12,-0.0735327},
    {9,-15,0.0133379},
    {-9,15,-0.0133379}
    };

    //orientation 7*PI/8
    double gabor83[10][3]={
    {2,0,0.946801},
    {-2,0,-0.946801},
    {5,-3,-0.619484},
    {-5,3,0.619484},
    {7,-8,0.263348},
    {-7,8,-0.263348},
    {12,-6,0.0735327},
    {-12,6,-0.0735327},
    {15,-9,0.0133379},
    {-15,9,-0.0133379}
    };

    //stdev=6, wavelength=4, phase=0, orientation=0
    double gabor91[17][3]={
    {-16,0, 0.0285655},
    {-14,0, -0.0657285},
    {-12,0, 0.135335},
    {-10,0, -0.249352},
    {-8,0, 0.411112},
    {-6,0, -0.606531},
    {-4,0, 0.800737},
    {-2,0, -0.945959},
    {0,0, 1},
    {16,0, 0.0285655},
    {14,0, -0.0657285},
    {12,0, 0.135335},
    {10,0, -0.249352},
    {8,0, 0.411112},
    {6,0, -0.606531},
    {4,0, 0.800737},
    {2,0, -0.945959}
    };

    //stdev=6, wavelength=4, phase=0, orientation=PI/8
    double gabor101[17][3]={
    {-16,-3,0.0292416},
    {-14,-2, -0.0656082},
    {-12,-2, 0.138167},
    {-10,-2, -0.248923},
    {-8,-1, 0.404746},
    {-6,-1, -0.609707},
    {-4,-1, 0.787721},
    {-2,0, -0.926472},
    {0,0, 1},
    {16,3,0.0292416},
    {14,2, -0.0656082},
    {12,2, 0.138167},
    {10,2, -0.248923},
    {8,1, 0.404746},
    {6,1, -0.609707},
    {4,1, 0.787721},
    {2,0, -0.926472}
    };
    //stdev=6, wavelength=4, phase=0, orientation=PI/4
    double gabor111[17][3]={
    {-11,-11, 0.026607},
    {-10,-10, -0.0606333},
    {-9,-8, 0.134318},
    {-7,-7, -0.253187},
    {-6,-5, 0.405626},
    {-4,-4, -0.550271},
    {-3,-3, 0.722915},
    {-2,-1, -0.922342},
    {0,0, 1},
    {11,11, 0.026607},
    {10,10, -0.0606333},
    {8,9, 0.134318},
    {7,7, -0.253187},
    {5,6, 0.405626},
    {4,4, -0.550271},
    {3,3, 0.722915},
    {1,2, -0.922342}
    };

    //3PI/8
     double gabor121[17][3]={
    {-5,-15, 0.0295778},
    {-5,-13, -0.0672137},
    {-2,-12, 0.138167},
    {-4,-9, -0.252577},
    {-1,-8, 0.404746},
    {-3,-5, -0.588397},
    {-1,-4, 0.787721},
    {-0,-2, -0.926472},
    {0,0, 1},
    {5,15, 0.0295778},
    {5,13, -0.0672137},
    {2,12, 0.138167},
    {4,9, -0.252577},
    {1,8, 0.404746},
    {3,5, -0.588397},
    {1,4, 0.787721},
    {0,2, -0.926472}
    };

    double gabor131[17][3]={
    {0,-16, 0.0285655},
    {0,-14, -0.0657285},
    {0,-12, 0.135335},
    {0,-10, -0.249352},
    {0,-8, 0.411112},
    {0,-6, -0.606531},
    {0,-4, 0.800737},
    {0,-2, -0.945959},
    {0,0, 1},
    {0,16, 0.0285655},
    {0,14, -0.0657285},
    {0,12, 0.135335},
    {0,10, -0.249352},
    {0,8, 0.411112},
    {0,6, -0.606531},
    {0,4, 0.800737},
    {0,2, -0.945959}
    };

    //5PI/8
    double gabor141[17][3]={
    {10,-13, 0.0296244},
    {5,-14, -0.0672137},
    {7,-10, 0.137794},
    {4,-9, -0.252577},
    {6,-6, 0.411811},
    {1,-6, -0.606531},
    {3,-3, 0.801129},
    {3,-1, -0.93537},
    {0,0, 1},
    {-10,13, 0.0296244},
    {-5,14, -0.0672137},
    {-7,10, 0.137794},
    {-4,9, -0.252577},
    {-6,6, 0.411811},
    {-1,6, -0.606531},
    {-3,-3, 0.801129},
    {-3,1, -0.93537}
    };

    //6PI/8
    double gabor151[17][3]={
    {11,-11, 0.026607},
    {10,-10, -0.0606333},
    {9,-8, 0.134318},
    {7,-7, -0.253187},
    {6,-5, 0.405626},
    {4,-4, -0.550271},
    {3,-3, 0.722915},
    {2,-1, -0.922342},
    {0,0, 1},
    {-11,11, 0.026607},
    {-10,10, -0.0606333},
    {-8,9, 0.134318},
    {-7,7, -0.253187},
    {-5,6, 0.405626},
    {-4,4, -0.550271},
    {-3,3, 0.722915},
    {-1,2, -0.922342}
    };

    double gabor161[17][3] = {
    {15,-5, 0.0295778},
    {13,-5, -0.0672137},
    {12,-2, 0.138167},
    {9,-4, -0.252577},
    {8,-1, 0.404746},
    {5,-3, -0.588397},
    {4,-1, 0.787721},
    {0,-2, -0.926472},
    {0,0, 1},
    {-15,5, 0.0295778},
    {-13,5, -0.0672137},
    {-12,2, 0.138167},
    {-9,4, -0.252577},
    {-8,1, 0.404746},
    {-5,3, -0.588397},
    {-4,1, 0.787721},
    {-2,0, -0.926472}
    };


    for(int i=0; i<6; i++) {

        descriptor[0]+=this->image->getPixel(round(x+factor*gabor11[i][0]),round(y+factor*gabor11[i][1]))*gabor11[i][2];
        descriptor[2]+=this->image->getPixel(round(x+factor*gabor21[i][0]),round(y+factor*gabor21[i][1]))*gabor21[i][2];
        descriptor[4]+=this->image->getPixel(round(x+factor*gabor31[i][0]),round(y+factor*gabor31[i][1]))*gabor31[i][2];
        descriptor[6]+=this->image->getPixel(round(x+factor*gabor41[i][0]),round(y+factor*gabor41[i][1]))*gabor41[i][2];

        descriptor[1]+=this->image->getPixel(round(x+factor*gabor12[i][0]),round(x+factor*gabor12[i][1]))*gabor12[i][2];
        descriptor[3]+=this->image->getPixel(round(x+factor*gabor22[i][0]),round(x+factor*gabor22[i][1]))*gabor22[i][2];
        descriptor[5]+=this->image->getPixel(round(x+factor*gabor32[i][0]),round(x+factor*gabor32[i][1]))*gabor32[i][2];
        descriptor[7]+=this->image->getPixel(round(x+factor*gabor42[i][0]),round(x+factor*gabor42[i][1]))*gabor42[i][2];

    }
    for(int i=0; i<10;i++) {

        descriptor[8]+=this->image->getPixel(round(x+factor*gabor13[i][0]),round(y+factor*gabor13[i][1]))*gabor13[i][2];
        descriptor[9]+=this->image->getPixel(round(x+factor*gabor23[i][0]),round(y+factor*gabor23[i][1]))*gabor23[i][2];
        descriptor[10]+=this->image->getPixel(round(x+factor*gabor33[i][0]),round(y+factor*gabor33[i][1]))*gabor33[i][2];
        descriptor[11]+=this->image->getPixel(round(x+factor*gabor43[i][0]),round(y+factor*gabor43[i][1]))*gabor43[i][2];

        descriptor[12]+=this->image->getPixel(round(x+factor*gabor53[i][0]),round(x+factor*gabor53[i][1]))*gabor53[i][2];
        descriptor[13]+=this->image->getPixel(round(x+factor*gabor63[i][0]),round(x+factor*gabor63[i][1]))*gabor63[i][2];
        descriptor[14]+=this->image->getPixel(round(x+factor*gabor73[i][0]),round(x+factor*gabor73[i][1]))*gabor73[i][2];
        descriptor[15]+=this->image->getPixel(round(x+factor*gabor83[i][0]),round(x+factor*gabor83[i][1]))*gabor83[i][2];
    }
    for(int i=0; i<17;i++) {

        descriptor[16]+=this->image->getPixel(round(x+factor*gabor91[i][0]),round(y+factor*gabor91[i][1]))*gabor91[i][2];
        descriptor[17]+=this->image->getPixel(round(x+factor*gabor101[i][0]),round(y+factor*gabor101[i][1]))*gabor101[i][2];
        descriptor[18]+=this->image->getPixel(round(x+factor*gabor111[i][0]),round(y+factor*gabor111[i][1]))*gabor111[i][2];
        descriptor[19]+=this->image->getPixel(round(x+factor*gabor121[i][0]),round(y+factor*gabor121[i][1]))*gabor121[i][2];

        descriptor[20]+=this->image->getPixel(round(x+factor*gabor13[i][0]),round(y+factor*gabor131[i][1]))*gabor131[i][2];
        descriptor[21]+=this->image->getPixel(round(x+factor*gabor141[i][0]),round(y+factor*gabor141[i][1]))*gabor141[i][2];
        descriptor[22]+=this->image->getPixel(round(x+factor*gabor151[i][0]),round(y+factor*gabor151[i][1]))*gabor151[i][2];
        descriptor[23]+=this->image->getPixel(round(x+factor*gabor161[i][0]),round(y+factor*gabor161[i][1]))*gabor161[i][2];

    }
}

void Descriptor::createDescriptors() {

    int regionSize;

    vector<vector<int> >::iterator iter1 = this->interestPoints->begin();
         while( iter1 != this->interestPoints->end()) { //loop over every interest point
            vector<int > interestPoint=*iter1;

            vector<double> descriptor;
            for(int i=0;i<DESCRIPTOR_SIZE;i++) {
            descriptor.push_back(0);
            }

            regionSize=round(8*this->_getMaxima(interestPoint[0], interestPoint[1]));

            int xStart=(interestPoint[0]-regionSize);
            int xEnd=(interestPoint[0]);
            int yStart=(interestPoint[1]-regionSize);
            int yEnd=(interestPoint[1]);

            //first subregion
            for(int i=xStart;i<=xEnd;i+=2) {
                for(int j=yStart; j<=yEnd;j+=2) {
                    //double dist=_euclidianDistance(interestPoint[0],interestPoint[1], i,j);
                    //if(dist<=(regionSize+0.5)) {   //if point is in the circle
                    this->_GaborResponse(i,j, this->_getMaxima(interestPoint[0], interestPoint[1]),&descriptor[0]);
                    //this->_RegionResponse(i,j, this->_getMaxima(interestPoint[0], interestPoint[1]),&descriptor[0]);
                    //}
                }
                //cout << "\n";
            }

            xStart=(interestPoint[0]-regionSize);
            xEnd=(interestPoint[0]);
            yStart=(interestPoint[1]);
            yEnd=(interestPoint[1]+regionSize);
            for(int i=xStart;i<=xEnd;i+=2) {
                for(int j=yStart; j<=yEnd;j+=2) {
                    //double dist=_euclidianDistance(interestPoint[0],interestPoint[1], i,j);
                    //if(dist<=(regionSize+0.5)) {   //if point is in the circle
                    this->_GaborResponse(i,j, this->_getMaxima(interestPoint[0], interestPoint[1]),&descriptor[24]);
                    //this->_RegionResponse(i,j, this->_getMaxima(interestPoint[0], interestPoint[1]),&descriptor[20]);
                    //}
                }
                //cout << "\n";
            }

            xStart=(interestPoint[0]);
            xEnd=(interestPoint[0]+regionSize);
            yStart=(interestPoint[1]-regionSize);
            yEnd=(interestPoint[1]);
            for(int i=xStart;i<=xEnd;i+=2) {
                for(int j=yStart; j<=yEnd;j+=2) {
                    //double dist=_euclidianDistance(interestPoint[0],interestPoint[1], i,j);
                    //if(dist<=(regionSize+0.5)) {   //if point is in the circle
                    this->_GaborResponse(i,j, this->_getMaxima(interestPoint[0], interestPoint[1]),&descriptor[48]);
                    //this->_RegionResponse(i,j, this->_getMaxima(interestPoint[0], interestPoint[1]),&descriptor[40]);
                    //}
                }
                //cout << "\n";
            }

            xStart=(interestPoint[0]);
            xEnd=(interestPoint[0]+regionSize);
            yStart=(interestPoint[1]);
            yEnd=(interestPoint[1]+regionSize);
            for(int i=xStart;i<=xEnd;i++) {
                for(int j=yStart; j<=yEnd;j++) {
                    //double dist=_euclidianDistance(interestPoint[0],interestPoint[1], i,j);
                    //if(dist<=(regionSize+0.5)) {   //if point is in the circle
                    this->_GaborResponse(i,j, this->_getMaxima(interestPoint[0], interestPoint[1]),&descriptor[72]);
                    //this->_RegionResponse(i,j, this->_getMaxima(interestPoint[0], interestPoint[1]),&descriptor[60]);
                    //}
                }
                //cout << "\n";
            }

            //find the max element of the descriptor
            /*int max=descriptor[0];
            int maxId=0;
            for(int i=1;i<8;i++) {
                if(descriptor[i]>max) {
                    max=descriptor[i];
                    maxId=i;
                }
            }
            //invariance to rotation
            //shift descriptor so that max element becomes first
            for(int i=0;i<maxId;i++) {
                int tmp=descriptor[0];
                descriptor.erase(descriptor.begin());
                descriptor.push_back(tmp);
            }*/
            this->descriptors.push_back(descriptor);
            iter1++;
         }
}
vector<vector<double> >* Descriptor::getDescriptors() {
    return &this->descriptors;
}
#if 0
void Descriptor::printDescriptorsXML(std::ostream &xmlFile) {
    xmlFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"<< endl;
    xmlFile << "<KeypointXMLList xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";
    xmlFile << "  <XDim>"<< this->image->getHeight() <<"</XDim>"<< endl;
    xmlFile << "  <YDim>"<< this->image->getWidth() <<"</YDim>"<< endl;
    xmlFile << "  <ImageFile>"<< this->image->getPath() <<"</ImageFile>"<< endl;
    xmlFile << "  <Arr>"<< endl;

    vector<vector<int> >::iterator iter1 = interestPoints->begin();
    vector<vector<double> >::iterator iterDesc = descriptors.begin();

    int c=0;
    while( iter1 != interestPoints->end()) {
         vector<int > tmp2=*iter1;

         xmlFile << "    <KeypointN>"<< endl;
         xmlFile << "      <X>"<< tmp2[1] <<"</X>"<<endl;
         xmlFile << "      <Y>"<< tmp2[0] <<"</Y>"<<endl;
         xmlFile << "      <Scale>"<< this->_getMaxima(tmp2[0],tmp2[1]) <<"</Scale>"<<endl;
         xmlFile << "      <Orientation>"<< 0 <<"</Orientation>"<<endl;
         xmlFile << "      <Dim>"<< DESCRIPTOR_SIZE <<"</Dim>"<<endl;
         xmlFile << "      <Descriptor>"<<endl;

         for (vector<double>::iterator it = iterDesc->begin(); it != iterDesc->end(); ++it) {
        	 xmlFile << "        <int>"<< (int)*it << "</int>" << endl;
         }
         xmlFile << "      </Descriptor>"<<endl;
         xmlFile << "    </KeypointN>"<< endl;

         iter1++;
         iterDesc++;
         c++;
    }

    xmlFile << "  </Arr>"<< endl;
    xmlFile << "</KeypointXMLList>"<< endl;
}
#endif
