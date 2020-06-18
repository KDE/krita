#include "my_paintop_plugin.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>
#include <kis_fixed_paint_device.h>
#include "kis_my_paintop.h"
#include "kis_my_paintop_settings.h"
#include "kis_my_paintop_settings_widget.h"
#include "kis_simple_paintop_factory.h"
#include <kis_my_paintop_factory.h>

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(MyPaintOpPluginFactory, "kritamypaintop.json", registerPlugin<MyPaintOpPlugin>();)


MyPaintOpPlugin::MyPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    //r->add(new KisSimplePaintOpFactory<KisMyPaintOp, KisMyPaintOpSettings, KisMyPaintOpSettingsWidget>("mypaintbrush", i18n("MyPaint"), KisPaintOpFactory::categoryStable() , "krita-mypaint.png", QString(), QStringList(), 6));
    r->add(new KisMyPaintOpFactory());
}

MyPaintOpPlugin::~MyPaintOpPlugin()
{
}

#include "my_paintop_plugin.moc"
