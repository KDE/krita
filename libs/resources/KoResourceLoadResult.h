/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KORESOURCELOADRESULT_H
#define KORESOURCELOADRESULT_H

#include <QSharedPointer>
#include <KoResourceSignature.h>
#include <KoEmbeddedResource.h>

class KoResource;
typedef QSharedPointer<KoResource> KoResourceSP;

class KRITARESOURCES_EXPORT KoResourceLoadResult
{
public:
    enum Type {
        ExistingResource,
        EmbeddedResource,
        FailedLink
    };
public:
    KoResourceLoadResult(KoResourceSP resource);
    KoResourceLoadResult(KoEmbeddedResource embeddedRresource);
    KoResourceLoadResult(KoResourceSignature signature);

    template <typename T, typename = typename std::is_convertible<T*, KoResource*>::type>
    KoResourceLoadResult(QSharedPointer<T> resource)
        : KoResourceLoadResult(KoResourceSP(resource))
    {
    }

    KoResourceLoadResult(const KoResourceLoadResult &rhs);
    KoResourceLoadResult& operator=(const KoResourceLoadResult &rhs);

    ~KoResourceLoadResult();

    /**
     * Returns existing resource that has been loaded from the Krita
     * database.
     *
     * Returns non-null pointer only when `type()` is equal to
     * `ExistingResource`
     */
    KoResourceSP resource() const;

    /**
     * Returns the embedded resource, for which there was no instance
     * has been found in the resource database. This resource should
     * be imported into the database manually.
     *
     * Returns a valid object only when `type()` is equal to
     * `EmbeddedResource`
     */
    KoEmbeddedResource embeddedResource();

    /**
     * Return a signature for the embedded/linked resource. This is
     * the only information available when `type()` is equal to
     * `FailedLink`
     */
    KoResourceSignature signature() const;

    /**
     * Describes the result of the resource loading. A copy of the resource
     * can be either found in the resource database, it can be loaded from
     * some embedded storage (and yet should be imported into the database
     * manually) or it can just fail to be found (e.g. when the resource
     * is not embedded and still not found in the database).
     */
    Type type() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KORESOURCELOADRESULT_H
