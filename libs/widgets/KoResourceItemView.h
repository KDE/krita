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

#include <KoTableView.h>
#include <KoIconToolTip.h>

class QEvent;
class QModelIndex;

/// The resource view
class KoResourceItemView : public KoTableView
{
    Q_OBJECT

public:

    explicit KoResourceItemView(QWidget *parent = 0);
    virtual ~KoResourceItemView() { disconnect(); }

    /// reimplemented
    virtual bool viewportEvent(QEvent *event);

Q_SIGNALS:

    void currentResourceChanged(const QModelIndex &);
    void contextMenuRequested(const QPoint &);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    KoIconToolTip m_tip;

};

#endif // KORESOURCEITEMVIEW_H
