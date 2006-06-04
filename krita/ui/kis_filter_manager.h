/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _KIS_FILTER_MANAGER_
#define _KIS_FILTER_MANAGER_

#include "q3dict.h"
#include "qobject.h"
#include "q3ptrlist.h"
#include "qsignalmapper.h"
#include "kactionclasses.h"
#include "kis_image.h"
#include "kis_selection.h"

#include <krita_export.h>

class KAction;
class KisView;
class KisDoc;
class KisFilter;
class KisFilterConfiguration;
class KAction;
class KActionCollection;
class KisPreviewDialog;

/**
 * Create all the filter actions for the specified view and implement re-apply filter
 */
class KRITAUI_EXPORT KisFilterManager : public QObject {

    Q_OBJECT

public:

    KisFilterManager(KisView * parent, KisDoc * doc);
    ~KisFilterManager();

    void setup(KActionCollection * ac);
    void updateGUI();


    bool apply();

protected slots:

    void slotApply();
    void slotConfigChanged();
    void slotApplyFilter(int);
    void refreshPreview();

private:

    KisView * m_view;
    KisDoc * m_doc;

    KAction * m_reapplyAction;

    Q3PtrList<KAction> m_filterActions;

    KisFilterConfiguration * m_lastFilterConfig;
    KisFilter * m_lastFilter;
    KisPreviewDialog * m_lastDialog;
    KisFilterConfigWidget * m_lastWidget;

    KoIDList m_filterList; // Map the actions in the signalmapper to the filters
    QSignalMapper * m_filterMapper;

    Q3Dict<KActionMenu> m_filterActionMenus;
};

#endif
