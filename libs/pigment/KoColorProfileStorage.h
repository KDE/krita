/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOCOLORPROFILESTORAGE_H
#define KOCOLORPROFILESTORAGE_H

#include <QScopedPointer>

class QByteArray;
class QString;
class KoColorProfile;
class KoColorSpaceFactory;

/**
 * @brief The KoColorProfileStorage class is a "composite subclass" of
 * KoColorSpaceRegistry that ensures that the access to profiles is guarded
 * by a separate lock and the hierarchy of locks is always followed (which
 * avoid deadlocks).
 *
 * Registry locking hierarchy is basically the following:
 *
 * 1) KoColorSpaceRegistry::Private::registrylock
 * 2) KoColorProfileStorage::Private::lock
 *
 * It means that we can take any single lock if we need it separately, but
 * if we need to take both of them, we should always take them is a specified
 * order.
 *
 * Encapsulation of the profile accesses inside a separate class lets us
 * follow this rule without even thinking of it. KoColorProfileStorage just
 * *never* calls any method of the registry, therefore lock order inversion is
 * not possible,
 */
class KoColorProfileStorage
{
public:
    KoColorProfileStorage();
    ~KoColorProfileStorage();

    /**
     * Register the profile in the storage
     * @param profile the new profile to be registered
     */
    void addProfile(KoColorProfile* profile);
    void addProfile(const KoColorProfile* profile); // TODO

    /**
     * Removes the profile from the storage.
     * Please note that the caller should delete the profile object himself!
     *
     * @param profile the profile to be removed
     */
    void removeProfile(KoColorProfile* profile);

    /**
     * @brief containsProfile shows if a profile is registered in the storage
     */
    bool containsProfile(const KoColorProfile *profile);

    /**
     * Create an alias to a profile with a different name. Then @ref profileByName
     * will return the profile @p to when passed @p name as a parameter.
     */
    void addProfileAlias(const QString& name, const QString& to);

    /**
     * @return the profile alias, or name if not aliased
     */
    QString profileAlias(const QString& name) const;

    /**
     * Return a profile by its given name, or 0 if none registered.
     * @return a profile by its given name, or 0 if none registered.
     * @param name the product name as set on the profile.
     * @see addProfile()
     * @see KoColorProfile::productName()
     */
    const KoColorProfile * profileByName(const QString & name) const ;


    /**
     * Returns a profile by its unique id stored/calculated in the header.
     * The first call to this function might take long, because the map is
     * created on the first use only (atm used by SVG only)
     * @param id unique ProfileID of the profile (MD5 sum of its header)
     * @return the profile or 0 if not found
     */
    const KoColorProfile *profileByUniqueId(const QByteArray &id) const;

    /**
     * Return the list of profiles for a colorspace represented by its factory.
     * Profiles will not work with any color space, you can query which profiles
     * that are registered with this registry can be used in combination with the
     * argument factory.
     * @param csf is a factory for the requested color space
     * @return a list of profiles for the factory
     */
    QList<const KoColorProfile *>  profilesFor(const KoColorSpaceFactory * csf) const;
    /**
     * @brief profilesFor
     * Retun the list of profiles for a colorspace represented by it's colorants and type.
     * @param colorants list of at the least 2 values representing the whitePoint xyY, or 8
     * representing whitePoint, red, green and blue x and y xyY values. If the colorant type
     * is set, the colorants will not be used.
     * @param colorantType the named colorantType, if 2, it's undefined and the colorants will be used.
     * @param transferType the name transfer type, if 2 it's undefined.
     * @param error the margin of error with which the colorants get compared.
     * @return list of available profiles.
     */
    QList<const KoColorProfile *> profilesFor(const QVector<double>& colorants,
                                              int colorantType,
                                              int transferType,
                                              double error = 0.00001);

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KOCOLORPROFILESTORAGE_H
