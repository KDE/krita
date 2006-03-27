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

#ifndef KOLINESTYLEACTION_H
#define KOLINESTYLEACTION_H

#include "KoSelectAction.h"
#include <koffice_export.h>
/** A line style selection action */
class KOFFICEUI_EXPORT KoLineStyleAction : public KoSelectAction
{
  Q_OBJECT
  public:
    /** Constructs a KoLineStyleAction with a text and an icon.
     * @param text The text that will be displayed.
     * @param icon The dynamically loaded icon that goes with this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KoLineStyleAction(const QString& text, const QString& icon, QObject* parent = 0, const char* name = 0);
    /** Same as above, but it also connects a slot to the selectionChanged(int) signal.
     * @param text The text that will be displayed.
     * @param icon The dynamically loaded icon that goes with this action.
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke when a selectionChanged(int) signal is emited.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KoLineStyleAction(const QString& text, const QString& icon, const QObject* receiver,
      const char* slot, QObject* parent, const char* name = 0);
    ~KoLineStyleAction();
    
  protected:
    /** Draws and adds each item of the menu. */
    void createMenu();
  
  private:
    class KoLineStyleActionPrivate;
    KoLineStyleActionPrivate* d;
};

#endif
