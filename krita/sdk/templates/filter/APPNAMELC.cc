
#include "%{APPNAMELC}.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>

K_PLUGIN_FACTORY(%{APPNAME}PluginFactory, registerPlugin<%{APPNAME}Plugin>();)
K_EXPORT_PLUGIN(%{APPNAME}PluginFactory("krita"))

%{APPNAME}Plugin::%{APPNAME}Plugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new %{APPNAME}Filter()));
    }
}

%{APPNAME}Plugin::~%{APPNAME}Plugin()
{
}

%{APPNAME}Filter::%{APPNAME}Filter() : KisFilter(id(), "adjust", i18n("%{APPNAME}"))
{
}

void %{APPNAME}Filter::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* /*config*/)
{
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());

    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height());
    KisRectConstIteratorPixel srcIt = src->createRectConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height());

    int pixelsProcessed = 0;
    setProgressTotalSteps(size.width() * size.height());

    KoColorSpace * cs = src->colorSpace();

    while( ! srcIt.isDone() )
    {
        if(srcIt.isSelected())
        {
          // TODO: implement the processing of the pixels
        }
        setProgress(++pixelsProcessed);
        ++srcIt;
        ++dstIt;
    }
    delete inverter;
    setProgressDone(); // Must be called even if you don't really support progression
}
