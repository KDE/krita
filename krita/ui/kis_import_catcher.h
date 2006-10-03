/*
 *  Copyright (c) 2006 Boudewijn Rempt  <boud@valdyas.org>
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

#ifndef KIS_IMPORT_CATCHER_H_
#define KIS_IMPORT_CATCHER_H_

#include <qobject.h>

#include <kurl.h>

#include <kis_types.h>

class KisView;
class KisDoc;

/**
 * This small helper class takes an url and an image; tries to import
 * the image at the url and shove the layers of the imported image
 * into the first image after loading is done.
 *
 * Caveat: this class calls "delete this", which means that you new
 * it and then never touch it again. Thanks you very much.
 */
class KisImportCatcher : QObject {

    Q_OBJECT

public:

    KisImportCatcher(KURL url, KisImageSP image);

public slots:

    void slotLoadingFinished();

private:
    KisDoc * m_doc;
    KisImage * m_image;
    KURL m_url;
};

#endif
