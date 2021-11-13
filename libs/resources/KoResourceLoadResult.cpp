/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoResourceLoadResult.h"

#include <boost/variant.hpp>

#include <KoResource.h>

struct KoResourceLoadResult::Private
{
    boost::variant<KoResourceSP, KoEmbeddedResource, KoResourceSignature> value;
};

KoResourceLoadResult::KoResourceLoadResult(KoResourceSP resource)
    : m_d(new Private)
{
    m_d->value = resource;
}

KoResourceLoadResult::KoResourceLoadResult(KoEmbeddedResource embeddedResource)
    : m_d(new Private)
{
    m_d->value = embeddedResource;
}

KoResourceLoadResult::KoResourceLoadResult(KoResourceSignature signature)
    : m_d(new Private)
{
    m_d->value = signature;
}

KoResourceLoadResult::KoResourceLoadResult(const KoResourceLoadResult &rhs)
    : m_d(new Private(*rhs.m_d))
{
}

KoResourceLoadResult &KoResourceLoadResult::operator=(const KoResourceLoadResult &rhs)
{
    m_d->value = rhs.m_d->value;
    return *this;
}

KoResourceLoadResult::~KoResourceLoadResult()
{
}

KoResourceSP KoResourceLoadResult::resource() const
{
    return m_d->value.which() == 0 ? boost::get<KoResourceSP>(m_d->value) : KoResourceSP();
}

KoEmbeddedResource KoResourceLoadResult::embeddedResource() const
{
    return m_d->value.which() == 1 ? boost::get<KoEmbeddedResource>(m_d->value) : KoEmbeddedResource();
}

KoResourceSignature KoResourceLoadResult::signature() const
{
    class SignatureVisitor : public boost::static_visitor<KoResourceSignature>
    {
    public:
        KoResourceSignature operator()(KoResourceSP resource) const
        {
            return resource->signature();
        }

        KoResourceSignature operator()(const KoEmbeddedResource &embeddedResource) const
        {
            return embeddedResource.signature();
        }

        KoResourceSignature operator()(const KoResourceSignature &signature) const
        {
            return signature;
        }
    };

    return boost::apply_visitor(SignatureVisitor(), m_d->value);
}

KoResourceLoadResult::Type KoResourceLoadResult::type() const
{
    Type result = FailedLink;

    switch (m_d->value.which())
    {
    case 0:
        result = ExistingResource;
        break;
    case 1:
        result = EmbeddedResource;
        break;
    default:
        result = FailedLink;
    }

    return result;
}
