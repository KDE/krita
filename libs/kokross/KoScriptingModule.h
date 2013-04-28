/***************************************************************************
 * KoScriptingModule.h
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

#ifndef KOKROSS_KOSCRIPTINGMODULE_H
#define KOKROSS_KOSCRIPTINGMODULE_H

#include <QObject>

#include "kokross_export.h"

class KoView;
class KoDocument;
class QWidget;

/**
* The KoScriptingModule provides the base class for Kross
* module functionality for Calligra applications.
*
* Each Calligra-application does provide an own implementation for
* this one to extend those module with additional own functionality.
*/
class KOKROSS_EXPORT KoScriptingModule : public QObject
{
    Q_OBJECT
public:
    KoScriptingModule(QObject *parent, const QString &name);
    virtual ~KoScriptingModule();

    KoView *view() const;
    virtual KoDocument *doc() = 0;

public slots:

    /** Return the \a KoApplicationAdaptor object. */
    virtual QObject *application();

    /** Return the \a KoMainWindow object. */
    virtual QObject *shell();

    /** Return the \a KXmlGuiWindow object. */
    virtual QWidget *mainWindow();

    /** Return the \a KoPartAdaptor object. */
    virtual QObject *document();

    /** Return a \a KoScriptingOdfStore object. */
    virtual QObject *store();

private:
    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif
