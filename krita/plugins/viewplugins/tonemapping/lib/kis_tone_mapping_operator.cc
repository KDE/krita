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

#include <KoColorSpaceTraits.h>

#include <kis_bookmarked_configuration_manager.h>
#include <kis_paint_device.h>
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

void KisToneMappingOperator::applyLuminance(KisPaintDeviceSP src, KisPaintDeviceSP lumi, const QRect& r) const
{
    KisHLineIterator itSrc = src->createHLineIterator( r.x(), r.y(), r.width());
    KisHLineIterator itL = lumi->createHLineIterator( 0,0, r.width());
    for(int y = 0; y < r.height(); y++)
    {
        while(not itSrc.isDone())
        {
            KoXyzTraits<float>::Pixel* dataSrc = reinterpret_cast< KoXyzTraits<float>::Pixel* >(itSrc.rawData());
            float* dataL = reinterpret_cast< float* >(itL.rawData());
            float scale = *dataL / dataSrc->Y;
            dataSrc->Y = *dataL;
            dataSrc->X *= scale;
            dataSrc->Z *= scale;
            ++itSrc;
            ++itL;
        }
        itSrc.nextRow();
        itL.nextRow();
    }
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
