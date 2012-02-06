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
#ifndef MUSIC_CORE_BAR_H
#define MUSIC_CORE_BAR_H

#include <QObject>
#include <QPointF>

namespace MusicCore {

class Sheet;
class Voice;
class VoiceBar;
class Staff;
class StaffElement;

/**
 * A bar (in the US also known as a measure) is a part of a piece of music. A piece of music is a two-dimensional
 * thing, with multiple staffs that are played concurrently and multiple bars that are played sequentially.
 */
class Bar : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new bar in the given sheet of music. The bar is not actually added to the sheet, to do that call
     * the addBar method of the sheet.
     *
     * @param sheet the sheet to create a bar for
     */
    explicit Bar(Sheet* sheet);

    /**
     * Destructor.
     */
    ~Bar();

    /**
     * Returns the sheet this bar is part of.
     */
    Sheet* sheet();

    /**
     * Changes the sheet this bar is part of. This method should not be called after the bar has been added to a sheet.
     *
     * @param sheet the sheet this bar is part of
     */
    void setSheet(Sheet* sheet);

    /**
     * Returns a VoiceBar instance for a specific voice.
     *
     * @param voice the voice for which to return the VoiceBar
     */
    VoiceBar* voice(Voice* voice);

    /**
     * Returns the top-left corner of the bounding box of the bar.
     */
    QPointF position() const;

    /**
     * Returns the (horizontal) size of the bar.
     */
    qreal size() const;
    
    /**
     * The prefix of a bar contains any staff elements at time 0 such as clefs and key signatures.
     * When this bar is the first bar of a staff system the prefix will be drawn at the end of the previous
     * system instead of the start of this system.
     *
     * The prefix is not included in the size of the bar, so the actual size of a bar as it will be drawn is
     * size() + prefix(), and the size a bar wants to have is desiredSize() + prefix(). The actual contents of the
     * bar start at position() + (size(), 0); all coordinates (including staff elements with start time 0) are relative
     * to position(). This means that elements in the prefix have negative x coordinates.
     */
    qreal prefix() const;
    
    /**
     * Set the size of the prefix of this bar. The prefix contains all staff elements at time 0.
     *
     * @param prefix the new size of the prefix of this bar
     */
    void setPrefix(qreal prefix);
    
    /**
     * Returns the position at which the prefix is drawn. This position is relative to the top-left corner of the sheet
     * the bar is in. Most often this is position() - (prefix(), 0).
     */
    QPointF prefixPosition() const;
    
    /**
     * Sets the position of the prefix of this bar.
     *
     * @param pos the new position of the prefix of this bar
     */
    void setPrefixPosition(const QPointF& pos);
    
    /**
     * Returns the desired size of this bar. The desired size is the space all the elements in this bar would ideally use.
     */
    qreal naturalSize() const;

     // number of noteheads of space to associate with shortest note in a measure
    qreal sizeFactor() const;

    /**
     * Returns the number of staff elements in the given staff in this bar.
     */
    int staffElementCount(Staff* staff) const;
    
    /**
     * Returns the staff element with the given index in the given staff in this bar.
     */
    StaffElement* staffElement(Staff* staff, int index);
    
    /**
     * Returns the index of the provided staff element in this bar (this index is not the same as the index that is used in calls to
     * staffElement, as this index is global for the bar, and that index is relative to the elements in a specific staff).
     */
    int indexOfStaffElement(StaffElement* element);
    
    /**
     * Adds a staff element to this bar. The indexHint parameter can be used to provide a hint as to what index the element should be
     * inserted. If inserting at that index does not result in a correct sort order for the staff elements, the indexHint is ignored.
     */
    void addStaffElement(StaffElement* element, int indexHint = -1);
    
    /**
     * Remove a staff element from this bar. If deleteElement is true, the element is not only removed but also deleted.
     */
    void removeStaffElement(StaffElement* element, bool deleteElement = true);
public slots:
    /**
     * Sets the top-left corner of the bounding box of this bar. If setPrefix is true, the position of the prefix is also set relative
     * to the position of the bar.
     */
    void setPosition(const QPointF& position, bool setPrefix=true);
    
    /**
     * Sets the size of the bar.
     *
     * @param size the new size of the bar
     */
    void setSize(qreal size);
signals:
    /**
     * This signal is emitted when the position of the bar is changed.
     */
    void positionChanged(const QPointF& position);
    
    /**
     * This signal is emitted when the size of the bar is changed.
     */
    void sizeChanged(qreal size);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_PART_H
