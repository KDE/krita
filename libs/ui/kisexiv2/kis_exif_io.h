/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_EXIF_IO_H_
#define _KIS_EXIF_IO_H_

#include <kis_meta_data_io_backend.h>

#include <klocalizedstring.h>

class KisExifIO : public KisMetaData::IOBackend
{
    struct Private;
public:
    KisExifIO();
    ~KisExifIO() override;
    QString id() const override {
        return "exif";
    }
    QString name() const override {
        return i18n("Exif");
    }
    BackendType type() const override {
        return Binary;
    }
    bool supportSaving() const override {
        return true;
    }
    bool saveTo(KisMetaData::Store* store, QIODevice* ioDevice, HeaderType headerType = NoHeader) const override;
    bool canSaveAllEntries(KisMetaData::Store* store) const override;
    bool supportLoading() const override {
        return true;
    }
    bool loadFrom(KisMetaData::Store* store, QIODevice* ioDevice) const override;
private:
    Private* const d;
};

#endif
