/* This file is part of the KDE project
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>

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
#ifndef KODETAILSPANE_H
#define KODETAILSPANE_H

#include <k3listview.h>

#include "koDetailsPaneBase.h"
//Added by qt3to4:
#include <QPixmap>
#include <QList>
#include <QEvent>

class KoTemplateGroup;
class KoTemplate;
class KInstance;
class Q3ListViewItem;
class KoRecentDocumentsPanePrivate;
class KoRichTextListItemPrivate;
class KFileItem;
class QPixmap;
class KJob;

class KoTemplatesPanePrivate;

/**
 * This widget is the right-side part of the template opening widget.
 * The parent widget is initial widget in the document space of each KOffice component.
 * This widget shows a list of templates and can show their details or open it.
 */
class KoTemplatesPane : public KoDetailsPaneBase
{
  Q_OBJECT
  public:
    /**
     * Constructor.
     * @param parent the parent widget
     * @param instance the instance object for the app
     * @param group the group of templates this widget will show.
     * @param defaultTemplate pointer to the default template. Used to select a
     * template when none has been selected before.
     */
    KoTemplatesPane(QWidget* parent, KInstance* instance,
                    KoTemplateGroup* group, KoTemplate* defaultTemplate);
    ~KoTemplatesPane();

    /// Returns true if a template in this group was the last one selected
    bool isSelected();

    virtual bool eventFilter(QObject* watched, QEvent* e);

  signals:
    void openTemplate(const QString&);
    /// Emited when the always use checkbox is selected
    void alwaysUseChanged(KoTemplatesPane* sender, const QString& alwaysUse);

    void splitterResized(KoDetailsPaneBase* sender, const QList<int>& sizes);

  public slots:
    void resizeSplitter(KoDetailsPaneBase* sender, const QList<int>& sizes);

  protected slots:
    void selectionChanged(Q3ListViewItem* item);
    void openTemplate();
    void openTemplate(Q3ListViewItem* item);
    void alwaysUseClicked();
    void changeAlwaysUseTemplate(KoTemplatesPane* sender, const QString& alwaysUse);

    void changePalette();

  private:
    KoTemplatesPanePrivate* d;
};


/**
 * This widget is the recent doc part of the template opening widget.
 * The parent widget is initial widget in the document space of each KOffice component.
 * This widget shows a list of recent documents and can show their details or open it.
 */
class KoRecentDocumentsPane : public KoDetailsPaneBase
{
  Q_OBJECT
  public:
    /**
     * Constructor.
     * @param parent the parent widget
     * @param instance the instance object for the app
     */
    KoRecentDocumentsPane(QWidget* parent, KInstance* instance);
    ~KoRecentDocumentsPane();

    virtual bool eventFilter(QObject* watched, QEvent* e);

  signals:
    void openFile(const QString&);

    void splitterResized(KoDetailsPaneBase* sender, const QList<int>& sizes);

  public slots:
    void resizeSplitter(KoDetailsPaneBase* sender, const QList<int>& sizes);

  protected slots:
    void selectionChanged(Q3ListViewItem* item);
    void openFile();
    void openFile(Q3ListViewItem* item);

    void previewResult(KJob* job);
    void updatePreview(const KFileItem* fileItem, const QPixmap& preview);

    void changePalette();

  private:
    KoRecentDocumentsPanePrivate* d;
};

#endif //KODETAILSPANE_H
