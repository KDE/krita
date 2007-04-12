/*
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
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
#ifndef _KIS_FILTER_H_
#define _KIS_FILTER_H_

#include <list>

#include <QString>

#include <ksharedptr.h>
#include <klocale.h>

#include "kis_types.h"
#include "kis_filter_registry.h"
#include "KoID.h"
#include "kis_paint_device.h"
#include "kis_progress_subject.h"
#include "kis_filter_configuration.h"
#include "KoColorSpace.h"
#include "krita_export.h"

class KoColorSpace;
class KisPreviewDialog;
class KisProgressDisplayInterface;
class KisFilterConfigWidget;
class QWidget;

/**
 * Basic interface of a Krita filter.
 */
class KRITAIMAGE_EXPORT KisFilter : public KisProgressSubject, public KisShared {
    Q_OBJECT
public:
    static const KoID ConfigDefault;
    static const KoID ConfigDesigner;
    static const KoID ConfigLastUsed;
public:

    /**
     * Construct a Krita filter
     */
    KisFilter(const KoID& id, const QString & category, const QString & entry);
    virtual ~KisFilter() {}

public:

    virtual void setProgressDisplay(KisProgressDisplayInterface * progressDisplay);

    /**
     * Override this function with the implementation of your filter.
     *
     * XXX: May the filter may assume that src and dst have the same
     * colorspace? (bsar)
     *
     * @param src the source paint device
     * @param srcTopLeft the top left coordinate where the filter starts to be applied
     * @param dst the destination paint device
     * @param dstTopLeft he top left coordinate for the destination paint device
     * @param size the size of the area that is filtered
     * @param channelFlags an array of bits that indicated which
     *        channels must be excluded and which channels must be included
     *        when filtering. Is _empty_ when all channels need to be filtered.
     * @param config the parameters of the filter
     */
    virtual void process(const KisPaintDeviceSP src,
                         const QPoint& srcTopLeft,
                         KisPaintDeviceSP dst,
                         const QPoint& dstTopLeft,
                         const QSize& size,
                         KisFilterConfiguration* config
        ) = 0;

    /**
     * Provided for convenience only when source and destination are the same
     */
    inline void process(KisPaintDeviceSP device, const QRect& rect, KisFilterConfiguration* config)
        {
            process(device, rect.topLeft(), device, rect.topLeft(), rect.size(), config);
        }


public:
    /**
     * This function return the configuration set as the default by the user or the default configuration from
     * the filter writer as returned by designerConfiguration.
     * This configuration is used by default for the configuration widget and to the process function if there is no
     * configuration widget.
     * @return the default configuration of this widget
     */
    virtual KisFilterConfiguration * defaultConfiguration(const KisPaintDeviceSP);

    /**
     * If true, this filter can be used in painting tools as a paint operation
     */
    virtual bool supportsPainting() { return false; };

    /// This filter can be displayed in a preview dialog
    virtual bool supportsPreview() { return false; };

    /// This filter can be used in adjustment layers
    virtual bool supportsAdjustmentLayers() { return supportsPreview(); };

    /**
     * Return a list of default configuration to demonstrates the use of the filter
     * @return a list with a null element if the filter do not use a configuration
     */
    QHash<QString, KisFilterConfiguration*> bookmarkedConfigurations( const KisPaintDeviceSP );

    /**
     * Save this filter configuration for a later use.
     * @param configname the name of this configuration in the kritarc
     * @param config the configuration object to save
     */
    void saveToBookmark(const QString& configname, KisFilterConfiguration* config);

    /**
     * Save this filter configuration for a later use.
     * @param configname the name of this configuration in the kritarc
     * @param config the configuration object to save
     */
    KisFilterConfiguration* loadFromBookmark(const QString& configname);

    /**
     * @return true if this configuration exist in the bookmarks
     */
    bool existInBookmark(const QString& configname);

    /**
     * Can this filter work incrementally when painting, or do we need to work
     * on the state as it was before painting started. The former is faster.
     */
    virtual bool supportsIncrementalPainting() { return true; };

    /**
     * This filter supports cutting up the work area and filtering
     * each chunk in a separate thread. Filters that need access to the
     * whole area for correct computations should return false.
     */
    virtual bool supportsThreading() { return true; };

    /**
     * Used when threading is used -- the overlap margin is passed to the
     * filter to use to compute pixels, but the margin is not pasted into the
     * resulting image.
     */
    virtual int overlapMarginNeeded(KisFilterConfiguration* = 0) const { return 0; }

     /**
     * Similar to overlapMarginNeeded: some filters will alter a lot of pixels that are
     * near to each other at the same time. So when you changed a single rectangle
     * in a device, the actual rectangle that will feel the influence of this change
     * might be bigger. Use this function to detirmine that rect.
     * The default implementation makes a guess using overlapMarginNeeded.
      */
    virtual QRect enlargeRect(QRect rect, KisFilterConfiguration* = 0) const;


    /**
     * Determine the colorspace independence of this filter.
     * @see ColorSpaceIndependence
     *
     * @return the degree of independence
     */
    virtual ColorSpaceIndependence colorSpaceIndependence() { return TO_RGBA8; };

    /**
     * Determine if this filter can work with this colorSpace. For instance, some
     * colorspaces don't depend on lcms, and cannot do certain tasks. The colorsfilters
     * are problems here.
     * BSAR: I'm still not convinced that this is the right approach. I think that every
     * colorspace should implement the api fully; and that the filter should simply call
     * that api. After all, you don't need lcms to desaturate.
     *
     * @param cs the colorspace that we want to know this filter works with
     */
    virtual bool workWith(KoColorSpace* cs) { Q_UNUSED(cs); return true; }

    virtual void enableProgress();
    virtual void disableProgress();

    bool autoUpdate();

    /// @return Unique identification for this filter
    inline QString id() const { return m_id.id(); }
    inline QString name() const { return m_id.name(); }

    /// @return the submenu in the filters menu does filter want to go?
    inline QString menuCategory() const { return m_category; };

    /// @return the i18n'ed string this filter wants to show itself in the menu
    inline QString menuEntry() const { return m_entry; };

    /**
     * Create the configuration widget for this filter.
     *
     * @param parent the Qt owner widget of this widget
     * @param dev the paintdevice this filter will act on
     */
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget * parent, const KisPaintDeviceSP dev);

    /**
     * call this to cancel the current filtering
     */
    virtual void cancel() { m_cancelRequested = true; }

    virtual void setAutoUpdate(bool set);
    bool progressEnabled() const { return m_progressEnabled; }
    /**
     * @return true if cancel was requested and if progress is enabled
     */
    inline bool cancelRequested() const { return m_progressEnabled && m_cancelRequested; }

protected:

    /// @return the name of config group in KConfig
    inline QString configEntryGroup() { return id() + "_filter_bookmarks"; }
    /// @return the default configuration as defined by whoever wrote the plugin
    virtual KisFilterConfiguration* designerConfiguration(const KisPaintDeviceSP); // FIXME: this name sucks so much

protected slots:

    // Convenience functions for progress display.
    void setProgressTotalSteps(qint32 totalSteps);
    void setProgress(qint32 progress);
    void incProgress();
    void setProgressStage(const QString& stage, qint32 progress);
    void setProgressDone();
    inline qint32 progress() { return m_progressSteps; }

private:
    bool m_cancelRequested;
    bool m_progressEnabled;
    bool m_autoUpdate;

protected:
    qint32 m_progressTotalSteps;
    qint32 m_lastProgressPerCent;
    qint32 m_progressSteps;

    KoID m_id;
    KisProgressDisplayInterface * m_progressDisplay;
    QString m_category; // The category in the filter menu this filter fits
    QString m_entry; // the i18n'ed accelerated menu text
};


#endif
