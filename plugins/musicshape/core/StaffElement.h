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
#ifndef MUSIC_CORE_STAFFELEMENT_H
#define MUSIC_CORE_STAFFELEMENT_H

#include <QObject>

namespace MusicCore {

class Staff;
class Bar;

/**
 * This is the base class for all musical elements that can be added to a staff.
 */
class StaffElement : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new StaffElement.
     *
     * @param startTime the starttime of the element
     */
    StaffElement(Staff* staff, int startTime);

    /**
     * Destructor.
     */
    virtual ~StaffElement();

    /**
     * Returns the staff this staff element is part of.
     */
    Staff* staff();
    Bar* bar();
    void setBar(Bar* bar);
    
    /**
     * Returns the x position of this musical element. The x position of an element is measured relative to the left
     * barline of the bar the element is in.
     */
    qreal x() const;

    /**
     * Returns the y position of this musical element. The y position of an element is measure relative to the center
     * of the staff it is in, although some musical elements that have a notion of pitch such as notes/rests/clefs or
     * key signatures might have a different reference point.
     */
    qreal y() const;

    /**
     * Returns the width of this musical element.
     */
    qreal width() const;

    /**
     * Returns the height of this musical element.
     */
    qreal height() const;

    /**
     * Returns the start time of this musical elements in ticks.
     */
    int startTime() const;
    
    /**
     * Returns the priority of this staff element with regard to order in which it should be sorted. The higher the
     * priority of an element is, it is sorted more to the left in a group of element with equal start time.
     */    
    virtual int priority() const = 0;
public slots:
    /**
     * Sets the x position of this musical element.
     */
    void setX(qreal x);

    /**
     * Sets the y position of this musical element.
     */
    void setY(qreal y);

    /**
     * Sets the start time of this musical element.
     */
    void setStartTime(int startTime);
protected slots:
    /**
     * Sets the width of this musical element.
     *
     * @param width the new width of this musical element
     */
    void setWidth(qreal width);

    /**
     * Sets the height of this musical element.
     *
     * @param height the new height of this musical element
     */
    void setHeight(qreal height);
signals:
    void xChanged(qreal x);
    void yChanged(qreal y);
    void startTimeChanged(int startTime);
    void widthChanged(qreal width);
    void heightChanged(qreal height);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_STAFFELEMENT_H
