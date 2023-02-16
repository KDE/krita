/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISEMBEDDEDTEXTUREDATA_H
#define KISEMBEDDEDTEXTUREDATA_H

#include "kis_types.h"
#include <boost/operators.hpp>
#include <kritapaintop_export.h>
#include <KoPattern.h>

class KisPropertiesConfiguration;


class PAINTOP_EXPORT KisEmbeddedTextureData : boost::equality_comparable<KisEmbeddedTextureData>
{
public:
    inline friend bool operator==(const KisEmbeddedTextureData &lhs, const KisEmbeddedTextureData &rhs) {
        return lhs.md5Base64 == rhs.md5Base64 &&
                lhs.md5sum == rhs.md5sum &&
                lhs.fileName == rhs.fileName &&
                lhs.name == rhs.name &&
                lhs.patternBase64 == rhs.patternBase64;
    }

    QString md5Base64;
    QString md5sum;
    QString fileName;
    QString name;
    QString patternBase64;

    bool isNull() const;

    static KisEmbeddedTextureData fromPattern(KoPatternSP pattern);
    KoResourceLoadResult loadLinkedPattern(KisResourcesInterfaceSP resourcesInterface) const;

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

private:
    KoResourceLoadResult tryFetchPattern(KisResourcesInterfaceSP resourcesInterface) const;
    KoResourceLoadResult tryLoadEmbeddedPattern() const;
};

#endif // KISEMBEDDEDTEXTUREDATA_H
