/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "FilterResourceServerProvider.h"
#include "FilterEffectResource.h"

#include <QFileInfo>
#include <QDir>

#include <KoResourcePaths.h>
#include <KoResourceServerProvider.h>
#include <resources/KoSegmentGradient.h>
#include <resources/KoStopGradient.h>

FilterResourceServerProvider *FilterResourceServerProvider::m_singleton = 0;

FilterResourceServerProvider::FilterResourceServerProvider()
{
    m_filterEffectServer = new KoResourceServerSimpleConstruction<FilterEffectResource>(ResourceType::FilterEffects, "*.svg");
    if (!QFileInfo(m_filterEffectServer->saveLocation()).exists()) {
        QDir().mkpath(m_filterEffectServer->saveLocation());
    }
    m_filterEffectServer->loadTags();
}

FilterResourceServerProvider::~FilterResourceServerProvider()
{
    delete m_filterEffectServer;
}

FilterResourceServerProvider *FilterResourceServerProvider::instance()
{
    if (FilterResourceServerProvider::m_singleton == 0) {
        FilterResourceServerProvider::m_singleton = new FilterResourceServerProvider();
    }
    return FilterResourceServerProvider::m_singleton;
}

KoResourceServer<FilterEffectResource> *FilterResourceServerProvider::filterEffectServer()
{
    return m_filterEffectServer;
}
