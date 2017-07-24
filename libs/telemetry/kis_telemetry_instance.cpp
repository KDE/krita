/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Kapustin <akapust1n@mail.ru>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "kis_telemetry_instance.h"
#include <kis_assert.h>


Q_GLOBAL_STATIC(KisTelemetryInstance, s_instance)

KisTelemetryInstance *KisTelemetryInstance::instance()
{
    return  s_instance;
}

void KisTelemetryInstance::setProvider(KisTelemetryAbstract *provider)
{
    if(!telemetryProvider.isNull()){
       KIS_SAFE_ASSERT_RECOVER_RETURN(false);
    }
   telemetryProvider.reset(provider);
}

KisTelemetryAbstract *KisTelemetryInstance::provider()
{
    if (telemetryProvider.isNull())
        return nullptr;
    return telemetryProvider.data();
}
