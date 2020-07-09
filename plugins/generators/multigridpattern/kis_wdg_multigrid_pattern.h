/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#ifndef KIS_WDG_MULTIGRID_PATTERN_H
#define KIS_WDG_MULTIGRID_PATTERN_H

#include <kis_config_widget.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

class Ui_WdgMultigridPatternOptions;

class KisWdgMultigridPattern : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgMultigridPattern(QWidget* parent = 0, const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8());
    ~KisWdgMultigridPattern() override;
public:
    inline const Ui_WdgMultigridPatternOptions* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgMultigridPatternOptions* m_widget;
    const KoColorSpace *m_cs;
};

#endif

