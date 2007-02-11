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

#ifndef KOSCRIPTINGDOCKER_H
#define KOSCRIPTINGDOCKER_H

#include <QDockWidget>
#include <KoDockFactory.h>

namespace Kross {
    class GUIClient;
}

#define KOKROSS_EXPORT KDE_EXPORT

class KOKROSS_EXPORT KoScriptingDockerFactory : public KoDockFactory
{
    public:
        KoScriptingDockerFactory(QWidget* parent, Kross::GUIClient* guiclient);
        virtual ~KoScriptingDockerFactory();
        Kross::GUIClient* guiClient() const;
        virtual QString dockId() const;
        virtual Qt::DockWidgetArea defaultDockWidgetArea() const;
        virtual QDockWidget* createDockWidget();
    private:
        class Private;
        Private* const d;
};

class KOKROSS_EXPORT KoScriptingDocker : public QDockWidget
{
        Q_OBJECT
    public:
        KoScriptingDocker(QWidget* parent, Kross::GUIClient* guiclient);
        virtual ~KoScriptingDocker();
        Kross::GUIClient* guiClient() const;
    private slots:
        void runScript();
        void stopScript();
        void configureScript();
    private:
        class Private;
        Private* const d;
};

#endif
