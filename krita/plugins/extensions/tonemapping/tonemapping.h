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

#ifndef _TONEMAPPING_H_
#define _TONEMAPPING_H_

#include <QVariant>

#include <kparts/plugin.h>

#include "kis_types.h"

class KisView2;

/**
 * Template of view plugin
 */
class tonemappingPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    tonemappingPlugin(QObject *parent, const QVariantList &);
    virtual ~tonemappingPlugin();

private slots:

    void slotToneMapping();
    void slotNodeChanged(const KisNodeSP);

private:

    KisView2 * m_view;
    KAction* m_toneMappingAction;

};

#endif // tonemappingPlugin_H
