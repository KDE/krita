/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KTOGGLETOOLBARACTION_H
#define KTOGGLETOOLBARACTION_H

#include <ktoggleaction.h>
#include <kritawidgetutils_export.h>

class KToolBar;

/**
 * An action that takes care of everything associated with
 * showing or hiding a toolbar by a menu action. It will
 * show or hide the toolbar with the given name when
 * activated, and check or uncheck itself if the toolbar
 * is manually shown or hidden.
 *
 * If you need to perform some additional action when the
 * toolbar is shown or hidden, connect to the toggled(bool)
 * signal. It will be emitted after the toolbar's
 * visibility has changed, whenever it changes.
 */
class KRITAWIDGETUTILS_EXPORT KToggleToolBarAction : public KToggleAction
{
    Q_OBJECT

public:
    /**
     * Create a KToggleToolbarAction that manages the toolbar
     * named @p toolBarName. This can be either the name of a
     * toolbar in an xml ui file, or a toolbar programmatically
     * created with that name.
     *
     * @param toolBarName The toolbar name.
     * @param text The toolbar hint text.
     * @param parent The action's parent object.
     */
    KToggleToolBarAction(const char *toolBarName, const QString &text, QObject *parent);

    /**
     * Create a KToggleToolbarAction that manages the @p toolBar.
     * This can be either the name of a toolbar in an xml ui file,
     * or a toolbar programmatically created with that name.
     *
     * @param toolBar the toolbar to be managed
     * @param text The action's text
     * @param parent The action's parent object.
     */
    KToggleToolBarAction(KToolBar *toolBar, const QString &text, QObject *parent);

    /**
     * Destroys toggle toolbar action.
     */
    ~KToggleToolBarAction() override;

    /**
     * Returns a pointer to the tool bar it manages.
     */
    KToolBar *toolBar();

    /**
     * Reimplemented from @see QObject.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void slotToggled(bool checked) override;

private:
    class Private;
    Private *const d;
};

#endif
