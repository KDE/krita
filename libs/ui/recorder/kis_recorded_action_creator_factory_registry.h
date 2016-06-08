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

#ifndef _KIS_RECORDED_ACTION_CREATOR_FACTORY_REGISTRY_H_
#define _KIS_RECORDED_ACTION_CREATOR_FACTORY_REGISTRY_H_

#include <kritaui_export.h>
#include <KoID.h>

class KisRecordedActionCreatorFactory;

/**
 * This class allow to create a creator for a specific recorded action.
 *
 */
class KRITAUI_EXPORT KisRecordedActionCreatorFactoryRegistry
{
public:
    KisRecordedActionCreatorFactoryRegistry();
    ~KisRecordedActionCreatorFactoryRegistry();
    static KisRecordedActionCreatorFactoryRegistry* instance();
    /**
     * Add a factory of action creator.
     */
    void add(KisRecordedActionCreatorFactory* factory);
    /**
     * @return an creator for the given action, or a null pointer if there is
     *         no factory for that action.
     */
    KisRecordedActionCreatorFactory* get(const QString& _id) const;
    /**
     * @return the list of creators
     */
    QList<KoID> creators() const;
private:
    struct Private;
    Private* const d;
};

#endif
