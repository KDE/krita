
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


#ifndef _CONFIG_DLG_H_
#define _CONFIG_DLG_H_

#include <qgrpbox.h> 
#include <qchkbox.h>
#include <qdialog.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qframe.h>
#include <qspinbox.h>

#include <kcolordlg.h>
#include "kcalc.h"



class ConfigDlg : public QDialog {

Q_OBJECT

public:

  ConfigDlg(QWidget *parent=0, const char *name=0,
	    	    KApplication* k=NULL, DefStruct *defstruct=NULL);

  ~ConfigDlg() {}

  DefStruct *defst ;
  QLineEdit *mail;
  QLineEdit *print;


private slots:

  void okButton();
  void cancelbutton();
  void set_fore_color();
  void set_background_color();
  void help();

signals:
  void color_change();

public:
  bool colors_changed;
  
private:


  QGroupBox *box;
  
  QPushButton *ok;
  QPushButton *cancel;

  QLabel *label1;
  QFrame *qframe1;
  QPushButton *button1;

  QLabel *label2;
  QFrame *qframe2;
  QPushButton *button2;

  QLabel *label3;
  QFrame *qframe3;
  QPushButton *button3;

  QLabel *label4;
  QFrame *qframe4;
  QPushButton *button4;
  KApplication* mykapp; // never use kapp;

  QLabel *label5;

  QLabel *label6;


  QLabel *label7;

  QLabel *label8;


  QGroupBox *gbox;
  QCheckBox *cb;  
  QCheckBox *cb2;
  QSpinBox* precspin;
  QSpinBox* precspin2;  
  QCheckBox *mybox;
  QCheckBox *frame3d;

  QButtonGroup *stylegroup;
  QLabel *stylelabel;
  QRadioButton *trigstyle;
  QRadioButton *statstyle;
  QRadioButton *tablestyle;
};
#endif

