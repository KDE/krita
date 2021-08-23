/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_IPTC_IO_H_
#define _KIS_IPTC_IO_H_

#include <klocalizedstring.h>

#include <kis_meta_data_io_backend.h>

class KisIptcIO : public KisMetaData::IOBackend
{
public:
    KisIptcIO();
    ~KisIptcIO() override;
    QString id() const override
    {
        return "iptc";
    }
    QString name() const override
    {
        return i18n("Iptc");
    }
    BackendType type() const override
    {
        return Binary;
    }
    bool supportSaving() const override
    {
        return true;
    }
    bool saveTo(KisMetaData::Store *store, QIODevice *ioDevice, HeaderType headerType = NoHeader) const override;
    bool canSaveAllEntries(KisMetaData::Store *store) const override;
    bool supportLoading() const override
    {
        return true;
    }
    bool loadFrom(KisMetaData::Store *store, QIODevice *ioDevice) const override;

private:
    void initMappingsTable() const;

private:
    struct Private;
    Private *const d;
};

#endif
