/*
 * This file is part of Krita
 *
 * Copyright (C) 2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCRIPTINGDOCKER_H
#define SCRIPTINGDOCKER_H

#include <QWidget>

class QTreeView;

namespace Kross {
    class GUIClient;
    class ActionCollectionProxyModel;
}

class ScriptingDocker : public QWidget
{
        Q_OBJECT
    public:
        ScriptingDocker(QWidget* parent, Kross::GUIClient* guiclient);
        virtual ~ScriptingDocker();

    private slots:
        void runScript();
        void stopScript();

    private:
        Kross::GUIClient* m_guiclient;
        Kross::ActionCollectionProxyModel* m_model;
        QTreeView* m_view;
};

#endif
