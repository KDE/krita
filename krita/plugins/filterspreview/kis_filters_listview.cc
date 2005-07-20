#include "kis_filters_listview.h"

#include "kis_types.h"
#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_filter.h"
#include "kis_filter_strategy.h"

namespace Krita {
namespace Plugins {
namespace FiltersPreview {

KisFiltersListView::KisFiltersListView(KisView* view, QWidget* parent) : KIconView(parent) , m_view(view)
{
	buildPreview();
	setItemsMovable(false);
	setSelectionMode(QIconView::Single);
}

void KisFiltersListView::buildPreview()
{
	kdDebug() << "kikoo" << endl;
	// Check which filters support painting
	KisImageSP img = m_view->currentImg();
	KisLayerSP activeLayer = img->activeLayer();
	m_thumb = new KisLayer(*activeLayer);
	m_imgthumb = new KisImage(0, m_thumb->exactBounds().width(), m_thumb->exactBounds().height(), m_thumb->colorStrategy(), "thumbnail");
	m_imgthumb->add(m_thumb,0);
	double sx = 100./m_thumb->exactBounds().width();
	double sy = 100./m_thumb->exactBounds().height();
	m_imgthumb->scale(sx, sy, 0, new KisMitchellFilterStrategy());
	
	KisIDList l = KisFilterRegistry::instance()->listKeys();
	KisIDList::iterator it;
	it = l.begin();
	for (it = l.begin(); it !=  l.end(); ++it) {
		KisFilterSP f = KisFilterRegistry::instance()->get(*it);
		
		if (f -> supportsPreview()) {
			kdDebug() << (*it).name() << endl;
			std::list<KisFilterConfiguration*> configlist = f->listOfExamplesConfiguration((KisPaintDeviceSP)m_thumb);
			// apply the filter
			for(std::list<KisFilterConfiguration*>::iterator itc = configlist.begin();
						 itc != configlist.end(); itc++)
			{
				KisImageSP imgthumbPreview = new KisImage(0, m_imgthumb->width(), m_imgthumb->height(), m_imgthumb->colorStrategy(), "preview");
				KisLayerSP thumbPreview = new KisLayer(*m_thumb/*imgthumbPreview,"",50*/);
				imgthumbPreview->add(thumbPreview,0);
				f->disableProgress();
				f->process((KisPaintDeviceSP)m_thumb, (KisPaintDeviceSP)thumbPreview,*itc, imgthumbPreview->bounds());
				QImage qimg =  thumbPreview->convertToQImage(thumbPreview->profile());
				new KisFiltersIconViewItem( this, (*it).name(),
																		QPixmap(qimg), *it, f, *itc );
			}
			
		}
	}

}

// KisImageSP KisFiltersListView::image() { return m_imgthumb; }
// KisLayerSP KisFiltersListView::layer() { return m_thumb; }


};
};
};
