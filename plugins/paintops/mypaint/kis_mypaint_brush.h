#ifndef KIS_MYPAINT_BRUSH_H
#define KIS_MYPAINT_BRUSH_H

#include <QObject>
#include <libmypaint/mypaint-brush.h>
#include <KoColor.h>
#include <kis_paintop_settings.h>
#include <kis_painter.h>
#include <KoResource.h>
#include <kis_paintop_preset.h>

class KisMyPaintBrush : public QObject, public KisPaintOpPreset
{
    Q_OBJECT

public:

    KisMyPaintBrush(const QString &fileName="");
    virtual ~KisMyPaintBrush();

    void setColor(const KoColor color, const KoColorSpace *colorSpace);
    void apply(KisPaintOpSettingsSP settings);
    MyPaintBrush* brush();

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool save() override;
    void reloadSettings();

    QByteArray getJsonData();
    float getSize();
    float getHardness();
    float getOpacity();
    float getOffset();
    float isEraser();

private:

    class Private;
    Private* const m_d;
    bool firstLoad = true;
};

#endif // KIS_MYPAINT_BRUSH_H
