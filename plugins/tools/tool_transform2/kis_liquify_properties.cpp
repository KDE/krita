/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_liquify_properties.h"

#include <QDomElement>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "kis_debug.h"
#include "kis_dom_utils.h"

KisLiquifyProperties::KisLiquifyProperties(const KisLiquifyProperties &rhs)
{
    m_mode = rhs.m_mode;
    m_size = rhs.m_size;
    m_amount = rhs.m_amount;
    m_spacing = rhs.m_spacing;
    m_sizeHasPressure = rhs.m_sizeHasPressure;
    m_amountHasPressure = rhs.m_amountHasPressure;
    m_reverseDirection = rhs.m_reverseDirection;
    m_useWashMode = rhs.m_useWashMode;
    m_flow = rhs.m_flow;
}

KisLiquifyProperties &KisLiquifyProperties::operator=(const KisLiquifyProperties &rhs)
{
    m_mode = rhs.m_mode;
    m_size = rhs.m_size;
    m_amount = rhs.m_amount;
    m_spacing = rhs.m_spacing;
    m_sizeHasPressure = rhs.m_sizeHasPressure;
    m_amountHasPressure = rhs.m_amountHasPressure;
    m_reverseDirection = rhs.m_reverseDirection;
    m_useWashMode = rhs.m_useWashMode;
    m_flow = rhs.m_flow;

    return *this;
}

bool KisLiquifyProperties::operator==(const KisLiquifyProperties &other) const
{
    return
        m_mode == other.m_mode &&
        m_size == other.m_size &&
        m_amount == other.m_amount &&
        m_spacing == other.m_spacing &&
        m_sizeHasPressure == other.m_sizeHasPressure &&
        m_amountHasPressure == other.m_amountHasPressure &&
        m_reverseDirection == other.m_reverseDirection &&
        m_useWashMode == other.m_useWashMode &&
        m_flow == other.m_flow;
}

QString liquifyModeString(KisLiquifyProperties::LiquifyMode mode)
{
    QString result;

    switch (mode) {
    case KisLiquifyProperties::MOVE:
        result = "Move";
        break;
    case KisLiquifyProperties::SCALE:
        result = "Scale";
        break;
    case KisLiquifyProperties::ROTATE:
        result = "Rotate";
        break;
    case KisLiquifyProperties::OFFSET:
        result = "Offset";
        break;
    case KisLiquifyProperties::UNDO:
        result = "Undo";
        break;
    case KisLiquifyProperties::N_MODES:
        qFatal("Unsupported mode");
    }

    return QString("LiquifyTool/%1").arg(result);
}

void KisLiquifyProperties::saveMode() const
{
    KConfigGroup cfg =
         KSharedConfig::openConfig()->group(liquifyModeString(m_mode));

    cfg.writeEntry("size", m_size);
    cfg.writeEntry("amount", m_amount);
    cfg.writeEntry("spacing", m_spacing);
    cfg.writeEntry("sizeHasPressure", m_sizeHasPressure);
    cfg.writeEntry("amountHasPressure", m_amountHasPressure);
    cfg.writeEntry("reverseDirection", m_reverseDirection);
    cfg.writeEntry("useWashMode", m_useWashMode);
    cfg.writeEntry("flow", m_flow);

    KConfigGroup globalCfg =  KSharedConfig::openConfig()->group("LiquifyTool");
    globalCfg.writeEntry("mode", (int)m_mode);
}

void KisLiquifyProperties::loadMode()
{
    KConfigGroup cfg =
         KSharedConfig::openConfig()->group(liquifyModeString(m_mode));

    m_size = cfg.readEntry("size", m_size);
    m_amount = cfg.readEntry("amount", m_amount);
    m_spacing = cfg.readEntry("spacing", m_spacing);
    m_sizeHasPressure = cfg.readEntry("sizeHasPressure", m_sizeHasPressure);
    m_amountHasPressure = cfg.readEntry("amountHasPressure", m_amountHasPressure);
    m_reverseDirection = cfg.readEntry("reverseDirection", m_reverseDirection);
    m_useWashMode = cfg.readEntry("useWashMode", m_useWashMode);
    m_flow = cfg.readEntry("flow", m_flow);
}

void KisLiquifyProperties::loadAndResetMode()
{
    KConfigGroup globalCfg =  KSharedConfig::openConfig()->group("LiquifyTool");
    m_mode = (LiquifyMode) globalCfg.readEntry("mode", (int)m_mode);

    loadMode();
}

void KisLiquifyProperties::toXML(QDomElement *e) const
{
    QDomDocument doc = e->ownerDocument();
    QDomElement liqEl = doc.createElement("liquify_properties");
    e->appendChild(liqEl);

    KisDomUtils::saveValue(&liqEl, "mode", (int)m_mode);
    KisDomUtils::saveValue(&liqEl, "size", m_size);
    KisDomUtils::saveValue(&liqEl, "amount", m_amount);
    KisDomUtils::saveValue(&liqEl, "spacing", m_spacing);
    KisDomUtils::saveValue(&liqEl, "sizeHasPressure", m_sizeHasPressure);
    KisDomUtils::saveValue(&liqEl, "amountHasPressure", m_amountHasPressure);
    KisDomUtils::saveValue(&liqEl, "reverseDirection", m_reverseDirection);
    KisDomUtils::saveValue(&liqEl, "useWashMode", m_useWashMode);
    KisDomUtils::saveValue(&liqEl, "flow", m_flow);
}

KisLiquifyProperties KisLiquifyProperties::fromXML(const QDomElement &e)
{
    KisLiquifyProperties props;
    bool result = false;

    QDomElement liqEl;
    int newMode = 0;

    result =
        KisDomUtils::findOnlyElement(e, "liquify_properties", &liqEl) &&

        KisDomUtils::loadValue(liqEl, "mode", &newMode) &&
        KisDomUtils::loadValue(liqEl, "size", &props.m_size) &&
        KisDomUtils::loadValue(liqEl, "amount", &props.m_amount) &&
        KisDomUtils::loadValue(liqEl, "spacing", &props.m_spacing) &&
        KisDomUtils::loadValue(liqEl, "sizeHasPressure", &props.m_sizeHasPressure) &&
        KisDomUtils::loadValue(liqEl, "amountHasPressure", &props.m_amountHasPressure) &&
        KisDomUtils::loadValue(liqEl, "reverseDirection", &props.m_reverseDirection) &&
        KisDomUtils::loadValue(liqEl, "useWashMode", &props.m_useWashMode) &&
        KisDomUtils::loadValue(liqEl, "flow", &props.m_flow);

    if (result && newMode >= 0 && newMode < N_MODES) {
        props.m_mode = (LiquifyMode) newMode;
    } else {
        result = false;
    }

    return props;
}


QDebug operator<<(QDebug dbg, const KisLiquifyProperties &props)
{
    dbg.nospace() << "\nKisLiquifyProperties(";
    dbg.space() << "\n    " << ppVar(props.mode());
    dbg.space() << "\n    " << ppVar(props.size());
    dbg.space() << "\n    " << ppVar(props.amount());
    dbg.space() << "\n    " << ppVar(props.spacing());
    dbg.space() << "\n    " << ppVar(props.sizeHasPressure());
    dbg.space() << "\n    " << ppVar(props.amountHasPressure());
    dbg.space() << "\n    " << ppVar(props.reverseDirection());
    dbg.space() << "\n    " << ppVar(props.useWashMode());
    dbg.space() << "\n    " << ppVar(props.flow());
    dbg.space() << "\n    );\n";
    return dbg.nospace();
}
