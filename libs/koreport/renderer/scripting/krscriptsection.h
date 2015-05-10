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
#ifndef KRSCRIPTSECTION_H
#define KRSCRIPTSECTION_H

#include <QObject>
#include <krsectiondata.h>
#include <kross/core/object.h>

namespace Scripting
{

/**
 @brief Report section object user scripting API.

 Contains methods for a report section object which can be called by user scripts. \n

 Example: \n
 \code
 function detail()
 {
  this.OnRender = function()
  {
   debug.print("Rendering detail section!");
  }
 }
 reportname.section_detail.initialize(new detail())
 \endcode
*/

class Section : public QObject
{
    Q_OBJECT
public:
    explicit Section(KRSectionData*);

    ~Section();

public Q_SLOTS:
    //! @return the background color of the section
    QColor backgroundColor() const;

    //! Sets the background color of the section to the given color
    void setBackgroundColor(const QColor&);

    //! @return the section height as a real number, in points
    qreal height() const;

    //! Sets the section height to the given value in points
    void setHeight(qreal);

    //! @return the name of the section
    QString name() const;

    //! @return an object in the section, by number
    QObject* objectByNumber(int);

    //! @return an object in the section, by name
    QObject* objectByName(const QString&);

    //! Assigns a user object to this section
    void initialize(Kross::Object::Ptr);

    //! Executed when the report is opened.  If a handler exists for this in the user object it is called.
    void eventOnRender();

private:
    KRSectionData *m_section;
    Kross::Object::Ptr m_scriptObject;
};
}
#endif
