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

#include "dlabel.h"
#include "dlabel.moc"
#include "kcalc.h"

DLabel::DLabel(QWidget *parent, const char *name)
  :QLabel(parent,name){

    button = 0;
    lit = false;
}

void DLabel::mousePressEvent(QMouseEvent *e){

  if(e->button() == LeftButton){
    lit = !lit;
    button = LeftButton;
  }
  else{
    button = MidButton;
  }


  emit clicked();
}


int DLabel::Button(){
  return button;
}

bool DLabel::isLit(){

  return lit;

}

void DLabel::setLit(bool _lit){
  lit = _lit;
}
