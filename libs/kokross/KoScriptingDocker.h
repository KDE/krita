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

#define KOKROSS_EXPORT KDE_EXPORT

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
        */
        explicit KoScriptingDockerFactory(QWidget* parent);

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

#endif
