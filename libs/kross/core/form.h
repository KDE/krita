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

#include <kdialog.h>

#include <koffice_export.h>

namespace Kross {

    /**
     * The Forms class offers access to the functionality provided by QFormBuilder
     * to be able to work with UI-files.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * window = Kross.activeWindow()
     * if window == None:
     *     raise "No active window"
     * form = Kross.createForm(window)
     * form.loadUiFile("./mywidget.ui")
     * form.show()
     * \endcode
     */
    class KROSS_EXPORT Form : public QWidget
    {
            Q_OBJECT
        public:
            Form(QWidget* parent);
            virtual ~Form();

        public slots:

            /**
             * Load the UI XML from the file \p filename .
             */
            bool loadUiFile(const QString& filename);

            /**
             * Save the UI XML to the file \p filename .
             */
            bool saveUiFile(const QString& filename);

            /**
             * \return the UI XML as a string.
             */
            QString toUiXml();

            /**
             * Set the UI XML from the string \p xml .
             */
            bool fromUiXml(const QString& xml);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * The Dialog class provides access to KDialog objects.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * dialog = Kross.createDialog("MyDialog")
     * dialog.setButtons("Ok|Cancel")
     * dialog.loadUiFile("./mydialog.ui")
     * result = dialog.exec_loop()
     * print result
     * \endcode
     */
    class KROSS_EXPORT Dialog : public KDialog
    {
            Q_OBJECT

        public:
            Dialog(const QString& caption);
            virtual ~Dialog();

        public slots:

            /**
             * Load the UI XML from the file \p filename .
             */
            bool loadUiFile(const QString& filename) { return m_form->loadUiFile(filename); }

            /**
             * Save the UI XML to the file \p filename .
             */
            bool saveUiFile(const QString& filename) { return m_form->saveUiFile(filename); }

            /**
             * \return the UI XML as a string.
             */
            QString toUiXml() { return m_form->toUiXml(); }

            /**
             * Set the UI XML from the string \p xml .
             */
            bool fromUiXml(const QString& xml) { return m_form->fromUiXml(xml); }

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
             * Shows the dialog as a modal dialog, blocking until the user
             * closes it and returns the execution result. The result may
             * for example "Ok", "Cancel", "Yes" or "No".
             */
            int exec() { return KDialog::exec(); }

            /**
             * Same as the \a exec() method above provided for PyQt-lovers :)
             */
            int exec_loop() { return exec(); }


            QString result();

        private slots:
            virtual void slotButtonClicked(int button);
        private:
            Form* m_form;
            KDialog::ButtonCode m_code;
    };

}

#endif

