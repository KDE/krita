
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

#include <kfontdialog.h>
#include <kapp.h>
#include <klocale.h>
#include "fontdlg.h"
#include "fontdlg.moc"

FontDlg::FontDlg(QWidget *parent, const char *name,
		     KApplication *mykapp,DefStruct *defstruct)
  : QDialog(parent, name)
{

  mykapp = kapp;
  defst = defstruct;

  box = new QGroupBox(this, "box");
  box->setGeometry(10,10,320,260);
  box->setTitle(i18n("Set Default Font"));

  button = new QPushButton(this);
  button->setGeometry(205,225,100,25);
  button->setText(i18n("Change"));
  connect(button,SIGNAL(clicked()),this,SLOT(setFont()));

  familylabel = new QLabel(this);
  familylabel->setGeometry(30,40,135,25);
  familylabel->setText(i18n("Family:"));

  familylabeldisp = new QLabel(this);
  familylabeldisp->setGeometry(130,40,150,23);	
  familylabeldisp->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  familylabeldisp->setText(defst->font.family());
//  familylabeldisp->setBackgroundColor(white);

  sizelabel = new QLabel(this);
  sizelabel->setGeometry(30,75,100,25);
  sizelabel->setText(i18n("Point Size:"));

  sizelabeldisp = new QLabel(this);
  sizelabeldisp->setGeometry(130,75,60,23);	
  sizelabeldisp->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
//  sizelabeldisp->setBackgroundColor(white);
  QString size;
  size.setNum(defst->font.pointSize());
  sizelabeldisp->setText(size);


  stylelabel = new QLabel(this);
  stylelabel->setGeometry(30,110,80,25);
  stylelabel->setText(i18n("Style:"));


  stylelabeldisp = new QLabel(this);
  stylelabeldisp->setGeometry(130,110,80,23);
  stylelabeldisp->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
//  stylelabeldisp->setBackgroundColor(white);

  if(defst->font.italic())
    stylelabeldisp->setText(i18n("Italic"));
  else
    stylelabeldisp->setText(i18n("Roman"));

  weightlabel = new QLabel(this);
  weightlabel->setGeometry(30,145,80,25);
  weightlabel->setText(i18n("Weight:"));



  weightlabeldisp = new QLabel(this);
  weightlabeldisp->setGeometry(130,145,80,23);
  weightlabeldisp->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
//  weightlabeldisp->setBackgroundColor(white);

  if(defst->font.bold())
    weightlabeldisp->setText(i18n("Bold"));
  else
    weightlabeldisp->setText(i18n("Normal"));

  connect(parent,SIGNAL(applyButtonPressed()),SLOT(okButton()));

}


void FontDlg::help(){

  mykapp->invokeHTMLHelp("","");

}

void FontDlg::okButton(){



}
void FontDlg::cancelbutton() {
  reject();
}


void FontDlg::setFont(){


  KFontDialog::getFont(defst->font);

  familylabeldisp->setText(defst->font.family());

  if(defst->font.bold())
    weightlabeldisp->setText(i18n("Bold"));
  else
    weightlabeldisp->setText(i18n("Normal"));

  if(defst->font.italic())
    stylelabeldisp->setText(i18n("Italic"));
  else
    stylelabeldisp->setText(i18n("Roman"));

  QString size;
  size.setNum(defst->font.pointSize());
  sizelabeldisp->setText(size);
}







