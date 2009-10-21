/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "widgets/kis_channelflags_widget.h"

#include <QBitArray>
#include <QScrollArea>
#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>

#include <klocale.h>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>

#include <kis_debug.h>

KisChannelFlagsWidget::KisChannelFlagsWidget(const KoColorSpace * colorSpace, QWidget * parent)
        : QScrollArea(parent)
        , m_colorSpace(colorSpace)
{
    setObjectName("KisChannelFlagsWidget");
    setToolTip(i18n("Check the active channels in this layer. Only these channels will be affected by any operation."));
    QWidget * w = new QWidget();
    setBackgroundRole(QPalette::Window);
    QVBoxLayout * vbl = new QVBoxLayout();

    for (int i = 0; i < colorSpace->channels().size(); ++i) {
        KoChannelInfo * channel = colorSpace->channels().at(i);
        QCheckBox * bx = new QCheckBox(channel->name(), w);
        bx->setCheckState(Qt::Checked);
        vbl->addWidget(bx);
        m_channelChecks.append(bx);
    }

    w->setLayout(vbl);
    setWidget(w);

}

KisChannelFlagsWidget::~KisChannelFlagsWidget()
{
}

void KisChannelFlagsWidget::setChannelFlags(const QBitArray & cf)
{
    dbgUI << "KisChannelFlagsWidget::setChannelFlags " << cf.isEmpty();
    if (cf.isEmpty()) return;

    QBitArray channelFlags = m_colorSpace->setChannelFlagsToColorSpaceOrder(cf);
    for (int i = 0; i < qMin(m_channelChecks.size(), channelFlags.size()); ++i) {
        m_channelChecks.at(i)->setChecked(channelFlags.testBit(i));
    }
}

QBitArray KisChannelFlagsWidget::channelFlags() const
{
    bool allTrue = true;
    QBitArray ba(m_channelChecks.size());

    for (int i = 0; i < m_channelChecks.size(); ++i) {

        bool flag = m_channelChecks.at(i)->isChecked();
        if (!flag) allTrue = false;

        ba.setBit(i, flag);
        dbgUI << " channel " << i << " is " << flag << ", allTrue = " << allTrue << ", so ba.testBit(" << i << ")" << " is " << ba.testBit(i);
    }
    if (allTrue)
        return QBitArray();
    else {
        QBitArray result = m_colorSpace->setChannelFlagsToPixelOrder(ba);
        for (int i = 0; i < result.size(); ++i) {
            dbgUI << "On conversion to the pixel order, flag " << i << " is " << result.testBit(i);
        }
        return result;
    }
}
