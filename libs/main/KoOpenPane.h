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

#include <QWidget>
#include <QPixmap>
#include <QList>

class KConfig;
class KoOpenPanePrivate;
class KComponentData;
class QPixmap;
class KoTemplatesPane;
class KoDetailsPane;
class KUrl;
class QTreeWidgetItem;

/// \internal
class KoOpenPane : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent the parent widget
     * @param instance the KComponentData to be used for KConfig data
     * @param templateType the template-type (group) that should be selected on creation.
     */
    KoOpenPane(QWidget *parent, const KComponentData &instance, const QStringList& mimeFilter, const QString& templateType = QString());
    virtual ~KoOpenPane();

    QTreeWidgetItem* addPane(const QString& title, const QString& icon, QWidget* widget, int sortWeight);
    QTreeWidgetItem* addPane(const QString& title, const QPixmap& icon, QWidget* widget, int sortWeight);

    /**
     * If the application has a way to create a document not based on a template, but on user
     * provided settings, the widget showing these gets set here.
     * @see KoDocument::createCustomDocumentWidget()
     * @param widget the widget.
     * @param title the title shown in the sidebar
     * @param icon the icon shown in the sidebar
     */
    void addCustomDocumentWidget(QWidget *widget, const QString& title = QString(), const QString& icon = QString());

protected slots:
    void updateSelectedWidget();
    void itemClicked(QTreeWidgetItem* item);

    /// Saves the splitter sizes for KoDetailsPaneBase based panes
    void saveSplitterSizes(KoDetailsPane* sender, const QList<int>& sizes);

signals:
    void openExistingFile(const KUrl&);
    void openTemplate(const KUrl&);

    /// Emitted when the always use template has changed
    void alwaysUseChanged(KoTemplatesPane* sender, const QString& alwaysUse);

    /// Emitted when one of the detail panes have changed it's splitter
    void splitterResized(KoDetailsPane* sender, const QList<int>& sizes);

protected:
    void initRecentDocs();
    /**
     * Populate the list with all templates the user can choose.
     * @param templateType the template-type (group) that should be selected on creation.
     */
    void initTemplates(const QString& templateType);

private:
    void initExistingFilesPane(const QStringList& mimeFilter);

    KoOpenPanePrivate * const d;
};

#endif //KOOPENPANE_H
