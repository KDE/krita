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

#include "pivotmain.h"
#include "pivotoptions.h"
#include "pivotfilters.h"
#include "ui_pivotmain.h"
//#include "ui_pivotoptions.h"

#include<QTimer>
#include<QObject>
#include<QColor>
#include<QPen>
#include<QMessageBox>

#include <sheets/Value.h>
#include <sheets/ValueCalc.h>
#include <sheets/ValueConverter.h>
#include <sheets/Map.h>
#include <sheets/DocBase.h>
#include <sheets/Cell.h>
#include <sheets/Style.h>
#include <sheets/Sheet.h>
#include <sheets/ui/Selection.h>


using namespace Calligra::Sheets;

class PivotMain::Private
{
public:
    Selection *selection;
    Ui::PivotMain mainWidget;
    QString func;
    QVector<QString> retVect;
    QVector<Value> posVect;
    QVector<QString> filterVect;
    Sheet* filterSheet;
    int filtersize;
};
PivotMain::PivotMain(QWidget* parent, Selection* selection) :
    KDialog(parent),
    d(new Private)
{
    QWidget* widget = new QWidget(this);
    d->mainWidget.setupUi(widget);
    setMainWidget(widget);
    d->selection=selection;
    setCaption(i18n("Pivot Main"));
    
    //Adding Buttons
    setButtons(Ok|Cancel|User2|User3);
  //  setButtonGuiItem(User1, KGuiItem(i18n("Options")));
    setButtonGuiItem(User2, KGuiItem(i18n("Add Filter")));
    setButtonGuiItem(User3, KGuiItem(i18n("Reset DnD")));
    enableButton(User1,true);
    enableButton(User2,true);
    enableButton(Ok,true);
    enableButton(User3,true);
    d->mainWidget.TotalRows->setChecked(true);
    d->mainWidget.TotalColumns->setChecked(true);


    d->mainWidget.Labels->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->mainWidget.Labels->setDragEnabled(true);
    d->mainWidget.Labels->setDragDropMode(QAbstractItemView::InternalMove);
    d->mainWidget.Labels->viewport()->setAcceptDrops(true);
    d->mainWidget.Labels->setDropIndicatorShown(true);

    d->mainWidget.Rows->setSelectionMode(QAbstractItemView::SingleSelection);
    d->mainWidget.Rows->setDragEnabled(true);
    d->mainWidget.Rows->setDragDropMode(QAbstractItemView::DropOnly);
    d->mainWidget.Rows->viewport()->setAcceptDrops(true);
    d->mainWidget.Rows->setDropIndicatorShown(true);

    d->mainWidget.Columns->setSelectionMode(QAbstractItemView::SingleSelection);
    d->mainWidget.Columns->setDragEnabled(true);
    d->mainWidget.Columns->setDragDropMode(QAbstractItemView::DropOnly);
    d->mainWidget.Columns->viewport()->setAcceptDrops(true);
    d->mainWidget.Columns->setDropIndicatorShown(true);

    d->mainWidget.Values->setSelectionMode(QAbstractItemView::SingleSelection);
    d->mainWidget.Values->setDragEnabled(true);
    d->mainWidget.Values->setDragDropMode(QAbstractItemView::DropOnly);
    d->mainWidget.Values->viewport()->setAcceptDrops(true);
    d->mainWidget.Values->setDropIndicatorShown(true);

    //The functionality for Page Fields is not added yet.Only added in GUI
/*    d->mainWidget.PageFields->setSelectionMode(QAbstractItemView::SingleSelection);
    d->mainWidget.PageFields->setDragEnabled(true);
    d->mainWidget.PageFields->setDragDropMode(QAbstractItemView::DropOnly);
    d->mainWidget.PageFields->viewport()->setAcceptDrops(true);
    d->mainWidget.PageFields->setDropIndicatorShown(true);
*/
    d->mainWidget.selectOption->addItem("prod");
    d->mainWidget.selectOption->addItem("devsq");

    connect(this,SIGNAL(user2Clicked()),this,SLOT(on_AddFilter_clicked()));
//  connect(this, SIGNAL(user1Clicked()), this, SLOT(on_Options_clicked()));
    extractColumnNames();
    connect(this, SIGNAL(okClicked()), this, SLOT(on_Ok_clicked()));
    connect(this, SIGNAL(user3Clicked()), this, SLOT(Reset()));
  
}

PivotMain::~PivotMain()
{
    delete d;
}//PivotMain

//Function to read the title of every column and add it to Labels.
void PivotMain::extractColumnNames()
{
    Sheet *const sheet = d->selection->lastSheet();
    const QRect range = d->selection->lastRange();
    
    int r = range.right();
    int row = range.top();

    Cell cell;
    QListWidgetItem * item;
    QString text;
    
    for (int i = range.left(); i <= r; ++i) {
        cell = Cell(sheet, i, row);
        text = cell.displayText();
       
	if(text.length() >0)
	{
        item = new QListWidgetItem(text);
	item->setFlags(item->flags());
        d->mainWidget.Labels->addItem(item);
	}  
    }  
}

/*//When the option button is clicked, a dialog for setting options appears
void PivotMain::on_Options_clicked()
{
    PivotOptions *pOptions=new PivotOptions(this,d->selection);
    pOptions->setModal(true);
    pOptions->exec();
    d->func=pOptions->returnFunction();
}//on_Options_Clicked
*/

//When add filter button is clicked, the dialog box for filtering data appears
void PivotMain::on_AddFilter_clicked()
{

        PivotFilters *pFilters=new PivotFilters(this,d->selection);
        pFilters->setModal(true);
        pFilters->exec();
	d->filterVect=pFilters->filterData();
}//on_AddFilter_clicked

//The function receives the data from Add Filter and filters the data. The filtered sheet is then used to further processing
Sheet* PivotMain::filter()
{
  
  Sheet *const sheet1 = d->selection->lastSheet();
  const QRect range2 = d->selection->lastRange();
  
  Map *mymap=sheet1->map();
  Sheet* sheet2=mymap->createSheet("Filtered Sheet");
  
  
  int r= range2.right();
  int b=range2.bottom();
  int row=range2.top();
  QVector<QString> vect;
  QVector<int> filtered;
  
  
  if(d->filterVect.count()<3)
      return sheet1;
  
  
  for(int k=row+1;k<=b;k++)
  {
      bool pass=true;

      if(d->filterVect.count()==3)
      {
	  pass=checkCondition(d->filterVect.at(0),d->filterVect.at(1),d->filterVect.at(2),k);
      }
      else if(d->filterVect.count()==7)
      {
	 
	  if(d->filterVect.at(3)=="And")
	    pass= checkCondition(d->filterVect.at(0),d->filterVect.at(1),d->filterVect.at(2),k) && 
				checkCondition(d->filterVect.at(4),d->filterVect.at(5),d->filterVect.at(6),k);
      
	  if(d->filterVect.at(3)=="Or") 
	      pass=  checkCondition(d->filterVect.at(0),d->filterVect.at(1),d->filterVect.at(2),k) || 
				checkCondition(d->filterVect.at(4),d->filterVect.at(5),d->filterVect.at(6),k);
				
	  
      }
      
      else if(d->filterVect.count()==11)
      {
	  
	  if(d->filterVect.at(3)=="And")
	    pass= checkCondition(d->filterVect.at(0),d->filterVect.at(1),d->filterVect.at(2),k) && 
				checkCondition(d->filterVect.at(4),d->filterVect.at(5),d->filterVect.at(6),k);
			
      
	  if(d->filterVect.at(3)=="Or") 
	    pass=  checkCondition(d->filterVect.at(0),d->filterVect.at(1),d->filterVect.at(2),k) || 
				checkCondition(d->filterVect.at(4),d->filterVect.at(5),d->filterVect.at(6),k);
				

	  if(d->filterVect.at(7)=="And")
	      pass= pass && checkCondition(d->filterVect.at(9),d->filterVect.at(10),d->filterVect.at(11),k);
				
      
	  if(d->filterVect.at(7)=="Or") 
	    pass=  pass || checkCondition(d->filterVect.at(4),d->filterVect.at(5),d->filterVect.at(6),k);
				
	  }
      
      if(pass==true)
	filtered.append(k);
    }

      for(int j=1;j<=r;j++)
	Cell(sheet2,j,1).setValue(Cell(sheet1,j,1).value());
      for(int i=0;i<filtered.count();i++)
      {
	for(int j=1;j<=r;j++)
	{
	  Cell(sheet2,j,i+2).setValue(Cell(sheet1,j,filtered.at(i)).value());
	}
      }
  d->filtersize=filtered.count()+1;
  return sheet2;
}

//This helps the filter function in analyzing the data(condition) received from Add Filter dialog.
bool PivotMain::checkCondition(QString field,QString condition,QString value,int line)
{
  Sheet *const sheet1 = d->selection->lastSheet();
  const QRect range2 = d->selection->lastRange();
  int r= range2.right();
  int b=range2.bottom();
  int row=range2.top();
  
  QVector<QString> vect;
  
  for(int i=range2.left();i<=r;i++)
    vect.append(Cell(sheet1,i,row).displayText());  
  
    if(condition==">"){
      if(Cell(sheet1,vect.indexOf(field)+1,line).displayText() > value){
		 

		  return true;
      }
      else
	    return false;
    }
	
    if(condition=="<"){
      if(Cell(sheet1,vect.indexOf(field)+1,line).displayText() < value){
	return true;}
      else
		  return false;
    }
		  
    if(condition== "=="){
      if(Cell(sheet1,vect.indexOf(field)+1,line).displayText() == value)
		  return true;
      else
		  return false;
    }
      
    if(condition=="!="){
      if(Cell(sheet1,vect.indexOf(field)+1,line).displayText() != value)
		  return true;
      else
		  return false;
    }
    return false;
    
}//checkCondition



//The core function which summarizes the data and forms the pivot table.
void PivotMain::Summarize()
{
  
    
    Map* myMap = d->selection->lastSheet()->map();
    const QRect range3=d->selection->lastRange();
    Sheet* sheet=myMap->createSheet("Filtered Sheet");
    
    sheet=filter();
    if(sheet==d->selection->lastSheet())
    {
      d->filtersize=range3.bottom();
    }
    const QRect range(1,1,d->selection->lastRange().right(),d->filtersize);
    
    QColor color,color2("lightGray");
    color.setBlue(50);
    QPen pen(color);
    

    Style st,st2,st3,str,stl,stb,stt;
    st.setFontUnderline(true);
    st3.setBackgroundColor("lightGray");
    st.setRightBorderPen(pen);
    st.setLeftBorderPen(pen);
    st.setTopBorderPen(pen);
    st.setBottomBorderPen(pen);
    str.setRightBorderPen(pen);
    stl.setLeftBorderPen(pen);
    stt.setTopBorderPen(pen);
    stb.setBottomBorderPen(pen);
    
    
    static int z=1;
    
    Sheet* mySheet=myMap->createSheet("Pivot Sheet"+QString::number(z++));
    
    int r = range.right();
    int row=range.top();
    int bottom=range.bottom();
    Cell cell;
    
    ValueConverter *c;
    
    Value res(0);
    ValueCalc *calc= new ValueCalc(c);
    
    QVector<Value> vect;    
    for (int i = 1; i <= r; ++i) {
	cell= Cell(sheet,i,row);
	vect.append(Value(cell.value()));
    }

  d->func=d->mainWidget.selectOption->currentText();//check for the function to be performed
  
  //For Creating QLists for Rows,Columns,Values and PageField
  int counter;
  QListWidgetItem *item1;
  QList<QListWidgetItem *> rowList,columnList,valueList,pageList;
  
  counter= d->mainWidget.Rows->count();
  for(int i=0;i<counter;i++)
  {
	
        item1=d->mainWidget.Rows->item(i);
        rowList.append(item1);
    
  }
  counter= d->mainWidget.Columns->count();
  for(int i=0;i<counter;i++)
  {
	
        item1=d->mainWidget.Columns->item(i);
        columnList.append(item1); 
  }
  /*counter= d->mainWidget.PageFields->count();
  for(int i=0;i<counter;i++)
  {
        item1=d->mainWidget.PageFields->item(i);
        pageList.append(item1);
  }*/
  counter= d->mainWidget.Values->count();
  for(int i=0;i<counter;i++)
  {
        item1=d->mainWidget.Values->item(i);
        valueList.append(item1); 
  }
  
  //Summarization using vectors
  int rowpos=-1,colpos=-1,valpos=-1;
  QVector<Value> rowVector,rowVectorArr[rowList.size()],columnVectorArr[columnList.size()],columnVector,valueVector;
  QVector<int> rowposVect,colposVect,valposVect;
  
  for(int i=0;i<rowList.size();i++)
  {
      rowpos=vect.indexOf(Value(rowList.at(i)->text()));
      for(int j=row+1;j<=bottom;j++)
      {
	cell =Cell(sheet,rowpos+1,j);
	if(rowVector.contains(Value(cell.value()))==0)
	{
	  rowVector.append(Value(cell.value()));
	  rowVectorArr[i].append(Value(cell.value()));
      
	}  
      }
      rowposVect.append(rowpos);
  }
    
  for(int i=0;i<columnList.size();i++)
  {
      colpos=vect.indexOf(Value(columnList.at(i)->text()));
      for(int j=row+1;j<=bottom;j++)
      {
	cell =Cell(sheet,colpos+1,j);
	if(columnVector.contains(Value(cell.value()))==0)
	{
 	columnVector.append(Value(cell.value()));
	columnVectorArr[i].append(Value(cell.value()));
	}
      }
      colposVect.append(colpos);
  }
  
  int count=1,count2=0,prevcount=1;
  QVector<Value> rowVect[rowposVect.count()];
  for(int i=0;i<rowposVect.count();i++)
  {
    for(int j=i+1;j<rowposVect.count();j++)
    {
      count*=rowVectorArr[j].count();
    }
    for(int k=0;k<(rowVectorArr[i].count())*prevcount;k++)
    {
      Cell(mySheet,((k)*count)+1+colposVect.count(),i+1).setValue(rowVectorArr[i].at(k%rowVectorArr[i].count()));
     
      for(int l=0;l<count;l++)
	rowVect[i].append(rowVectorArr[i].at(k%rowVectorArr[i].count()));
      
      count2++;
    }
    prevcount=count2;
    count=1;
    count2=0;
  }

  count=1,count2=0,prevcount=1;
  QVector<Value> colVect[colposVect.count()];
  for(int i=0;i<colposVect.count();i++)
  {
    for(int j=i+1;j<colposVect.count();j++)
    {
      count*=columnVectorArr[j].count();
    }
    for(int k=0;k<(columnVectorArr[i].count())*prevcount;k++)
    {
       
      Cell(mySheet,i+1,((k)*count)+1+rowposVect.count()).setValue(columnVectorArr[i].at(k%columnVectorArr[i].count()));
//     Cell(mySheet,i+1,((k)*count)+1+rowposVect.count()).setStyle(st2);
      for(int l=0;l<count;l++)
	colVect[i].append(columnVectorArr[i].at(k%columnVectorArr[i].count()));
      
      count2++;
    }
    
    
    prevcount=count2;
    count=1;
    count2=0;
  } 
  
  // Styling
  
  for(int m=0;m<colVect[0].count();m++)
    {
      Cell(mySheet,1,m+1+rowList.count()).setStyle(stl);
      Cell(mySheet,columnList.count(),m+1+rowList.count()).setStyle(str);
      Cell(mySheet,columnList.count()+rowVect[0].count(),m+1+rowList.count()).setStyle(str);
      
    }
  
  
  for(int m=0;m<rowVect[0].count();m++)
    {
      Cell(mySheet,m+1+columnList.count(),1).setStyle(stt);
      Cell(mySheet,m+1+columnList.count(),rowList.count()).setStyle(stb);
      Cell(mySheet,m+1+columnList.count(),rowList.count()+colVect[0].count()).setStyle(stb);
      
    }
    
   for(int m=0;m<rowList.count();m++)
    {
      Cell(mySheet,columnList.count()+1,m+1).setStyle(stl);
      Cell(mySheet,columnList.count()+rowVect[0].count(),m+1).setStyle(str);
    }
  
  
  for(int m=0;m<columnList.count();m++)
    {
      Cell(mySheet,m+1,rowList.count()+1).setStyle(stt);
      Cell(mySheet,m+1,rowList.count()+colVect[0].count()).setStyle(stb);
    } 
      
    
    
    
   //Styling Done
  
  for(int i=0;i<valueList.size();i++)
  {
     valpos=vect.indexOf(Value(valueList.at(i)->text()));
     valposVect.append(valpos);    
  }
  
  
  QString title=d->func + "-" + valueList.at(0)->text();
  Cell(mySheet,1,1).setValue(Value(title));
  Cell(mySheet,1,1).setStyle(st);
  for(int l=0;l<rowVect[0].count();l++)
  {
    for(int m=0;m<colVect[0].count();m++)
      {

	      QVector<Value> aggregate;
	      for(int k=row+1;k<=bottom;k++)
	      {
		int flag=0;
		for(int i=0;i<rowposVect.count();i++)
		{
		  for(int j=0;j<colposVect.count();j++)
		    {
 
		      if(!(Cell(sheet,rowposVect.at(i)+1,k).value()==rowVect[i].at(l) && Cell(sheet,colposVect.at(j)+1,k).value()==colVect[j].at(m)))
			flag=1;
		      
		    }
		  }
		if(flag==0)
		aggregate.append(Cell(sheet,valpos+1,k).value());
	      }
		if(d->func!="average")
		calc->arrayWalk(aggregate,res,calc->awFunc(d->func),Value(0));
 		
		else
		{
		  calc->arrayWalk(aggregate,res,calc->awFunc("sum"),Value(0));
		  if(aggregate.count()!=0)
		  res=calc->div(res,aggregate.count());
		  
		}
		Cell(mySheet,l+colposVect.count()+1,m+rowposVect.count()+1).setValue(res);
		if(m%2==0)
		  Cell(mySheet,l+colposVect.count()+1,m+rowposVect.count()+1).setStyle(st3);
		
		aggregate.clear();
		res=Value(0);
	    
      }
    }
  
  
  //For Adding the functions: Total Rows & Total Columns
  int colmult=1,rowmult=1;
  
  for(int x=0;x<columnList.size();x++)
  colmult*=columnVectorArr[x].count();
  
  for(int x=0;x<rowList.size();x++)
  rowmult*=rowVectorArr[x].count();
  
  
  
  
  //Totalling Columns
  
  if(d->mainWidget.TotalColumns->isChecked())
  {
    
  Cell(mySheet,1,rowList.size()+colmult+1).setValue(Value("Total Column"));
  
  for(int z=columnList.size()+1;z<=rowmult+columnList.size();z++)
  {
    QVector<Value> vector;
    for(int y=rowList.size()+1;y<=colmult+rowList.size();y++)
    {
      vector.append(Cell(mySheet,z,y).value());
      
    }
    if(d->func!="average")
      calc->arrayWalk(vector,res,calc->awFunc(d->func),Value(0));
    else
    {
      calc->arrayWalk(vector,res,calc->awFunc("sum"),Value(0));
      if(vector.count()!=0)
	  res=calc->div(res,vector.count());
    }
    
    
    Cell(mySheet,z,rowList.size()+colmult+1).setValue(res);
    res=Value(0);
    
  }
  }
  
  
  //Totalling Rows
  if(d->mainWidget.TotalRows->isChecked())
  {
    
  Cell(mySheet,columnList.size()+rowmult+1,1).setValue(Value("Total Row"));
    
  for(int z=rowList.size()+1;z<=colmult+rowList.size();z++)
  {
    QVector<Value> vector;
    for(int y=columnList.size()+1;y<=rowmult+columnList.size();y++)
    {
      vector.append(Cell(mySheet,y,z).value());
      
    }
    
    if(d->func!="average")
      calc->arrayWalk(vector,res,calc->awFunc(d->func),Value(0));
    else
    {
      calc->arrayWalk(vector,res,calc->awFunc("sum"),Value(0));
      if(vector.count()!=0)
	  res=calc->div(res,vector.count());
    }
    
    Cell(mySheet,columnList.size()+rowmult+1,z).setValue(res);
    res=Value(0);
     
  }
  }
  
  //Clearing Vectors
  rowVector.clear();
  columnVector.clear();
  valueVector.clear();
  rowposVect.clear();
  colposVect.clear();
  valposVect.clear();
  
  //Adding built sheet to myMap for viewing
  myMap->addSheet(mySheet);
  
}//Summarize

QVector<QString> PivotMain::ValueData(QString str)
{
  
      Sheet *const sheet = d->selection->lastSheet();
      const QRect range = d->selection->lastRange();
      
      int row = range.top();
      int bottom = range.bottom();
      int r=range.right();
      
      ValueConverter *conv;
    
      for (int i = range.left(); i <= r; ++i) {
	d->posVect.append(Value(Cell(sheet,i,row).value()));
      }
      
      int position=d->posVect.indexOf(Value(str));
     
      for(int j=row+1;j<=bottom;j++)
      {
	if(!Cell(sheet,position+1,j).value().isString())
	{
	
	  if(d->retVect.contains(QString::number(conv->toInteger(Value(Cell(sheet,position+1,j).value()))))==0)
       d->retVect.append(QString::number(conv->toInteger(Value(Cell(sheet,position+1,j).value()))));
	}
	else if(d->retVect.contains(conv->toString(Value(Cell(sheet,position+1,j).value())))==0)
	   d->retVect.append(conv->toString(Value(Cell(sheet,position+1,j).value())));
      }
      return d->retVect;
}//ValueData

void PivotMain::Reset()
{
  d->mainWidget.Rows->clear();
  d->mainWidget.Values->clear();
  d->mainWidget.Columns->clear();
  //d->mainWidget.PageFields->clear();
  extractColumnNames();
}//Reset
void PivotMain::on_Ok_clicked()
{
  
  Summarize();
  QMessageBox msgBox;
  msgBox.setText("Pivot Tables Built");
  msgBox.exec();
}//on_Ok_clicked
