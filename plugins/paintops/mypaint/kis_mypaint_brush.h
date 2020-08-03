#ifndef KIS_MYPAINT_BRUSH_H
#define KIS_MYPAINT_BRUSH_H

#include <QObject>
#include <libmypaint/mypaint-brush.h>
#include <KoColor.h>
#include <kis_paintop_settings.h>
#include <kis_painter.h>
#include <KoResource.h>

class KisMyPaintBrush : public QObject, public KoResource
{
    Q_OBJECT

public:

    KisMyPaintBrush(const QString &fileName="");
    virtual ~KisMyPaintBrush() {} ;

    void setColor(const KoColor color);
    void apply(KisPaintOpSettingsSP settings);
    MyPaintBrush* brush();

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;

    QByteArray getJsonData();
    float getSize();
    float getHardness();
    float getOpacity();
    float isEraser();

private:

    class Private;
    Private* const m_d;
};

#endif // KIS_MYPAINT_BRUSH_H
