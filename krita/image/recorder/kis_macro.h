/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_MACRO_H_
#define _KIS_MACRO_H_

#include <QList>
#include <QObject>
#include <krita_export.h>

#include "kis_serializable_configuration.h"
#include "kis_types.h"

class KisPlayInfo;
class KisRecordedAction;

/**
 * This is the base class for macro in Krita. It's basically a list of recorded action,KisRecordedAction
 * that can be stored as XML, played, and edited.
 */
class KRITAIMAGE_EXPORT KisMacro : public QObject, public KisSerializableConfiguration
{
    Q_OBJECT
public:
    KisMacro();
    KisMacro(const QList<KisRecordedAction*>& _actions);
    ~KisMacro();
public:
    /**
     * Append a list of actions, and make clone of them.
     */
    void appendActions(const QList<KisRecordedAction*>& actions);
    /**
     * Remove the list of actions. Actions are deleted.
     */
    void removeActions(const QList<KisRecordedAction*>& actions);
    /**
     * Move the action before an other one.
     * @param before if null, the action is moved to the end
     */
    void moveAction(const KisRecordedAction* action, const KisRecordedAction* before);
public:
    /**
     * Play the macro on the image.
     */
    void play(const KisPlayInfo&) const;
public: // serialization functions

    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;
    const QList<KisRecordedAction*>& actions() const;
public slots:
    /**
     * add a single action, and make a clone of it.
     * @param before if non null, the action will be added before that action,
     *               otherwise, if null, the action will be added at the end.
     */
    virtual void addAction(const KisRecordedAction& action, const KisRecordedAction* before = 0);
private:
    struct Private;
    Private* const d;
};

#endif
