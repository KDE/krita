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

#ifndef FILTERRESOURCESERVERPROVIDER_H
#define FILTERRESOURCESERVERPROVIDER_H

#include "KoResourceServer.h"

class FilterEffectResource;

/// Provides resource server for filter effect resources
class FilterResourceServerProvider : public QObject
{
    Q_OBJECT

public:
    ~FilterResourceServerProvider() override;

    static FilterResourceServerProvider *instance();

    KoResourceServer<FilterEffectResource> *filterEffectServer();

private:

    FilterResourceServerProvider();
    FilterResourceServerProvider(const FilterResourceServerProvider &);
    FilterResourceServerProvider operator=(const FilterResourceServerProvider &);

    static FilterResourceServerProvider *m_singleton;
    KoResourceServer<FilterEffectResource> *m_filterEffectServer;
};

#endif // FILTERRESOURCESERVERPROVIDER_H
