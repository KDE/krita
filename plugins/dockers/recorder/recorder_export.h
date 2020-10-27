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

#ifndef RECORDER_EXPORT_H
#define RECORDER_EXPORT_H

#include <QDialog>

struct RecorderExportSettings
{
    QString name;
    QString inputDirectory;
};

class RecorderExport : public QDialog
{
    Q_OBJECT

public:
    explicit RecorderExport(QWidget *parent = nullptr);
    ~RecorderExport();

    void setup(const RecorderExportSettings &settings);

protected:
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void reject() override;

    // first page
    void onButtonBrowseDirectoryClicked();
    void onSpinInputFpsValueChanged(int value);
    void onSpinFpsValueChanged(int value);
    void onCheckResizeToggled(bool checked);
    void onSpinScaleWidthValueChanged(int value);
    void onSpinScaleHeightValueChanged(int value);
    void onButtonLockRatioToggled(bool checked);
    void onButtonBrowseFfmpegClicked();
    void onComboProfileIndexChanged(int index);
    void onButtonEditProfileClicked();
    void onEditVideoPathChanged(const QString &videoFilePath);
    void onButtonBrowseExportClicked();
    void onButtonExportClicked();
    // second page
    void onButtonCancelExportClicked();
    // ffmpeg
    void onFFMpegStarted();
    void onFFMpegFinished();
    void onFFMpegFinishedWithError(QString error);
    void onFFMpegProgressUpdated(int frameNo);
    // third page
    void onButtonWatchItClicked();
    void onButtonShowInFolderClicked();
    void onButtonRestartClicked();

private:
     bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Q_DISABLE_COPY(RecorderExport)
    class Private;
    Private *const d;
};

#endif // RECORDER_EXPORT_H
