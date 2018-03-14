/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef OFFSETIMAGE_H
#define OFFSETIMAGE_H

#include <QVariant>

#include <KisActionPlugin.h>
#include <kis_types.h>
#include <kis_node.h>

class KUndo2MagicString;


class OffsetImage : public KisActionPlugin
{
    Q_OBJECT
public:
    OffsetImage(QObject *parent, const QVariantList &);
    ~OffsetImage() override;

public Q_SLOTS:

    void slotOffsetImage();
    void slotOffsetLayer();

    void offsetImpl(const KUndo2MagicString &actionName, KisNodeSP node, const QPoint &offsetPoint);
private:
    QRect offsetWrapRect();

};

#endif // OFFSETIMAGE_H
