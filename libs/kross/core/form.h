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

#include <koffice_export.h>

class KDialog;

namespace Kross {

    /**
     * The Forms class provides access to displayed QWidget objects.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * window = Kross.activeWindow()
     * if window == None:
     *     raise "No window active"
     * form = Kross.createForm(window)
     * form.loadUiFile("/path/to/myuifile.ui")
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
     * The DialogForms class provides access to KDialog objects.
     */
    class KROSS_EXPORT FormDialog : public Form
    {
            Q_OBJECT
        public:
            FormDialog(KDialog* dialog);
            virtual ~FormDialog();

        public slots:

            /**
             * Shows the dialog as a modal dialog, blocking until the user
             * closes it.
             */
            int exec();

            /**
             * Same as the \a exec() method above provided for PyQt-lovers :)
             */
            int exec_loop();

        private:
            KDialog* m_dialog;
    };

}

#endif

