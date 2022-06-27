/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KO_ID_H_
#define _KO_ID_H_

#include <QString>
#include <QMetaType>
#include <QDebug>
#include <boost/optional.hpp>

#include <klocalizedstring.h>
#include <KisLazyStorage.h>

/**
 * A KoID is a combination of a user-visible string and a string that uniquely
 * identifies a given resource across languages.
 */
class KoID
{
private:
    struct TranslatedString : public QString
    {
        TranslatedString(const boost::optional<KLocalizedString> &source)
            : QString(!source->isEmpty() ? source->toString() : QString())
        {
        }

        TranslatedString(const QString &value)
            : QString(value)
        {
        }
    };

    using StorageType =
        KisLazyStorage<TranslatedString,
        boost::optional<KLocalizedString>>;

    struct KoIDPrivate {
        KoIDPrivate(const QString &_id, const KLocalizedString &_name)
            : id(_id),
              name(_name)
        {}

        KoIDPrivate(const QString &_id, const QString &_name)
            : id(_id),
              name(StorageType::init_value_tag(), _name)
        {}

        QString id;
        StorageType name;
    };

public:
    KoID()
        : m_d(new KoIDPrivate(QString(), QString()))
    {}

    /**
     * Construct a KoID with the given id, and name, id is the untranslated
     * official name of the id, name should be translatable as it will be used
     * in the UI.
     *
     * @code
     * KoID("id", i18n("name"))
     * @endcode
     */
    explicit KoID(const QString &id, const QString &name = QString())
        : m_d(new KoIDPrivate(id, name))
    {}

    /**
     * Use this constructore for static KoID. as KoID("id", ki18n("name"));
     * the name will be translated the first time it is needed. This is
     * important because static objects are constructed before translations
     * are initialized.
     */
    explicit KoID(const QString &id, const KLocalizedString &name)
        : m_d(new KoIDPrivate(id, name))
    {}

    KoID(const KoID &rhs)
        : m_d(rhs.m_d)
    {
    }

    KoID &operator=(const KoID &rhs)
    {
        if (this != &rhs) {
            m_d = rhs.m_d;
        }
        return *this;
    }

    QString id() const
    {
        return m_d->id;
    }

    QString name() const
    {
        return *m_d->name;
    }

    friend inline bool operator==(const KoID &, const KoID &);
    friend inline bool operator!=(const KoID &, const KoID &);
    friend inline bool operator<(const KoID &, const KoID &);
    friend inline bool operator>(const KoID &, const KoID &);

    static bool compareNames(const KoID &id1, const KoID &id2)
    {
        return id1.name() < id2.name();
    }

private:
    QSharedPointer<KoIDPrivate> m_d;
};

Q_DECLARE_METATYPE(KoID)

inline bool operator==(const KoID &v1, const KoID &v2)
{
    return v1.m_d == v2.m_d || v1.m_d->id == v2.m_d->id;
}

inline bool operator!=(const KoID &v1, const KoID &v2)
{
    return !(v1 == v2);
}

inline bool operator<(const KoID &v1, const KoID &v2)
{
    return v1.m_d->id < v2.m_d->id;
}

inline bool operator>(const KoID &v1, const KoID &v2)
{
    return v1.m_d->id > v2.m_d->id;;
}

inline QDebug operator<<(QDebug dbg, const KoID &id)
{
    dbg.nospace() << id.name() << " (" << id.id() << " )";

    return dbg.space();
}

#endif
