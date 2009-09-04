/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KORESOURCEITEMVIEW_H
#define KORESOURCEITEMVIEW_H

#include <QTableView>
#include <KoIconToolTip.h>

class QEvent;
class KoResourceModel;

/// The resource view
class KoResourceItemView : public QTableView
{
public:
    KoResourceItemView( QWidget * parent = 0 );
    virtual ~KoResourceItemView() {}
    /// reimplemented
    virtual void resizeEvent ( QResizeEvent * event );
    /// reimplemented
    virtual bool viewportEvent( QEvent * event );
private:
    KoIconToolTip m_tip;
};

#endif // KORESOURCEITEMVIEW_H
