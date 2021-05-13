/*
 * SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

namespace detail {

template< class, class = std::void_t<> >
struct has_prepare_linked_resources : std::false_type { };

template< class T >
struct has_prepare_linked_resources<T, std::void_t<decltype(std::declval<T>().prepareLinkedResources(KisPaintOpSettingsSP(),KisResourcesInterfaceSP()))>> : std::true_type { };

template <typename T>
QList<KoResourceSP> prepareLinkedResources(const KisPaintOpSettingsSP settings,
                                           KisResourcesInterfaceSP resourcesInterface,
                                           std::enable_if_t<has_prepare_linked_resources<T>::value> * = 0)
{
    return T::prepareLinkedResources(settings, resourcesInterface);
}

template <typename T>
QList<KoResourceSP> prepareLinkedResources(const KisPaintOpSettingsSP settings,
                                           KisResourcesInterfaceSP resourcesInterface,
                                           std::enable_if_t<!has_prepare_linked_resources<T>::value> * = 0)
{
    Q_UNUSED(settings);
    Q_UNUSED(resourcesInterface);
    // noop
    return {};
}


template< class, class = std::void_t<> >
struct has_prepare_embedded_resources : std::false_type { };

template< class T >
struct has_prepare_embedded_resources<T, std::void_t<decltype(std::declval<T>().prepareEmbeddedResources(KisPaintOpSettingsSP(),KisResourcesInterfaceSP()))>> : std::true_type { };

template <typename T>
QList<KoResourceSP> prepareEmbeddedResources(const KisPaintOpSettingsSP settings,
                                             KisResourcesInterfaceSP resourcesInterface,
                                             std::enable_if_t<has_prepare_embedded_resources<T>::value> * = 0)
{
    return T::prepareEmbeddedResources(settings, resourcesInterface);
}

template <typename T>
QList<KoResourceSP> prepareEmbeddedResources(const KisPaintOpSettingsSP settings,
                                             KisResourcesInterfaceSP resourcesInterface,
                                             std::enable_if_t<!has_prepare_embedded_resources<T>::value> * = 0)
{
    Q_UNUSED(settings);
    Q_UNUSED(resourcesInterface);
    // noop
    return {};
}

template< class, class = std::void_t<> >
struct has_create_interstroke_data_factory : std::false_type { };

template< class T >
struct has_create_interstroke_data_factory<T, std::void_t<decltype(std::declval<T>().createInterstrokeDataFactory(KisPaintOpSettingsSP(), KisResourcesInterfaceSP()))>> : std::true_type { };

template <typename T>
KisInterstrokeDataFactory* createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface,
                                                        std::enable_if_t<has_create_interstroke_data_factory<T>::value> * = 0)
{
    return T::createInterstrokeDataFactory(settings, resourcesInterface);
}

template <typename T>
KisInterstrokeDataFactory* createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface,
                                                        std::enable_if_t<!has_create_interstroke_data_factory<T>::value> * = 0)
{
    Q_UNUSED(settings);
    Q_UNUSED(resourcesInterface);
    // noop
    return 0;
}

}

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

    QList<KoResourceSP> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) override {
        return detail::prepareLinkedResources<Op>(settings, resourcesInterface);
    }

    QList<KoResourceSP> prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) override {
        return detail::prepareEmbeddedResources<Op>(settings, resourcesInterface);
    }

    KisInterstrokeDataFactory* createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) const override {
        return detail::createInterstrokeDataFactory<Op>(settings, resourcesInterface);
    }

    KisPaintOpSettingsSP createSettings(KisResourcesInterfaceSP resourcesInterface) override {
        KisPaintOpSettingsSP settings = new OpSettings(resourcesInterface);
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
