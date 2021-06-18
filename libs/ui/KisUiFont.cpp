/*
 * SPDX-FileCopyrightText: 2020 Halla Rempt <halla@valdyas.org>
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisUiFont.h"

#include <kis_config.h>

#include <boost/optional.hpp>

#include <QtGlobal>
#include <QFontDatabase>

namespace KisUiFont
{

static const QString useCustomSystemFontCfgName = QStringLiteral("use_custom_system_font");
static const QString customSystemFontCfgName = QStringLiteral("custom_system_font");
static const QString customFontSizeCfgName = QStringLiteral("custom_font_size");

/**
 * @brief Gets the system default UI font.
 * @return the system default UI font.
 */
static QFont systemDefaultUiFont();

/**
 * @brief Returns the font the user has configured.
 * @return the user font if it has been set, otherwise `boost::none`.
 */
static boost::optional<QFont> userCfgUiFont();

QFont systemDefaultUiFont()
{
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

boost::optional<QFont> userCfgUiFont()
{
    KisConfig cfg(true);
    if (cfg.readEntry<bool>(useCustomSystemFontCfgName, false)) {
        QString fontName = cfg.readEntry<QString>(customSystemFontCfgName, QString());
        if (fontName.isEmpty()) {
            return boost::none;
        }
        int fontSize = cfg.readEntry<int>(customFontSizeCfgName, -1);
        if (fontSize <= 6) {
            fontSize = systemDefaultUiFont().pointSize();
        }
        return QFont(fontName, fontSize);
    } else {
        return boost::none;
    }
}

QFont normalFont()
{
    if (boost::optional<QFont> userFont = userCfgUiFont()) {
        return *userFont;
    } else {
        return systemDefaultUiFont();
    }
}

QFont dockFont()
{
    QFont font = normalFont();
    font.setPointSizeF(font.pointSizeF() * 0.9);
    return font;
}

} // namespace KisUiFont
