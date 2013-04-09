/* This file is part of the KDE project
   Copyright (C) 2012-2013 Jigar Raisinghani <jigarraisinghani@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Local

#include "pivot.h"
#include "ui_pivot.h"
#include "ui_pivotmain.h"
#include "pivotmain.h"

#include <QApplication>
#include <QMessageBox>

#include <sheets/Sheet.h>
#include <sheets/ui/Selection.h>
using namespace Calligra::Sheets;

class Pivot::Private
{
public:
    Selection *selection;
    Ui::Pivot mainWidget;
};


Pivot::Pivot(QWidget* parent,Selection* selection):
    KDialog(parent),
    d(new Private)
{
    setCaption(i18n("Select Source"));
    
  
    QWidget* widget = new QWidget(this);
    d->mainWidget.setupUi(widget);
    setButtons(Ok|Cancel);   
    d->mainWidget.Current->setChecked(true);
    setMainWidget(widget);
    d->selection=selection;
    connect(this, SIGNAL(okClicked()), this, SLOT(slotUser2Clicked()));

}

Pivot::~Pivot()
{
    delete d;
}

void Pivot::slotUser2Clicked()
{
	  if(d->mainWidget.Current->isChecked())
	  {
	      PivotMain *pMain= new PivotMain(this,d->selection);
	      pMain->setModal(true);
	      pMain->exec();
	  }
	  
	  if(d->mainWidget.External->isChecked())
	  {
	      QMessageBox msgBox;
	      msgBox.setText("Functionality Yet to be Added");
	      msgBox.exec();
	      
	      Pivot *p=new Pivot(this,d->selection);
	      p->setModal(true);
	      p->exec();
	  }
}
