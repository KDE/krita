/*
 *  Copyright (C) 2013 Juan Palacios <jpalaciosdev@gmail.com>
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

#ifndef KOSIZEGROUP_H
#define KOSIZEGROUP_H

#include <QObject>
#include "kowidgetutils_export.h"

class QWidget;
class KoSizeGroupPrivate;

/**
 * KoSizeGroup provides a mechanism to group widgets together so they all
 * request the same amount of space. Also, the widgets will share the same
 * amount of minimum space. The mode of KoSizeGroup determines the direction of
 * the space that are affected by the size group.
 *
 * All widgets inside of KoSizeGroup will use the same size hint value, computed
 * as the maximum of all of his size hint values. The same value is used for the
 * minimum size of all widgets. When KoSizeGroup ignore hidden widgets, the
 * size of widgets that are not visible don't count in the computation of the
 * current size value. When one of these widgets becomes visible again, a new
 * size value is computed and applied to all visible widgets.
 *
 * KoSizeGroup cannot share the same widget with other size groups, so one
 * widget can be in one, and only one, KoSizeGroup at time.
 *
 * NOTE: Added widgets in size groups must be laid out inside of a valid
 * layout. The current implementation sopports widgets laid out inside of
 * QGridLayout and QFormLayout. If the parent widget layout is not one of them,
 * then the group size will not affect the widget size.
 */
class KOWIDGETUTILS_EXPORT KoSizeGroup : public QObject
{
    Q_OBJECT

public:
    /**
     * Determines the direction in which the size group affects the requested
     * and minimum sizes of his component widgets.
     */
    enum mode
    {
        KO_SIZE_GROUP_NONE = 0,                                                 //! group has no effect
        KO_SIZE_GROUP_HORIZONTAL = 1 << 0,                                      //! group affects horizontal size
        KO_SIZE_GROUP_VERTICAL = 1 << 1,                                        //! group affects vertical size
        KO_SIZE_GROUP_BOTH = (KO_SIZE_GROUP_HORIZONTAL | KO_SIZE_GROUP_VERTICAL)//! group affects horizontal and vertical size
    };

    /**
     * Creates a new size group.
     *
     * By default, the mode of the size group is KO_SIZE_GROUP_HORIZONTAL and
     * the group will not ignore hidden widgets.
     */
    explicit KoSizeGroup(QObject* parent = 0,
                         KoSizeGroup::mode mode = KoSizeGroup::KO_SIZE_GROUP_HORIZONTAL,
                         bool ignoreHidden = false);

    ~KoSizeGroup();

    /// Changes the group size mode.
    void setMode(KoSizeGroup::mode mode);

    /// Returns the current mode of the group size.
    KoSizeGroup::mode getMode() const;

    /// Sets whether the group will ignore not visible widgets
    void setIgnoreHidden(bool ignoreHidden);

    /// Returns whether the group ignores not visible widgets
    bool isIgnoreHidden() const;

    /// Adds a new widget to the group.
    /// WARNING: adding the same widget to multiple size groups is not supported!
    void addWidget(QWidget *widget);

    /// Removes a widget from the size group.
    void removeWidget(QWidget *widget);

private:
    KoSizeGroupPrivate* const d;
};

#endif // KOSIZEGROUP_H
