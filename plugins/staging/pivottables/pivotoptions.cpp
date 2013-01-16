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
#include "pivotoptions.h"
#include "ui_pivotoptions.h"
#include <sheets/Sheet.h>
#include <sheets/ui/Selection.h>
#include<QtGui>
#include<QListWidgetItem>
#include<QListWidget>
#include<QObject>
using namespace Calligra::Sheets;

class PivotOptions::Private
{
public:
    Selection *selection;
    Ui::PivotOptions mainWidget;
    QString function;
};

PivotOptions::PivotOptions(QWidget* parent,Selection* selection) :
    KDialog(parent),
    d(new Private)
{
   
   setButtons(Ok|Cancel);
   QWidget* widget = new QWidget;
   d->mainWidget.setupUi(widget);
   setCaption(i18n("Pivot Options"));
   setMainWidget(widget);
   d->selection= selection;
//    selectBase();   
   enableButton(Ok,true);
   d->mainWidget.SelectFunction->addItem("prod");
   d->mainWidget.SelectFunction->addItem("devsq");
   //qDebug()<<returnFunction();
   connect(this, SIGNAL(okClicked()), this, SLOT(on_Ok_clicked()));   
     
}

QString PivotOptions::returnFunction()
{
    d->function=d->mainWidget.SelectFunction->currentText();
    return d->function;
  
}


/*
void PivotOptions::selectBase()
{
    Sheet *const sheet = d->selection->lastSheet();
    const QRect range = d->selection->lastRange();

    int r = range.right();
    int row = range.top();

    Cell cell;
    
    QString text;

    int index = 0;
    for (int i = range.left(); i <= r; ++i) {
        cell = Cell(sheet, i, row);
        text = cell.displayText();
	
	if(text.length() >0)
	{
           d->mainWidget.BaseItem->addItem(text);
	   d->mainWidget.BaseField->addItem(text); 
	  
	}
    }
}
*/
void PivotOptions::on_Ok_clicked()
{
  //returnFunction();
}

PivotOptions::~PivotOptions()
{
    delete d;
}
//#include "PivotOptions.moc"
