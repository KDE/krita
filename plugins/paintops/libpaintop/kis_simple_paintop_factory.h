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

#include <brushengine/kis_paintop_factory.h>
#include <brushengine/kis_paintop_settings.h>
#include <kis_icon.h>
#include <KisCppQuirks.h>

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND

namespace detail {

template< class, class = std::void_t<> >
struct has_preinitialize_statically : std::false_type { };

template< class T >
struct has_preinitialize_statically<T, std::void_t<decltype(std::declval<T>().preinitializeOpStatically(KisPaintOpSettingsSP()))>> : std::true_type { };


template <typename T>
void preinitializeOpStatically(const KisPaintOpSettingsSP settings, typename std::enable_if_t<has_preinitialize_statically<T>::value> * = 0)
{
    T::preinitializeOpStatically(settings);
}

template <typename T>
void preinitializeOpStatically(const KisPaintOpSettingsSP settings, typename std::enable_if_t<!has_preinitialize_statically<T>::value> * = 0)
{
    Q_UNUSED(settings);
    // noop
}

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

    ~KisSimplePaintOpFactory() override {
    }

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    void preinitializePaintOpIfNeeded(const KisPaintOpSettingsSP settings) override {
        detail::preinitializeOpStatically<Op>(settings);
    }
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

    KisPaintOp *createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) override {
        KisPaintOp * op = new Op(settings, painter, node, image);
        Q_CHECK_PTR(op);
        return op;
    }

    KisPaintOpSettingsSP createSettings() override {
        KisPaintOpSettingsSP settings = new OpSettings();
        settings->setModelName(m_model);
        return settings;
    }

    KisPaintOpConfigWidget* createConfigWidget(QWidget* parent) override {
        return new OpSettingsWidget(parent);
    }

    QString id() const override {
        return m_id;
    }

    QString name() const override {
        return m_name;
    }

    QIcon icon() override {
        return KisIconUtils::loadIcon(id());
    }

    QString category() const override {
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
