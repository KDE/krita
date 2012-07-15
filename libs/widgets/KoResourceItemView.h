/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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
    enum ViewMode{
        FIXED_COLUMS,  /// The number of columns is fixed
        FIXED_ROWS     /// The number of rows is fixed
    };
    
    KoResourceItemView( QWidget * parent = 0 );
    virtual ~KoResourceItemView() {}
    
    /** reimplemented
    * This will draw a number of rows based on the number of columns if m_viewMode is FIXED_COLUMS
    * And it will draw a number of columns based on the number of rows if m_viewMode is FIXED_ROWS
    */
    virtual void resizeEvent ( QResizeEvent * event );
    
    /// reimplemented
    virtual bool viewportEvent( QEvent * event );
    
    void setViewMode(ViewMode mode);
    
private:
    KoIconToolTip m_tip;
    ViewMode m_viewMode;
};

#endif // KORESOURCEITEMVIEW_H
