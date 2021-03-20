/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
