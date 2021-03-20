/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2007 Aurélien Gâteau <agateau@kde.org>
   SPDX-FileCopyrightText: 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
   SPDX-FileCopyrightText: 2012 Jarosław Staniek <staniek@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KOGROUPBUTTON_H
#define KOGROUPBUTTON_H

#include "kritawidgetutils_export.h"

#include <KisHighlightedToolButton.h>

/**
 * A thin tool button which can be visually grouped with other buttons.
 * 
 * The group can thus look like one solid bar: ( button1 | button2 | button3 )
 * 
 * For grouping layout can be used. For exclusive checkable behaviour assign QButtonGroup on the buttons.
 */
class KRITAWIDGETUTILS_EXPORT KoGroupButton : public KisHighlightedToolButton
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

    ~KoGroupButton() override;

    void setGroupPosition(KoGroupButton::GroupPosition groupPosition);

    KoGroupButton::GroupPosition groupPosition() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    class Private;
    Private *const d;
};

#endif /* KOGROUPBUTTON_H */
