/* This file is part of the KDE project
   Copyright (C) 2004 Peter Simonsson <psn@linux.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOSELECTACTION_H
#define KOSELECTACTION_H

#include <kaction.h>
#include <kofficeui_export.h>
class KMenu;
class QPoint;

/** An action that provides a menu with items that can be selected.
 * The main difference between this action and a KSelectAction is that
 * it is plugged into a toolbar as a dropdown menu and not as a combobox.
 */
class KOFFICEUI_EXPORT KoSelectAction : public KAction
{
  Q_OBJECT
  public:
    /** Constructs a KoSelectAction with a text and an icon.
     * @param text The text that will be displayed.
     * @param icon The dynamically loaded icon that goes with this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KoSelectAction(const QString& text, const QString& icon, QObject* parent = 0, const char* name = 0);
    /** Same as above, but it also connects a slot to the selectionChanged(int) signal.
     * @param text The text that will be displayed.
     * @param icon The dynamically loaded icon that goes with this action.
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke when a selectionChanged(int) signal is emitted.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KoSelectAction(const QString& text, const QString& icon, const QObject* receiver,
      const char* slot, QObject* parent, const char* name = 0);
    ~KoSelectAction();

    /** Returns a pointer to the popup menu. */
    KMenu* popupMenu() const;
    /** Shows the popup menu.
     * @param global Position at which the popup menu is shown.
     */
    void popup(const QPoint& global);
  
    virtual int plug(QWidget* widget, int index = -1);
    
    /** Returns the index of the currently selected item. */
    virtual int currentSelection();

    /** If the current selection selection should be shown or not in the menu */
    void setShowCurrentSelection(bool show);

  signals:
    /** Emitted when the selection changed */
    void selectionChanged(int);

  public slots:
    /** Set which item that should be selected.
     * @param index Index of item that should be selected
     */
    virtual void setCurrentSelection(int index);
  
  protected slots:
    /** Execute an item. By default it sets the item as selected and emits the selectionChanged signal.
     * @param index Index of the item that should be executed.
     */
    virtual void execute(int index);
        
  private:
    class KoSelectActionPrivate;
    KoSelectActionPrivate* d;
};

#endif
