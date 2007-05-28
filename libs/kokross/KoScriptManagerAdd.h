/***************************************************************************
 * scriptmanageradd.h
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_SCRIPTMANAGERADD_H
#define KROSS_SCRIPTMANAGERADD_H

#include <QtCore/QObject>
#include <QtGui/QWidget>

#include <kassistantdialog.h>

class QRadioButton;
class KPageWidgetItem;

namespace Kross {

    //class Action;
    class ActionCollection;
    class ActionCollectionEditor;
    class ScriptManagerCollection;
    class ScriptManagerAddWizard;
    class FormFileWidget;

    /**
    * The ScriptManagerAddTypeWidget widget is the first page within
    * the \a ScriptManagerAddWizard dialog that displays the different
    * ways to add resources.
    *
    * There exist 4 ways to add resources:
    *     \li Add script file.
    *         The next widget will be a \a ScriptManagerAddFileWidget
    *         widget to choose a script file and then the
    *         \a ScriptManagerAddScriptWidget widget is displayed to
    *         tweak the settings for the new \a Action instance before
    *         it's added.
    *     \li Add collection folder.
    *         The next widget will be a \a ScriptManagerAddCollectionWidget
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
    class ScriptManagerAddTypeWidget : public QWidget
    {
            Q_OBJECT
        public:
            explicit ScriptManagerAddTypeWidget(ScriptManagerAddWizard* wizard);
            virtual ~ScriptManagerAddTypeWidget();
        public Q_SLOTS:
            void slotUpdate();
        private:
            ScriptManagerAddWizard* m_wizard;
            QRadioButton *m_scriptCheckbox, *m_collectionCheckbox, *m_installCheckBox, *m_onlineCheckbox;
    };

    /**
    * The ScriptManagerAddFileWidget widget displays a simple embedded
    * KFileDialog to choose a script file (*.py, *.rb, *.js, etc.).
    */
    class ScriptManagerAddFileWidget : public QWidget
    {
            Q_OBJECT
        public:
            explicit ScriptManagerAddFileWidget(ScriptManagerAddWizard* wizard, const QString& startDirOrVariable = QString());
            virtual ~ScriptManagerAddFileWidget();
            /// \return the currently selected file.
            QString selectedFile() const;
        public Q_SLOTS:
            void slotUpdate();
        private:
            class Private;
            Private* const d;
    };

    /**
    * The ScriptManagerAddScriptWidget widget displays a \a ActionCollectionEditor
    * widget to configure the settings of the new \a Action instance.
    */
    class ScriptManagerAddScriptWidget : public QWidget
    {
            Q_OBJECT
        public:
            explicit ScriptManagerAddScriptWidget(ScriptManagerAddWizard* wizard);
            virtual ~ScriptManagerAddScriptWidget();
        public Q_SLOTS:
            void slotUpdate();
            //bool back();
            //bool next();
            bool accept();
        private:
            virtual void showEvent(QShowEvent* event);
        private:
            class Private;
            Private* const d;
    };

    /**
    * The ScriptManagerAddScriptWidget widget displays a \a ActionCollectionEditor
    * widget to configure the settings of the new \a ActionCollection instance.
    */
    class ScriptManagerAddCollectionWidget : public QWidget
    {
            Q_OBJECT
        public:
            explicit ScriptManagerAddCollectionWidget(ScriptManagerAddWizard* wizard);
            virtual ~ScriptManagerAddCollectionWidget();
        public Q_SLOTS:
            void slotUpdate();
        private:
            ScriptManagerAddWizard* m_wizard;
            ActionCollectionEditor* m_editor;
    };

    /**
    * The ScriptManagerAddWizard dialog implements a wizard that
    * guides the user through the process of adding a new resource
    * like a new \a Action or \a ActionCollection instance to
    * the scripts-collection.
    */
    class ScriptManagerAddWizard : public KAssistantDialog
    {
            Q_OBJECT
            friend class ScriptManagerAddTypeWidget;
            friend class ScriptManagerAddFileWidget;
            friend class ScriptManagerAddScriptWidget;
            friend class ScriptManagerAddCollectionWidget;
        public:
            explicit ScriptManagerAddWizard(QWidget* parent, ActionCollection* collection = 0);
            virtual ~ScriptManagerAddWizard();
        public Q_SLOTS:
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
            ActionCollection* m_collection;
            KPageWidgetItem *m_typeItem, *m_fileItem, *m_scriptItem, *m_collectionItem;
            ScriptManagerAddTypeWidget *m_typewidget;
            ScriptManagerAddFileWidget *m_filewidget;
            ScriptManagerAddScriptWidget *m_scriptwidget;
            ScriptManagerAddCollectionWidget *m_collectionwidget;
    };
}

#endif
