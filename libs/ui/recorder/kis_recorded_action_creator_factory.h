/*
 *  Copyright (c) 2009,2011 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_RECORDED_ACTION_CREATOR_FACTORY_H_
#define _KIS_RECORDED_ACTION_CREATOR_FACTORY_H_

#include <kritaui_export.h>

class KisRecordedAction;
class KisRecordedActionCreator;
class QString;
class QWidget;

/**
 * This class allows to create widgets that are used to create new actions.
 */
class KRITAUI_EXPORT KisRecordedActionCreatorFactory
{
public:
    KisRecordedActionCreatorFactory(const QString& _id, const QString& _name);
    virtual ~KisRecordedActionCreatorFactory();
    QString id() const;
    QString name() const;
    /**
     * @return true if the creation of this action require the use of a creator widget
     */
    virtual bool requireCreator() const = 0;
    /**
     * Create an creator for the action.
     */
    virtual KisRecordedActionCreator* createCreator(QWidget* parent) const;
    /**
     * Create an action. If the action require a creator, it should return 0.
     */
    virtual KisRecordedAction* createAction() const;
private:
    struct Private;
    Private* const d;
};

#endif
