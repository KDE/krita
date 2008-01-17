/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_filter.h"
#include "krs_module.h"
#include "krs_paint_device.h"

#include <kis_filter.h>
#include <kis_paint_layer.h>
#include <kis_filter_configuration.h>
#include <kis_bookmarked_configuration_manager.h>

using namespace Scripting;

Filter::Filter(Module* module, KisFilter* filter)
    : QObject(module)
    , m_filter(filter)
    , m_config(0)
{
    setObjectName("KritaFilter");
}

Filter::~Filter()
{
    delete m_config;
    m_config = 0;
}

KisFilterConfiguration* Filter::config()
{
    /*
    KisBookmarkedConfigurationManager* bm = m_filter->bookmarkManager();
    Q_ASSERT(bm);
    KisFilterConfiguration* c = dynamic_cast<KisFilterConfiguration*>( bm->defaultConfiguration() );
    if( c )
        return c;
    */
    if( ! m_config )
        m_config = new KisFilterConfiguration(m_filter->id(), 0);
    return m_config;
}

const QString Filter::name()
{
    return m_filter->name();
}

const QVariant Filter::property(const QString& name)
{
    return config()->getProperty(name);
}

void Filter::setProperty(const QString& name, const QVariant& value)
{
    config()->setProperty(name, value);
}

QVariantMap Filter::properties()
{
     return config()->getProperties();
}

void Filter::fromXML(const QString& xml)
{
    config()->fromLegacyXML(xml);
}

const QString Filter::toXML()
{
    return config()->toLegacyXML();
}

bool Filter::process(QObject* layer)
{
    PaintDevice* paintDevice = dynamic_cast< PaintDevice* >(layer);
    if(! paintDevice || ! m_filter->workWith( paintDevice->paintDevice()->colorSpace()))
    {
        kWarning(41011) << i18n("An error has occurred in %1",QString("process"));
        return false;
    }

    QRect r1 = paintDevice->paintDevice()->extent();
    QRect rect;
// XXX: Don't get the image from the device anymore
//     if(paintDevice->paintDevice()->image())
//     {
//         QRect r2 = paintDevice->paintDevice()->image()->bounds();
//         rect = r1.intersect(r2);
//     } else {
        rect = r1;
//     }

    //m_filter->process(paintDevice->paintDevice(), rect, config()); //sebsauer, 20080117: crashes with filterstest.rb
    m_filter->process(paintDevice->paintDevice(), rect, m_config ? config() : 0);
    return true;
}

bool Filter::process(QObject* layer, int x, int y, int width, int height)
{
    PaintDevice* paintDevice = dynamic_cast< PaintDevice* >(layer);
    if(! paintDevice || ! m_filter->workWith( paintDevice->paintDevice()->colorSpace()))
    {
        kWarning(41011) << i18n("An error has occurred in %1",QString("process"));
        return false;
    }
    QRect rect(x, y, width, height);

    //m_filter->process(paintDevice->paintDevice(), rect, config()); //sebsauer, 20080117: crashes with filterstest.rb
    m_filter->process(paintDevice->paintDevice(), rect, m_config ? config() : 0);
    return true;
}

#include "krs_filter.moc"
