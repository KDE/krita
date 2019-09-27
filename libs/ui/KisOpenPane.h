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
#ifndef KISOPENPANE_H
#define KISOPENPANE_H

#include <QDialog>
#include <QWidget>
#include <QPixmap>
#include <QList>

class KisDetailsPane;
class KisDocument;
class KisOpenPanePrivate;
class KisTemplatesPane;
class QPixmap;
class QString;
class QStringList;
class QTreeWidgetItem;
class QUrl;

/// \internal
class KisOpenPane : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent the parent widget.
     * @param mimeFilter the template-type (group) that should be selected on creation.
     * @param templatesResourcePath the path to the templates.
     */
    KisOpenPane(QWidget *parent, const QStringList& mimeFilter, const QString& templatesResourcePath = QString());
    ~KisOpenPane() override;

    QTreeWidgetItem* addPane(const QString &title, const QString &untranslatedName, const QString &iconName, QWidget *widget, int sortWeight);
    QTreeWidgetItem* addPane(const QString &title, const QString &untranslatedName, const QPixmap& icon, QWidget* widget, int sortWeight);

    /**
     * If the application has a way to create a document not based on a template, but on user
     * provided settings, the widget showing these gets set here.
     * @see KisDocument::createCustomDocumentWidget()
     * @param widget the widget.
     * @param title the title shown in the sidebar
     * @param icon the icon shown in the sidebar
     */
    void addCustomDocumentWidget(QWidget *widget, const QString& title, const QString &untranslatedName, const QString& icon = QString());


Q_SIGNALS:
    /// this signal is emitted (as defined by KisDocument) the moment the document is 'ready'
    void documentSelected(KisDocument*);

protected Q_SLOTS:
    void updateSelectedWidget();
    void itemClicked(QTreeWidgetItem* item);

    /// Saves the splitter sizes for KisDetailsPaneBase based panes
    void saveSplitterSizes(KisDetailsPane* sender, const QList<int>& sizes);

private Q_SLOTS:
    /// when clicked "Open Existing Document" button
    void openFileDialog();
    
Q_SIGNALS:
    void openExistingFile(const QUrl&);
    void openTemplate(const QUrl&);

    /// Emitted when the always use template has changed
    void alwaysUseChanged(KisTemplatesPane* sender, const QString& alwaysUse);

    /// Emitted when one of the detail panes have changed it's splitter
    void splitterResized(KisDetailsPane* sender, const QList<int>& sizes);
    void cancelButton();

protected:
    /**
     * Populate the list with all templates the user can choose.
     * @param templatesResourcePath the template-type (group) that should be selected on creation.
     */
    void initTemplates(const QString& templatesResourcePath);

    // QWidget overrides
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * event) override;

private:
    QStringList m_mimeFilter;

    KisOpenPanePrivate * const d;
};

#endif //KOOPENPANE_H
