/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINTOP_CONFIG_WIDGET_H_
#define KIS_PAINTOP_CONFIG_WIDGET_H_

#include "kritaimage_export.h"

#include "kis_config_widget.h"
#include "kis_image.h"
#include <kis_debug.h>
#include <kis_properties_configuration.h>

class KisResourcesInterface;
using KisResourcesInterfaceSP = QSharedPointer<KisResourcesInterface>;

class KisPaintopLodLimitations;

/**
 * Base class for widgets that are used to edit and display paintop settings.
 */
class KRITAIMAGE_EXPORT KisPaintOpConfigWidget : public KisConfigWidget
{
    Q_OBJECT

public:
    KisPaintOpConfigWidget(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisPaintOpConfigWidget() override;

    void writeConfigurationSafe(KisPropertiesConfigurationSP config) const;
    void setConfigurationSafe(const KisPropertiesConfigurationSP config);

protected:

    friend class CompositeOpModel;

    void setConfiguration(const KisPropertiesConfigurationSP  config) override = 0;
    virtual void writeConfiguration(KisPropertiesConfigurationSP config) const = 0;

public:


    virtual KisPaintopLodLimitations lodLimitations() const = 0;

    virtual void setImage(KisImageWSP image);
    virtual void setNode(KisNodeWSP node);
    virtual void setResourcesInterface(KisResourcesInterfaceSP resourcesInterface);

    KisResourcesInterfaceSP resourcesInterface() const;

    void setView(KisViewManager *view) override;

    /**
     * Some paintops are more complicated and require full canvas with layers, projections and KisImage etc.
     * Example is duplicate paintop. In this case simple canvas like scratchbox does not work.
     * Every paintop supports the scratchbox by default, override and return false if paintop does not.
     */
    virtual bool supportScratchBox();

protected:
    KisImageWSP m_image;
    KisNodeWSP m_node;

    KisResourcesInterfaceSP m_resourcesInterface;

    mutable int m_isInsideUpdateCall;
};

#endif
