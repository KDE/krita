#ifndef _KIS_IMAGE_LIST_VIEW_H_
#define _KIS_IMAGE_LIST_VIEW_H_

#include <kiconview.h>

#include "kis_id.h"
#include "kis_types.h"

class KisView;
class KisFilter;
class KisFilterConfiguration;
class KisPreviewView;

namespace Krita {
namespace Plugins {
namespace FiltersPreview {

	class KisFiltersIconViewItem : public QIconViewItem {
		public:
			KisFiltersIconViewItem( QIconView * parent, const QString & text, const QPixmap & icon, KisID id, KisFilter* filter, KisFilterConfiguration* filterConfig ) : QIconViewItem(parent, text, icon), m_id(id), m_filter(filter), m_filterconfig(filterConfig)
			{
			}
			inline KisID id() { return m_id; }
			inline KisFilter* filter() { return m_filter; }
                        inline void setFilterConfiguration(KisFilterConfiguration* fc) { m_filterconfig = fc; }
		private:
			KisID m_id;
			KisFilter* m_filter;
			KisFilterConfiguration* m_filterconfig;
	};
	class KisFiltersListView : public KIconView {
		public:
			KisFiltersListView(KisView* view, QWidget* parent);
		public:
			void buildPreview();
// 			KisImageSP image();
// 			KisLayerSP layer();
		private:
			KisView* m_view;
			KisImageSP m_imgthumb;
			KisLayerSP m_thumb;
	};

};
};
};
#endif
