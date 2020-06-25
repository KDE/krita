#include "kis_my_paintop_factory.h"

#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <kis_mypaint_brush.h>
#include <kis_my_paintop.h>
#include <kis_my_paintop_settings.h>
#include <kis_my_paintop_settings_widget.h>
#include <KisResourceServerProvider.h>
#include <kis_my_paintop_option.h>
#include <kis_icon.h>

class KisMyPaintOpFactory::Private {

    public:
        KoResourceServer<KisMyPaintBrush> *brushServer;
        QMap<QString, KisMyPaintBrush*> brushes;
};

KisMyPaintOpFactory::KisMyPaintOpFactory()
    : m_d(new Private)
{

    m_d->brushServer = new KoResourceServerSimpleConstruction<KisMyPaintBrush>("mypaint_brushes", "*.myb");
    m_d->brushServer->loadResources(KoResourceServerProvider::blacklistFileNames(m_d->brushServer->fileNames(), m_d->brushServer->blackListedFiles()));
}

KisMyPaintOpFactory::~KisMyPaintOpFactory() {

//    delete m_d->brushServer;
//    delete m_d;
}

KisPaintOp* KisMyPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) {

    KisPaintOp* op = new KisMyPaintOp(settings, painter, node, image);

    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisMyPaintOpFactory::settings() {

    KisPaintOpSettingsSP settings = new KisMyPaintOpSettings();
//    settings->setModelName(m_model);
    return settings;
}

KisPaintOpConfigWidget* KisMyPaintOpFactory::createConfigWidget(QWidget* parent) {

    return new KisMyPaintOpSettingsWidget(parent);
}

QString KisMyPaintOpFactory::id() const {

    return "mypaintbrush";
}

QString KisMyPaintOpFactory::name() const {

    return "MyPaint";
}

QIcon KisMyPaintOpFactory::icon() {

    return KisIconUtils::loadIcon(id());
}

QString KisMyPaintOpFactory::category() const {

    return KisPaintOpFactory::categoryStable();
}

void KisMyPaintOpFactory::processAfterLoading() {

    KisPaintOpPresetResourceServer *paintOpServer = KisResourceServerProvider::instance()->paintOpPresetServer();

    foreach(KisMyPaintBrush* brush, m_d->brushServer->resources()) {

        QFileInfo fileInfo(brush->filename());

        KisPaintOpSettingsSP s = new KisMyPaintOpSettings();
        s->setProperty("paintop", id());
        s->setProperty("filename", brush->filename());
        s->setProperty("json_string", brush->getJsonData());
        s->setProperty(MYPAINT_DIAMETER, brush->getSize());

        KisPaintOpPresetSP preset = new KisPaintOpPreset();
        preset->setName(fileInfo.baseName());
        preset->setSettings(s);
        KoID paintOpID(id(), name());
        preset->setPaintOp(paintOpID);
        preset->setValid(true);
        preset->setImage(brush->image());

        paintOpServer->addResource(preset, false);
    }

}

#include "kis_my_paintop_factory.moc"
