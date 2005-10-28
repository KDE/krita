/* 
 * kis_texture_filter.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef _TEXTURE_FILTER_H
#define _TEXTURE_FILTER_H

#include <qobject.h>
#include <qtimer.h>
#include <kactionclasses.h>

class KisView;

class TextureFilter : public QObject
{
Q_OBJECT
public:
    TextureFilter(KisView* view);
    virtual ~TextureFilter() {}
    
private slots:
    void slotActivated();

private:
    KisView * m_view;
};

#endif // _TEXTURE_FILTER_H
