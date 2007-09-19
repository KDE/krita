/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tone_mapping_operator.h"

#include <QString>

#include <kis_bookmarked_configuration_manager.h>
#include <kis_properties_configuration.h>

struct KisToneMappingOperator::Private {
    KisBookmarkedConfigurationManager* bookmarkManager;
    QString id;
    QString name;
};

KisToneMappingOperator::KisToneMappingOperator(QString _id, QString _name) : d(new Private)
{
    d->id = _id;
    d->name = _name;
    d->bookmarkManager = (new KisBookmarkedConfigurationManager(configEntryGroup(), new KisPropertiesConfigurationFactory() ));
}

KisToneMappingOperator::~KisToneMappingOperator()
{
    delete d->bookmarkManager;
    delete d;
}

QString KisToneMappingOperator::id() const
{
    return d->id;
}

QString KisToneMappingOperator::name() const
{
    return d->name;
}

KisToneMappingOperatorConfigurationWidget* KisToneMappingOperator::createConfigurationWidget(QWidget*) const
{
    return 0;
}

QString KisToneMappingOperator::configEntryGroup()
{
    return id() + "_tone_mapping_operator_bookmarks";
}

KisBookmarkedConfigurationManager* KisToneMappingOperator::bookmarkManager()
{
    return d->bookmarkManager;
}
