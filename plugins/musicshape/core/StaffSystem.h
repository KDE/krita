/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef MUSIC_CORE_STAFFSYSTEM_H
#define MUSIC_CORE_STAFFSYSTEM_H

#include <QtCore/QObject>
#include <QtCore/QList>

namespace MusicCore {

class Sheet;
class Clef;
class Staff;

class StaffSystem : public QObject
{
    Q_OBJECT
public:
    StaffSystem(Sheet* sheet);
    ~StaffSystem();

    double top() const;
    double height() const;
    void setHeight(double height);
    int firstBar() const;
    double indent() const;
    void setIndent(double indent);
    double lineWidth() const;
    void setLineWidth(double width);
    QList<Clef*> clefs() const;
    Clef* clef(Staff* staff) const;
    void setClefs(QList<Clef*> clefs);
public slots:
    void setTop(double top);
    void setFirstBar(int bar);
signals:
    void topChanged(double top);
    void firstBarChanged(int bar);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_STAFFSYSTEM_H
