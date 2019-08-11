/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_BASE_PROCESSOR_H_
#define _KIS_BASE_PROCESSOR_H_

#include <list>

#include <QString>

#include <klocalizedstring.h>
#include <QKeySequence>

#include "KoID.h"
#include "KoColorSpace.h"

#include "kis_types.h"
#include "kis_shared.h"
#include "kis_image.h"

#include "kritaimage_export.h"

class QWidget;

class KisBookmarkedConfigurationManager;
class KisFilterConfiguration;
class KisConfigWidget;

/**
 * Base class for classes that process areas of pixels.
 * Processors can either read in pixels and write out pixels
 * or just write out pixels, using a certain set of configuration
 * pixels.
 *
 * in-out processing is typically filtering: @see KisFilter.
 * out-only processing is typically generating: @see KisGenerator.
 *
 * Information about the area that needs to be processed is contained
 * @see KisProcessingInformation and @see KisConstProcessingInformation.
 */
class KRITAIMAGE_EXPORT KisBaseProcessor : public KisShared
{
    friend class KisBaseProcessorConfigurationFactory;

public:


    KisBaseProcessor(const KoID& id, const KoID & category, const QString & entry);

    virtual ~KisBaseProcessor();

    /**
     * Return the configuration set as the default by the user or the default
     * configuration from the filter writer as returned by factoryConfiguration.
     *
     * This configuration is used by default for the configuration widget and
     * given to the process function if there is no configuration widget.
     *
     * @return the default configuration of this widget
     */
    KisFilterConfigurationSP  defaultConfiguration() const;

    /**
     * @return the bookmark manager for this processor
     */
    KisBookmarkedConfigurationManager* bookmarkManager();

    /**
     * @return the bookmark manager for this processor
     */
    const KisBookmarkedConfigurationManager* bookmarkManager() const;

    /// @return Unique identification for this processor
    QString id() const;

    /// @return User-visible identification for this processor
    QString name() const;

    /// @return the submenu in the filters menu does processor want to go?
    KoID menuCategory() const;

    /// @return the i18n'ed string this filter wants to show itself in the menu
    QString menuEntry() const;

    /**
     * Return the default keyboard shortcut for activation of this filter
     *
     * @return the shortcut
     */
    QKeySequence shortcut() const;

    /**
     * Create the configuration widget for this processor.
     *
     * @param parent the Qt owner widget of this widget
     * @param dev the paintdevice this filter will act on
     * @param useForMasks shown if the filer is going to be used in a mask. Some filters
     *        may provide limited options when applied as a mask (e.g. Gaussian Blur)
     */
    virtual KisConfigWidget * createConfigurationWidget(QWidget * parent, const KisPaintDeviceSP dev, bool useForMasks) const;
    // "Support" functions
public:
    /**
     * If true, this filter can be used in painting tools as a paint operation
     */
    bool supportsPainting() const;

    /// This filter can be used in adjustment layers
    bool supportsAdjustmentLayers() const;

    /**
     * This filter supports cutting up the work area and filtering
     * each chunk in a separate thread. Filters that need access to the
     * whole area for correct computations should return false.
     */
    bool supportsThreading() const;

    /// If true, the filter wants to show a configuration widget
    bool showConfigurationWidget();

    /**
     * Determine the colorspace independence of this filter.
     * @see ColorSpaceIndependence
     *
     * @return the degree of independence
     */
    ColorSpaceIndependence colorSpaceIndependence() const;

    /// @return the default configuration as defined by whoever wrote the plugin
    virtual KisFilterConfigurationSP factoryConfiguration() const;

protected:

    void setSupportsPainting(bool v);
    void setSupportsAdjustmentLayers(bool v);
    void setSupportsThreading(bool v);
    void setColorSpaceIndependence(ColorSpaceIndependence v);
    void setShowConfigurationWidget(bool v);

    /**
     * Set the default shortcut for activation of this filter.
     */
    void setShortcut(const QKeySequence & shortcut);

protected:

    void init(const QString& configEntryGroup);

private:
    struct Private;
    Private* const d;
};


#endif
