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

#ifndef KIS_TEMPLATES_PANE_H
#define KIS_TEMPLATES_PANE_H

#include "KisDetailsPane.h"

class KisTemplate;
class KisTemplateGroup;

class KisTemplatesPanePrivate;

/**
 * This widget is the right-side part of the template opening widget.
 * The parent widget is initial widget in the document space of each Calligra component.
 * This widget shows a list of templates and can show their details or open it.
 */
class KisTemplatesPane : public KisDetailsPane
{

    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent the parent widget
     * @param header string used as header text in the listview
     * @param group the group of templates this widget will show.
     * @param defaultTemplate pointer to the default template. Used to select a
     * template when none has been selected before.
    */
    KisTemplatesPane(QWidget* parent, const QString& header,
                    KisTemplateGroup* group, KisTemplate* defaultTemplate);
    ~KisTemplatesPane() override;

    /// Returns true if a template in this group was the last one selected
    bool isSelected();

Q_SIGNALS:
    /// Emitted when the always use checkbox is selected
    void alwaysUseChanged(KisTemplatesPane* sender, const QString& alwaysUse);

protected Q_SLOTS:
    void selectionChanged(const QModelIndex& index) override;

    void openFile() override;
    void openFile(const QModelIndex& index) override;
    void alwaysUseClicked();
    void changeAlwaysUseTemplate(KisTemplatesPane* sender, const QString& alwaysUse);

private:
    KisTemplatesPanePrivate * const d;
};

#endif
