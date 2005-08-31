/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include <qobject.h>
#include <qwidget.h>
#include <qstring.h>

#include <ksharedptr.h>
#include <klocale.h>

#include "kis_types.h"
#include "kis_filter_registry.h"
#include "kis_id.h"
#include "koffice_export.h"
#include "kis_progress_subject.h"
#include "kis_paint_device_impl.h"

class KisPreviewDialog;
class KisProgressDisplayInterface;

/**
 * Empty interface for passing filter configuration data
 * from the configuration widget to the filter.
 */
class KisFilterConfiguration {
};

class KisFilterConfigWidget : public QWidget {

    Q_OBJECT

public:

    KisFilterConfigWidget(QWidget * parent, const char * name = 0, WFlags f = 0 ) : QWidget(parent, name, f) {};
    virtual ~KisFilterConfigWidget() {};
signals:
    void sigPleaseUpdatePreview();
};

/**
 * Basic interface of a Krita filter.
 */
class KRITACORE_EXPORT KisFilter : public KisProgressSubject, public KShared {
    Q_OBJECT
public:

    /**
     * Construct a Krita filter
     */
    KisFilter(const KisID& id, const QString & category, const QString & entry);
    virtual ~KisFilter() {}

public:

    virtual void setProgressDisplay(KisProgressDisplayInterface * progressDisplay);
    
    virtual void process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration*, const QRect&) = 0;

public:
    virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceImplSP dev);

    /**
         * If true, this filter can be used in painting tools as a paint operation
         */
    virtual bool supportsPainting() { return false; };

    /// This filter can be displayed in a preview dialog
    virtual bool supportsPreview() { return false; };

    /** 
         * Return a list of default configuration to demonstrates the use of the filter
     * @return a list with a null element if the filter do not use a configuration
     */
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceImplSP )
    { std::list<KisFilterConfiguration*> list; list.insert(list.begin(), 0); return list; }
    
    // Can this filter work incrementally when painting, or do we need to work
    // on the state as it was before painting started. The former is faster.
    virtual bool supportsIncrementalPainting() { return true; };
    
    // This filter supports cutting up the work area and filtering
    // each chunk in a separate thread. Filters that need access to the
    // whole area for correct computations should return false.
    virtual bool supportsThreading() { return true; };
    
    // Used when threading is used -- the overlap margin is passed to the
    // filter to use to compute pixels, but the margin is not pasted into the
    // resulting image.
    virtual int  overlapMarginNeeded() { return 0; };

    virtual void enableProgress();
    virtual void disableProgress();
    
    bool autoUpdate();

    // Unique identification for this filter
    inline const KisID id() const { return m_id; };
    // Which submenu in the filters menu does filter want to go?

    inline QString menuCategory() const { return m_category; };
    // The i18n'ed string this filter wants to show itself in the menu

    inline QString menuEntry() const { return m_entry; };

    /**
     * Create the configuration widget for this filter.
     *
     * @param parent the Qt owner widget of this widget
     * @param dev the paintdevice this filter will act on
     */
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget * parent, KisPaintDeviceImplSP dev);

    virtual void cancel() { m_cancelRequested = true; }

    virtual void setAutoUpdate(bool set);
    bool progressEnabled() const { return m_progressEnabled; }
    inline bool cancelRequested() const { return m_progressEnabled && m_cancelRequested; }

protected:

    // Convenience functions for progress display.
    void setProgressTotalSteps(Q_INT32 totalSteps);
    void setProgress(Q_INT32 progress);
    void incProgress();
    void setProgressStage(const QString& stage, Q_INT32 progress);
    void setProgressDone();

private:
    bool m_cancelRequested;
    bool m_progressEnabled;
    bool m_autoUpdate;

protected:
    Q_INT32 m_progressTotalSteps;
    Q_INT32 m_lastProgressPerCent;
    Q_INT32 m_progressSteps;
    
    KisID m_id;
    KisProgressDisplayInterface * m_progressDisplay;
    QString m_category; // The category in the filter menu this filter fits
    QString m_entry; // the i18n'ed accelerated menu text
    
};


#endif
