/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoResourceLoadResult.h"

#include <variant>
#include <KisMpl.h>

#include <KoResource.h>

struct KoResourceLoadResult::Private
{
    // XXX: this should take a monostate for null resources
    // AAA: no, it shouldn't, null resource must be a KoResourceSignature only
    std::variant<KoResourceSP, KoEmbeddedResource, KoResourceSignature> value;
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

KoResourceSP KoResourceLoadResult::resource() const noexcept
{
    return std::holds_alternative<KoResourceSP>(m_d->value) ? std::get<KoResourceSP>(m_d->value) : KoResourceSP();
}

KoEmbeddedResource KoResourceLoadResult::embeddedResource() const noexcept
{
    return std::holds_alternative<KoEmbeddedResource>(m_d->value) ? std::get<KoEmbeddedResource>(m_d->value) : KoEmbeddedResource();
}

KoResourceSignature KoResourceLoadResult::signature() const
{
    return std::visit(
        kismpl::overloaded {
            [](const KoResourceSignature &signature) { return signature; },
            [](const KoResourceSP &resource) { return resource->signature(); },
            [](const KoEmbeddedResource &resource) { return resource.signature(); }
        }, m_d->value);
}

KoResourceLoadResult::Type KoResourceLoadResult::type() const
{
    return std::visit(
        kismpl::overloaded {
            [](auto) { return FailedLink; },
            [](const KoResourceSP &) { return ExistingResource; },
            [](const KoEmbeddedResource &) { return EmbeddedResource; }
        }, m_d->value);
}

QDebug operator<<(QDebug debug, const KoResourceLoadResult &result)
{
    QDebugStateSaver saver(debug);
    debug.nospace();

    switch (result.type()) {
    case KoResourceLoadResult::ExistingResource:
        debug << "KoResourceLoadResult(ExistingResource:" << result.signature() << ")";
        break;
    case KoResourceLoadResult::EmbeddedResource:
        debug << "KoResourceLoadResult(EmbeddedResource:" << result.signature() << ")";
        break;
    case KoResourceLoadResult::FailedLink:
        debug << "KoResourceLoadResult(FailedLink:" << result.signature() << ")";
        break;
    }

    return debug;
}
