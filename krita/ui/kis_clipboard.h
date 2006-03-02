/*
 *  kis_clipboard.h - part of Krayon
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __KIS_CLIPBOARD_H_
#define __KIS_CLIPBOARD_H_


#include <qsize.h>
#include "kis_types.h"

class QImage;

/**
 * The Krita clipboard is a clipboard that can store paint devices
 * instead of just qimage's.
 */
class KisClipboard : public QObject {

    Q_OBJECT

public:

    virtual ~KisClipboard();

    static KisClipboard* instance();

    /**
     * Sets the clipboard to the contents of the specified paint device; also
     * set the system clipboard to a QImage representation of the specified 
     * paint device.
     */
    void setClip(KisPaintDeviceSP layer);
    
    /**
     * Get the contents of the clipboard in the form of a paint device.
     */
    KisPaintDeviceSP clip();

    bool hasClip();

    QSize clipSize();
    
private slots:

    void clipboardDataChanged();
private:

    KisClipboard();
    KisClipboard(const KisClipboard &);
    KisClipboard operator=(const KisClipboard &);

    static KisClipboard * m_singleton;

    KisPaintDeviceSP m_clip;
    bool m_hasClip;

    bool m_pushedClipboard;


};

#endif // __KIS_CLIPBOARD_H_
