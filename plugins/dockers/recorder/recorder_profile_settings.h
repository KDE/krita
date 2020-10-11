/*
 *  Copyright (c) 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef RECORDER_PROFILE_SETTINGS_H
#define RECORDER_PROFILE_SETTINGS_H

#include <QDialog>

namespace Ui
{
class RecorderProfileSettings;
}

struct RecorderProfile;

class RecorderProfileSettings : public QDialog
{
    Q_OBJECT

public:
    explicit RecorderProfileSettings(QWidget *parent = nullptr);
    ~RecorderProfileSettings();

    // returns true if profile changed
    bool editProfile(RecorderProfile *profile, const RecorderProfile &defaultProfile);

private Q_SLOTS:
    void onInputChanged();

private:
    void fillProfile(const RecorderProfile &profile);

private:
    Ui::RecorderProfileSettings *ui;
};

#endif // RECORDER_PROFILE_SETTINGS_H
