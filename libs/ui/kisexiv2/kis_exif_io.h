/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
