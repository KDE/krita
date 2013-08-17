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

enum OutputMode {   IN_PLACE = 0,
                        NEW_LAYERS,
                        NEW_ACTIVE_LAYERS,
                        NEW_IMAGE
};

static QStringList OUTPUT_MODE_STRINGS = QStringList() << "In place (default)"
    << "New layer(s)"
    << "New active layer(s)"
    << "New image";

// this enum is also index in LAYER_MODE_STRINGS list
enum InputLayerMode {   NONE = 0,
                        ACTIVE_LAYER,
                        ALL_LAYERS,
                        ACTIVE_LAYER_BELOW_LAYER,
                        ACTIVE_LAYER_ABOVE_LAYER,
                        ALL_VISIBLE_LAYERS,
                        ALL_INVISIBLE_LAYERS,
                        ALL_VISIBLE_LAYERS_DECR,
                        ALL_INVISIBLE_DECR,
                        ALL_DECR
};

static QStringList INPUT_MODE_STRINGS = QStringList() << "None"
    << "Active (default)"
    << "All"
    << "Active & below"
    << "Active & above"
    << "All visibles"
    << "All invisibles"
    << "All visibles (decr.)"
    << "All invisibles (decr.)"
    << "All (decr.)";

class KisGmicFilterSetting
{
public:
    KisGmicFilterSetting();
    ~KisGmicFilterSetting();

    void setGmicCommand(QString cmd);
    const QString& gmicCommand() const;

    InputLayerMode inputLayerMode();
    void setInputLayerMode(InputLayerMode mode);

    OutputMode outputMode();
    void setOutputMode(OutputMode mode);

private:
    QString m_gmicCommand;
    InputLayerMode m_inputLayerMode;
    OutputMode m_outputMode;
};

Q_DECLARE_METATYPE(KisGmicFilterSetting *)

#endif
