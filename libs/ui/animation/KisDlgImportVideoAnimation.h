#ifndef KISDLGIMPORTVIDEOANIMATION_H
#define KISDLGIMPORTVIDEOANIMATION_H

#include <QObject>
#include <QDir>
#include <QJsonObject>
#include "KoDialog.h"
#include "KisView.h"
#include "kis_properties_configuration.h"
#include "ui_VideoImportDialog.h"

class KisDocument;
class KisMainWindow;

struct KisBasicVideoInfo
{
    QString file;
    int stream = -1;
    int width = 0;
    int height = 0;
    float fps = 0;
    int frames = 0;
    float duration = 0;
    QString encoding;
    QString pixFormat;
    bool hasOverriddenFPS = 0;

};

class KisDlgImportVideoAnimation : public KoDialog
{
    Q_OBJECT

public:
    KisDlgImportVideoAnimation(KisMainWindow *m_mainWindow, KisView *m_activeView);
    QStringList showOpenFileDialog();
    QStringList renderFrames();
    QStringList documentInfo();
    void cleanupWorkDir();


protected Q_SLOTS:
    void slotAddFile();
    void slotNextFrame();
    void slotPrevFrame();
    void slotFrameNumberChanged(int frame);
    void slotVideoSliderChanged();
    void slotVideoTimerTimeout();
    
    void slotDocumentHandlerChanged(int selectedIndex);

    
    void slotFFProbeFile();
    void slotFFMpegFile();



private:
    void toggleInputControls(bool toggleBool);
    void loadVideoFile(const QString &file);
    void CurrentFrameChanged(int frame);
    void updateVideoPreview();
    QStringList makeVideoMimeTypesList();
    KisBasicVideoInfo loadVideoInfo(const QString &inputFile);
    KisPropertiesConfigurationSP loadLastUsedConfiguration(QString configurationID);
    void saveLastUsedConfiguration(QString configurationID, KisPropertiesConfigurationSP config);

private:
    Ui_VideoImportDialog m_ui;
    KisMainWindow *m_mainWindow;
    KisView *m_activeView;

    QTimer *videoSliderTimer;
    QDir videoWorkDir;
    KisBasicVideoInfo videoInfo;
    int currentFrame;
 


    int ffmpegFindInput;


};

#endif // KISDLGIMPORTVIDEOANIMATION_H

