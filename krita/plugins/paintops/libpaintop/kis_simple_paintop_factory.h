/*
 * Copyright (c) 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_SIMPLE_PAINTOP_FACTORY_H
#define KIS_SIMPLE_PAINTOP_FACTORY_H

#include <kis_paintop_factory.h>
#include <kis_paintop_settings.h>


#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

template <typename T>
struct __impl_has_typedef_needs_preinitialization {
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
    static yes& test(typename C::needs_preinitialization*);

    template <typename>
    static no& test(...);

    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};

template <typename T>
struct has_typedef_needs_preinitialization
        : public boost::integral_constant <bool, __impl_has_typedef_needs_preinitialization<T>::value>
{};

template <typename T>
void preinitializeOpStatically(const KisPaintOpSettingsSP settings, typename boost::enable_if<has_typedef_needs_preinitialization<T> >::type * = 0)
{
    T::preinitializeOpStatically(settings);
}

template <typename T>
void preinitializeOpStatically(const KisPaintOpSettingsSP settings, typename boost::disable_if<has_typedef_needs_preinitialization<T> >::type * = 0)
{
    Q_UNUSED(settings);
}

#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

/**
 * Base template class for simple paintop factories
 */
template <class Op, class OpSettings, class OpSettingsWidget> class KisSimplePaintOpFactory  : public KisPaintOpFactory
{

public:

    KisSimplePaintOpFactory(const QString& id, const QString& name, const QString& category,
                            const QString& pixmap, const QString& model = QString(),
                            const QStringList& whiteListedCompositeOps = QStringList(), int priority = 100)
        : KisPaintOpFactory(whiteListedCompositeOps)
        , m_id(id)
        , m_name(name)
        , m_category(category)
        , m_pixmap(pixmap)
        , m_model(model) {
        setPriority(priority);
    }

    virtual ~KisSimplePaintOpFactory() {
    }

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    void preinitializePaintOpIfNeeded(const KisPaintOpSettingsSP settings) {
        preinitializeOpStatically<Op>(settings);
    }
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

    KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image) {
        const OpSettings *opSettings = dynamic_cast<const OpSettings *>(settings.data());
        Q_ASSERT(settings == 0 || opSettings != 0);

        KisPaintOp * op = new Op(opSettings, painter, node, image);
        Q_CHECK_PTR(op);
        return op;
    }

    KisPaintOpSettingsSP settings() {
        KisPaintOpSettingsSP settings = new OpSettings();
        settings->setModelName(m_model);
        return settings;
    }

    KisPaintOpConfigWidget* createConfigWidget(QWidget* parent) {
        return new OpSettingsWidget(parent);
    }

    QString id() const {
        return m_id;
    }

    QString name() const {
        return m_name;
    }

    QString pixmap() {
        return m_pixmap;
    }

    QString category() const {
        return m_category;
    }
private:
    QString m_id;
    QString m_name;
    QString m_category;
    QString m_pixmap;
    QString m_model;
};

#endif // KIS_SIMPLE_PAINTOP_FACTORY_H
