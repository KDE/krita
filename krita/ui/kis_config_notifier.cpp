/*
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_config_notifier.h"

KisConfigNotifier *KisConfigNotifier::s_instance = 0;

KisConfigNotifier::KisConfigNotifier()
{
}

KisConfigNotifier *KisConfigNotifier::instance()
{
    if (s_instance == 0) {
        s_instance = new KisConfigNotifier();
    }
    return s_instance;
}

void KisConfigNotifier::notifyConfigChanged(void)
{
    emit configChanged();
}

#include "kis_config_notifier.moc"

