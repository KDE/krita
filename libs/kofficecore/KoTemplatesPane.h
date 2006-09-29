/* This file is part of the KDE project
   Copyright (C) 2005-2006 Peter Simonsson <psn@linux.se>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KOTEMPLATESPANE
#define KOTEMPLATESPANE

#include "KoDetailsPane.h"

class KoTemplate;
class KoTemplateGroup;

class KoTemplatesPanePrivate;

/**
 * This widget is the right-side part of the template opening widget.
 * The parent widget is initial widget in the document space of each KOffice component.
 * This widget shows a list of templates and can show their details or open it.
 */
class KoTemplatesPane : public KoDetailsPane
{
  Q_OBJECT
  public:
    /**
     * Constructor.
     * @param parent the parent widget
     * @param _instance the instance object for the app
     * @param header string used as header text in the listview
     * @param group the group of templates this widget will show.
     * @param defaultTemplate pointer to the default template. Used to select a
     * template when none has been selected before.
    */
    KoTemplatesPane(QWidget* parent, KInstance* _instance, const QString& header,
                    KoTemplateGroup* group, KoTemplate* defaultTemplate);
    ~KoTemplatesPane();

    /// Returns true if a template in this group was the last one selected
    bool isSelected();

  signals:
    /// Emited when the always use checkbox is selected
    void alwaysUseChanged(KoTemplatesPane* sender, const QString& alwaysUse);

  protected slots:
    void selectionChanged(const QModelIndex& index);
    void openFile(const QModelIndex& index);
    void alwaysUseClicked();
    void changeAlwaysUseTemplate(KoTemplatesPane* sender, const QString& alwaysUse);

  private:
    KoTemplatesPanePrivate* d;
};

#endif
