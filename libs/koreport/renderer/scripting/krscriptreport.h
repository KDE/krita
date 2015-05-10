/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SCRIPTINGKRSCRIPTREPORT_H
#define SCRIPTINGKRSCRIPTREPORT_H

#include <QObject>
#include <kross/core/object.h>

class KoReportReportData;
class KoReportItemBase;
namespace Scripting
{

/**
 @brief Report object user scripting API.

 Contains methods for a report object which can be called by user scripts. \n

 Example: \n
 \code
 function report()
 {
  this.OnOpen = function()
  {
   debug.print("Report opened!");
  }
 }
 reportname.initialize(new report())
 \endcode
*/
class Report : public QObject
{
    Q_OBJECT
public:
    explicit Report(KoReportReportData*);

    ~Report();

public Q_SLOTS:
    //! @return the title of the report as a string
    QString title() const;

    //! @return the name of the report as a string
    QString name() const;

    //! @return the record source (data source, table, query etc) as a string
    QString recordSource() const;

    //! @return an object in the report given its name, or NULL
    QObject* objectByName(const QString &);

    //! @return a section in the report given its name, or NULL
    QObject* sectionByName(const QString &);


    //! Assigns a user object to this report
    void initialize(Kross::Object::Ptr);

    //! Executed when the report is opened.  If a handler exists for this in the user object it is called.
    void eventOnOpen();

    //! Executed when the report is complete.  If a handler exists for this in the user object it is called.
    void eventOnComplete();

    //! Executed when a new page is created.  If a handler exists for this in the user object it is called.
    void eventOnNewPage();

private:
    KoReportReportData *m_reportData;
    Kross::Object::Ptr m_scriptObject;
};

}

#endif
