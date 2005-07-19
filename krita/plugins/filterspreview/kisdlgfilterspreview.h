//
// C++ Interface: kisdlgfilterspreview
//
// Description: 
//
//
// Author: Cyrille Berger <cberger@cberger.net>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KISDLGFILTERSPREVIEW_H
#define KISDLGFILTERSPREVIEW_H

#include <kdialogbase.h>

class KisView;
class KisFilter;
class QIconViewItem;
class QHBoxLayout;
class KisPreviewWidget;

namespace Krita {
namespace Plugins {
namespace FiltersPreview {
	class KisFiltersListView;

/**
@author Cyrille Berger
*/
class KisDlgFiltersPreview : public KDialogBase
{
	Q_OBJECT
	public:
		KisDlgFiltersPreview(KisView* view, QWidget* parent,const char *name = "");

                ~KisDlgFiltersPreview();
        public:
          inline KisFilter* currentFilter() { return m_currentFilter; };
	private slots:
		void refreshPreview();
		void selectionHasChanged ( QIconViewItem * item );
	private:
		KisPreviewWidget* m_previewWidget;
		KisView* m_view;
		KisFiltersListView* m_kflw;
		QWidget* m_currentConfigWidget;
		KisFilter* m_currentFilter;
		QHBoxLayout* m_hlayout;
};

};
};
};

#endif
