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
#ifndef KOOPENPANE_H
#define KOOPENPANE_H

#include <koOpenPaneBase.h>
//Added by qt3to4:
#include <QPixmap>
#include <QList>

class KoCustomDocumentCreator;
class KConfig;
class KoTemplateGroup;
class KoOpenPanePrivate;
class KInstance;
class QPixmap;
class K3ListViewItem;
class KoTemplatesPane;
class KoDetailsPaneBase;

class KoOpenPane : public KoOpenPaneBase
{
  Q_OBJECT

  public:
    /**
     * Constructor
     * @param parent the parent widget
     * @param instance the KInstance to be used for KConfig data
     * @param templateType the template-type (group) that should be selected on creation.
     */
    KoOpenPane(QWidget *parent, KInstance* instance, const QString& templateType = QString::null);
    virtual ~KoOpenPane();

    Q3ListViewItem* addPane(const QString& title, const QString& icon, QWidget* widget, int sortWeight);
    Q3ListViewItem* addPane(const QString& title, const QPixmap& icon, QWidget* widget, int sortWeight);

    /**
     * If the application has a way to create a document not based on a template, but on user
     * provided settings, the widget showing these gets set here.
     * @see KoDocument::createCustomDocumentWidget()
     * @param widget the widget.
     */
    void setCustomDocumentWidget(QWidget *widget);

  protected slots:
    void showOpenFileDialog();

    void selectionChanged(Q3ListViewItem* item);
    void itemClicked(Q3ListViewItem* item);

    /// Saves the splitter sizes for KoDetailsPaneBase based panes
    void saveSplitterSizes(KoDetailsPaneBase* sender, const QList<int>& sizes);

  signals:
    void openExistingFile(const QString&);
    void openTemplate(const QString&);

    /// Emitted when the always use template has changed
    void alwaysUseChanged(KoTemplatesPane* sender, const QString& alwaysUse);

    /// Emitted when one of the detail panes have changed it's splitter
    void splitterResized(KoDetailsPaneBase* sender, const QList<int>& sizes);

  protected:
    void initRecentDocs();
     /**
      * Populate the list with all templates the user can choose.
      * @param templateType the template-type (group) that should be selected on creation.
      */
    void initTemplates(const QString& templateType);

  private:
    KoOpenPanePrivate* d;
};

#endif //KOOPENPANE_H
