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
class QVBoxLayout;

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
	private slots:
		void selectionHasChanged ( QIconViewItem * item );
	private:
		KisView* m_view;
		KisFiltersListView* m_kflw;
		QWidget* m_currentConfigWidget;
		KisFilter* m_currentFilter;
		QVBoxLayout* m_vlayout;
};

};
};
};

#endif
