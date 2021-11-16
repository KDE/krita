/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOEMBEDDEDRESOURCE_H
#define KOEMBEDDEDRESOURCE_H

#include <KoResourceSignature.h>


/**
 * A simple srapper class for a resource that has been embedded into
 * another resource. It stores the buffer with the raw resource data,
 * so that it could be imported into Krita database without any
 * conversions and/or changing MD5 signature of it.
 */
class KRITARESOURCES_EXPORT KoEmbeddedResource
{
public:
    KoEmbeddedResource();
    KoEmbeddedResource(const KoResourceSignature &sig, const QByteArray &data);

    const KoResourceSignature &signature() const;
    QByteArray data() const;

    bool sanityCheckMd5() const;

    bool isValid() const;

private:
    KoResourceSignature m_sig;
    QByteArray m_data;
};

#endif // KOEMBEDDEDRESOURCE_H
