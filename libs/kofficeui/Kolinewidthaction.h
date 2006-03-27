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

#ifndef KOLINEWIDTHACTION_H
#define KOLINEWIDTHACTION_H

#include <kdialogbase.h>

#include <KoUnit.h>
#include <KoSelectAction.h>
#include <koffice_export.h>

/** A line width selection action */
class KOFFICEUI_EXPORT KoLineWidthAction : public KoSelectAction
{
  Q_OBJECT
  public:
    /** Constructs a KoLineWidthAction with a text and an icon.
     * @param text The text that will be displayed.
     * @param icon The dynamically loaded icon that goes with this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KoLineWidthAction(const QString& text, const QString& icon, QObject* parent = 0, const char* name = 0);
    /** Same as above, but it also connects a slot to the selectionChanged(int) signal.
     * @param text The text that will be displayed.
     * @param icon The dynamically loaded icon that goes with this action.
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke when a lineWidthChanged(double) signal is emited.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KoLineWidthAction(const QString& text, const QString& icon, const QObject* receiver,
      const char* slot, QObject* parent, const char* name = 0);
    ~KoLineWidthAction();

    /** Returns the currently selected line width */
    double currentWidth() const;

  signals:
    /** Emited when a new line width have been selected */
    void lineWidthChanged(double);

  public slots:
    /** Set the current width.
     * @param width The new width.
     */
    void setCurrentWidth(double width);
    /** Set which unit to use in the custom width dialog.
     * @param unit The unit to use.
     */
    void setUnit(KoUnit::Unit unit);

  protected slots:
    /** Reimplemented from KoSelectAction.
     * Emits lineWidthChanged(double) when a new width is selected.
     * @param index Index of the selected item
     */
    void execute(int index);

  protected:
    /** Draws and adds each item of the menu. */
    void createMenu();

  private:
    class KoLineWidthActionPrivate;
    KoLineWidthActionPrivate* d;
};

/** This class provides a dialog for setting a custom line width. */
class KoLineWidthChooser : public KDialogBase
{
  Q_OBJECT
  public:
    KoLineWidthChooser(QWidget* parent = 0, const char* name = 0);
    ~KoLineWidthChooser();

    /** Returns the selected line width in points. */
    double width() const;

  public slots:
    /** Set unit to use when showing the width.
     * @param unit Unit to use.
     */
    void setUnit(KoUnit::Unit unit);
    /** Set line width.
     * @param width Line width in points.
     */
    void setWidth(double width);

  private:
    class KoLineWidthChooserPrivate;
    KoLineWidthChooserPrivate* d;
};

#endif
