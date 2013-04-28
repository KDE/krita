/* This file is part of the KDE libraries
   Copyright (C) 2007 Aurélien Gâteau <agateau@kde.org>
   Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
   Copyright (C) 2012 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KOGROUPBUTTON_H
#define KOGROUPBUTTON_H

#include "kowidgetutils_export.h"

// Qt
#include <QToolButton>

/**
 * A thin tool button which can be visually grouped with other buttons.
 * 
 * The group can thus look like one solid bar: ( button1 | button2 | button3 )
 * 
 * For groupping layout can be used. For exclusive checkable behaviour assign QButtonGroup on the buttons.
 */
class KOWIDGETUTILS_EXPORT KoGroupButton : public QToolButton
{
    Q_OBJECT
    Q_ENUMS( GroupPosition )
    Q_PROPERTY( GroupPosition groupPosition READ groupPosition WRITE setGroupPosition )
public:
    /**
     * Position of the button within the button group what affects the appearance.
     */
    enum GroupPosition {
        NoGroup,     //!< No particular position, gives the button unchanged appearance
        GroupLeft,   //!< The button is at the left of the group, so it would have rounded the left part
        GroupRight,  //!< The button is at the right of the group, so it would have rounded the right part
        GroupCenter  //!< The button is on the center of the group, so it would have separators on both sides
    };

    explicit KoGroupButton(GroupPosition position, QWidget* parent = 0);

    /**
     * Creates button with no NoGroup position.
     */
    explicit KoGroupButton(QWidget* parent = 0);

    virtual ~KoGroupButton();

    void setGroupPosition(KoGroupButton::GroupPosition groupPosition);

    KoGroupButton::GroupPosition groupPosition() const;

protected:
    virtual void paintEvent(QPaintEvent* event);

private:
    class Private;
    Private *const d;
};

#endif /* KOGROUPBUTTON_H */
