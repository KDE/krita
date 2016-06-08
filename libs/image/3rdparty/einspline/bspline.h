/////////////////////////////////////////////////////////////////////////////
//  einspline:  a library for creating and evaluating B-splines            //
//  Copyright (C) 2007 Kenneth P. Esler, Jr.                               //
//                                                                         //
//  This program is free software; you can redistribute it and/or modify   //
//  it under the terms of the GNU General Public License as published by   //
//  the Free Software Foundation; either version 2 of the License, or      //
//  (at your option) any later version.                                    //
//                                                                         //
//  This program is distributed in the hope that it will be useful,        //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//  GNU General Public License for more details.                           //
//                                                                         //
//  You should have received a copy of the GNU General Public License      //
//  along with this program; if not, write to the Free Software            //
//  Foundation, Inc., 51 Franklin Street, Fifth Floor,                     //
//  Boston, MA  02110-1301  USA                                            //
/////////////////////////////////////////////////////////////////////////////

#ifndef BSPLINE_H
#define BSPLINE_H

#include "bspline_base.h"
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////           Bspline structure definitions            ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#include "bspline_structs.h"
#include "multi_bspline_structs.h"

// Currently, some of the single-precision routines use SSE2 instructions
#ifdef HAVE_SSE2
#include "bspline_eval_sse_s.h"
#include "bspline_eval_sse_c.h"
#include "bspline_eval_sse_d.h"
#include "bspline_eval_sse_z.h"
#elif defined HAVE_SSE
#include "bspline_eval_sse_s.h"
#include "bspline_eval_sse_c.h"
#include "bspline_eval_std_d.h"
#include "bspline_eval_std_z.h"
#elif defined USE_ALTIVEC
#include "bspline_eval_altivec_s.h"
#include "bspline_eval_std_c.h"
#include "bspline_eval_std_d.h"
#include "bspline_eval_std_z.h"
#else
#include "bspline_eval_std_s.h"
#include "bspline_eval_std_c.h"
#include "bspline_eval_std_d.h"
#include "bspline_eval_std_z.h"
#endif

#include "bspline_create.h"
#include "multi_bspline_create.h"
#endif
