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

#ifndef KORECENTDOCUMENTSPANE_H
#define KORECENTDOCUMENTSPANE_H

#include "KoDetailsPane.h"

class Q3ListViewItem;
class KFileItem;
class QPixmap;
class KJob;

class KoRecentDocumentsPanePrivate;

/**
 * This widget is the recent doc part of the template opening widget.
 * The parent widget is initial widget in the document space of each KOffice component.
 * This widget shows a list of recent documents and can show their details or open it.
 */
class KoRecentDocumentsPane : public KoDetailsPane
{
  Q_OBJECT
  public:
    /**
     * Constructor.
     * @param parent the parent widget
     * @param _instance the instance object for the app
     * @param header string used as header text in the listview
     */
    KoRecentDocumentsPane(QWidget* parent, KInstance* _instance, const QString& header);
    ~KoRecentDocumentsPane();

  protected slots:
    void selectionChanged(Q3ListViewItem* item);
    void openFile(Q3ListViewItem* item);

    void previewResult(KJob* job);
    void updatePreview(const KFileItem* fileItem, const QPixmap& preview);

  private:
    KoRecentDocumentsPanePrivate* d;
};

#endif
