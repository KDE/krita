/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DROP_BUTTON_H
#define KIS_DROP_BUTTON_H

#include <kis_tool_button.h>


class KisView2;

#include <kis_node.h>

/**
 * A toolbutton that implements a drop target.
 */
class KisDropButton : public KisToolButton
{
    Q_OBJECT
public:
    explicit KisDropButton(QWidget *parent = 0);

signals:

    void createFromNode(const KisNodeSP node);
    void replaceFromNode(const KisNodeSP node);

protected:
    
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    
};

#endif // KIS_DROP_BUTTON_H
