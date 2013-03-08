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

#ifndef PIVOTMAIN_H
#define PIVOTMAIN_H

#include <KDialog>

#include <sheets/Sheet.h>
namespace Calligra{
  namespace Sheets{

class Selection;
class PivotMain : public KDialog
    {
	Q_OBJECT

    public:
      explicit PivotMain(QWidget* parent,Selection* selection);
      QVector<QString> ValueData(QString str);
      bool checkCondition(QString field,QString condition,QString value,int line);
      Sheet* filter();
      ~PivotMain();

    private slots:
	  
      void on_Options_clicked();

      void on_AddFilter_clicked();
      void on_Ok_clicked();
      void Summarize();
      void Reset();

    private:
      void extractColumnNames();
      PivotMain *ui;
      class Private;
      Private *const d;
    };
  }//Sheets
}//Calligra
#endif // PIVOTMAIN_H
