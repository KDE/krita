/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoEmbeddedResource.h"

#include <KoMD5Generator.h>

KoEmbeddedResource::KoEmbeddedResource()
{
}

KoEmbeddedResource::KoEmbeddedResource(const KoResourceSignature &sig, const QByteArray &data)
    : m_sig(sig),
      m_data(data)
{

}

const KoResourceSignature &KoEmbeddedResource::signature() const
{
    return m_sig;
}

QByteArray KoEmbeddedResource::data() const
{
    return m_data;
}

bool KoEmbeddedResource::sanityCheckMd5() const
{
    return m_sig.md5.isEmpty() || KoMD5Generator::generateHash(m_data) == m_sig.md5;
}

bool KoEmbeddedResource::isValid() const
{
    return !m_data.isEmpty();
}
