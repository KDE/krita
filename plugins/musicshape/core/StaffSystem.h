/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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

#include <QObject>
#include <QList>

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

    qreal top() const;
    qreal height() const;
    void setHeight(qreal height);
    int firstBar() const;
    qreal indent() const;
    void setIndent(qreal indent);
    qreal lineWidth() const;
    void setLineWidth(qreal width);
    QList<Clef*> clefs() const;
    Clef* clef(Staff* staff) const;
    void setClefs(QList<Clef*> clefs);
public slots:
    void setTop(qreal top);
    void setFirstBar(int bar);
signals:
    void topChanged(qreal top);
    void firstBarChanged(int bar);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_STAFFSYSTEM_H
