/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    m_preserveShapeRotation = rhs.m_preserveShapeRotation;
    m_preserveShapeScale = rhs.m_preserveShapeScale;
    m_preserveShapeStretch = rhs.m_preserveShapeStretch;
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
    m_preserveShapeRotation = rhs.m_preserveShapeRotation;
    m_preserveShapeScale = rhs.m_preserveShapeScale;
    m_preserveShapeStretch = rhs.m_preserveShapeStretch;

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
        m_flow == other.m_flow &&
        m_preserveShapeRotation == other.m_preserveShapeRotation &&
        m_preserveShapeScale == other.m_preserveShapeScale &&
        m_preserveShapeStretch == other.m_preserveShapeStretch;
}

bool KisLiquifyProperties::supportsReverseDirection(LiquifyMode mode)
{
    return mode != UNDO &&
        mode != RESTORE_SHAPE;
}

bool KisLiquifyProperties::supportsWashMode(LiquifyMode mode)
{
    return mode != UNDO &&
        mode != RESTORE_SHAPE;
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
    case KisLiquifyProperties::RESTORE_SHAPE:
        result = "RestoreShape";
        break;
    case KisLiquifyProperties::N_MODES:
        qFatal("Unsupported mode");
    }

    return QString("LiquifyTool/%1").arg(result);
}

qreal defaultAmountForMode(KisLiquifyProperties::LiquifyMode mode)
{
    switch (mode) {
    case KisLiquifyProperties::RESTORE_SHAPE:
        return 0.03;
    case KisLiquifyProperties::MOVE:
    case KisLiquifyProperties::SCALE:
    case KisLiquifyProperties::ROTATE:
    case KisLiquifyProperties::OFFSET:
    case KisLiquifyProperties::UNDO:
        return 0.05;
    case KisLiquifyProperties::N_MODES:
        qFatal("Unsupported mode");
    }

    return 0.05;
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
    cfg.writeEntry("preserveShapeRotation", m_preserveShapeRotation);
    cfg.writeEntry("preserveShapeScale", m_preserveShapeScale);
    cfg.writeEntry("preserveShapeStretch", m_preserveShapeStretch);

    KConfigGroup globalCfg =  KSharedConfig::openConfig()->group("LiquifyTool");
    globalCfg.writeEntry("mode", (int)m_mode);
}

void KisLiquifyProperties::loadMode()
{
    KConfigGroup cfg =
         KSharedConfig::openConfig()->group(liquifyModeString(m_mode));

    m_size = cfg.readEntry("size", m_size);
    m_amount = cfg.readEntry("amount", defaultAmountForMode(m_mode));
    m_spacing = cfg.readEntry("spacing", m_spacing);
    m_sizeHasPressure = cfg.readEntry("sizeHasPressure", m_sizeHasPressure);
    m_amountHasPressure = cfg.readEntry("amountHasPressure", m_amountHasPressure);
    m_reverseDirection = cfg.readEntry("reverseDirection", m_reverseDirection);
    m_useWashMode = cfg.readEntry("useWashMode", m_useWashMode);
    m_flow = cfg.readEntry("flow", m_flow);
    m_preserveShapeRotation = cfg.readEntry("preserveShapeRotation", m_preserveShapeRotation);
    m_preserveShapeScale = cfg.readEntry("preserveShapeScale", m_preserveShapeScale);
    m_preserveShapeStretch = cfg.readEntry("preserveShapeStretch", m_preserveShapeStretch);
}

void KisLiquifyProperties::loadAndResetMode()
{
    KConfigGroup globalCfg =  KSharedConfig::openConfig()->group("LiquifyTool");
    const int loadedMode = globalCfg.readEntry("mode", (int)m_mode);
    if (loadedMode >= 0 && loadedMode < N_MODES) {
        m_mode = (LiquifyMode) loadedMode;
    } else if (loadedMode == N_MODES) {
        m_mode = RESTORE_SHAPE;
    } else {
        m_mode = MOVE;
    }

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
    KisDomUtils::saveValue(&liqEl, "preserveShapeRotation", m_preserveShapeRotation);
    KisDomUtils::saveValue(&liqEl, "preserveShapeScale", m_preserveShapeScale);
    KisDomUtils::saveValue(&liqEl, "preserveShapeStretch", m_preserveShapeStretch);
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
    } else if (result && newMode == N_MODES) {
        props.m_mode = RESTORE_SHAPE;
    } else {
        result = false;
    }

    if (result) {
        KisDomUtils::loadValue(liqEl, "preserveShapeRotation", &props.m_preserveShapeRotation);
        KisDomUtils::loadValue(liqEl, "preserveShapeScale", &props.m_preserveShapeScale);
        KisDomUtils::loadValue(liqEl, "preserveShapeStretch", &props.m_preserveShapeStretch);
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
    dbg.space() << "\n    " << ppVar(props.preserveShapeRotation());
    dbg.space() << "\n    " << ppVar(props.preserveShapeScale());
    dbg.space() << "\n    " << ppVar(props.preserveShapeStretch());
    dbg.space() << "\n    );\n";
    return dbg.nospace();
}
