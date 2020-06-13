#ifndef KIS_MYPAINT_BRUSH_H
#define KIS_MYPAINT_BRUSH_H

#include <QObject>
#include <libmypaint/mypaint-brush.h>
#include <KoColor.h>
#include <kis_paintop_settings.h>
#include <kis_painter.h>

class KisMyPaintBrush : public QObject
{
    Q_OBJECT
public:
    explicit KisMyPaintBrush(KisPainter *painter, QObject *parent = nullptr);
    void setColor(const KoColor color);
    void apply(KisPaintOpSettingsSP settings);
    MyPaintBrush* brush();

private:
    MyPaintBrush *m_brush;
    KisPainter *m_painter;
};

#endif // KIS_MYPAINT_BRUSH_H
