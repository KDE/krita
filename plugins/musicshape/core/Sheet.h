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
#ifndef MUSIC_CORE_SHEET_H
#define MUSIC_CORE_SHEET_H

#include <QObject>
#include <QString>

namespace MusicCore {

class Part;
class PartGroup;
class Bar;
class StaffSystem;

/**
 * A sheet is the largest unit in a piece of music. A sheet consists of zero or more
 * parts. A sheet can also contain zero or more part groups, which can be used to group similar parts
 * together (for example all string parts).
 *
 * A sheet also contains bars. Musical elements are always inserted in a specific bar/part combination (where the
 * part is further divided in a staff and a voice).
 */
class Sheet : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor, this will create a sheet containing no parts.
     */
    explicit Sheet(QObject* parent = 0);

    /**
     * Destructor.
     */
    ~Sheet();

    /**
     * Returns the number of parts this piece of music contains.
     */
    int partCount() const;

    /**
     * Returns the part at the given index, or NULL if index < 0 or >= partCount().
     *
     * @param index the index of the part to return
     */
    Part* part(int index);

    /**
     * Returns the index of the given part, or -1 if the part is not found.
     *
     * @param part the part to search for
     */
    int partIndex(Part* part);

    /**
     * Adds a new part to this sheet. The part will have the given name, and will be added after all currently
     * existing parts.
     *
     * @param name the name of the part to create
     */
    Part* addPart(const QString& name);

    /**
     * Adds an existing part to this sheet. The part should not have been added to any other sheet, as adding a
     * part to a sheet gives the sheet ownership of the sheet. When the sheet is deleted, all parts in the sheet will
     * also be deleted.
     *
     * @param part the part to add to the sheet
     */
    void addPart(Part* part);

    /**
     * Inserts a new part into this sheet. The part will be inserted before the part with index before, and be names
     * name.
     *
     * @param before index of the part before which to insert a new part
     * @param name the name of the part to create
     */
    Part* insertPart(int before, const QString& name);

    /**
     * Inserts an existing part into this sheet. The part  should not have been added to any other sheet, as adding
     * a part to a sheet gives the sheet ownership of the sheet. When the sheet is deleted all parts in the sheet will
     * also be deleted.
     *
     * @param before index of the part before which to insert the new part
     * @param part the part to insert into the sheet
     */
    void insertPart(int before, Part* part);

    /**
     * Removes the part with the given index from this sheet. If deletePart is true, the part will also be deleted, if
     * not the caller of this method is expected to delete the part when it is no longer used.
     *
     * @param index the index of the part to remove
     * @param deletePart should the part that is removed also be deleted
     */
    void removePart(int index, bool deletePart = true);

    /**
     * Removes the given part from this sheet. If deletePart is true, the part will also be deleted, if
     * not the caller of this method is expected to delete the part when it is no longer used.
     *
     * @param part the part to remove
     * @param deletePart should the part that is removed also be deleted
     */
    void removePart(Part* part, bool deletePart = true);

    /**
     * Returns the number of groups in this sheet.
     */
    int partGroupCount() const;

    /**
     * Returns the group at the given index. Part groups are ordered by the order in which they were added.
     *
     * @param index the index of the part group to return
     */
    PartGroup* partGroup(int index);

    /**
     * Adds a new part group to this sheet. The part group starts at the part indexed firstPart, and ends at the part
     * indexed lastPart.
     *
     * @param firstPart the index of the first part that is part of the new part group
     * @param lastPart the index of the last part that is part of the new part group
     */
    PartGroup* addPartGroup(int firstPart, int lastPart);

    /**
     * Adds an existing part group to this sheet. A part group should only be added to one sheet as the sheet
     * gains ownership of the group when it is added to the sheet. When the sheet is deleted, all part groups in
     * it are also deleted.
     *
     * @param group the group to add to this sheet
     */
    void addPartGroup(PartGroup* group);

    /**
     * Removes a part group from this sheet. If deleteGroup is false the group is not only removed from the sheet but
     * also deleted, otherwise the caller is responsible for cleaning up the group when it is no longer used.
     *
     * @param group the group to remove from this sheet
     * @param deleteGroup should the group be deleted after removing it
     */
    void removePartGroup(PartGroup* group, bool deleteGroup = true);

    /**
     * Returns the number of bars/measures in this piece of music.
     */
    int barCount() const;

    /**
     * Returns the bar with the given index.
     *
     * @param index the index of the bar to return.
     */
    Bar* bar(int index);

    int indexOfBar(Bar* bar);

    /**
     * Adds count new bars at the end of this piece of music.
     *
     * @param count the number of bars to add to this sheet.
     */
    void addBars(int count);

    /**
     * Adds a new bar at the end of this piece of music.
     */
    Bar* addBar();

    /**
     * Adds an existing bar at the end of this piece of music.
     *
     * @param bar the bar to add
     */
    void addBar(Bar* bar);

    /**
     * Inserts a new bar before the bar with index before.
     *
     * @param before the index of the bar before which to insert the new bar.
     */
    Bar* insertBar(int before);

    /**
     * Inserts an existing bar before the bar with index before.
     *
     * @param before the index of the bar before which to insert the bar.
     * @param bar the bar to insert
     */
    void insertBar(int before, Bar* bar);

    /**
     * Removes a bar from the sheet. If deleteBar is true the bar is not only removed but also deleted.
     *
     * @param index the index of the bar to remove
     * @param deleteBar should the bar also be deleted
     */
    void removeBar(int index, bool deleteBar = true);

    /**
     * Removes one or more bars from the sheet. If deleteBar is true the bars are not only removed but also
     * deleted.
     *
     * @param index the index of the first bar to remove
     * @param count the number of bars to remove
     * @param deleteBar should the bars also be deleted
     */
    void removeBars(int index, int count, bool deleteBar = true);

    StaffSystem* staffSystem(int index);

    void setStaffSystemCount(int count);
    int staffSystemCount();
    
    void updateAccidentals();
signals:
    void partAdded(int index, MusicCore::Part* part);
    void partRemoved(int index, MusicCore::Part* part);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_SHEET_H
