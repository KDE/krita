/***************************************************************************
 * KoScriptingDocker.h
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

#ifndef KOKROSS_KOSCRIPTINGDOCKER_H
#define KOKROSS_KOSCRIPTINGDOCKER_H

#include <KoDockFactoryBase.h>

#include <QDockWidget>
#include <QPointer>
#include <QMap>

namespace Kross {
    class Action;
    class ActionCollectionView;
}

class KoScriptingModule;
class QAction;

/**
* The KoScriptingDockerFactory class implements a factory to
* create \a KoScriptingDocker instances.
*/
class KoScriptingDockerFactory : public KoDockFactoryBase
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
    explicit KoScriptingDockerFactory(QWidget *parent = 0, KoScriptingModule *module = 0, Kross::Action *action = 0);

    /**
    * \return the id the docker has. This will be always "Scripting".
    */
    virtual QString id() const;

    /**
    * \return the default docking area.
    */
    virtual KoDockFactoryBase::DockPosition defaultDockPosition() const;

    /**
    * \return a newly created \a KoScriptingDocker instance.
    */
    virtual QDockWidget *createDockWidget();

private:
    /// The parent QWidget instance.
    QPointer<QWidget> m_parent;
    /// The module, can be 0.
    KoScriptingModule *m_module;
    /// The action, can be 0.
    Kross::Action *m_action;
};

/**
* The KoScriptingDocker class implements a docking widget that displays
* the scripts using the \a Kross::ActionCollectionView widget.
*/
class KoScriptingDocker : public QDockWidget
{
    Q_OBJECT
public:
    /**
    * Constructor.
    *
    * \param parent The parent QWidget of the \a KoScriptingDocker .
    */
    explicit KoScriptingDocker(QWidget *parent = 0);

protected slots:

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
    /// The view we are using to display the collections and there actions.
    Kross::ActionCollectionView *m_view;
    /// The map of actions we are using to display toolbar-buttons like "run" and "stop".
    QMap<QString, QAction*> m_actions;
};

/**
* The KoScriptingActionDocker class implements a docking widget that displays
* a docker and uses a \a Kross::Action instance to create optional widgets
* in it using a scripting language.
*/
class KoScriptingActionDocker : public QDockWidget
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
    KoScriptingActionDocker(KoScriptingModule *module, Kross::Action *action, QWidget *parent = 0);

    /**
    * Destructor.
    */
    virtual ~KoScriptingActionDocker();

public slots:

    /**
    * Returns the widget that should be displayed within this docker.
    */
    QWidget* widget();

    /**
    * Set the widget that should be displayed within this docker.
    */
    void setWidget(QWidget* widget);

signals:
    //void visibilityChanged(bool visible);

private slots:
    void slotVisibilityChanged(bool visible);

private:
    QPointer<KoScriptingModule> m_module;
    Kross::Action *m_action;
};

#endif
