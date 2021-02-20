/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_XMP_IO_H_
#define _KIS_XMP_IO_H_

#include <kis_meta_data_io_backend.h>

#include <klocalizedstring.h>

class KisXMPIO : public KisMetaData::IOBackend
{
    struct Private;
public:
    KisXMPIO();
    ~KisXMPIO() override;
    QString id() const override {
        return "xmp";
    }
    QString name() const override {
        return i18n("XMP");
    }
    BackendType type() const override {
        return Text;
    }
    bool supportSaving() const override {
        return true;
    }
    bool saveTo(KisMetaData::Store* store, QIODevice* ioDevice, HeaderType headerType = NoHeader) const override;
    bool canSaveAllEntries(KisMetaData::Store*) const override {
        return true;
    }
    bool supportLoading() const override {
        return true;
    }
    bool loadFrom(KisMetaData::Store* store, QIODevice* ioDevice) const override;
};

#endif
