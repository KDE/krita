/*

 $Id$

 KCalc 

 Copyright (C) Bernd Johannes Wuebben
               wuebben@math.cornell.edu
	       wuebben@kde.org

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */


#ifndef _D_LABEL_H_
#define _D_LABEL_H_

#include <qlabel.h>


class DLabel : public QLabel {

Q_OBJECT

public:

DLabel(QWidget *parent=0, const char *name=0);

  ~DLabel() {}

protected:
  
  void  mousePressEvent ( QMouseEvent *);

public:
  bool isLit();
  void setLit(bool _lit);
  int Button();

private:
  int button;
  bool lit;

signals:
  void clicked();

};



#endif
