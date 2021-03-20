/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CHANNEL_SEPARATOR_H_
#define _KIS_CHANNEL_SEPARATOR_H_

class KoUpdater;
class KisViewManager;

enum enumSepAlphaOptions {
    COPY_ALPHA_TO_SEPARATIONS = 0,
    DISCARD_ALPHA = 1,
    CREATE_ALPHA_SEPARATION = 2
};


enum enumSepSource {
    CURRENT_LAYER = 0,
    ALL_LAYERS = 1,
    VISIBLE_LAYERS = 2
};

class KisChannelSeparator
{

public:

    KisChannelSeparator(KisViewManager * view);
    virtual ~KisChannelSeparator() {}

    void separate(KoUpdater * progress, enumSepAlphaOptions alphaOps, enumSepSource sourceOps, bool downscale, bool toColor, bool activateCurrentChannel);

private:

    KisViewManager *m_viewManager;

};

#endif
