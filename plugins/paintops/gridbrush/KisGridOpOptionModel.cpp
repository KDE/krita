/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGridOpOptionModel.h"

KisGridOpOptionModel::KisGridOpOptionModel(lager::cursor<KisGridOpOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(diameter) {_optionData[&KisGridOpOptionData::diameter]}
    , LAGER_QT(grid_width) {_optionData[&KisGridOpOptionData::grid_width]}
    , LAGER_QT(grid_height) {_optionData[&KisGridOpOptionData::grid_height]}
    
    , LAGER_QT(horizontal_offset) {_optionData[&KisGridOpOptionData::horizontal_offset]}
    , LAGER_QT(vertical_offset) {_optionData[&KisGridOpOptionData::vertical_offset]}
    , LAGER_QT(grid_division_level) {_optionData[&KisGridOpOptionData::grid_division_level]}
    
    , LAGER_QT(grid_pressure_division) {_optionData[&KisGridOpOptionData::grid_pressure_division]}
    , LAGER_QT(grid_scale) {_optionData[&KisGridOpOptionData::grid_scale]}
    , LAGER_QT(grid_vertical_border) {_optionData[&KisGridOpOptionData::grid_vertical_border]}
    
    , LAGER_QT(grid_horizontal_border) {_optionData[&KisGridOpOptionData::grid_horizontal_border]}
    , LAGER_QT(grid_random_border) {_optionData[&KisGridOpOptionData::grid_random_border]}
{
}
