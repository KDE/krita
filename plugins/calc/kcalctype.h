/*
    $Id$

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no
   
    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifndef KCALC_TYPE_H
#define KCALC_TYPE_H

// The following for all the poor devels out there who don't have long double math.
// I guess it's time to switch to LINUX guys .....

// Undefine HAVE_LONG_DOUBLE for Beta 4 since RedHat 5.0 comes with a borken
// glibc

#ifdef HAVE_LONG_DOUBLE
#undef HAVE_LONG_DOUBLE
#endif

#ifdef HAVE_LONG_DOUBLE

/* should be detected by autoconf and defined in config.h
   Be carefull when modifying these lines. HAVE_LONG_DOUBLE
   is used all over kcalc's sources to determine whether 
   long double of double is the fundamental data type for kcalc*/



#define CALCAMNT        long double
#else
#define CALCAMNT        double
#endif


#ifdef HAVE_LONG_DOUBLE
#define FABS(X)   	fabsl(X)
#define MODF(X,Y)       modfl(X,Y)
#define FMOD(X,Y)   	fmodl(X,Y)
#define SIN(X)		sinl(X)
#define ASIN(X)		asinl(X)
#define SINH(X)		sinhl(X)
#define ASINH(X)	asinhl(X)
#define COS(X)		cosl(X)
#define COSH(X)		coshl(X)
#define ACOS(X)		acosl(X)
#define ACOSH(X)	acoshl(X)
#define TAN(X)		tanl(X)
#define TANH(X)		tanhl(X)
#define ATAN(X)		atanl(X)
#define ATANH(X)	atanhl(X)
#define EXP(X)		expl(X)
#define POW(X,Y)	powl(X,Y)
#define LOG(X)		logl(X)
#define LOG_TEN(X)	log10l(X)
#define SQRT(X)		sqrtl(X)
#else
#define FABS(X)		fabs(X)
#define MODF(X,Y)    	modf(X,Y)
#define FMOD(X,Y)   	fmod(X,Y)
#define SIN(X)		sin(X)
#define ASIN(X)		asin(X)
#define SINH(X)		sinh(X)
#define ASINH(X)	asinh(X)
#define COS(X)		cos(X)
#define COSH(X)		cosh(X)
#define ACOS(X)		acos(X)
#define ACOSH(X)	acosh(X)
#define TAN(X)		tan(X)
#define TANH(X)		tanh(X)
#define ATAN(X)		atan(X)
#define ATANH(X)	atanh(X)
#define EXP(X)		exp(X)
#define POW(X,Y)	pow(X,Y)
#define LOG(X)		log(X)
#define LOG_TEN(X)	log10(X)
#define SQRT(X)		sqrt(X)
#endif


#endif 
