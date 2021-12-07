/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *  SPDX-FileCopyrightText: 2004-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_CONFIG_WIDGET_H_
#define _KIS_CONFIG_WIDGET_H_

#include <QWidget>
#include <kritaimage_export.h>

#include "kis_signal_compressor.h"
#include <kis_properties_configuration.h>

class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;


class KisViewManager;

/**
 * Empty base class. Configurable resources like filters, paintops etc.
 * can build their own configuration widgets that inherit this class.
 * The configuration widget should emit sigConfigurationItemChanged
 * when it wants a preview updated; there is a timer that
 * waits a little time to see if there are more changes coming
 * and then emits sigConfigurationUpdated.
 */
class KRITAIMAGE_EXPORT KisConfigWidget : public QWidget
{

    Q_OBJECT

protected:

    KisConfigWidget(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags(), int delay = 200);

public:
    ~KisConfigWidget() override;

    /**
     * @param config the configuration for this configuration widget.
     */
    virtual void setConfiguration(const KisPropertiesConfigurationSP  config) = 0;

    /**
     * @return the configuration
     */
    virtual KisPropertiesConfigurationSP configuration() const = 0;

    /**
     * Sets the view object that can be used by the configuration
     * widget for richer functionality
     */
    virtual void setView(KisViewManager *view);

    virtual void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    virtual KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

Q_SIGNALS:

    /**
     * emitted whenever it makes sense to update the preview
     */
    void sigConfigurationUpdated();

    /**
     * Subclasses should emit this signal whenever the preview should be
     * be recalculated. This kicks of a timer, so it's perfectly fine
     * to connect this to the changed signals of the widgets in your configuration
     * widget.
     */
    void sigConfigurationItemChanged();
    void sigSaveLockedConfig(KisPropertiesConfigurationSP p);
    void sigDropLockedConfig(KisPropertiesConfigurationSP p);

private Q_SLOTS:

    void slotConfigChanged();

private:
    KisSignalCompressor m_compressor;
    KoCanvasResourcesInterfaceSP m_canvasResourcesInterface;
};


#endif
