/**
 * @brief Gaussian Pyramid for Michael Ashikhmin tone mapping operator
 * 
 * This file is a part of PFSTMO package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Grzegorz Krawczyk
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 *
 * @author Akiko Yoshida, <yoshida@mpi-sb.mpg.de>
 *
 * $Id: pyramid.h,v 1.4 2004/11/16 13:40:45 yoshida Exp $
 */

#ifndef PYRAMID_ASHIKHMIN_H
#define PYRAMID_ASHIKHMIN_H

#include <array2d.h>

class Pyramid { // each level of a Gaussian pyramid
 public:
  Pyramid() {
    flag = 0;
    GP = NULL;
  };

  ~Pyramid() {
    //    delete GP;
  };

  int width;
  int height;
  int size;
  int kernel_size;

  double lambda;

  pfs::Array2D* GP;
  int flag;

  inline double getPixel(int x, int y) {
    return (*GP)(x,y);
  }

  inline void setPixel(int x, int y, double val) {
    (*GP)(x,y) = val;
  }
};

////////////////////////////////////////////////////////

class GaussianPyramid {
 public:
  GaussianPyramid() {};

  GaussianPyramid(pfs::Array2D* lum_map, int im_height, int im_width) {
    // set a=0.4 (considered by Burt and Adelson, 1983).
    // obtained by Outer((0.05, 0.25, 0.4, 0.25, 0.05),(0.05, 0.25, 0.4, 0.25, 0.05))
    g_weights[0][0] = g_weights[0][4] = 0.0025;
    g_weights[0][1] = g_weights[0][3] = 0.0125;
    g_weights[0][2] = 0.0200;

    g_weights[1][0] = g_weights[1][4] = 0.0125;
    g_weights[1][1] = g_weights[1][3] = 0.0625;
    g_weights[1][2] = 0.1000;

    g_weights[2][0] = g_weights[2][4] = 0.0200;
    g_weights[2][1] = g_weights[2][3] = 0.1000;
    g_weights[2][2] = 0.1600;

    g_weights[3][0] = g_weights[3][4] = 0.0125;
    g_weights[3][1] = g_weights[3][3] = 0.0625;
    g_weights[3][2] = 0.1000;

    g_weights[4][0] = g_weights[4][4] = 0.0025;
    g_weights[4][1] = g_weights[4][3] = 0.0125;
    g_weights[4][2] = 0.0200;

    constructPyramid(lum_map, im_width, im_height);
  };

  ~GaussianPyramid() {
    for( int i=0 ; i< PYRAMID ; i++ )
      delete p[i].GP;
  };

  double InterpolateLum(double newX, double newY, Pyramid *pl) {
    int X_int = (int)newX;
    int Y_int = (int)newY;
    
    double dx, omdx, dy, omdy;
    dx = newX - (double)X_int;
    omdx = 1.0 - dx;
    dy = newY - (double)Y_int;
    omdy = 1.0 - dy;
    double lum;

    if(X_int < pl->width-1 && Y_int < pl->height-1)
      lum = omdx * omdy * pl->getPixel(X_int, Y_int)
	+ dx * omdy * pl->getPixel(X_int+1, Y_int)
	+ omdx * dy * pl->getPixel(X_int, Y_int+1)
	+ dx * dy * pl->getPixel(X_int+1, Y_int+1);
    else if(X_int < pl->width-1 && Y_int >= pl->height) {
      Y_int = pl->height-1;
      lum = omdx * pl->getPixel(X_int, Y_int) + dx * pl->getPixel(X_int+1, Y_int);
    }
    else if(X_int >= pl->width && Y_int < pl->height-1) {
      X_int = pl->width-1;
      lum = omdy * pl->getPixel(X_int, Y_int) + dy * pl->getPixel(X_int, Y_int+1);
    }
    else
      lum = pl->getPixel(pl->width-1, pl->height-1);

    return lum;
  }


  void Interpolate(int bottom, int top) {
    //         fprintf(stderr, "Interpolating kernel_size=%2d and kernel_size=%2d\n", p[bottom].kernel_size, p[top].kernel_size);
    for(int i=bottom+1; i<top && i<PYRAMID; i++) {
      //      fprintf(stderr, "into %2d\n", i+1);

      int new_kernel_size = i+1;
      double lambda = 1.0 - (double)(p[bottom].kernel_size-new_kernel_size) / (double)(p[bottom].kernel_size-p[top].kernel_size) * 0.5;
      int new_w = (int)((double)p[bottom].width  * lambda);
      int new_h = (int)((double)p[bottom].height * lambda);
      initializeNewLevel(i, new_w, new_h, new_kernel_size, lambda*p[bottom].lambda);

      for(int y=0; y<p[i].height; y++) {
	for(int x=0; x<p[i].width; x++) {
	  // top
	  double newX = (int)((double)x * 1/2 / lambda);
	  double newY = (int)((double)y * 1/2 / lambda);
	  double top_lum = InterpolateLum(newX, newY, &p[top]);

	  // bottom
	  newX = (int)((double)x/lambda);
	  newY = (int)((double)y/lambda);
	  double bottom_lum = InterpolateLum(newX, newY, &p[bottom]);

	  (*p[i].GP)(x,y) = (1.0-lambda)*top_lum + lambda*bottom_lum;
	}
      }
    }
  }


  void InterpolatePyramid() {
    int bottom=0, top=bottom+1;
    while(bottom < PYRAMID-1) {
      while(p[top].flag == 0)
	top++;
    
      Interpolate(bottom, top);

      bottom = top;
      top++;
    }
  }

  int constructNext(int current_index) {
    int w = (int)(p[current_index].width / 2);
    int h = (int)(p[current_index].height / 2);

    int k_size;
    if(p[current_index].kernel_size == 1)
      k_size = 5;
    else
      k_size = 2 * p[current_index].kernel_size;

    int next_index = k_size-1;
    double new_lambda = p[current_index].lambda * 0.5;
    initializeNewLevel(next_index, w, h, k_size, new_lambda);

    // apply 5*5 kernel
    int X, Y;
    for(int y=0; y<h; y++) {
      for(int x=0; x<w; x++) {
	double sum = 0.0;
	for(int n=-2; n<3; n++) {
	  for(int m=-2; m<3; m++) {
	    if(2*x+m >= 0 && 2*x+m < p[current_index].width)
	      X = 2*x+m;
	    else if(2*x+m < 0)
	      X = 0;
	    else
	      X = p[current_index].width-1;
	      
	    if(2*y+n >= 0 && 2*y+n < p[current_index].height)
	      Y = 2*y+n;
	    else if(2*y+n < 0)
	      Y = 0;
	    else
	      Y = p[current_index].height-1;

	    sum += g_weights[m+2][n+2] * p[current_index].getPixel(X, Y);
	  }
	}

	p[next_index].setPixel(x, y, sum);
      }
    }
    return next_index;
  }

  void showPyramid() {
    fprintf(stderr, "showPyramid()...\n");
    for(int i=0; i<PYRAMID; i++)
      fprintf(stderr, "index=%2d: kernel_size=%2d, w h=%4d %4d, lambda=%f\n", i, p[i].kernel_size, p[i].width, p[i].height, p[i].lambda);
  }

  void initializeNewLevel(int index, int w, int h, int k_size, double lambda) {
    p[index].width = w;
    p[index].height = h;
    p[index].size = w * h;
    p[index].kernel_size = k_size;
    p[index].lambda = lambda;
    p[index].GP = new pfs::Array2DImpl(w, h);
    p[index].flag = 1;
  }

  void constructPyramid(pfs::Array2D* lum_map, int im_width, int im_height) {
    initializeNewLevel(0, im_width, im_height, 1, 1.0);
    for(int y=0; y<im_height; y++) 
      for(int x=0; x<im_width; x++)
	p[0].setPixel(x, y, (*lum_map)(x,y));

    int current_index = 0;
    while(p[current_index].kernel_size < PYRAMID)
      current_index = constructNext(current_index);

    InterpolatePyramid();

    //showPyramid();
  }

  static const int PYRAMID = 20;
  Pyramid p[PYRAMID];
  double g_weights[5][5];

};


#endif
