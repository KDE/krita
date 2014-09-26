/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>
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

#ifndef _COLORSLIDER_H_
#define _COLORSLIDER_H_

#include <QObject>
#include <QVariant>

class KisView2;

/**
 * Template of view plugin
 */
class ColorSliderPlugin : public QObject
{
    Q_OBJECT
public:
    ColorSliderPlugin(QObject *parent, const QVariantList &);
    virtual ~ColorSliderPlugin();
private:
    KisView2* m_view;
};

#endif
