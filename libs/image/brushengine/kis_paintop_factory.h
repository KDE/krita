/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINTOP_FACTORY_H_
#define KIS_PAINTOP_FACTORY_H_

#include "kis_types.h"
#include "kritaimage_export.h"
#include <QObject>
#include <QString>
#include <QIcon>
#include <QStringList>
#include <kis_threaded_text_rendering_workaround.h>
#include <brushengine/kis_paintop_settings.h>

class KisPainter;
class KisPaintOp;
class QWidget;
class KisPaintOpConfigWidget;
class KisInterstrokeDataFactory;

class KoResource;
using KoResourceSP = QSharedPointer<KoResource>;

class KisResourcesInterface;
using KisResourcesInterfaceSP = QSharedPointer<KisResourcesInterface>;

/**
 * The paintop factory is responsible for creating paintops of the specified class.
 * If there is an optionWidget, the derived paintop itself must support settings,
 * and it's up to the factory to do that.
 */
class KRITAIMAGE_EXPORT KisPaintOpFactory : public QObject
{
    Q_OBJECT

public:

    enum PaintopVisibility {
        AUTO,
        ALWAYS,
        NEVER
    };

    /**
     * @param whiteListedCompositeOps list of compositeops that don't work with this paintop
     */
    KisPaintOpFactory(const QStringList & whiteListedCompositeOps = QStringList());
    ~KisPaintOpFactory() override {}

    static QString categoryStable();

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    virtual void preinitializePaintOpIfNeeded(const KisPaintOpSettingsSP settings);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

    /**
     * Create a KisPaintOp with the given settings and painter.
     * @param settings the settings associated with the input device
     * @param painter the painter used to draw
     * @param node the node used to draw
     * @param image the image used to draw
     */
    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image) = 0;
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString category() const = 0;

    /**
     * @return all the resources linked to \p settings.
     */
    virtual QList<KoResourceSP> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) = 0;

    /**
     * @return all the resources embedded into \p settings. The resources are first tried to be loaded
     * from \p resourcesInterface, and, if it fails, loaded from the embedded data.
     */
    virtual QList<KoResourceSP> prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) = 0;

    virtual KisInterstrokeDataFactory* createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) const;

    /**
     * List of usually hidden compositeops that are useful for this paintop.
     */
    QStringList whiteListedCompositeOps() const;

    /**
     * @brief icon
     * @return the icon to represent this paintop.
     */
    virtual QIcon icon();

    /**
     * Create and return an settings object for this paintop.
     */
    virtual KisPaintOpSettingsSP createSettings(KisResourcesInterfaceSP resourcesInterface) = 0;

    /**
     * create a widget that can display paintop settings
     */
    virtual KisPaintOpConfigWidget* createConfigWidget(QWidget* parent) = 0;

    /**
     * Set the priority of this paintop, as it is shown in the UI; lower number means
     * it will be show more to the front of the list.
     * @param newPriority the priority
     */
    void setPriority(int newPriority);

    int priority() const;

private:
    QStringList m_whiteListedCompositeOps;
    int m_priority;
    PaintopVisibility m_visibility;
};

#endif
