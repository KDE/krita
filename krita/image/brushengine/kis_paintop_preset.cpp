/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_paintop_preset.h"

#include <QFile>
#include <QImage>

#include <KoColorSpaceRegistry.h>

#include "kis_types.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_registry.h"
#include "kis_painter.h"
#include "kis_paint_information.h"
#include "kis_paint_device.h"
#include "kis_image.h"

struct KisPaintOpPreset::Private {
    KisPaintOpSettingsSP settings;
    QImage img;
};


KisPaintOpPreset::KisPaintOpPreset()
        : KoResource(QString())
        , m_d(new Private)
{
    m_d->settings = 0;
}

KisPaintOpPreset::KisPaintOpPreset(const QString & fileName)
        : KoResource(fileName)
        , m_d(new Private)
{
}

KisPaintOpPreset::~KisPaintOpPreset()
{
    delete m_d;
}

KisPaintOpPresetSP KisPaintOpPreset::clone() const
{
    KisPaintOpPreset * preset = new KisPaintOpPreset();
    if (settings()) {
        preset->setSettings(settings()->clone());
    }
    preset->setPaintOp(paintOp());
    preset->setName(name());
    preset->setValid(valid());

    return preset;
}



void KisPaintOpPreset::setPaintOp(const KoID & paintOp)
{
    m_d->settings->setProperty("paintop", paintOp.id());
}

KoID KisPaintOpPreset::paintOp() const
{
    return KoID(m_d->settings->getString("paintop"), name());
}

void KisPaintOpPreset::setSettings(KisPaintOpSettingsSP settings)
{
    m_d->settings = settings->clone();
}

KisPaintOpSettingsSP KisPaintOpPreset::settings() const
{
    return m_d->settings;
}

bool KisPaintOpPreset::load()
{
    if (filename().isEmpty())
        return false;

    QFile f(filename());
    f.open(QIODevice::ReadOnly);
    m_d->settings->fromXML(QString(f.readAll()));
    f.close();
    return true;

    setName(m_d->settings->getString("name"));
}

bool KisPaintOpPreset::save()
{
    if (filename().isEmpty())
        return false;

    QFile f(filename());
    f.open(QIODevice::WriteOnly);
    f.write(m_d->settings->toXML().toUtf8());
    f.close();
    return true;
}

QImage KisPaintOpPreset::img() const
{
    return m_d->img;
}

void KisPaintOpPreset::updateImg()
{
#if 0 // XXX: port to KisPaintOpPreset
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP mydevice = new KisPaintDevice(cs, "preview device");
    Q_CHECK_PTR(mydevice);

    KisPainter painter(mydevice);
    KoColor c(Qt::black, cs);
    painter.setPaintColor(c);

    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("sumibrush", this, &painter);
    painter.setPaintOp(op);

    QLabel *label = m_options->previewLbl;
    int width = label->width();
    int height = label->height();

    /*    QPointF p1(0,0);
          KisPaintInformation pi1(p1, 0.5);
          QPointF p2(width,height);
          KisPaintInformation pi2(p2, 1.0);

          painter.paintLine(pi1,pi2);*/
    painter.end();

    QPointF p1(1.0 / 6.0*width, 2.0 / 3.0*height);
    QPointF p2(2.0 / 6.0*width, 1.0 / 3.0*height);
    QPointF p3(4.0 / 6.0*width, 2.0 / 3.0*height);
    QPointF p4(5.0 / 6.0*width, 1.0 / 3.0*height);

    float pathLength;

    //p2-p1
    float dx = p2.x() - p1.x();
    float dy = p2.y() - p1.y();
    pathLength += sqrt(dx * dx + dy * dy);

    dx = p3.x() - p2.x();
    dy = p3.y() - p2.y();
    pathLength += sqrt(dx * dx + dy * dy);

    dx = p4.x() - p3.x();
    dy = p4.y() - p3.y();
    pathLength += sqrt(dx * dx + dy * dy);

    m_options->inkAmountSpinBox->setValue((int)pathLength);

    KisPaintInformation pi1(p1, 0.0);
    KisPaintInformation pi2(p2, 0.95);
    KisPaintInformation pi3(p3, 0.75);
    KisPaintInformation pi4(p4, 0.0);

    QPointF c1(p1.x(), p1.y() - 5);
    QPointF c2(p1.x(), p1.y() + 5);
    painter.paintBezierCurve(pi1, c1, c2, pi2, 0);
    c1.setX(p2.x());
    c1.setY(p2.y() - 5);
    c2.setX(p2.x());
    c2.setY(p2.y() + 5);
    painter.paintBezierCurve(pi2, c1, c2, pi3, 0);
    c1.setX(p3.x());
    c1.setY(p3.y() - 5);
    c2.setX(p3.x());
    c2.setY(p3.y() + 5);
    painter.paintBezierCurve(pi3, c1, c2, pi4, 0);

    qint32  xx = 0,
                 yy = 0,
                      ww = 0,
                           hh = 0;

    mydevice->extent(xx, yy, ww, hh);

    /*    dbgPlugins << "Coords extend()";
          dbgPlugins << xx << " " << yy << " " << ww << " " << hh;
          dbgPlugins << mydevice->extent();*/
    QImage img = mydevice->convertToQImage(0, xx, yy, ww, hh);

    /*    const QString filePath("/tmp/sumi-e-preview.png");
          bool r = img.save(filePath);

          dbgPlugins << "saved to?: "<< r << " " << filePath;*/
    QPixmap pixmap = QPixmap::fromImage(img);

    Q_CHECK_PTR(label);
    label->setPixmap(pixmap);
#endif
}

