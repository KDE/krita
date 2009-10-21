/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_EXIF_IO_H_
#define _KIS_EXIF_IO_H_

#include <kis_meta_data_io_backend.h>

#include <klocale.h>

class KisExifIO : public KisMetaData::IOBackend
{
    struct Private;
public:
    KisExifIO();
    virtual ~KisExifIO();
    virtual QString id() const {
        return "exif";
    }
    virtual QString name() const {
        return i18n("Exif");
    }
    virtual BackendType type() const {
        return Binary;
    }
    virtual bool supportSaving() const {
        return true;
    }
    virtual bool saveTo(KisMetaData::Store* store, QIODevice* ioDevice, HeaderType headerType = NoHeader) const;
    virtual bool canSaveAllEntries(KisMetaData::Store* store) const;
    virtual bool supportLoading() const {
        return true;
    }
    virtual bool loadFrom(KisMetaData::Store* store, QIODevice* ioDevice) const;
private:
    Private* const d;
};

#endif
