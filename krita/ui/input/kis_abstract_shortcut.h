/*
 *  Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ABSTRACT_SHORTCUT_H
#define __KIS_ABSTRACT_SHORTCUT_H

#include <Qt>
#include <QList>
#include <krita_export.h>
class KisAbstractInputAction;


class KRITAUI_EXPORT KisAbstractShortcut
{
public:
    KisAbstractShortcut(KisAbstractInputAction *action, int index);
    virtual ~KisAbstractShortcut();

    /**
     * The priority of the shortcut. The shortcut with the
     * greatest value will be chosen for executution
     */
    virtual int priority() const = 0;

    /**
     * The action associated with this shortcut.
     */
    KisAbstractInputAction* action() const;

    /**
     * Set the action associated with this shortcut.
     */
    void setAction(KisAbstractInputAction *action);

    /**
     * The index of the shortcut.
     *
     * \see KisAbstractInputAction::begin()
     */
    int shortcutIndex() const;

    /**
     * Set the index of the shortcut.
     */
    void setShortcutIndex(int index);

protected:
    bool compareKeys(const QList<Qt::Key> &keys1,
                     const QList<Qt::Key> &keys2);

private:
    class Private;
    Private * const m_d;
};

#endif /* __KIS_ABSTRACT_SHORTCUT_H */
