/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_SEPARATE_CHANNELS_PLUGIN_H_
#define _KIS_SEPARATE_CHANNELS_PLUGIN_H_

#include <QVariant>

#include <KisActionPlugin.h>

class KisSeparateChannelsPlugin : public KisActionPlugin
{
    Q_OBJECT
public:
    KisSeparateChannelsPlugin(QObject *parent, const QVariantList &);
    ~KisSeparateChannelsPlugin() override;

private Q_SLOTS:

    void slotSeparate();
};

#endif
