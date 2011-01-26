/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CHANNELDOCKER_H
#define CHANNELDOCKER_H

#include <QObject>
#include <QVariant>

class KisView2;

/**
 * Docker showing the channels of the current layer
 */
class ChannelDockerPlugin : public QObject
{
    Q_OBJECT
public:
    ChannelDockerPlugin(QObject *parent, const QVariantList &);
    virtual ~ChannelDockerPlugin();
};

#endif
