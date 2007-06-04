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

using namespace Scripting;

Filter::Filter(Module* module, KisFilter* filter)
    : QObject(module)
    , m_filter(filter)
{
    setObjectName("KritaFilter");
}

Filter::~Filter()
{
}

const QString Filter::name()
{
    return m_filter->name();
}

const QVariant Filter::property(const QString& name)
{
    QVariant value;
//     return m_filter->configuration()->getProperty(name, value) ? value : QVariant();
    return value;
}

void Filter::setProperty(const QString& name, const QVariant& value)
{
//     m_filter->configuration()->setProperty(name, value);
}

QVariantMap Filter::properties()
{
//     return m_filter->configuration()->getProperties();
     return QVariantMap();
}

void Filter::fromXML(const QString& xml)
{
//     m_filter->configuration()->fromXML( xml );
}

const QString Filter::toXML()
{
//     return m_filter->configuration()->toString();
     return QString::null;
}

bool Filter::process(QObject* layer)
{
    PaintDevice* paintDevice = dynamic_cast< PaintDevice* >(layer);
    if(! paintDevice || ! m_filter->workWith( paintDevice->paintDevice()->colorSpace()))
    {
        kWarning(41011) << i18n("An error has occurred in %1",QString("process")) << endl;
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
    m_filter->process(paintDevice->paintDevice(), rect, 0/*m_filter->configuration()*/);
    return true;
}

bool Filter::process(QObject* layer, int x, int y, int width, int height)
{
    PaintDevice* paintDevice = dynamic_cast< PaintDevice* >(layer);
    if(! paintDevice || ! m_filter->workWith( paintDevice->paintDevice()->colorSpace()))
    {
        kWarning(41011) << i18n("An error has occurred in %1",QString("process")) << endl;
        return false;
    }
    QRect rect(x, y, width, height);
    m_filter->process(paintDevice->paintDevice(), rect, 0/*m_filter->configuration()*/);
    return true;
}

#include "krs_filter.moc"
