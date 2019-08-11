/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DUPLICATEOP_OPTION_H
#define KIS_DUPLICATEOP_OPTION_H

#include <kis_paintop_option.h>

const QString DUPLICATE_HEALING = "Duplicateop/Healing";
const QString DUPLICATE_CORRECT_PERSPECTIVE = "Duplicateop/CorrectPerspective";
const QString DUPLICATE_MOVE_SOURCE_POINT = "Duplicateop/MoveSourcePoint";
const QString DUPLICATE_RESET_SOURCE_POINT = "Duplicateop/ResetSourcePoint";
const QString DUPLICATE_CLONE_FROM_PROJECTION = "Duplicateop/CloneFromProjection";

class KisDuplicateOpOptionsWidget;

class KisDuplicateOpOption : public KisPaintOpOption
{
public:
    KisDuplicateOpOption();

    ~KisDuplicateOpOption() override;
private:
    bool healing() const;
    void setHealing(bool healing);

    bool correctPerspective() const;
    void setPerspective(bool perspective);

    bool moveSourcePoint() const;
    void setMoveSourcePoint(bool move);

    bool resetSourcePoint() const;
    void setResetSourcePoint(bool resetSource);

    bool cloneFromProjection() const;
    void setCloneFromProjection(bool cloneFromProjection);
public:
    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void setImage(KisImageWSP image) override;

private:

    KisDuplicateOpOptionsWidget * m_optionWidget;

};

struct KisDuplicateOptionProperties : public KisPaintopPropertiesBase
{
    bool duplicate_healing;
    bool duplicate_correct_perspective;
    bool duplicate_move_source_point;
    bool duplicate_reset_source_point;
    bool duplicate_clone_from_projection;

    void readOptionSettingImpl(const KisPropertiesConfiguration* setting) override {
        duplicate_healing = setting->getBool(DUPLICATE_HEALING, false);
        duplicate_correct_perspective = setting->getBool(DUPLICATE_CORRECT_PERSPECTIVE, false);
        duplicate_move_source_point = setting->getBool(DUPLICATE_MOVE_SOURCE_POINT, true);
        duplicate_reset_source_point = setting->getBool(DUPLICATE_RESET_SOURCE_POINT, false);
        duplicate_clone_from_projection = setting->getBool(DUPLICATE_CLONE_FROM_PROJECTION, false);
    }

    void writeOptionSettingImpl(KisPropertiesConfiguration *setting) const override {
        setting->setProperty(DUPLICATE_HEALING, duplicate_healing);
        setting->setProperty(DUPLICATE_CORRECT_PERSPECTIVE, duplicate_correct_perspective);
        setting->setProperty(DUPLICATE_MOVE_SOURCE_POINT, duplicate_move_source_point);
        setting->setProperty(DUPLICATE_RESET_SOURCE_POINT, duplicate_reset_source_point);
        setting->setProperty(DUPLICATE_CLONE_FROM_PROJECTION, duplicate_clone_from_projection);
    }
};

#endif
