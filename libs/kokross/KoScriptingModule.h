/*
 * This file is part of the KOffice project
 *
 * Copyright (C) 2006-2007 by Sebastian Sauer (mail@dipe.org)
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
 */

#ifndef KOSCRIPTINGMODULE_H
#define KOSCRIPTINGMODULE_H

#include <QObject>
#include <QWidget>
#include <QPointer>

#include <KoView.h>
#include <KoDocument.h>

#define KOKROSS_EXPORT KDE_EXPORT

/**
 * The KoScriptingModule provides the base class for Kross
 * module functionality for KOffice applications.
 */
class KOKROSS_EXPORT KoScriptingModule : public QObject
{
        Q_OBJECT
    public:
        explicit KoScriptingModule(const QString& name);
        virtual ~KoScriptingModule();

        KoView* view() const;
        void setView(KoView* view = 0);

        virtual KoDocument* doc() = 0;

    public Q_SLOTS:

        /** Return the \a KoApplicationAdaptor object. */
        QObject* application();

        /** Return the \a KoMainWindow object. */
        QObject* shell();

        /** Return the \a KMainWindow object. */
        QWidget* mainWindow();

        /** Return the \a KoDocumentAdaptor object. */
        QObject* document();

    private:
        QPointer<KoView> m_view;
};

#endif
