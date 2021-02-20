/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
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

    void setPreview(const QString &preview);

Q_SIGNALS:
    void requestPreview(QString arguments);

private Q_SLOTS:
    void onInputChanged();
    void onLinkActivated(const QString &link);
    void onPreviewToggled(bool checked);

private:
    void fillProfile(const RecorderProfile &profile);

private:
    Ui::RecorderProfileSettings *ui;
};

#endif // RECORDER_PROFILE_SETTINGS_H
