/***************************************************************************
 * KoScriptManagerAdd.h
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KOKROSS_KOSCRIPTMANAGERADD_H
#define KOKROSS_KOSCRIPTMANAGERADD_H

#include <kassistantdialog.h>
class KoScriptManagerAddWizard;

class QRadioButton;
class KPageWidgetItem;
class KFileWidget;

namespace Kross {
    class ActionCollection;
    class ActionCollectionEditor;
}

/**
* The KoScriptManagerAddTypeWidget widget is the first page within
* the \a KoScriptManagerAddWizard dialog that displays the different
* ways to add resources.
*
* There exist 4 ways to add resources:
*     \li Add script file.
*         The next widget will be a \a KoScriptManagerAddFileWidget
*         widget to choose a script file and then the
*         \a KoScriptManagerAddScriptWidget widget is displayed to
*         tweak the settings for the new \a Action instance before
*         it's added.
*     \li Add collection folder.
*         The next widget will be a \a KoScriptManagerAddCollectionWidget
*         widget to define the settings for the new \a ActionCollection
*         instance before it's added.
*     \li Install script package file.
*         Not done yet. The idea is to allow to add script packages which
*         are simple tarballs + a scripts.rc file that contains the
*         settings for the new \a Action and/or \a ActionCollection
*         instances that should be added.
*     \li Install online script package.
*         Not done yet. Same as the "Install script package file" above
*         except that GetHotNewStuff is used to fetch the script package
*         from an online address.
*/
class KoScriptManagerAddTypeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoScriptManagerAddTypeWidget(KoScriptManagerAddWizard *wizard);
    virtual ~KoScriptManagerAddTypeWidget();

public slots:
    void slotUpdate();

private:
    KoScriptManagerAddWizard *m_wizard;
    QRadioButton *m_scriptCheckbox, *m_collectionCheckbox, *m_installCheckBox, *m_onlineCheckbox;
};

/**
* The KoScriptManagerAddFileWidget widget displays a simple embedded
* KFileDialog to choose a script file (*.py, *.rb, *.js, etc.).
*/
class KoScriptManagerAddFileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoScriptManagerAddFileWidget(KoScriptManagerAddWizard *wizard, const QString &startDirOrVariable = QString());
    virtual ~KoScriptManagerAddFileWidget();
    /// \return the currently selected file.
    QString selectedFile() const;

public slots:
    void slotUpdate();
    void slotFileHighlighted(const QString &file);

private:
    KoScriptManagerAddWizard *const m_wizard;
    KFileWidget *m_filewidget;
    QString m_file;
};

/**
* The KoScriptManagerAddScriptWidget widget displays a \a ActionCollectionEditor
* widget to configure the settings of the new \a Action instance.
*/
class KoScriptManagerAddScriptWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoScriptManagerAddScriptWidget(KoScriptManagerAddWizard *wizard);
    virtual ~KoScriptManagerAddScriptWidget();
public slots:
    void slotUpdate();
    //bool back();
    //bool next();
    bool accept();

private:
    virtual void showEvent(QShowEvent* event);

private:
    KoScriptManagerAddWizard *const m_wizard;
    Kross::ActionCollectionEditor *m_editor;
};

/**
* The KoScriptManagerAddScriptWidget widget displays a \a ActionCollectionEditor
* widget to configure the settings of the new \a ActionCollection instance.
*/
class KoScriptManagerAddCollectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoScriptManagerAddCollectionWidget(KoScriptManagerAddWizard *wizard);
    virtual ~KoScriptManagerAddCollectionWidget();

    QString uniqueName() const;

public slots:
    void slotUpdate();
    bool accept();

private:
    KoScriptManagerAddWizard *m_wizard;
    Kross::ActionCollectionEditor *m_editor;
};

/**
* The KoScriptManagerAddWizard dialog implements a wizard that
* guides the user through the process of adding a new resource
* like a new \a Action or \a ActionCollection instance to
* the scripts-collection.
*/
class KoScriptManagerAddWizard : public KAssistantDialog
{
    Q_OBJECT
public:
    explicit KoScriptManagerAddWizard(QWidget* parent, Kross::ActionCollection* collection = 0);
    virtual ~KoScriptManagerAddWizard();

public slots:
    /// Show the modal wizard dialog.
    virtual int exec();
    /// Called when the user clicks the Back button.
    virtual void back();
    /// Called when the user clicks the Next button.
    virtual void next();
    /// Called when the user clicks the Finish button.
    virtual void accept();

private:
    bool invokeWidgetMethod(const char* member);

private:
    friend class KoScriptManagerAddTypeWidget;
    friend class KoScriptManagerAddFileWidget;
    friend class KoScriptManagerAddScriptWidget;
    friend class KoScriptManagerAddCollectionWidget;

    Kross::ActionCollection *m_collection;
    KPageWidgetItem *m_typeItem, *m_fileItem, *m_scriptItem, *m_collectionItem;
    KoScriptManagerAddTypeWidget *m_typewidget;
    KoScriptManagerAddFileWidget *m_filewidget;
    KoScriptManagerAddScriptWidget *m_scriptwidget;
    KoScriptManagerAddCollectionWidget *m_collectionwidget;
};

#endif
