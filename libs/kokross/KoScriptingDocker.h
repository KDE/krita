/***************************************************************************
 * KoScriptingDocker.h
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

#ifndef KOKROSS_KOSCRIPTINGDOCKER_H
#define KOKROSS_KOSCRIPTINGDOCKER_H

#include <QDockWidget>
#include <KoDockFactory.h>

#include "kokross_export.h"

namespace Kross {
    class Action;
}

class KoScriptingModule;

/**
* The KoScriptingDockerFactory class implements a factory to
* create \a KoScriptingDocker instances.
*/
class KOKROSS_EXPORT KoScriptingDockerFactory : public KoDockFactory
{
    public:

        /**
        * Constructor.
        *
        * \param parent The parent QWidget of the \a KoScriptingDocker .
        * \param module The optional \a KoScriptingModule instance.
        * \param action The optional action. If this is NULL we will use a
        * \a KoScriptingDocker class else a \a KoScriptingActionDocker is used.
        */
        explicit KoScriptingDockerFactory(QWidget* parent, KoScriptingModule* module = 0, Kross::Action* action = 0);

        /**
        * Destructor.
        */
        virtual ~KoScriptingDockerFactory();

        /**
        * \return the id the docker has. This will be always "Scripting".
        */
        virtual QString id() const;

        /**
        * \return the default docking area.
        */
        virtual Qt::Dock defaultDockPosition() const;

        /**
        * \return a newly created \a KoScriptingDocker instance.
        */
        virtual QDockWidget* createDockWidget();

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
};

/**
* The KoScriptingDocker class implements a docking widget that displays
* the scripts using the \a Kross::ActionCollectionView widget.
*/
class KOKROSS_EXPORT KoScriptingDocker : public QDockWidget
{
        Q_OBJECT
    public:

        /**
        * Constructor.
        *
        * \param parent The parent QWidget of the \a KoScriptingDocker .
        */
        explicit KoScriptingDocker(QWidget* parent);

        /**
        * Destructor.
        */
        virtual ~KoScriptingDocker();

    protected Q_SLOTS:

        /**
        * This slow got called if the "Script Manager" toolbar-button
        * got activated.
        */
        void slotShowScriptManager();

        /**
        * This slot got called if the enabled-state of the run or stop
        * actions the used \a Kross::ActionCollectionView provides us
        * changed.
        */
        void slotEnabledChanged(const QString&);

        /**
        * This slot got called on doubleclick on the used
        * \a Kross::ActionCollectionView instance and executes the
        * selected action.
        */
        void slotDoubleClicked();

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
};

/**
* The KoScriptingActionDocker class implements a docking widget that displays
* a docker and uses a \a Kross::Action instance to create optional widgets
* in it using a scripting language.
*/
class KOKROSS_EXPORT KoScriptingActionDocker : public QDockWidget
{
        Q_OBJECT
    public:

        /**
        * Constructor.
        *
        * \param parent The parent QWidget of the \a KoScriptingDocker .
        * \param module The \a KoScriptingModule instance.
        * \param action The action the docker should dacorate. This action
        * will be used to create the widgets, etc. in the docker using scripts.
        */
        KoScriptingActionDocker(QWidget* parent, KoScriptingModule* module, Kross::Action* action);

        /**
        * Destructor.
        */
        virtual ~KoScriptingActionDocker();

    public Q_SLOTS:

        /**
        * Returns the widget that should be displayed within this docker.
        */
        QWidget* widget();
    
        /**
        * Set the widget that should be displayed within this docker.
        */
        void setWidget(QWidget* widget);

    Q_SIGNALS:
        //void visibilityChanged(bool visible);

    private Q_SLOTS:
        void slotVisibilityChanged(bool visible);

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
};

#endif
