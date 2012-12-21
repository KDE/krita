/* This file is part of the KDE libraries
   Copyright (C) 2007 Aurélien Gâteau <agateau@kde.org>
   Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

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
 * A thin tool button which can be grouped with another and look like one solid
 * bar:
 *
 * ( button1 | button2 )
 */
class KOWIDGETUTILS_EXPORT KoGroupButton : public QToolButton
{
    Q_OBJECT
    Q_ENUMS( GroupPosition )
    Q_PROPERTY( GroupPosition groupPosition READ groupPosition WRITE setGroupPosition )
public:
    enum GroupPosition {
        NoGroup,
        GroupLeft,
        GroupRight,
        GroupCenter
    };

    explicit KoGroupButton(GroupPosition position, QWidget* parent = 0);
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
