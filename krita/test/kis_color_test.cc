/*
 *  kis_color_test.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <kis_color.h>
#include <kdebug.h>

void dumpColor(const KisColor& k)
{
    kdDebug()<<" rgb : "<< k.R()<<" "<< k.G() <<" "<< k.B()<<endl;
    kdDebug()<<" hsv : "<< k.h()<<" "<<k.s() <<" "<< k.v()<<endl;
    kdDebug()<<" lab : "<<  k.l()<<" "<< k.a()<<" "<< k.b()<<endl;
    kdDebug()<<" cmyk: "<< k.c()<<" "<< k.m()<<" "<< k.y()<<" "<< k.k()<<endl;

  switch (k.native())
    {
    case RGB:
        kdDebug()<<" RGB is native color space.\n";
      break;
    case HSV:
        kdDebug()<<" HSV is native color space.\n";
      break;
    case LAB:
        kdDebug()<<" Lab is native color space.\n";
      break;
    case CMYK:
        kdDebug()<<" CMYK is native color space.\n";
      break;
    default:
        kdDebug()<<" Warning: No native color space.\n";
      break;
    }
}
int main( int argc, char **argv )
{
    kdDebug()<<"\n--> KColor a;\n";
  KisColor a;
  kdDebug()<<"Dump a:\n";
  dumpColor(a);

  kdDebug()<<"\n--> KisColor b(0, 0, 255);\n";
  KisColor b(0, 0, 255);
  kdDebug()<<"Dump b:\n";
  dumpColor(b);

  kdDebug()<<"\n--> KisColor c(0, 255, 255, 0);\n";
  KisColor c(0, 255, 255, 0);
  kdDebug()<<"Dump c:\n";
  dumpColor(c);

  kdDebug()<<"\n--> a = c;\n";
  a = c;
  kdDebug()<<"Dump a:\n";
  dumpColor(a);

  kdDebug()<<"\n--> a = KisColor::red();\n";
  a = KisColor::red();
  kdDebug()<<"Dump a:\n";
  dumpColor(a);

  kdDebug()<<"\n--> KisColor d = KisColor::darkBlue();\n";
  KisColor d = KisColor::darkBlue();
  kdDebug()<<"Dump d:\n";
  dumpColor(d);

  kdDebug()<<"\n--> KisColor e = KisColor(QColor(0, 255, 0));\n";
  KisColor e = KisColor(QColor(0, 255, 0));
  kdDebug()<<"Dump e:\n";
  dumpColor(e);
  kdDebug()<<"Dump e again:\n";
  dumpColor(e);

  kdDebug()<<"\n--> e.setCMYK(255, 0, 0, 0);\n";
  e.setCMYK(255, 0, 0, 0);
  kdDebug()<<"Dump e:\n";
  dumpColor(e);

  return 0;
}


