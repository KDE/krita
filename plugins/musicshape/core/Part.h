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
#ifndef MUSIC_CORE_PART_H
#define MUSIC_CORE_PART_H

#include <QObject>
#include <QString>

namespace MusicCore {

class Staff;
class Voice;
class Sheet;

/**
 * A Part in a piece of music can be for example one instrument. A Part consists of one or more staves (a
 * piano part will typically have two staves, but most other instruments typically have one staff), and one
 * or more voices. Musical elements can be added to a voice+staff combination, where the voice decides where
 * in a bar a musical element should be placed, and the staff controls on what staff to draw the element.
 */
class Part : public QObject {
    Q_OBJECT
public:
    /**
     * Creates a new part in the given sheet with the given name. The part is not added to the sheet, to do that
     * either call Sheet::addPart(Part*) or use Sheet::addPart(QString) to create a new part.
     */
    Part(Sheet* sheet, const QString& name);

    /**
     * Destructor.
     */
    ~Part();

    /**
     * Returns the sheet this part is part of.
     */
    Sheet* sheet();

    /**
     * Return the name of this part.
     */
    QString name() const;

    /**
     * Returns the short name of this part. In typical music the long name of a part is printed before the first
     * staff system, and the short name will be printed in front of the other staff systems. If no short name has been
     * specified, the normal name will be returned if useFull is true, otherwise a null string will be returned.
     */
    QString shortName(bool useFull = true) const;

    /**
     * Returns the number of staves in this part.
     */
    int staffCount() const;

    /**
     * Returns the staff at the given index. If index < 0 or index >= staffCount, this method returns NULL.
     *
     * @param index the index of the staff to return.
     */
    Staff* staff(int index);

    /**
     * Adds a new staff to this part. The staff is added after all existing staves.
     */
    Staff* addStaff();

    void addStaff(Staff* staff);
    
    /**
     * Inserts a new staff into this part. The staff is inserted before the staff with index before.
     *
     * @param before the index of the staff before which the new staff is inserted
     */
    Staff* insertStaff(int before);

    int indexOfStaff(Staff* staff);
    
    void removeStaff(Staff* staff, bool deleteStaff=true);
    
    /**
     * Returns the number of voices in this part. Normally the number of voices will be at least as high as the
     * number of staves, but this is in no way enforced.
     */
    int voiceCount() const;

    /**
     * Returns the voice at the given index.
     *
     * @param index the index of the voice to return.
     */
    Voice* voice(int index);

    /**
     * Adds a voice to this part. The created voice is returned.
     */
    Voice* addVoice();

    int indexOfVoice(Voice* voice);
public slots:
    /**
     * Change the name of this part.
     *
     * @param name the new name of the part
     */
    void setName(const QString& name);

    /**
     * Change the short name of this part.
     *
     * @param shortName the new short name of the part
     */
    void setShortName(const QString& shortName);
signals:
    void nameChanged(const QString& name);
    void shortNameChanged(const QString& shortName);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_PART_H
