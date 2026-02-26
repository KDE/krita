/*
 * SPDX-FileCopyrightText: 2025 Krita Project
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ComfyUIRemoteDock.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QTimer>
#include <QRandomGenerator>
#include <QPointer>
#include <QInputDialog>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QHttpMultiPart>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>

#include <KSharedConfig>
#include <KConfigGroup>
#include <klocalizedstring.h>
#include <kis_icon_utils.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_signal_auto_connection.h>
#include <kis_image_manager.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

// Minimal ComfyUI default workflow (text2img). Node keys "3".."9" as in ComfyUI basic_api_example.
static const char defaultWorkflow[] = R"({
 "3": {"class_type": "KSampler", "inputs": {"cfg": 8, "denoise": 1, "latent_image": ["5", 0], "model": ["4", 0], "negative": ["7", 0], "positive": ["6", 0], "sampler_name": "euler", "scheduler": "normal", "seed": 0, "steps": 20}},
 "4": {"class_type": "CheckpointLoaderSimple", "inputs": {"ckpt_name": "v1-5-pruned-emaonly.safetensors"}},
 "5": {"class_type": "EmptyLatentImage", "inputs": {"batch_size": 1, "height": 512, "width": 512}},
 "6": {"class_type": "CLIPTextEncode", "inputs": {"clip": ["4", 1], "text": ""}},
 "7": {"class_type": "CLIPTextEncode", "inputs": {"clip": ["4", 1], "text": ""}},
 "8": {"class_type": "VAEDecode", "inputs": {"samples": ["3", 0], "vae": ["4", 2]}},
 "9": {"class_type": "SaveImage", "inputs": {"filename_prefix": "ComfyUI", "images": ["8", 0]}}
})";

// Minimal inpainting workflow: LoadImage (1=image, 2=mask), Checkpoint (4), CLIP (5,6), VAEEncodeForInpaint (7), KSampler (8), VAEDecode (9), SaveImage (10).
static const char inpaintingWorkflowTemplate[] = R"({
 "1": {"class_type": "LoadImage", "inputs": {"image": "IMAGE_PLACEHOLDER"}},
 "2": {"class_type": "LoadImage", "inputs": {"image": "MASK_PLACEHOLDER"}},
 "4": {"class_type": "CheckpointLoaderSimple", "inputs": {"ckpt_name": "CKPT_PLACEHOLDER"}},
 "5": {"class_type": "CLIPTextEncode", "inputs": {"clip": ["4", 1], "text": "PROMPT_PLACEHOLDER"}},
 "6": {"class_type": "CLIPTextEncode", "inputs": {"clip": ["4", 1], "text": "NEGATIVE_PLACEHOLDER"}},
 "7": {"class_type": "VAEEncodeForInpaint", "inputs": {"grow_mask_by": 6, "mask": ["2", 1], "pixels": ["1", 0], "vae": ["4", 2]}},
 "8": {"class_type": "KSampler", "inputs": {"cfg": 8, "denoise": 1, "latent_image": ["7", 0], "model": ["4", 0], "negative": ["6", 0], "positive": ["5", 0], "sampler_name": "euler", "scheduler": "normal", "seed": 0, "steps": 20}},
 "9": {"class_type": "VAEDecode", "inputs": {"samples": ["8", 0], "vae": ["4", 2]}},
 "10": {"class_type": "SaveImage", "inputs": {"filename_prefix": "ComfyUI_region", "images": ["9", 0]}}
})";

struct ComfyUIRemoteDock::Private
{
    QPointer<KisViewManager> viewManager;
    QPointer<KisCanvas2> canvas;

    QLineEdit *editServerUrl = nullptr;
    QComboBox *comboCheckpoint = nullptr;
    QPushButton *btnRefreshCheckpoints = nullptr;
    QComboBox *comboPreset = nullptr;
    QComboBox *comboSizePreset = nullptr;
    QPushButton *btnSaveAsPreset = nullptr;
    QPushButton *btnDeletePreset = nullptr;
    QLineEdit *editPrompt = nullptr;
    QLineEdit *editNegative = nullptr;
    QSpinBox *spinWidth = nullptr;
    QSpinBox *spinHeight = nullptr;
    QPushButton *btnTest = nullptr;
    QPushButton *btnGenerate = nullptr;
    QPushButton *btnCancelQueue = nullptr;
    QPushButton *btnInpaint = nullptr;
    QLabel *labelStatus = nullptr;
    QListWidget *listHistory = nullptr;
    QPushButton *btnHistoryReRun = nullptr;
    QPlainTextEdit *editCustomWorkflow = nullptr;
    QPushButton *btnLoadWorkflow = nullptr;

    struct RegionEntry {
        QString name;
        QString prompt;
        QString maskSource; // "selection" or "layer:LayerName"
    };
    QList<RegionEntry> regionEntries;
    QListWidget *listRegions = nullptr;
    QPushButton *btnAddRegion = nullptr;
    QPushButton *btnRemoveRegion = nullptr;
    QPushButton *btnMoveRegionUp = nullptr;
    QPushButton *btnMoveRegionDown = nullptr;
    QPushButton *btnEditRegion = nullptr;
    QPushButton *btnGenerateRegions = nullptr;

    void refreshRegionsList();
    void loadRegionsFromConfig();
    void saveRegionsToConfig();

    struct HistoryEntry {
        QString prompt, negative, checkpoint;
        int width = 512, height = 512;
    };
    QList<HistoryEntry> historyEntries;
    QMap<QString, HistoryEntry> pendingHistoryByPromptId;
    static const int maxHistoryEntries = 20;

    static const int builtinPresetCount = 5; // None, Portrait, Landscape, Anime, Realistic

    void refreshHistoryList();

    QNetworkAccessManager *nam = nullptr;
    QTimer *pollTimer = nullptr;
    QString currentPromptId;      // the one we're currently polling
    QStringList jobQueue;         // prompt_ids waiting (first is running)
    int pollCount = 0;
    static const int maxPollCount = 300; // 5 min at 1s
    KisSignalAutoConnectionsStore connections;

    // Region generation state (used during slotGenerateRegions async chain)
    QImage regionCurrentImage;
    int regionIndex = 0;
    QString regionUploadedImageName;
    QString regionUploadedImageSubfolder;
    QString regionPromptId;
    QString regionMaskUploadedName;
    QString regionMaskUploadedSubfolder;
    int regionPollCount = 0;
    static const int regionMaxPollCount = 300;
};

ComfyUIRemoteDock::ComfyUIRemoteDock()
    : QDockWidget()
    , m_d(new Private)
{
    m_d->nam = new QNetworkAccessManager(this);
    m_d->pollTimer = new QTimer(this);
    m_d->pollTimer->setSingleShot(true);
    connect(m_d->pollTimer, &QTimer::timeout, this, [this]() {
        if (m_d->currentPromptId.isEmpty()) return;
        QUrl base(m_d->editServerUrl->text().trimmed());
        if (!base.isValid()) return;
        base.setPath(base.path() + "/history/" + m_d->currentPromptId);
        QNetworkRequest req(base);
        QNetworkReply *reply = m_d->nam->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                m_d->labelStatus->setText(i18n("History error: %1", reply->errorString()));
                m_d->currentPromptId.clear();
                if (!m_d->jobQueue.isEmpty()) {
                    m_d->currentPromptId = m_d->jobQueue.takeFirst();
                    m_d->pollCount = 0;
                    startPolling();
                } else {
                    m_d->btnGenerate->setEnabled(true);
                }
                updateQueueStatus();
                return;
            }
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject hist = doc.object().value(m_d->currentPromptId).toObject();
            QJsonObject outputs = hist.value("outputs").toObject();
            if (outputs.isEmpty()) {
                m_d->pollCount++;
                if (m_d->pollCount >= Private::maxPollCount) {
                    m_d->labelStatus->setText(i18n("Generation timed out."));
                    m_d->currentPromptId.clear();
                    if (!m_d->jobQueue.isEmpty()) {
                        m_d->currentPromptId = m_d->jobQueue.takeFirst();
                        m_d->pollCount = 0;
                        startPolling();
                    } else {
                        m_d->btnGenerate->setEnabled(true);
                    }
                    updateQueueStatus();
                    return;
                }
                updateQueueStatus();
                m_d->pollTimer->start(1000);
                return;
            }
            // Find SaveImage output (node "9" in default workflow)
            QString filename, subfolder;
            for (const QString &nodeId : outputs.keys()) {
                QJsonObject nodeOut = outputs.value(nodeId).toObject();
                QJsonArray images = nodeOut.value("images").toArray();
                if (!images.isEmpty()) {
                    QJsonObject img = images.at(0).toObject();
                    filename = img.value("filename").toString();
                    subfolder = img.value("subfolder").toString();
                    break;
                }
            }
            if (filename.isEmpty()) {
                m_d->labelStatus->setText(i18n("No image in output."));
                m_d->currentPromptId.clear();
                if (!m_d->jobQueue.isEmpty()) {
                    m_d->currentPromptId = m_d->jobQueue.takeFirst();
                    m_d->pollCount = 0;
                    startPolling();
                } else {
                    m_d->btnGenerate->setEnabled(true);
                }
                updateQueueStatus();
                return;
            }
            QString completedId = m_d->currentPromptId;
            m_d->currentPromptId.clear();
            if (m_d->jobQueue.isEmpty()) {
                m_d->btnGenerate->setEnabled(true);
            }
            QUrl baseUrl(m_d->editServerUrl->text().trimmed());
            QUrl viewUrl(baseUrl);
            QString path = viewUrl.path();
            if (!path.endsWith('/')) path += '/';
            path += "view";
            viewUrl.setPath(path);
            QUrlQuery q;
            q.addQueryItem("filename", filename);
            if (!subfolder.isEmpty()) q.addQueryItem("subfolder", subfolder);
            viewUrl.setQuery(q);
            QNetworkRequest req(viewUrl);
            QNetworkReply *getReply = m_d->nam->get(req);
            connect(getReply, &QNetworkReply::finished, this, [this, getReply, completedId]() {
                getReply->deleteLater();
                if (getReply->error() != QNetworkReply::NoError) {
                    m_d->labelStatus->setText(i18n("Download error: %1", getReply->errorString()));
                    if (!m_d->jobQueue.isEmpty()) {
                        m_d->currentPromptId = m_d->jobQueue.takeFirst();
                        m_d->pollCount = 0;
                        startPolling();
                    } else {
                        m_d->btnGenerate->setEnabled(true);
                    }
                    updateQueueStatus();
                    return;
                }
                QByteArray data = getReply->readAll();
                QString suffix = "png";
                if (data.startsWith("\x89PNG")) suffix = "png";
                else if (data.startsWith("\xff\xd8")) suffix = "jpg";
                QTemporaryFile tmp;
                tmp.setFileTemplate(tmp.fileTemplate() + "." + suffix);
                if (!tmp.open()) {
                    m_d->labelStatus->setText(i18n("Could not create temp file."));
                    if (!m_d->jobQueue.isEmpty()) {
                        m_d->currentPromptId = m_d->jobQueue.takeFirst();
                        m_d->pollCount = 0;
                        startPolling();
                    } else {
                        m_d->btnGenerate->setEnabled(true);
                    }
                    updateQueueStatus();
                    return;
                }
                tmp.write(data);
                tmp.close();
                if (!m_d->viewManager || !m_d->viewManager->imageManager()) {
                    m_d->labelStatus->setText(i18n("No document open."));
                    if (!m_d->jobQueue.isEmpty()) {
                        m_d->currentPromptId = m_d->jobQueue.takeFirst();
                        m_d->pollCount = 0;
                        startPolling();
                    } else {
                        m_d->btnGenerate->setEnabled(true);
                    }
                    updateQueueStatus();
                    return;
                }
                qint32 n = m_d->viewManager->imageManager()->importImage(QUrl::fromLocalFile(tmp.fileName()), "KisPaintLayer");
                if (n > 0) {
                    if (m_d->canvas) m_d->canvas->update();
                }
                if (m_d->pendingHistoryByPromptId.contains(completedId)) {
                    m_d->historyEntries.prepend(m_d->pendingHistoryByPromptId.take(completedId));
                    while (m_d->historyEntries.size() > Private::maxHistoryEntries)
                        m_d->historyEntries.removeLast();
                    refreshHistoryList();
                }
                if (!m_d->jobQueue.isEmpty()) {
                    m_d->currentPromptId = m_d->jobQueue.takeFirst();
                    m_d->pollCount = 0;
                    startPolling();
                }
                updateQueueStatus();
            });
        });
    });

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    m_d->editServerUrl = new QLineEdit();
    m_d->editServerUrl->setPlaceholderText(i18n("e.g. http://192.168.1.10:8188"));
    m_d->editServerUrl->setClearButtonEnabled(true);
    layout->addWidget(new QLabel(i18n("ComfyUI server URL:")));
    layout->addWidget(m_d->editServerUrl);

    m_d->comboCheckpoint = new QComboBox();
    m_d->comboCheckpoint->setEditable(true);
    m_d->comboCheckpoint->setInsertPolicy(QComboBox::NoInsert);
    m_d->comboCheckpoint->addItem("v1-5-pruned-emaonly.safetensors");
    m_d->btnRefreshCheckpoints = new QPushButton(i18n("Refresh"));
    m_d->btnRefreshCheckpoints->setToolTip(i18n("Load checkpoint list from server"));
    connect(m_d->btnRefreshCheckpoints, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotRefreshCheckpoints);
    QHBoxLayout *checkpointRow = new QHBoxLayout();
    checkpointRow->addWidget(new QLabel(i18n("Checkpoint:")));
    checkpointRow->addWidget(m_d->comboCheckpoint, 1);
    checkpointRow->addWidget(m_d->btnRefreshCheckpoints);
    layout->addLayout(checkpointRow);

    m_d->comboPreset = new QComboBox();
    m_d->comboPreset->addItem(i18n("None"));
    m_d->comboPreset->addItem(i18n("Portrait"));
    m_d->comboPreset->addItem(i18n("Landscape"));
    m_d->comboPreset->addItem(i18n("Anime"));
    m_d->comboPreset->addItem(i18n("Realistic"));
    KConfigGroup cfg = KSharedConfig::openConfig()->group("ComfyUIRemote");
    QStringList customNames = cfg.readEntry("PresetNames", QStringList());
    for (const QString &name : customNames) {
        if (!name.isEmpty()) m_d->comboPreset->addItem(name);
    }
    connect(m_d->comboPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ComfyUIRemoteDock::slotPresetChanged);
    m_d->btnSaveAsPreset = new QPushButton(i18n("Save as preset"));
    m_d->btnDeletePreset = new QPushButton(i18n("Delete preset"));
    connect(m_d->btnSaveAsPreset, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotSaveAsPreset);
    connect(m_d->btnDeletePreset, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotDeletePreset);
    QHBoxLayout *presetRow = new QHBoxLayout();
    presetRow->addWidget(new QLabel(i18n("Preset:")));
    presetRow->addWidget(m_d->comboPreset, 1);
    presetRow->addWidget(m_d->btnSaveAsPreset);
    presetRow->addWidget(m_d->btnDeletePreset);
    layout->addLayout(presetRow);
    m_d->btnDeletePreset->setEnabled(false);

    layout->addWidget(new QLabel(i18n("Custom workflow (optional, API JSON):")));
    m_d->editCustomWorkflow = new QPlainTextEdit();
    m_d->editCustomWorkflow->setPlaceholderText(i18n("Paste ComfyUI API workflow from File → Export (API), or leave empty for default text2img."));
    m_d->editCustomWorkflow->setMaximumHeight(80);
    layout->addWidget(m_d->editCustomWorkflow);
    m_d->btnLoadWorkflow = new QPushButton(i18n("Load from file…"));
    connect(m_d->btnLoadWorkflow, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotLoadWorkflowFromFile);
    layout->addWidget(m_d->btnLoadWorkflow);

    m_d->editPrompt = new QLineEdit();
    m_d->editPrompt->setPlaceholderText(i18n("Positive prompt"));
    layout->addWidget(new QLabel(i18n("Prompt:")));
    layout->addWidget(m_d->editPrompt);

    m_d->editNegative = new QLineEdit();
    m_d->editNegative->setPlaceholderText(i18n("Negative prompt"));
    layout->addWidget(new QLabel(i18n("Negative:")));
    layout->addWidget(m_d->editNegative);

    m_d->comboSizePreset = new QComboBox();
    m_d->comboSizePreset->addItem(i18n("512×512 (default)"), QSize(512, 512));
    m_d->comboSizePreset->addItem(i18n("768×768"), QSize(768, 768));
    m_d->comboSizePreset->addItem(i18n("1024×1024"), QSize(1024, 1024));
    m_d->comboSizePreset->addItem(i18n("2048×2048 (4k)"), QSize(2048, 2048));
    m_d->comboSizePreset->addItem(i18n("4096×4096 (8k)"), QSize(4096, 4096));
    connect(m_d->comboSizePreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        QSize s = m_d->comboSizePreset->itemData(idx).toSize();
        if (s.isValid()) {
            m_d->spinWidth->setValue(s.width());
            m_d->spinHeight->setValue(s.height());
        }
    });
    layout->addWidget(new QLabel(i18n("Size preset:")));
    layout->addWidget(m_d->comboSizePreset);
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    m_d->spinWidth = new QSpinBox();
    m_d->spinWidth->setRange(64, 8192);
    m_d->spinWidth->setValue(512);
    m_d->spinHeight = new QSpinBox();
    m_d->spinHeight->setRange(64, 8192);
    m_d->spinHeight->setValue(512);
    sizeLayout->addWidget(new QLabel(i18n("Width:")));
    sizeLayout->addWidget(m_d->spinWidth);
    sizeLayout->addWidget(new QLabel(i18n("Height:")));
    sizeLayout->addWidget(m_d->spinHeight);
    layout->addLayout(sizeLayout);

    m_d->btnTest = new QPushButton(i18n("Test connection"));
    m_d->btnTest->setIcon(KisIconUtils::loadIcon("network-connect"));
    connect(m_d->btnTest, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotTestConnection);
    layout->addWidget(m_d->btnTest);

    m_d->btnGenerate = new QPushButton(i18n("Generate"));
    m_d->btnGenerate->setIcon(KisIconUtils::loadIcon("run-build"));
    connect(m_d->btnGenerate, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotGenerate);
    layout->addWidget(m_d->btnGenerate);

    m_d->btnInpaint = new QPushButton(i18n("Inpaint (selection)"));
    m_d->btnInpaint->setToolTip(i18n("Generate in selection: use a custom inpainting workflow. Export your image and mask from Krita, upload to ComfyUI, then paste the workflow JSON above."));
    connect(m_d->btnInpaint, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotInpaint);
    layout->addWidget(m_d->btnInpaint);

    QPushButton *btnCancel = new QPushButton(i18n("Cancel queue"));
    btnCancel->setIcon(KisIconUtils::loadIcon("dialog-cancel"));
    connect(btnCancel, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotCancelQueue);
    layout->addWidget(btnCancel);
    m_d->btnCancelQueue = btnCancel;
    m_d->btnCancelQueue->setEnabled(false);

    m_d->labelStatus = new QLabel(i18n("Set server URL and optionally test."));
    m_d->labelStatus->setWordWrap(true);
    layout->addWidget(m_d->labelStatus);

    layout->addWidget(new QLabel(i18n("History:")));
    m_d->listHistory = new QListWidget();
    m_d->listHistory->setMaximumHeight(100);
    connect(m_d->listHistory, &QListWidget::itemSelectionChanged, this, &ComfyUIRemoteDock::slotHistoryItemSelected);
    connect(m_d->listHistory, &QListWidget::doubleClicked, this, &ComfyUIRemoteDock::slotHistoryReRun);
    layout->addWidget(m_d->listHistory);
    m_d->btnHistoryReRun = new QPushButton(i18n("Re-run selected"));
    connect(m_d->btnHistoryReRun, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotHistoryReRun);
    layout->addWidget(m_d->btnHistoryReRun);
    m_d->btnHistoryReRun->setEnabled(false);

    layout->addWidget(new QLabel(i18n("Regions (different prompt per area):")));
    m_d->listRegions = new QListWidget();
    m_d->listRegions->setMaximumHeight(80);
    connect(m_d->listRegions, &QListWidget::doubleClicked, this, &ComfyUIRemoteDock::slotEditRegion);
    layout->addWidget(m_d->listRegions);
    QHBoxLayout *regionBtns = new QHBoxLayout();
    m_d->btnAddRegion = new QPushButton(i18n("Add"));
    m_d->btnRemoveRegion = new QPushButton(i18n("Remove"));
    m_d->btnMoveRegionUp = new QPushButton(i18n("Up"));
    m_d->btnMoveRegionDown = new QPushButton(i18n("Down"));
    m_d->btnEditRegion = new QPushButton(i18n("Edit"));
    m_d->btnGenerateRegions = new QPushButton(i18n("Generate regions"));
    connect(m_d->btnAddRegion, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotAddRegion);
    connect(m_d->btnRemoveRegion, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotRemoveRegion);
    connect(m_d->btnMoveRegionUp, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotMoveRegionUp);
    connect(m_d->btnMoveRegionDown, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotMoveRegionDown);
    connect(m_d->btnEditRegion, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotEditRegion);
    connect(m_d->btnGenerateRegions, &QPushButton::clicked, this, &ComfyUIRemoteDock::slotGenerateRegions);
    regionBtns->addWidget(m_d->btnAddRegion);
    regionBtns->addWidget(m_d->btnRemoveRegion);
    regionBtns->addWidget(m_d->btnMoveRegionUp);
    regionBtns->addWidget(m_d->btnMoveRegionDown);
    regionBtns->addWidget(m_d->btnEditRegion);
    regionBtns->addWidget(m_d->btnGenerateRegions);
    layout->addLayout(regionBtns);

    layout->addStretch();
    setWidget(widget);
    setWindowTitle(i18n("AI Image (ComfyUI)"));
    setEnabled(false);

    loadRegionsFromConfig();
    refreshRegionsList();
}

ComfyUIRemoteDock::~ComfyUIRemoteDock()
{
}

void ComfyUIRemoteDock::setViewManager(KisViewManager *viewManager)
{
    m_d->connections.clear();
    m_d->viewManager = viewManager;
}

void ComfyUIRemoteDock::setCanvas(KoCanvasBase *canvas)
{
    KisCanvas2 *c = dynamic_cast<KisCanvas2 *>(canvas);
    m_d->canvas = c;
    setEnabled(canvas != nullptr);
}

void ComfyUIRemoteDock::unsetCanvas()
{
    setCanvas(nullptr);
}

void ComfyUIRemoteDock::slotPresetChanged(int index)
{
    m_d->btnDeletePreset->setEnabled(index >= Private::builtinPresetCount);
    if (index <= 0) return; // None
    if (index < Private::builtinPresetCount) {
        switch (index) {
        case 1: // Portrait
            m_d->editPrompt->setText("portrait, face, detailed skin, soft lighting");
            m_d->editNegative->setText("blurry, deformed");
            m_d->spinWidth->setValue(512);
            m_d->spinHeight->setValue(768);
            break;
        case 2: // Landscape
            m_d->editPrompt->setText("landscape, scenery, detailed environment, atmosphere");
            m_d->editNegative->setText("blurry, text");
            m_d->spinWidth->setValue(768);
            m_d->spinHeight->setValue(512);
            break;
        case 3: // Anime
            m_d->editPrompt->setText("anime style, vibrant colors, clean lines");
            m_d->editNegative->setText("realistic, photo");
            m_d->spinWidth->setValue(512);
            m_d->spinHeight->setValue(512);
            break;
        case 4: // Realistic
            m_d->editPrompt->setText("photorealistic, 8k, detailed, high quality");
            m_d->editNegative->setText("cartoon, anime, painting");
            m_d->spinWidth->setValue(512);
            m_d->spinHeight->setValue(512);
            break;
        default:
            break;
        }
        return;
    }
    // Custom preset: load from config
    QString name = m_d->comboPreset->itemText(index);
    KConfigGroup cfg = KSharedConfig::openConfig()->group("ComfyUIRemote_Preset_" + name);
    m_d->editPrompt->setText(cfg.readEntry("Prompt", ""));
    m_d->editNegative->setText(cfg.readEntry("Negative", ""));
    m_d->spinWidth->setValue(cfg.readEntry("Width", 512));
    m_d->spinHeight->setValue(cfg.readEntry("Height", 512));
    QString ckpt = cfg.readEntry("Checkpoint", "");
    if (!ckpt.isEmpty()) {
        int i = m_d->comboCheckpoint->findText(ckpt);
        if (i >= 0) m_d->comboCheckpoint->setCurrentIndex(i);
        else m_d->comboCheckpoint->setCurrentText(ckpt);
    }
}

void ComfyUIRemoteDock::slotSaveAsPreset()
{
    QString name = QInputDialog::getText(this, i18n("Save preset"), i18n("Preset name:"), QLineEdit::Normal, QString());
    if (name.trimmed().isEmpty()) return;
    name = name.trimmed();
    KConfigGroup mainCfg = KSharedConfig::openConfig()->group("ComfyUIRemote");
    QStringList names = mainCfg.readEntry("PresetNames", QStringList());
    if (!names.contains(name)) names << name;
    mainCfg.writeEntry("PresetNames", names);
    KConfigGroup presetCfg = KSharedConfig::openConfig()->group("ComfyUIRemote_Preset_" + name);
    presetCfg.writeEntry("Prompt", m_d->editPrompt->text());
    presetCfg.writeEntry("Negative", m_d->editNegative->text());
    presetCfg.writeEntry("Width", m_d->spinWidth->value());
    presetCfg.writeEntry("Height", m_d->spinHeight->value());
    presetCfg.writeEntry("Checkpoint", m_d->comboCheckpoint->currentText());
    mainCfg.config()->sync();
    if (m_d->comboPreset->findText(name) < 0)
        m_d->comboPreset->addItem(name);
    m_d->comboPreset->setCurrentText(name);
    m_d->labelStatus->setText(i18n("Saved preset \"%1\".", name));
}

void ComfyUIRemoteDock::slotDeletePreset()
{
    int idx = m_d->comboPreset->currentIndex();
    if (idx < Private::builtinPresetCount) return;
    QString name = m_d->comboPreset->currentText();
    KConfigGroup mainCfg = KSharedConfig::openConfig()->group("ComfyUIRemote");
    QStringList names = mainCfg.readEntry("PresetNames", QStringList());
    names.removeAll(name);
    mainCfg.writeEntry("PresetNames", names);
    KSharedConfig::openConfig()->deleteGroup("ComfyUIRemote_Preset_" + name);
    mainCfg.config()->sync();
    m_d->comboPreset->removeItem(idx);
    m_d->comboPreset->setCurrentIndex(0);
    m_d->labelStatus->setText(i18n("Deleted preset \"%1\".", name));
}

void ComfyUIRemoteDock::slotLoadWorkflowFromFile()
{
    QString path = QFileDialog::getOpenFileName(this, i18n("Load workflow"), QString(), i18n("JSON files (*.json);;All files (*)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, i18n("Load workflow"), i18n("Could not open file: %1", f.errorString()));
        return;
    }
    m_d->editCustomWorkflow->setPlainText(QString::fromUtf8(f.readAll()));
    m_d->labelStatus->setText(i18n("Loaded workflow from file."));
}

void ComfyUIRemoteDock::slotRefreshCheckpoints()
{
    QString urlStr = m_d->editServerUrl->text().trimmed();
    if (urlStr.isEmpty()) {
        m_d->labelStatus->setText(i18n("Enter a server URL first."));
        return;
    }
    QUrl url(urlStr);
    if (!url.isValid()) {
        m_d->labelStatus->setText(i18n("Invalid URL."));
        return;
    }
    QString path = url.path();
    if (path.isEmpty() || path == "/") url.setPath("/object_info");
    else if (!path.endsWith('/')) url.setPath(path + "/object_info");
    else url.setPath(path + "object_info");
    m_d->labelStatus->setText(i18n("Loading checkpoints…"));
    m_d->btnRefreshCheckpoints->setEnabled(false);
    QNetworkRequest req(url);
    QNetworkReply *reply = m_d->nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_d->btnRefreshCheckpoints->setEnabled(true);
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_d->labelStatus->setText(i18n("Failed to load checkpoints: %1", reply->errorString()));
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        QJsonObject nodeInfo = root.value("CheckpointLoaderSimple").toObject();
        QJsonObject input = nodeInfo.value("input").toObject();
        QJsonObject required = input.value("required").toObject();
        QJsonValue ckptVal = required.value("ckpt_name");
        QStringList names;
        if (ckptVal.isArray()) {
            QJsonArray arr = ckptVal.toArray();
            if (!arr.isEmpty() && arr.at(0).isArray()) {
                for (const QJsonValue &v : arr.at(0).toArray())
                    names << v.toString();
            }
        }
        m_d->comboCheckpoint->clear();
        if (names.isEmpty()) {
            m_d->comboCheckpoint->addItem("v1-5-pruned-emaonly.safetensors");
            m_d->labelStatus->setText(i18n("No checkpoint list in server response (use custom name)."));
        } else {
            m_d->comboCheckpoint->addItems(names);
            m_d->labelStatus->setText(i18n("Loaded %1 checkpoints.", names.size()));
        }
        m_d->comboCheckpoint->setCurrentIndex(0);
    });
}

void ComfyUIRemoteDock::slotTestConnection()
{
    QString urlStr = m_d->editServerUrl->text().trimmed();
    if (urlStr.isEmpty()) {
        m_d->labelStatus->setText(i18n("Enter a server URL."));
        return;
    }
    QUrl url(urlStr);
    if (!url.isValid()) {
        m_d->labelStatus->setText(i18n("Invalid URL."));
        return;
    }
    // ComfyUI GET /system_stats returns server info; use it to test connection
    QString path = url.path();
    if (path.isEmpty() || path == "/") url.setPath("/system_stats");
    else if (!path.endsWith('/')) url.setPath(path + "/system_stats");
    else url.setPath(path + "system_stats");
    m_d->labelStatus->setText(i18n("Connecting…"));
    m_d->btnTest->setEnabled(false);
    QNetworkRequest req(url);
    QNetworkReply *reply = m_d->nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_d->btnTest->setEnabled(true);
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
            m_d->labelStatus->setText(i18n("Connected to ComfyUI."));
        } else {
            m_d->labelStatus->setText(i18n("Connection failed: %1", reply->errorString()));
        }
    });
}

void ComfyUIRemoteDock::slotGenerate()
{
    QString urlStr = m_d->editServerUrl->text().trimmed();
    if (urlStr.isEmpty()) {
        m_d->labelStatus->setText(i18n("Enter a server URL."));
        return;
    }
    QUrl baseUrl(urlStr);
    if (!baseUrl.isValid()) {
        m_d->labelStatus->setText(i18n("Invalid URL."));
        return;
    }
    if (!m_d->viewManager || !m_d->viewManager->image()) {
        m_d->labelStatus->setText(i18n("Open a document first."));
        return;
    }

    QJsonObject workflow;
    QString customJson = m_d->editCustomWorkflow->toPlainText().trimmed();
    if (!customJson.isEmpty()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(customJson.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            m_d->labelStatus->setText(i18n("Custom workflow JSON error: %1", err.errorString()));
            return;
        }
        workflow = doc.object();
    } else {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray(defaultWorkflow), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            m_d->labelStatus->setText(i18n("Workflow JSON error."));
            return;
        }
        workflow = doc.object();
        {
            QJsonObject n3 = workflow["3"].toObject();
            QJsonObject i3 = n3["inputs"].toObject();
            i3["seed"] = static_cast<double>(QRandomGenerator::global()->bounded(2147483647));
            n3["inputs"] = i3;
            workflow["3"] = n3;
        }
        {
            QJsonObject n4 = workflow["4"].toObject();
            QJsonObject i4 = n4["inputs"].toObject();
            QString ckpt = m_d->comboCheckpoint->currentText().trimmed();
            i4["ckpt_name"] = ckpt.isEmpty() ? QString("v1-5-pruned-emaonly.safetensors") : ckpt;
            n4["inputs"] = i4;
            workflow["4"] = n4;
        }
        {
            QJsonObject n5 = workflow["5"].toObject();
            QJsonObject i5 = n5["inputs"].toObject();
            i5["width"] = m_d->spinWidth->value();
            i5["height"] = m_d->spinHeight->value();
            n5["inputs"] = i5;
            workflow["5"] = n5;
        }
        {
            QJsonObject n6 = workflow["6"].toObject();
            QJsonObject i6 = n6["inputs"].toObject();
            i6["text"] = m_d->editPrompt->text().trimmed().isEmpty()
                ? QString("a beautiful painting")
                : m_d->editPrompt->text();
            n6["inputs"] = i6;
            workflow["6"] = n6;
        }
        {
            QJsonObject n7 = workflow["7"].toObject();
            QJsonObject i7 = n7["inputs"].toObject();
            i7["text"] = m_d->editNegative->text().trimmed();
            n7["inputs"] = i7;
            workflow["7"] = n7;
        }
    }

    QJsonObject payload;
    payload["prompt"] = workflow;
    QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QString path = baseUrl.path();
    if (path.isEmpty() || path == "/") baseUrl.setPath("/prompt");
    else if (!path.endsWith('/')) baseUrl.setPath(path + "/prompt");
    else baseUrl.setPath(path + "prompt");

    m_d->labelStatus->setText(i18n("Submitting…"));

    QNetworkRequest req(baseUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_d->nam->post(req, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_d->labelStatus->setText(i18n("Submit error: %1", reply->errorString()));
            m_d->btnGenerate->setEnabled(true);
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        if (obj.contains("error")) {
            m_d->labelStatus->setText(i18n("Server error: %1", obj["error"].toString()));
            m_d->btnGenerate->setEnabled(true);
            return;
        }
        QString promptId = obj["prompt_id"].toString();
        if (promptId.isEmpty()) {
            m_d->labelStatus->setText(i18n("No prompt_id in response."));
            return;
        }
        Private::HistoryEntry entry;
        entry.prompt = m_d->editPrompt->text();
        entry.negative = m_d->editNegative->text();
        entry.checkpoint = m_d->comboCheckpoint->currentText();
        entry.width = m_d->spinWidth->value();
        entry.height = m_d->spinHeight->value();
        m_d->pendingHistoryByPromptId.insert(promptId, entry);
        m_d->jobQueue.append(promptId);
        if (m_d->currentPromptId.isEmpty()) {
            m_d->currentPromptId = m_d->jobQueue.takeFirst();
            m_d->pollCount = 0;
            startPolling();
        }
        updateQueueStatus();
    });
}

void ComfyUIRemoteDock::startPolling()
{
    m_d->labelStatus->setText(i18n("Generating… %1", m_d->pollCount));
    m_d->pollTimer->start(1000);
}

void ComfyUIRemoteDock::updateQueueStatus()
{
    int running = m_d->currentPromptId.isEmpty() ? 0 : 1;
    int queued = m_d->jobQueue.size();
    if (running + queued > 0) {
        if (queued > 0) {
            m_d->labelStatus->setText(i18n("Queue: 1 running, %1 queued.", queued));
        } else {
            m_d->labelStatus->setText(i18n("Generating… %1", m_d->pollCount));
        }
    } else {
        m_d->labelStatus->setText(i18n("Ready."));
    }
    m_d->btnCancelQueue->setEnabled(running + queued > 0);
}

void ComfyUIRemoteDock::refreshHistoryList()
{
    m_d->listHistory->clear();
    for (const Private::HistoryEntry &e : m_d->historyEntries) {
        QString snippet = e.prompt.left(50);
        if (e.prompt.size() > 50) snippet += "…";
        m_d->listHistory->addItem(QString("%1 (%2×%3)").arg(snippet).arg(e.width).arg(e.height));
    }
}

void ComfyUIRemoteDock::slotHistoryItemSelected()
{
    m_d->btnHistoryReRun->setEnabled(m_d->listHistory->currentRow() >= 0);
}

void ComfyUIRemoteDock::refreshRegionsList()
{
    m_d->listRegions->clear();
    for (const Private::RegionEntry &r : m_d->regionEntries) {
        QString src = r.maskSource == "selection" ? i18n("(selection)") : r.maskSource;
        m_d->listRegions->addItem(r.name + " — " + r.prompt.left(30) + (r.prompt.size() > 30 ? "…" : "") + " " + src);
    }
}

void ComfyUIRemoteDock::loadRegionsFromConfig()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("ComfyUIRemote");
    int n = cfg.readEntry("RegionsCount", 0);
    m_d->regionEntries.clear();
    for (int i = 0; i < n; i++) {
        Private::RegionEntry e;
        e.name = cfg.readEntry(QString("Region_%1_Name").arg(i), QString());
        e.prompt = cfg.readEntry(QString("Region_%1_Prompt").arg(i), QString());
        e.maskSource = cfg.readEntry(QString("Region_%1_MaskSource").arg(i), "selection");
        if (!e.name.isEmpty())
            m_d->regionEntries.append(e);
    }
}

void ComfyUIRemoteDock::saveRegionsToConfig()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("ComfyUIRemote");
    cfg.writeEntry("RegionsCount", m_d->regionEntries.size());
    for (int i = 0; i < m_d->regionEntries.size(); i++) {
        const Private::RegionEntry &e = m_d->regionEntries.at(i);
        cfg.writeEntry(QString("Region_%1_Name").arg(i), e.name);
        cfg.writeEntry(QString("Region_%1_Prompt").arg(i), e.prompt);
        cfg.writeEntry(QString("Region_%1_MaskSource").arg(i), e.maskSource);
    }
    cfg.config()->sync();
}

void ComfyUIRemoteDock::slotAddRegion()
{
    QStringList maskSources;
    maskSources << "selection";
    if (m_d->viewManager && m_d->viewManager->image()) {
        KisImageSP image = m_d->viewManager->image();
        if (image->rootLayer()) {
            QList<KisNodeSP> nodes;
            nodes.append(image->rootLayer());
            while (!nodes.isEmpty()) {
                KisNodeSP n = nodes.takeFirst();
                if (KisLayerSP layer = dynamic_cast<KisLayer*>(n.data())) {
                    if (!layer->name().isEmpty())
                        maskSources << "layer:" + layer->name();
                }
                for (int i = 0; i < n->childCount(); i++)
                    nodes.append(n->at(i));
            }
        }
    }
    QDialog dlg(this);
    dlg.setWindowTitle(i18n("Add region"));
    QFormLayout *form = new QFormLayout(&dlg);
    QLineEdit *editName = new QLineEdit();
    editName->setPlaceholderText(i18n("e.g. Background"));
    QLineEdit *editPrompt = new QLineEdit();
    editPrompt->setPlaceholderText(i18n("Prompt for this area"));
    QComboBox *comboMask = new QComboBox();
    comboMask->addItem(i18n("Current selection"), "selection");
    for (const QString &s : maskSources) {
        if (s == "selection") continue;
        comboMask->addItem(s, s);
    }
    form->addRow(i18n("Name:"), editName);
    form->addRow(i18n("Prompt:"), editPrompt);
    form->addRow(i18n("Mask source:"), comboMask);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(box);
    if (dlg.exec() != QDialog::Accepted) return;
    Private::RegionEntry e;
    e.name = editName->text().trimmed().isEmpty() ? i18n("Region %1", m_d->regionEntries.size() + 1) : editName->text().trimmed();
    e.prompt = editPrompt->text().trimmed();
    e.maskSource = comboMask->currentData().toString();
    m_d->regionEntries.append(e);
    saveRegionsToConfig();
    refreshRegionsList();
    m_d->labelStatus->setText(i18n("Added region \"%1\".", e.name));
}

void ComfyUIRemoteDock::slotRemoveRegion()
{
    int row = m_d->listRegions->currentRow();
    if (row < 0 || row >= m_d->regionEntries.size()) return;
    QString name = m_d->regionEntries.at(row).name;
    m_d->regionEntries.removeAt(row);
    saveRegionsToConfig();
    refreshRegionsList();
    m_d->labelStatus->setText(i18n("Removed region \"%1\".", name));
}

void ComfyUIRemoteDock::slotMoveRegionUp()
{
    int row = m_d->listRegions->currentRow();
    if (row <= 0 || row >= m_d->regionEntries.size()) return;
    m_d->regionEntries.move(row, row - 1);
    saveRegionsToConfig();
    refreshRegionsList();
    m_d->listRegions->setCurrentRow(row - 1);
}

void ComfyUIRemoteDock::slotMoveRegionDown()
{
    int row = m_d->listRegions->currentRow();
    if (row < 0 || row >= m_d->regionEntries.size() - 1) return;
    m_d->regionEntries.move(row, row + 1);
    saveRegionsToConfig();
    refreshRegionsList();
    m_d->listRegions->setCurrentRow(row + 1);
}

void ComfyUIRemoteDock::slotEditRegion()
{
    int row = m_d->listRegions->currentRow();
    if (row < 0 || row >= m_d->regionEntries.size()) return;
    QStringList maskSources;
    maskSources << "selection";
    if (m_d->viewManager && m_d->viewManager->image()) {
        KisImageSP image = m_d->viewManager->image();
        if (image->rootLayer()) {
            QList<KisNodeSP> nodes;
            nodes.append(image->rootLayer());
            while (!nodes.isEmpty()) {
                KisNodeSP n = nodes.takeFirst();
                if (KisLayerSP layer = dynamic_cast<KisLayer*>(n.data())) {
                    if (!layer->name().isEmpty())
                        maskSources << "layer:" + layer->name();
                }
                for (int i = 0; i < n->childCount(); i++)
                    nodes.append(n->at(i));
            }
        }
    }
    QDialog dlg(this);
    dlg.setWindowTitle(i18n("Edit region"));
    QFormLayout *form = new QFormLayout(&dlg);
    QLineEdit *editName = new QLineEdit(m_d->regionEntries.at(row).name);
    QLineEdit *editPrompt = new QLineEdit(m_d->regionEntries.at(row).prompt);
    QComboBox *comboMask = new QComboBox();
    comboMask->addItem(i18n("Current selection"), "selection");
    for (const QString &s : maskSources) {
        if (s == "selection") continue;
        comboMask->addItem(s, s);
    }
    int idx = comboMask->findData(m_d->regionEntries.at(row).maskSource);
    if (idx >= 0) comboMask->setCurrentIndex(idx);
    else comboMask->setCurrentText(m_d->regionEntries.at(row).maskSource);
    form->addRow(i18n("Name:"), editName);
    form->addRow(i18n("Prompt:"), editPrompt);
    form->addRow(i18n("Mask source:"), comboMask);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(box);
    if (dlg.exec() != QDialog::Accepted) return;
    m_d->regionEntries[row].name = editName->text().trimmed().isEmpty() ? m_d->regionEntries[row].name : editName->text().trimmed();
    m_d->regionEntries[row].prompt = editPrompt->text().trimmed();
    m_d->regionEntries[row].maskSource = comboMask->currentData().toString();
    saveRegionsToConfig();
    refreshRegionsList();
}

namespace
{
QImage getCanvasAsQImage(KisImageSP image)
{
    if (!image || !image->projection()) return QImage();
    QRect bounds = image->bounds();
    if (bounds.isEmpty()) return QImage();
    const KoColorProfile *profile = image->colorSpace() ? image->colorSpace()->profile() : nullptr;
    return image->projection()->convertToQImage(profile, bounds,
        KoColorConversionTransformation::internalRenderingIntent(),
        KoColorConversionTransformation::internalConversionFlags());
}

QImage getMaskAsQImage(KisImageSP image, KisViewManager *viewManager, const QString &maskSource)
{
    QRect bounds = image->bounds();
    if (bounds.isEmpty()) return QImage();
    QImage maskImage(bounds.width(), bounds.height(), QImage::Format_Grayscale8);
    maskImage.fill(0);

    if (maskSource == "selection") {
        KisSelectionSP sel = viewManager ? viewManager->selection() : nullptr;
        if (!sel || !sel->pixelSelection()) return QImage();
        KisPaintDeviceSP dev = sel->pixelSelection();
        QRect rect = dev->selectedExactRect();
        rect &= bounds;
        if (rect.isEmpty()) return QImage();
        int ps = dev->pixelSize();
        QVector<quint8> data(rect.width() * rect.height() * ps);
        dev->readBytes(data.data(), rect.x(), rect.y(), rect.width(), rect.height());
        for (int y = 0; y < rect.height(); y++) {
            for (int x = 0; x < rect.width(); x++) {
                int srcIdx = (y * rect.width() + x) * ps;
                quint8 v = ps > 0 ? data.value(srcIdx, 0) : 0;
                maskImage.setPixel(rect.x() + x, rect.y() + y, qRgb(v, v, v));
            }
        }
        return maskImage;
    }

    if (maskSource.startsWith("layer:")) {
        QString layerName = maskSource.mid(6);
        KisNodeSP root = image->rootLayer();
        if (!root) return QImage();
        QList<KisNodeSP> nodes;
        nodes.append(root);
        KisNodeSP foundNode;
        while (!nodes.isEmpty()) {
            KisNodeSP n = nodes.takeFirst();
            if (n->name() == layerName) { foundNode = n; break; }
            for (int i = 0; i < n->childCount(); i++) nodes.append(n->at(i));
        }
        KisLayer *foundLayer = foundNode ? dynamic_cast<KisLayer*>(foundNode.data()) : nullptr;
        if (!foundLayer || !foundLayer->projection()) return QImage();
        const KoColorProfile *profile = image->colorSpace() ? image->colorSpace()->profile() : nullptr;
        QImage rgba = foundLayer->projection()->convertToQImage(profile, bounds,
            KoColorConversionTransformation::internalRenderingIntent(),
            KoColorConversionTransformation::internalConversionFlags());
        if (rgba.isNull() || rgba.size() != maskImage.size()) return QImage();
        for (int y = 0; y < rgba.height(); y++) {
            for (int x = 0; x < rgba.width(); x++) {
                int a = qAlpha(rgba.pixel(x, y));
                maskImage.setPixel(x, y, qRgb(a, a, a));
            }
        }
        return maskImage;
    }
    return QImage();
}

// Composite result over current using mask (mask white = use result pixel).
void compositeWithMask(QImage &current, const QImage &result, const QImage &mask)
{
    if (current.size() != result.size() || current.size() != mask.size() || result.format() != QImage::Format_RGB32) return;
    if (current.format() != QImage::Format_ARGB32 && current.format() != QImage::Format_RGB32)
        current = current.convertToFormat(QImage::Format_ARGB32);
    QImage res = result.format() == QImage::Format_ARGB32 ? result : result.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < current.height(); y++) {
        for (int x = 0; x < current.width(); x++) {
            int m = qGray(mask.pixel(x, y));
            if (m <= 0) continue;
            QRgb cur = current.pixel(x, y);
            QRgb resPix = res.pixel(x, y);
            if (m >= 255) {
                current.setPixel(x, y, resPix);
            } else {
                int inv = 255 - m;
                current.setPixel(x, y, qRgba(
                    (qRed(cur) * inv + qRed(resPix) * m) / 255,
                    (qGreen(cur) * inv + qGreen(resPix) * m) / 255,
                    (qBlue(cur) * inv + qBlue(resPix) * m) / 255,
                    (qAlpha(cur) * inv + qAlpha(resPix) * m) / 255));
            }
        }
    }
}
} // namespace

void ComfyUIRemoteDock::slotGenerateRegions()
{
    if (m_d->regionEntries.isEmpty()) {
        m_d->labelStatus->setText(i18n("Add at least one region (name, prompt, mask source)."));
        return;
    }
    if (!m_d->viewManager || !m_d->viewManager->image()) {
        m_d->labelStatus->setText(i18n("Open a document first."));
        return;
    }
    QString urlStr = m_d->editServerUrl->text().trimmed();
    if (urlStr.isEmpty()) {
        m_d->labelStatus->setText(i18n("Enter a server URL."));
        return;
    }
    QUrl baseUrl(urlStr);
    if (!baseUrl.isValid()) {
        m_d->labelStatus->setText(i18n("Invalid URL."));
        return;
    }
    KisImageSP image = m_d->viewManager->image();
    m_d->regionCurrentImage = getCanvasAsQImage(image);
    if (m_d->regionCurrentImage.isNull()) {
        m_d->labelStatus->setText(i18n("Could not export canvas."));
        return;
    }
    m_d->regionCurrentImage = m_d->regionCurrentImage.convertToFormat(QImage::Format_ARGB32);
    m_d->regionIndex = 0;
    m_d->regionUploadedImageName.clear();
    m_d->regionUploadedImageSubfolder.clear();
    m_d->btnGenerateRegions->setEnabled(false);

    auto setUploadPath = [baseUrl](QUrl &url, const QString &pathSuffix) {
        QString p = url.path();
        if (p.isEmpty() || p == "/") url.setPath("/" + pathSuffix);
        else if (!p.endsWith('/')) url.setPath(p + "/" + pathSuffix);
        else url.setPath(p + pathSuffix);
    };

    // Step 1: Upload canvas image (once).
    QUrl uploadUrl = baseUrl;
    setUploadPath(uploadUrl, "upload/image");
    QTemporaryFile *tmpImage = new QTemporaryFile(this);
    tmpImage->setFileTemplate(tmpImage->fileTemplate() + ".png");
    tmpImage->open();
    tmpImage->write(QByteArray()); // ensure file exists
    tmpImage->close();
    if (!m_d->regionCurrentImage.save(tmpImage->fileName())) {
        m_d->labelStatus->setText(i18n("Could not save temp image."));
        m_d->btnGenerateRegions->setEnabled(true);
        return;
    }
    tmpImage->open();
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant("form-data; name=\"image\"; filename=\"krita_region_canvas.png\""));
    imagePart.setBodyDevice(tmpImage);
    tmpImage->setParent(multiPart);
    multiPart->append(imagePart);
    QNetworkRequest req(uploadUrl);
    QNetworkReply *reply = m_d->nam->post(req, multiPart);
    multiPart->setParent(reply);
    m_d->labelStatus->setText(i18n("Uploading canvas…"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_d->labelStatus->setText(i18n("Upload error: %1", reply->errorString()));
            m_d->btnGenerateRegions->setEnabled(true);
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        m_d->regionUploadedImageName = obj.value("name").toString();
        m_d->regionUploadedImageSubfolder = obj.value("subfolder").toString();
        if (m_d->regionUploadedImageName.isEmpty()) {
            m_d->labelStatus->setText(i18n("Server did not return image name."));
            m_d->btnGenerateRegions->setEnabled(true);
            return;
        }
        runNextRegionInpainting();
    });
}

void ComfyUIRemoteDock::runNextRegionInpainting()
{
    QString urlStr = m_d->editServerUrl->text().trimmed();
    if (urlStr.isEmpty()) { m_d->btnGenerateRegions->setEnabled(true); return; }
    QUrl baseUrl(urlStr);
    if (!baseUrl.isValid()) { m_d->btnGenerateRegions->setEnabled(true); return; }

    if (m_d->regionIndex >= m_d->regionEntries.size()) {
        if (!m_d->viewManager || !m_d->viewManager->imageManager()) {
            m_d->labelStatus->setText(i18n("Regions done (no document to paste)."));
            m_d->btnGenerateRegions->setEnabled(true);
            return;
        }
        QTemporaryFile tmp;
        tmp.setFileTemplate(tmp.fileTemplate() + ".png");
        if (!tmp.open() || !m_d->regionCurrentImage.save(tmp.fileName())) {
            m_d->labelStatus->setText(i18n("Could not save result."));
            m_d->btnGenerateRegions->setEnabled(true);
            return;
        }
        tmp.close();
        qint32 n = m_d->viewManager->imageManager()->importImage(QUrl::fromLocalFile(tmp.fileName()), "KisPaintLayer");
        if (n > 0 && m_d->canvas) m_d->canvas->update();
        m_d->labelStatus->setText(i18n("Regions done. Result added as new layer."));
        m_d->btnGenerateRegions->setEnabled(true);
        return;
    }

    const Private::RegionEntry &region = m_d->regionEntries.at(m_d->regionIndex);
    KisImageSP image = m_d->viewManager->image();
    QImage maskImg = getMaskAsQImage(image, m_d->viewManager, region.maskSource);
    if (maskImg.isNull()) {
        m_d->labelStatus->setText(i18n("Region \"%1\": could not get mask.", region.name));
        m_d->regionIndex++;
        QTimer::singleShot(0, this, &ComfyUIRemoteDock::runNextRegionInpainting);
        return;
    }
    m_d->labelStatus->setText(i18n("Region %1/%2: %3…", m_d->regionIndex + 1, m_d->regionEntries.size(), region.name));

    // Save mask as PNG with alpha (ComfyUI uses alpha: 0 = inpaint after invert).
    QImage maskPng(maskImg.size(), QImage::Format_ARGB32);
    for (int y = 0; y < maskImg.height(); y++)
        for (int x = 0; x < maskImg.width(); x++) {
            int g = qGray(maskImg.pixel(x, y));
            maskPng.setPixel(x, y, qRgba(255, 255, 255, 255 - g));
        }
    QTemporaryFile *tmpMask = new QTemporaryFile(this);
    tmpMask->setFileTemplate(tmpMask->fileTemplate() + ".png");
    tmpMask->open();
    tmpMask->close();
    if (!maskPng.save(tmpMask->fileName())) {
        m_d->regionIndex++;
        QTimer::singleShot(0, this, &ComfyUIRemoteDock::runNextRegionInpainting);
        return;
    }
    tmpMask->open();
    QUrl uploadUrl(baseUrl);
    QString upPath = uploadUrl.path();
    if (upPath.isEmpty() || upPath == "/") uploadUrl.setPath("/upload/image");
    else if (!upPath.endsWith('/')) uploadUrl.setPath(upPath + "/upload/image");
    else uploadUrl.setPath(upPath + "upload/image");
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant("form-data; name=\"image\"; filename=\"krita_region_mask.png\""));
    part.setBodyDevice(tmpMask);
    tmpMask->setParent(multiPart);
    multiPart->append(part);
    QNetworkRequest req(uploadUrl);
    QNetworkReply *reply = m_d->nam->post(req, multiPart);
    multiPart->setParent(reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_d->labelStatus->setText(i18n("Mask upload error: %1", reply->errorString()));
            m_d->regionIndex++;
            runNextRegionInpainting();
            return;
        }
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        m_d->regionMaskUploadedName = obj.value("name").toString();
        m_d->regionMaskUploadedSubfolder = obj.value("subfolder").toString();
        if (m_d->regionMaskUploadedName.isEmpty()) {
            m_d->regionIndex++;
            runNextRegionInpainting();
            return;
        }
        const Private::RegionEntry &r = m_d->regionEntries.at(m_d->regionIndex);
        QJsonParseError err;
        QJsonObject workflow = QJsonDocument::fromJson(QByteArray(inpaintingWorkflowTemplate), &err).object();
        if (err.error != QJsonParseError::NoError) {
            m_d->regionIndex++;
            runNextRegionInpainting();
            return;
        }
        QJsonObject n1 = workflow["1"].toObject();
        QJsonObject i1 = n1["inputs"].toObject();
        i1["image"] = m_d->regionUploadedImageName;
        n1["inputs"] = i1;
        workflow["1"] = n1;
        QJsonObject n2 = workflow["2"].toObject();
        QJsonObject i2 = n2["inputs"].toObject();
        i2["image"] = m_d->regionMaskUploadedName;
        n2["inputs"] = i2;
        workflow["2"] = n2;
        QJsonObject n4 = workflow["4"].toObject();
        QJsonObject i4 = n4["inputs"].toObject();
        i4["ckpt_name"] = m_d->comboCheckpoint->currentText().trimmed().isEmpty() ? QString("v1-5-pruned-emaonly.safetensors") : m_d->comboCheckpoint->currentText().trimmed();
        n4["inputs"] = i4;
        workflow["4"] = n4;
        QJsonObject n5 = workflow["5"].toObject();
        QJsonObject i5 = n5["inputs"].toObject();
        i5["text"] = r.prompt.trimmed().isEmpty() ? QString("a beautiful painting") : r.prompt.trimmed();
        n5["inputs"] = i5;
        workflow["5"] = n5;
        QJsonObject n6 = workflow["6"].toObject();
        QJsonObject i6 = n6["inputs"].toObject();
        i6["text"] = m_d->editNegative->text().trimmed();
        n6["inputs"] = i6;
        workflow["6"] = n6;
        QJsonObject n8 = workflow["8"].toObject();
        QJsonObject i8 = n8["inputs"].toObject();
        i8["seed"] = static_cast<double>(QRandomGenerator::global()->bounded(2147483647));
        n8["inputs"] = i8;
        workflow["8"] = n8;
        QJsonObject payload;
        payload["prompt"] = workflow;
        QUrl promptUrl(m_d->editServerUrl->text().trimmed());
        QString p = promptUrl.path();
        if (p.isEmpty() || p == "/") promptUrl.setPath("/prompt");
        else if (!p.endsWith('/')) promptUrl.setPath(p + "/prompt");
        else promptUrl.setPath(p + "prompt");
        QNetworkRequest reqPrompt(promptUrl);
        reqPrompt.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply *replyPrompt = m_d->nam->post(reqPrompt, QJsonDocument(payload).toJson(QJsonDocument::Compact));
        connect(replyPrompt, &QNetworkReply::finished, this, [this, replyPrompt]() {
            replyPrompt->deleteLater();
            if (replyPrompt->error() != QNetworkReply::NoError) {
                m_d->labelStatus->setText(i18n("Submit error: %1", replyPrompt->errorString()));
                m_d->regionIndex++;
                runNextRegionInpainting();
                return;
            }
            QJsonObject obj = QJsonDocument::fromJson(replyPrompt->readAll()).object();
            if (obj.contains("error")) {
                m_d->labelStatus->setText(i18n("Server: %1", obj["error"].toString()));
                m_d->regionIndex++;
                runNextRegionInpainting();
                return;
            }
            m_d->regionPromptId = obj["prompt_id"].toString();
            if (m_d->regionPromptId.isEmpty()) {
                m_d->regionIndex++;
                runNextRegionInpainting();
                return;
            }
            m_d->regionPollCount = 0;
            pollRegionHistory();
        });
    });
}

void ComfyUIRemoteDock::pollRegionHistory()
{
    if (m_d->regionPromptId.isEmpty()) return;
    QString urlStr = m_d->editServerUrl->text().trimmed();
    if (urlStr.isEmpty()) return;
    QUrl baseUrl(urlStr);
    QString path = baseUrl.path();
    if (path.isEmpty() || path == "/") baseUrl.setPath("/history/" + m_d->regionPromptId);
    else if (!path.endsWith('/')) baseUrl.setPath(path + "/history/" + m_d->regionPromptId);
    else baseUrl.setPath(path + "history/" + m_d->regionPromptId);
    QNetworkRequest req(baseUrl);
    QNetworkReply *reply = m_d->nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_d->labelStatus->setText(i18n("History error: %1", reply->errorString()));
            m_d->regionIndex++;
            runNextRegionInpainting();
            return;
        }
        QJsonObject hist = QJsonDocument::fromJson(reply->readAll()).object().value(m_d->regionPromptId).toObject();
        QJsonObject outputs = hist.value("outputs").toObject();
        if (outputs.isEmpty()) {
            m_d->regionPollCount++;
            if (m_d->regionPollCount >= Private::regionMaxPollCount) {
                m_d->labelStatus->setText(i18n("Region generation timed out."));
                m_d->regionIndex++;
                runNextRegionInpainting();
                return;
            }
            QTimer::singleShot(1000, this, &ComfyUIRemoteDock::pollRegionHistory);
            return;
        }
        QString filename, subfolder;
        for (const QString &nodeId : outputs.keys()) {
            QJsonArray images = outputs.value(nodeId).toObject().value("images").toArray();
            if (!images.isEmpty()) {
                QJsonObject img = images.at(0).toObject();
                filename = img.value("filename").toString();
                subfolder = img.value("subfolder").toString();
                break;
            }
        }
        if (filename.isEmpty()) {
            m_d->regionIndex++;
            runNextRegionInpainting();
            return;
        }
        QUrl viewUrl(m_d->editServerUrl->text().trimmed());
        QString vp = viewUrl.path();
        if (!vp.endsWith('/')) vp += '/';
        vp += "view";
        viewUrl.setPath(vp);
        QUrlQuery q;
        q.addQueryItem("filename", filename);
        if (!subfolder.isEmpty()) q.addQueryItem("subfolder", subfolder);
        viewUrl.setQuery(q);
        QNetworkRequest reqView(viewUrl);
        QNetworkReply *replyView = m_d->nam->get(reqView);
        connect(replyView, &QNetworkReply::finished, this, [this, replyView]() {
            replyView->deleteLater();
            if (replyView->error() != QNetworkReply::NoError) {
                m_d->regionIndex++;
                runNextRegionInpainting();
                return;
            }
            QImage result;
            result.loadFromData(replyView->readAll());
            if (!result.isNull() && result.size() == m_d->regionCurrentImage.size()) {
                QImage maskImg = getMaskAsQImage(m_d->viewManager->image(), m_d->viewManager, m_d->regionEntries.at(m_d->regionIndex).maskSource);
                if (!maskImg.isNull())
                    compositeWithMask(m_d->regionCurrentImage, result.convertToFormat(QImage::Format_ARGB32), maskImg);
            }
            m_d->regionPromptId.clear();
            m_d->regionIndex++;
            runNextRegionInpainting();
        });
    });
}

void ComfyUIRemoteDock::slotInpaint()
{
    if (!m_d->viewManager || !m_d->viewManager->image()) {
        m_d->labelStatus->setText(i18n("Open a document first."));
        return;
    }
    KisSelectionSP sel = m_d->viewManager->selection();
    if (!sel || !sel->pixelSelection()) {
        m_d->labelStatus->setText(i18n("Make a selection to inpaint. Then use a custom inpainting workflow (paste JSON that uses LoadImage + mask)."));
        return;
    }
    QRect rect = sel->pixelSelection()->selectedExactRect();
    if (rect.isEmpty()) {
        m_d->labelStatus->setText(i18n("Selection is empty. Draw a selection, then use a custom inpainting workflow."));
        return;
    }
    m_d->labelStatus->setText(i18n("Inpainting: use File → Export to save image and selection mask. Upload both to ComfyUI (e.g. via its web UI), then paste an inpainting workflow JSON above and click Generate."));
}

void ComfyUIRemoteDock::slotHistoryReRun()
{
    int row = m_d->listHistory->currentRow();
    if (row < 0 || row >= m_d->historyEntries.size()) return;
    const Private::HistoryEntry &e = m_d->historyEntries.at(row);
    m_d->editPrompt->setText(e.prompt);
    m_d->editNegative->setText(e.negative);
    m_d->spinWidth->setValue(e.width);
    m_d->spinHeight->setValue(e.height);
    int i = m_d->comboCheckpoint->findText(e.checkpoint);
    if (i >= 0) m_d->comboCheckpoint->setCurrentIndex(i);
    else m_d->comboCheckpoint->setCurrentText(e.checkpoint);
    slotGenerate();
}

void ComfyUIRemoteDock::slotCancelQueue()
{
    m_d->pollTimer->stop();
    for (const QString &id : m_d->jobQueue)
        m_d->pendingHistoryByPromptId.remove(id);
    if (!m_d->currentPromptId.isEmpty())
        m_d->pendingHistoryByPromptId.remove(m_d->currentPromptId);
    m_d->jobQueue.clear();
    m_d->currentPromptId.clear();
    m_d->btnGenerate->setEnabled(true);
    m_d->btnCancelQueue->setEnabled(false);
    m_d->labelStatus->setText(i18n("Cancelled."));
    QString urlStr = m_d->editServerUrl->text().trimmed();
    if (urlStr.isEmpty()) return;
    QUrl url(urlStr);
    if (!url.isValid()) return;
    QString path = url.path();
    if (path.isEmpty() || path == "/") url.setPath("/interrupt");
    else if (!path.endsWith('/')) url.setPath(path + "/interrupt");
    else url.setPath(path + "interrupt");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_d->nam->post(req, QByteArray("{}"));
}
