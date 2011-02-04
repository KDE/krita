/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_MACRO_PLAYER_H_
#define _KIS_MACRO_PLAYER_H_

#include <QThread>

#include <krita_export.h>
#include <kis_paint_device.h>

class KisMacro;
class KisPlayInfo;
class KoUpdater;

/**
 * This class play a macro inside a thread.
 */
class KRITAIMAGE_EXPORT KisMacroPlayer : public QThread {
    Q_OBJECT
public:
    KisMacroPlayer(KisMacro* _macro, const KisPlayInfo& info, KoUpdater * updater = 0, QObject* _parent = 0);
    virtual ~KisMacroPlayer();
public slots:
    void pause();
    void resume();
protected:
    virtual void run();
private:
    struct Private;
    Private* const d;
};

#endif
