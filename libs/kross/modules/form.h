/***************************************************************************
 * form.h
 * This file is part of the KDE project
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_FORM_H
#define KROSS_FORM_H

#include <QString>
#include <QWidget>

#include <kpagedialog.h>
#include <kfiledialog.h>

#include <koffice_export.h>

namespace Kross {

    /**
     * The FormFileWidget class provides a in a widget embedded KFileDialog.
     */
    class KROSS_EXPORT FormFileWidget : public QWidget
    {
            Q_OBJECT
            Q_ENUMS(Mode)

        public:
            FormFileWidget(QWidget* parent, const QString& startDirOrVariable);
            virtual ~FormFileWidget();

            /**
             * The Mode the FormFileWidget could have.
             */
            typedef enum Mode { Other = 0, Opening, Saving };

        public slots:

            /**
             * Set the \a Mode the FormFileWidget should have to \p mode .
             * Valid modes are "Other", "Opening" or "Saving".
             */
            void setMode(const QString& mode);

            /**
             * \return the current filter.
             */
            QString currentFilter() const;

            /**
             * Set the filter to \p filter .
             */
            void setFilter(QString filter);

            /**
             * \return the current mimetype filter.
             */
            QString currentMimeFilter() const;

            /**
             * Set the mimetype filter to \p filter .
             */
            void setMimeFilter(const QStringList& filter);

            /**
             * \return the currently selected file.
             */
            QString selectedFile() const;

            //QStringList selectedFiles() const { return KFileDialog::selectedFiles(); }
            //QString selectedUrl() const { return KFileDialog::selectedUrl().toLocalFile(); }

        private:
            virtual void showEvent(QShowEvent* event);
            virtual void hideEvent(QHideEvent* event);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * The FormDialog class provides access to KDialog objects as
     * top-level containers.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * forms = Kross.module("forms")
     * mydialog = forms.createDialog("MyDialog")
     * mydialog.setButtons("Ok|Cancel")
     * mydialog.setFaceType("Plain") #Auto Plain List Tree Tabbed
     * mywidget = forms.createWidgetFromUIFile(mydialog, "./mywidget.ui")
     * mywidget["QLineEdit"].setText("some string")
     * if mydialog.exec_loop():
     *     if mydialog.result() == "Ok":
     *         print mywidget["QLineEdit"].text
     * \endcode
     */
    class KROSS_EXPORT FormDialog : public KPageDialog
    {
            Q_OBJECT

        public:
            FormDialog(const QString& caption);
            virtual ~FormDialog();

        public slots:

            /**
             * Set the buttons.
             *
             * \param buttons string that defines the displayed buttons. For example the
             * string may look like "Ok" or "Ok|Cancel" or "Yes|No|Cancel".
             * \return true if the passed \p buttons string was valid and setting the
             * buttons was successfully else false is returned.
             */
            bool setButtons(const QString& buttons);

            /**
             * Set the face type of the dialog.
             *
             * \param facetype the face type which could be "Auto", "Plain", "List",
             * "Tree" or "Tabbed" as defined in \a KPageView::FaceType .
             */
            bool setFaceType(const QString& facetype);

            /**
             * \return the name of the currently selected page. Use the \a page()
             * method to get the matching page QWidget instance.
             */
            QString currentPage() const;

            /**
             * Set the current page to \p name . If there exists no page with
             * such a pagename the method returns false else (if the page was
             * successfully set) true is returned.
             */
            bool setCurrentPage(const QString& name);

            /**
             * \return the QWidget page instance which has the pagename \p name
             * or NULL if there exists no such page.
             */
            QWidget* page(const QString& name) const;

            /**
             * Add and return a new page.
             *
             * \param name The name the page has. This name is for example returned
             * at the \a currentPage() method and should be unique. The name is also
             * used to display a short title for the page.
             * \param header The longer header title text used for display purposes.
             * \param iconname The name of the icon which the page have. This could
             * be for example "about_kde", "fileopen", "configure" or any other
             * iconname known by KDE.
             * \return the new QWidget page instance.
             */
            QWidget* addPage(const QString& name, const QString& header, const QString& iconname);

            /**
             * Shows the dialog as a modal dialog, blocking until the user
             * closes it and returns the execution result.
             *
             * \return >=1 if the dialog was accepted (e.g. "Ok" pressed) else
             * the user rejected the dialog (e.g. by pressing "Cancel" or just
             * closing the dialog by pressing the escape-key).
             */
            int exec() { return KDialog::exec(); }

            /**
             * Same as the \a exec() method above provided for Python-lovers (python
             * does not like functions named "exec" and PyQt named it "exec_loop", so
             * just let's do the same).
             */
            int exec_loop() { return exec(); }

            /**
             * \return the result. The result may for example "Ok", "Cancel", "Yes" or "No".
             */
            QString result();

        private slots:
            virtual void slotButtonClicked(int button);
            void slotCurrentPageChanged(KPageWidgetItem* current);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * The FormModule provides access to UI functionality like dialogs or widgets.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * forms = Kross.module("forms")
     * dialog = forms.createDialog("My Dialog")
     * dialog.setButtons("Ok|Cancel")
     * page = dialog.addPage("Welcome","Welcome Page","fileopen")
     * label = self.forms.createWidget("QLabel", page)
     * label.text = "Hello World Label"
     * if self.dialog.exec_loop():
     *     forms.showMessageBox("Information", "Okay...", "The Ok-button was pressed")
     * \endcode
     */
    class KROSS_EXPORT FormModule : public QObject
    {
            Q_OBJECT

        public:
            FormModule();
            virtual ~FormModule();

        public slots:

            /**
             * \return the active modal widget. Modal widgets are special top-level
             * widgets which are subclasses of QDialog and are modal.
             */
            QWidget* activeModalWidget();

            /**
             * \return the application top-level window that has the keyboard input
             * focus, or NULL if no application window has the focus.
             */
            QWidget* activeWindow();

            /**
             * Show a messagebox.
             *
             * \param dialogtype The type of the dialog which could be one
             * of the following;
             *      \li QuestionYesNo
             *      \li WarningYesNo
             *      \li WarningContinueCancel
             *      \li WarningYesNoCancel
             *      \li Information
             *      \li Sorry
             *      \li Error
             *      \li QuestionYesNoCancel
             * \param caption The caption the messagedialog displays.
             * \param message The message that is displayed in the messagedialog.
             * \return The buttoncode which chould be one of the following;
             *      \li Ok
             *      \li Cancel
             *      \li Yes
             *      \li No
             *      \li Continue
             */
            QString showMessageBox(const QString& dialogtype, const QString& caption, const QString& message);

            /**
             * Show a progressdialog to provide visible feedback on the progress
             * of a slow operation.
             *
             * \param caption The caption the progressdialog displays.
             * \param labelText The displayed label.
             * \return The QProgressDialog widget instance.
             */
            QWidget* showProgressDialog(const QString& caption, const QString& labelText);

            /**
             * Create and return a new \a FormDialog instance.
             *
             * \param caption The displayed caption of the dialog.
             */
            QWidget* createDialog(const QString& caption);

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the parent QWidget the new QWidget is a child of.
             * \param className the name of the class that should be created
             * and returned. For example "QLabel" or "QForm".
             * \param name the objectName the new widget has.
             */
            QWidget* createWidget(QWidget* parent, const QString& className, const QString& name = QString());

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the new QWidget is a child of parent.
             * \param xml the UI XML string used to construct the new widget.
             * \return the new QWidget instance or NULL.
             */
            QWidget* createWidgetFromUI(QWidget* parent, const QString& xml);

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the parent QWidget the new QWidget is a child of.
             * \param filename the full filename of the UI file which is readed
             * and it's UI XML content is used to construct the new widget.
             */
            QWidget* createWidgetFromUIFile(QWidget* parent, const QString& filename);

            /**
             * Create and return a new \a FormFileWidget instance.
             *
             * \param parent the parent QWidget the new \a FormFileWidget instance
             * is a child of.
             * \param startDirOrVariable the start-directory or -variable.
             * \return the new \a FormFileWidget instance or NULL.
             */
            QWidget* createFileWidget(QWidget* parent, const QString& startDirOrVariable);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };
}

#endif

