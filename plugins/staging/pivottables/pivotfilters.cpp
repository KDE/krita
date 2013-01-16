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
#include "pivotfilters.h"
#include "ui_pivotfilters.h"
#include <sheets/Sheet.h>
#include <sheets/ui/Selection.h>
#include "pivotmain.h"
#include<QtGui>
#include<QString>
using namespace Calligra::Sheets;

class PivotFilters::Private
{
public:
    Selection *selection;
    Ui::PivotFilters mainWidget;
    int flag1,flag2;
    bool flag;
    QVector<QString> conditions;
};


PivotFilters::PivotFilters(QWidget* parent,Selection* selection):
    KDialog(parent),
    d(new Private)
{
    QWidget* widget = new QWidget(this);
    d->mainWidget.setupUi(widget);    
    setMainWidget(widget);
    d->selection=selection;
    selectFields(d->mainWidget.Field);
    d->flag1=1;
    d->flag2=1;
    d->flag=false;
    
//     setButtons(Ok|Cancel);
//     enableButton(Ok,"true");
    connect(d->mainWidget.Operator,SIGNAL(activated(const QString&)),this,SLOT(activateBoxes()));
    connect(d->mainWidget.Operator2,SIGNAL(activated(const QString&)),this,SLOT(activateBoxes2()));
    connect(d->mainWidget.Field,SIGNAL(activated(const QString&)),this,SLOT(fillValue()));
    connect(d->mainWidget.Field2,SIGNAL(activated(const QString&)),this,SLOT(fillValue2()));
    connect(d->mainWidget.Field3,SIGNAL(activated(const QString&)),this,SLOT(fillValue3()));
  
}


void PivotFilters::selectFields(QComboBox* box)
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
	  box->addItem(text); 
	}
    }
}
void PivotFilters::activateBoxes()
{
  if(d->mainWidget.Operator->currentText()=="None")
  {
    d->mainWidget.Field2->clear();
    d->mainWidget.Operator2->clear();
    d->mainWidget.Field3->clear();
    d->mainWidget.Value2->clear();
    d->mainWidget.Value3->clear();
    d->mainWidget.Condition2->clear();
    d->mainWidget.Condition3->clear();
    d->flag2=0;
    d->flag1=0;
    d->flag=true;
  }
  if(d->flag1==1)
  {
    selectFields(d->mainWidget.Field2);
    d->mainWidget.Operator2->addItem("None");
    d->mainWidget.Operator2->addItem("And");
    d->mainWidget.Operator2->addItem("Or");
    d->mainWidget.Condition2->addItem("<");
    d->mainWidget.Condition2->addItem(">");
    d->mainWidget.Condition2->addItem("==");
    d->mainWidget.Condition2->addItem("!=");
    
  }
  d->flag1++;
}
void PivotFilters::activateBoxes2()
{
  if(d->mainWidget.Operator2->currentText()=="None")
  {
    d->mainWidget.Field3->clear();
    d->mainWidget.Value3->clear();
    d->mainWidget.Condition3->clear();
    
    d->flag2=0;
  }
    
  
  if(d->flag2==1 || d->flag==true)
  {
    selectFields(d->mainWidget.Field3);
    d->mainWidget.Condition3->addItem("<");
    d->mainWidget.Condition3->addItem(">");
    d->mainWidget.Condition3->addItem("==");
    d->mainWidget.Condition3->addItem("!=");
    d->flag=false;
  }
  d->flag2++;
}
void PivotFilters::fillValue()
{
  PivotMain *pmain=new PivotMain(this,d->selection);
  QVector<QString> str=pmain->ValueData(d->mainWidget.Field->currentText());
  d->mainWidget.Value->clear();
  for(int i=0;i<str.count();i++)
  {
    d->mainWidget.Value->addItem(str.at(i));
  }
}
void PivotFilters::fillValue2()
{
  PivotMain *pmain=new PivotMain(this,d->selection);
  QVector<QString> str=pmain->ValueData(d->mainWidget.Field2->currentText());
  d->mainWidget.Value2->clear();
  for(int i=0;i<str.count();i++)
  {
    d->mainWidget.Value2->addItem(str.at(i));
  }
}
void PivotFilters::fillValue3()
{
  PivotMain *pmain=new PivotMain(this,d->selection);
  QVector<QString> str=pmain->ValueData(d->mainWidget.Field3->currentText());
  d->mainWidget.Value3->clear();
  for(int i=0;i<str.count();i++)
  {
    d->mainWidget.Value3->addItem(str.at(i));
  }
}

QVector<QString> PivotFilters::filterData()
{
  QVector<QString> data;
  data.append(d->mainWidget.Field->currentText());
  data.append(d->mainWidget.Condition->currentText());
  data.append(d->mainWidget.Value->currentText());
  qDebug()<<"operator"<<d->mainWidget.Operator->currentText();
  if(d->mainWidget.Operator->currentText()!="None")
  {
    data.append(d->mainWidget.Operator->currentText());
    data.append(d->mainWidget.Field2->currentText());
    data.append(d->mainWidget.Condition2->currentText());
    data.append(d->mainWidget.Value2->currentText());   
  }
  if(d->mainWidget.Operator2->currentText()!="None" && d->mainWidget.Operator->currentText()!="None")
  {
    data.append(d->mainWidget.Operator2->currentText());
    data.append(d->mainWidget.Field3->currentText());
    data.append(d->mainWidget.Condition3->currentText());
    data.append(d->mainWidget.Value3->currentText());   
  }
    
    return data;
}





PivotFilters::~PivotFilters()
{
    delete d;
}
