/*
 *  kis_timer.cc - part of KImageShop
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <iostream.h>
#include "kis_timer.h"

struct timeval KisTimer::tv1, KisTimer::tv2;
struct timezone KisTimer::tz;


void KisTimer::start()
{
  gettimeofday( &tv1, &tz );
}

void KisTimer::stop( const char* text)
{
  gettimeofday( &tv2, &tz );
  float time = float( tv2.tv_sec - tv1.tv_sec ) + ( tv2.tv_usec - tv1.tv_usec ) / 1000000.;
  cout << text << " took " << time << " seconds\n";
}
