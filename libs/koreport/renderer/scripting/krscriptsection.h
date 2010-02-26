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

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
namespace Scripting
{
class Section : public QObject
{
    Q_OBJECT
public:
    Section(KRSectionData*);

    ~Section();

public slots:
    /**Returns the background color of the section*/
    QColor backgroundColor();

    /**Sets the background color of the section to the given color
    */
    void setBackgroundColor(const QColor&);

    /**Returns the section height as a real number, in points*/
    qreal height();
    /**Sets the section height to the given value in points*/
    void setHeight(qreal);

    /**Returns the name of the section*/
    QString name();

    /**Returns an object in the section, by number*/
    QObject* objectByNumber(int);
    /**Returns an object in the section, by name*/
    QObject* objectByName(const QString&);

    void initialize(Kross::Object::Ptr);
    void eventOnRender();

private:
    KRSectionData *m_section;
    Kross::Object::Ptr m_scriptObject;
};
}
#endif
