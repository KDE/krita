/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#ifndef KIS_GMIC_FILTER_SETTING_H_
#define KIS_GMIC_FILTER_SETTING_H_

#include <QStringList>
#include <QMetaType>

enum InputLayerMode
{
        NONE = 0, ACTIVE, ALL, ACTIVE_AND_BELOW, ACTIVE_AND_ABOVE, ALL_VISIBLE, ALL_VISIBLE_DECR, ALL_INVISIBLE_DECR, ALL_DECR
};

static QStringList LAYER_MODE_STRINGS = QStringList() << "None"
    << "Active (default)"
    << "All"
    << "Active & below"
    << "Active & above"
    << "All visibles"
    << "All invisibles"
    << "All visibles (decr.)"
    << "All invisibles (decr.)"
    <<"All (decr.)";

class KisGmicFilterSetting
{
public:
    KisGmicFilterSetting();
    ~KisGmicFilterSetting();

    void setGmicCommand(QString cmd);
    const QString& gmicCommand() const;

    InputLayerMode inputLayerMode();
    void setInputLayerMode(InputLayerMode mode);

private:
    QString m_gmicCommand;
    InputLayerMode m_inputLayerMode;
};

Q_DECLARE_METATYPE(KisGmicFilterSetting *)

#endif
