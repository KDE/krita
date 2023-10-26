/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KisPaintingModeOptionData.h>

#include "kis_grid_paintop_settings.h"
#include "kis_grid_paintop_settings_widget.h"

#include "KisGridShapeOptionData.h"
#include "KisGridOpOptionData.h"

#include <KisColorOptionData.h>
#include <KisOptimizedBrushOutline.h>

struct KisGridPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisGridPaintOpSettings::KisGridPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::NO_OPTION,
                                                     resourcesInterface),
    m_d(new Private),
    m_modifyOffsetWithShortcut(false)
{
}

void KisGridPaintOpSettings::setPaintOpSize(qreal value)
{
    KisGridOpOptionData option;
    option.read(this);
    option.diameter = value;
    option.write(this);
}

qreal KisGridPaintOpSettings::paintOpSize() const
{
    KisGridOpOptionData option;
    option.read(this);
    return option.diameter;
}

void KisGridPaintOpSettings::setPaintOpAngle(qreal value)
{
    Q_UNUSED(value);
}

qreal KisGridPaintOpSettings::paintOpAngle() const
{
    return 0.0;
}

KisGridPaintOpSettings::~KisGridPaintOpSettings()
{
}

bool KisGridPaintOpSettings::paintIncremental()
{
    KisPaintingModeOptionData data;
    data.read(this);
    return data.paintingMode == enumPaintingMode::BUILDUP;
}

bool KisGridPaintOpSettings::mousePressEvent(const KisPaintInformation& info, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode)
{
    Q_UNUSED(currentNode);

    KisGridOpOptionData option;
    option.read(this);
    bool eventIgnored = true;
    qreal newHorizontalOffset = std::fmod(info.pos().x() + option.grid_width/2.0, (float)option.grid_width);
    qreal newVerticalOffset = std::fmod(info.pos().y() + option.grid_height/2.0, (float)option.grid_height);

    // If pressing ctrl+alt change the offset according to mouse position
    if (modifiers == (Qt::ControlModifier | Qt::AltModifier) || m_modifyOffsetWithShortcut) {
        m_modifyOffsetWithShortcut = true;
        newHorizontalOffset = (newHorizontalOffset / (float)option.grid_width);
        newVerticalOffset = (newVerticalOffset / (float)option.grid_height);

        if (newHorizontalOffset > 0.5) {
            newHorizontalOffset = newHorizontalOffset - 1;
        }
        if (newVerticalOffset > 0.5) {
            newVerticalOffset = newVerticalOffset -1;
        }
        option.horizontal_offset = newHorizontalOffset * option.grid_width;
        option.vertical_offset = newVerticalOffset * option.grid_height;
        option.write(this);
        eventIgnored = false;
    }
    return eventIgnored;
}

bool KisGridPaintOpSettings::mouseReleaseEvent()
{
    m_modifyOffsetWithShortcut = false;
    bool ignoreEvent = true;
    return ignoreEvent;
}
KisOptimizedBrushOutline KisGridPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    KisOptimizedBrushOutline path;
    KisGridOpOptionData option;
    option.read(this);
    KisGridShapeOptionData shapeOption;
    shapeOption.read(this);
    if (mode.isVisible) {
        qreal sizex = option.diameter * option.grid_scale;
        qreal sizey = option.diameter * option.grid_scale;
        QRectF rc(0, 0, sizex, sizey);
        rc.translate(-rc.center());
        path.addRect(rc);

        path = outlineFetcher()->fetchOutline(info, this, path, mode, alignForZoom);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info, QPointF(0.0, 0.0), sizex * 0.5, 3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, 1.0, 0.0, true, 0, 0));
        }
    }
    else if (m_modifyOffsetWithShortcut) {
        qreal gridWidth = option.diameter * option.grid_scale ;
        qreal gridHeight = option.diameter * option.grid_scale ;

        qreal cellWidth = option.grid_width * option.grid_scale ;
        qreal cellHeight = option.grid_height * option.grid_scale;

        qreal horizontalOffset = option.horizontal_offset;
        qreal verticalOffset = option.vertical_offset;

        int divide;
        if (option.grid_pressure_division) {
            divide = option.grid_division_level * info.pressure();
        }
        else {
            divide = option.grid_division_level;
        }

        divide = qRound(option.grid_scale * divide);

        //Adjust the start position of the drawn grid to the top left of the brush instead of in the center
        qreal posX = info.pos().x() - (gridWidth/2) + (cellWidth/2) - horizontalOffset;
        qreal posY = info.pos().y() - (gridHeight/2) + (cellHeight/2) - verticalOffset;

        //Lock the grid alignment
        posX = posX - std::fmod(posX, cellWidth) + horizontalOffset;
        posY = posY - std::fmod(posY, cellHeight) + verticalOffset;
        const QRectF dabRect(posX , posY , cellWidth, cellHeight);

        divide = qMax(1, divide);
        const qreal yStep = cellHeight / (qreal)divide;
        const qreal xStep = cellWidth / (qreal)divide;

        QRectF tile;
        QPainterPath cellPath;
        for (int y = 0; y < (gridHeight)/yStep; y++) {
            for (int x = 0; x < (gridWidth)/xStep; x++) {
                tile = QRectF(dabRect.x() + x * xStep, dabRect.y() + y * yStep, xStep, yStep);
                switch (shapeOption.shape) {
                case 0: {
                    cellPath.addEllipse(tile);
                    break;
                }
                case 1: {
                    cellPath.addRect(tile);
                    break;
                }
                case 2: {
                    cellPath.moveTo(tile.topRight());
                    cellPath.lineTo(tile.bottomLeft());
                    break;
                }
                case 3: {
                    cellPath.moveTo(tile.topRight());
                    cellPath.lineTo(tile.bottomLeft());
                    break;
                }
                case 4: {
                    cellPath.moveTo(tile.topRight());
                    cellPath.lineTo(tile.bottomLeft());
                    break;
                }
                default: {
                break;
                }
                }
            }
        }
        KisOptimizedBrushOutline cellPath2 = outlineFetcher()->fetchOutline(info, this, cellPath, mode, alignForZoom);
        path.addPath(cellPath2);
    }
    return path;
}


#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "KisPaintOpPresetUpdateProxy.h"

QList<KisUniformPaintOpPropertySP> KisGridPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisIntSliderBasedPaintOpPropertyCallback *prop = new KisIntSliderBasedPaintOpPropertyCallback(KisIntSliderBasedPaintOpPropertyCallback::Int,
                                                                                                          KoID("grid_divisionlevel", i18n("Division Level")),
                                                                                                          settings,
                                                                                                          0);

            prop->setRange(1, 25);
            prop->setSingleStep(1);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisGridOpOptionData option;
                    option.read(prop->settings().data());

                    prop->setValue(int(option.grid_division_level));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisGridOpOptionData option;
                    option.read(prop->settings().data());
                    option.grid_division_level = prop->value().toInt();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisPaintOpSettings::uniformProperties(settings, updateProxy) + props;
}
