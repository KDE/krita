/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ABSTRACT_INPUT_ACTION_H
#define KIS_ABSTRACT_INPUT_ACTION_H

#include <QHash>

class QPointF;
class QEvent;
class KisInputManager;

/**
 * \brief Abstract base class for input actions.
 *
 * Input actions represent actions to be performed when interacting
 * with the canvas. They are managed by KisInputManager and activated
 * when KisShortcut detects it matches a certain set of inputs.
 *
 * The begin() method uses an index for the type of behaviour to activate.
 * This index can be used to trigger behaviour when different events occur.
 * For example, in the Pan action, this is used to have a single toggle
 * behaviour and four additional options to pan a fixed amount in a certain
 * direction. Each action will always have at least one behaviour.
 */
class KisAbstractInputAction
{
public:
    /**
     * Constructor.
     *
     * \param manager The InputManager this action belongs to.
     */
    explicit KisAbstractInputAction(KisInputManager *manager);
    /**
     * Destructor.
     */
    virtual ~KisAbstractInputAction();

    /**
     * Begin the action.
     *
     * \param shortcut The index of the behaviour to trigger.
     */
    virtual void begin(int shortcut) = 0;
    /**
     * End the action.
     */
    virtual void end() = 0;
    /**
     * Process an input event.
     *
     * \param event An event to process.
     */
    virtual void inputEvent(QEvent* event) = 0;

    /**
     * Does this action handle tablet events in a special way?
     */
    virtual bool handleTablet() const;
    /**
     * The indexes of shortcut behaviours available.
     */
    virtual QHash<QString, int> shortcutIndexes() const;
    /**
     * The name of this action.
     */
    virtual QString name() const;
    /**
     * A short description of this action.
     */
    virtual QString description() const;

    /**
     * Does this action block auto repeat events?
     */
    virtual bool isBlockingAutoRepeat() const;

protected:
    /**
     * The input manager this action belongs to.
     */
    KisInputManager *inputManager() const;
    /**
     * Set the name of this action.
     *
     * \param name The new name.
     */
    void setName(const QString &name);
    /**
     * Set the description of this action.
     *
     * \param description The new description.
     */
    void setDescription(const QString &description);
    /**
     * Set the available indexes of shortcut behaviours.
     *
     * \param indexes The new indexes.
     */
    void setShortcutIndexes(const QHash<QString, int> &indexes);
    /**
     * Return the locally cached mouse position.
     */
    QPointF mousePosition() const;
    /**
     * Set a mouse position to cache locally.
     */
    void setMousePosition(const QPointF &position);

private:
    class Private;
    Private * const d;
};

#endif // KIS_ABSTRACT_INPUT_ACTION_H
