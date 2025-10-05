/*
 *  preferencesdlg.cc - part of KImageShop
 *
 *  SPDX-FileCopyrightText: 1999 Michael Koch <koch@kde.org>
 *  SPDX-FileCopyrightText: 2003-2011 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dlg_preferences.h"

#include <config-hdr.h>
#include <opengl/kis_opengl.h>

#include <QBitmap>
#include <QCheckBox>
#include <QComboBox>
#include <QClipboard>
#include <QCursor>
#include <QScreen>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMdiArea>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSlider>
#include <QStandardPaths>
#include <QThread>
#include <QStyleFactory>
#include <QScreen>
#include <QFontComboBox>
#include <QFont>
#include <QSurfaceFormat>
#include <QColorSpace>

#include <KisApplication.h>
#include <KisDocument.h>
#include <kis_icon.h>
#include <KisPart.h>
#include <KisSpinBoxI18nHelper.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpaceEngine.h>
#include <KoConfigAuthorPage.h>
#include <KoConfig.h>
#include <KoPointerEvent.h>

#include <KoFileDialog.h>
#include "KoID.h"
#include <KoVBox.h>

#include <KTitleWidget>
#include <KoResourcePaths.h>
#include <kformat.h>
#include <klocalizedstring.h>
#include <kstandardguiitem.h>
#include <kundo2stack.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>

#include "KisProofingConfiguration.h"
#include "KisProofingConfigModel.h"
#include "KoColorConversionTransformation.h"
#include "kis_action_registry.h"
#include <kis_image.h>
#include <KisSqueezedComboBox.h>
#include "kis_clipboard.h"
#include "widgets/kis_cmb_idlist.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "kis_canvas_resource_provider.h"
#include "kis_color_manager.h"
#include "kis_config.h"
#include "kis_image_config.h"
#include "kis_preference_set_registry.h"
#include "KisMainWindow.h"
#include "KisMimeDatabase.h"
#include "kis_file_name_requester.h"
#include <KisWidgetConnectionUtils.h>
#include <dialogs/KisFrameRateLimitModel.h>

#include "slider_and_spin_box_sync.h"

// for the performance update
#include <kis_cubic_curve.h>
#include <kis_signals_blocker.h>

#include "input/config/kis_input_configuration_page.h"
#include "input/wintab/drawpile_tablettester/tablettester.h"

#include "KisDlgConfigureCumulativeUndo.h"
#include <config-qt-patches-present.h>

#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
// this include is Qt5-only, the switch to WinTab is embedded in Qt
#  include "config_qt5_has_wintab_switch.h"
#else
#  include <QtGui/private/qguiapplication_p.h>
#  include <QtGui/qpa/qplatformintegration.h>
#endif
#include "config-high-dpi-scale-factor-rounding-policy.h"
#include "KisWindowsPackageUtils.h"
#endif

/**
 * HACK ALERT: this is a function from a private Qt's header qfont_p.h,
 * we don't include the whole header, because it is painful in the
 * environments we don't fully control, e.g. in distribution packages.
 */
Q_GUI_EXPORT int qt_defaultDpi();

QString shortNameOfDisplay(int index)
{
    if (QGuiApplication::screens().length() <= index) {
        return QString();
    }
    QScreen* screen = QGuiApplication::screens()[index];
    QString resolution = QString::number(screen->geometry().width()).append("x").append(QString::number(screen->geometry().height()));
    QString name = screen->name();
    // name and resolution for a specific screen can change, so they are not used in the identifier; but they can help people understand which screen is which

    KisConfig cfg(true);
    QString shortName = resolution + " " + name + " " + cfg.getScreenStringIdentfier(index);
    return shortName;
}


struct WritableLocationValidator : public QValidator {
    WritableLocationValidator(QObject *parent)
        : QValidator(parent)
    {
    }

    State validate(QString &line, int &/*pos*/) const override
    {
        QFileInfo fi(line);
        if (!fi.isWritable()) {
            return Invalid;
        }
        return Acceptable;
    }
};

struct BackupSuffixValidator : public QValidator {
    BackupSuffixValidator(QObject *parent)
        : QValidator(parent)
        , invalidCharacters(QStringList()
                            << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9"
                            << "/" << "\\" << ":" << ";" << " ")
    {}

    ~BackupSuffixValidator() override {}

    const QStringList invalidCharacters;

    State validate(QString &line, int &/*pos*/) const override
    {
        Q_FOREACH(const QString invalidChar, invalidCharacters) {
            if (line.contains(invalidChar)) {
                return Invalid;
            }
        }
        return Acceptable;
    }
};

/*
 * We need this because the final item in the ComboBox is a used as an action to launch file selection dialog.
 * So disabling it makes sure that user doesn't accidentally scroll and select it and get confused why the
 * file picker launched.
 */
class UnscrollableComboBox : public QObject
{
public:
    UnscrollableComboBox(QObject *parent)
        : QObject(parent)
    {
    }

    bool eventFilter(QObject *, QEvent *event) override
    {
        if (event->type() == QEvent::Wheel) {
            event->accept();
            return true;
        }
        return false;
    }
};

GeneralTab::GeneralTab(QWidget *_parent, const char *_name)
    : WdgGeneralSettings(_parent, _name)
{
    KisConfig cfg(true);

    // HACK ALERT!
    // QScrollArea contents are opaque at multiple levels
    // The contents themselves AND the viewport widget
    {
        scrollAreaWidgetContents->setAutoFillBackground(false);
        scrollAreaWidgetContents->parentWidget()->setAutoFillBackground(false);
    }

    //
    // Cursor Tab
    //

    QStringList cursorItems = QStringList()
            << i18n("No Cursor")
            << i18n("Tool Icon")
            << i18n("Arrow")
            << i18n("Small Circle")
            << i18n("Crosshair")
            << i18n("Triangle Righthanded")
            << i18n("Triangle Lefthanded")
            << i18n("Black Pixel")
            << i18n("White Pixel");

    QStringList outlineItems = QStringList()
            << i18nc("Display options label to not DISPLAY brush outline", "No Outline")
            << i18n("Circle Outline")
            << i18n("Preview Outline")
            << i18n("Tilt Outline");

    // brush

    m_cmbCursorShape->addItems(cursorItems);

    m_cmbCursorShape->setCurrentIndex(cfg.newCursorStyle());

    m_cmbOutlineShape->addItems(outlineItems);

    m_cmbOutlineShape->setCurrentIndex(cfg.newOutlineStyle());

    m_showOutlinePainting->setChecked(cfg.showOutlineWhilePainting());
    m_changeBrushOutline->setChecked(!cfg.forceAlwaysFullSizedOutline());

    KoColor cursorColor(KoColorSpaceRegistry::instance()->rgb8());
    cursorColor.fromQColor(cfg.getCursorMainColor());
    cursorColorButton->setColor(cursorColor);

    // eraser

    m_chkSeparateEraserCursor->setChecked(cfg.separateEraserCursor());

    m_cmbEraserCursorShape->addItems(cursorItems);
    m_cmbEraserCursorShape->addItem(i18n("Eraser"));

    m_cmbEraserCursorShape->setCurrentIndex(cfg.eraserCursorStyle());

    m_cmbEraserOutlineShape->addItems(outlineItems);

    m_cmbEraserOutlineShape->setCurrentIndex(cfg.eraserOutlineStyle());

    m_showEraserOutlinePainting->setChecked(cfg.showEraserOutlineWhilePainting());
    m_changeEraserBrushOutline->setChecked(!cfg.forceAlwaysFullSizedEraserOutline());

    KoColor eraserCursorColor(KoColorSpaceRegistry::instance()->rgb8());
    eraserCursorColor.fromQColor(cfg.getEraserCursorMainColor());
    eraserCursorColorButton->setColor(eraserCursorColor);

    //
    // Window Tab
    //
    chkUseCustomFont->setChecked(cfg.readEntry<bool>("use_custom_system_font", false));
    cmbCustomFont->findChild <QComboBox*>("stylesComboBox")->setVisible(false);

    QString fontName = cfg.readEntry<QString>("custom_system_font", "");
    if (fontName.isEmpty()) {
        cmbCustomFont->setCurrentFont(qApp->font());

    }
    else {
        int pointSize = qApp->font().pointSize();
        cmbCustomFont->setCurrentFont(QFont(fontName, pointSize));
    }
    int fontSize = cfg.readEntry<int>("custom_font_size", -1);
    if (fontSize < 0) {
        intFontSize->setValue(qApp->font().pointSize());
    }
    else {
        intFontSize->setValue(fontSize);
    }

    m_cmbMDIType->setCurrentIndex(cfg.readEntry<int>("mdi_viewmode", (int)QMdiArea::TabbedView));
    enableSubWindowOptions(m_cmbMDIType->currentIndex());
    connect(m_cmbMDIType, SIGNAL(currentIndexChanged(int)), SLOT(enableSubWindowOptions(int)));

    m_backgroundimage->setText(cfg.getMDIBackgroundImage());
    connect(m_bnFileName, SIGNAL(clicked()), SLOT(getBackgroundImage()));
    connect(clearBgImageButton, SIGNAL(clicked()), SLOT(clearBackgroundImage()));

    QString xml = cfg.getMDIBackgroundColor();
    KoColor mdiColor = KoColor::fromXML(xml);
    m_mdiColor->setColor(mdiColor);

    m_chkRubberBand->setChecked(cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));

    m_chkCanvasMessages->setChecked(cfg.showCanvasMessages());

    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
    m_chkHiDPI->setChecked(kritarc.value("EnableHiDPI", true).toBool());
#ifdef HAVE_HIGH_DPI_SCALE_FACTOR_ROUNDING_POLICY
    m_chkHiDPIFractionalScaling->setChecked(kritarc.value("EnableHiDPIFractionalScaling", false).toBool());
#else
    m_wdgHiDPIFractionalScaling->setEnabled(false);
#endif
    chkUsageLogging->setChecked(kritarc.value("LogUsage", true).toBool());


    //
    // Tools tab
    //
    m_radioToolOptionsInDocker->setChecked(cfg.toolOptionsInDocker());
    cmbFlowMode->setCurrentIndex((int)!cfg.readEntry<bool>("useCreamyAlphaDarken", true));
    cmbCmykBlendingMode->setCurrentIndex((int)!cfg.readEntry<bool>("useSubtractiveBlendingForCmykColorSpaces", true));
    m_chkSwitchSelectionCtrlAlt->setChecked(cfg.switchSelectionCtrlAlt());
    cmbTouchPainting->addItem(
        KoPointerEvent::tabletInputReceived() ? i18nc("touch painting", "Auto (Disabled)")
                                              : i18nc("touch painting", "Auto (Enabled)"));
    cmbTouchPainting->addItem(i18nc("touch painting", "Enabled"));
    cmbTouchPainting->addItem(i18nc("touch painting", "Disabled"));
    cmbTouchPainting->setCurrentIndex(int(cfg.touchPainting()));
    chkTouchPressureSensitivity->setChecked(cfg.readEntry("useTouchPressureSensitivity", true));
    connect(cmbTouchPainting, SIGNAL(currentIndexChanged(int)),
            SLOT(updateTouchPressureSensitivityEnabled(int)));
    updateTouchPressureSensitivityEnabled(cmbTouchPainting->currentIndex());

    chkEnableTransformToolAfterPaste->setChecked(cfg.activateTransformToolAfterPaste());
    chkZoomHorizontally->setChecked(cfg.zoomHorizontal());

    chkEnableLongPress->setChecked(cfg.longPressEnabled());

    m_groupBoxKineticScrollingSettings->setChecked(cfg.kineticScrollingEnabled());

    m_cmbKineticScrollingGesture->addItem(i18n("On Touch Drag"));
    m_cmbKineticScrollingGesture->addItem(i18n("On Click Drag"));
    m_cmbKineticScrollingGesture->addItem(i18n("On Middle-Click Drag"));
    //m_cmbKineticScrollingGesture->addItem(i18n("On Right Click Drag"));

    spnZoomSteps->setValue(cfg.zoomSteps());


    m_cmbKineticScrollingGesture->setCurrentIndex(cfg.kineticScrollingGesture());
    m_kineticScrollingSensitivitySlider->setRange(0, 100);
    m_kineticScrollingSensitivitySlider->setValue(cfg.kineticScrollingSensitivity());
    m_chkKineticScrollingHideScrollbars->setChecked(cfg.kineticScrollingHiddenScrollbars());

    intZoomMarginSize->setValue(cfg.zoomMarginSize());

    chkEnableSelectionActionBar->setChecked(cfg.selectionActionBar());

    //
    // File handling
    //
    int autosaveInterval = cfg.autoSaveInterval();
    //convert to minutes
    m_autosaveSpinBox->setValue(autosaveInterval / 60);
    m_autosaveCheckBox->setChecked(autosaveInterval > 0);
    chkHideAutosaveFiles->setChecked(cfg.readEntry<bool>("autosavefileshidden", true));

    m_chkCompressKra->setChecked(cfg.compressKra());
    chkZip64->setChecked(cfg.useZip64());
    m_chkTrimKra->setChecked(cfg.trimKra());
    m_chkTrimFramesImport->setChecked(cfg.trimFramesImport());

    m_backupFileCheckBox->setChecked(cfg.backupFile());
    cmbBackupFileLocation->setCurrentIndex(cfg.readEntry<int>("backupfilelocation", 0));
    txtBackupFileSuffix->setText(cfg.readEntry<QString>("backupfilesuffix", "~"));
    QValidator *validator = new BackupSuffixValidator(txtBackupFileSuffix);
    txtBackupFileSuffix->setValidator(validator);
    intNumBackupFiles->setValue(cfg.readEntry<int>("numberofbackupfiles", 1));

    cmbDefaultExportFileType->clear(); 
    QStringList mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);

        QMap<QString, QString> mimeTypeMap;

        foreach (const QString &mimeType, mimeFilter) {
            QString description = KisMimeDatabase::descriptionForMimeType(mimeType);
            mimeTypeMap.insert(description, mimeType);
        }

        // Sort after we get the description because mimeType values have image, application, etc... in front
        QStringList sortedDescriptions = mimeTypeMap.keys();
        sortedDescriptions.sort(Qt::CaseInsensitive);

        cmbDefaultExportFileType->addItem(i18n("All Supported Files"), "all/mime");
        foreach (const QString &description, sortedDescriptions) {
            const QString &mimeType = mimeTypeMap.value(description);
            cmbDefaultExportFileType->addItem(description, mimeType);
        }

        const QString mimeTypeToFind = cfg.exportMimeType(false).toUtf8();
        const int index = cmbDefaultExportFileType->findData(mimeTypeToFind);

        if (index >= 0) {
            cmbDefaultExportFileType->setCurrentIndex(index);
        } else {
            // Index can't be found, default set to image/png
            const QString defaultMimeType = "image/png";
            const int defaultIndex = cmbDefaultExportFileType->findData(defaultMimeType);
            if (defaultIndex >= 0) {
                cmbDefaultExportFileType->setCurrentIndex(defaultIndex);
            } else {
                // Case where the default mime type is also not found in the combo box
                qDebug() << "Default mime type not found in the combo box.";
            }
        }

    QString selectedMimeType = cmbDefaultExportFileType->currentData().toString();

    //
    // Animation tab
    //
    m_chkAutoPin->setChecked(cfg.autoPinLayersToTimeline());
    m_chkAdaptivePlaybackRange->setChecked(cfg.adaptivePlaybackRange());
    m_chkAutoZoom->setChecked(cfg.autoZoomTimelineToPlaybackRange());

    //
    // Miscellaneous tab
    //
    cmbStartupSession->addItem(i18n("Open default window"));
    cmbStartupSession->addItem(i18n("Load previous session"));
    cmbStartupSession->addItem(i18n("Show session manager"));
    cmbStartupSession->setCurrentIndex(cfg.sessionOnStartup());

    chkSaveSessionOnQuit->setChecked(cfg.saveSessionOnQuit(false));

    m_chkConvertOnImport->setChecked(cfg.convertToImageColorspaceOnImport());

    m_undoStackSize->setValue(cfg.undoStackLimit());
    chkCumulativeUndo->setChecked(cfg.useCumulativeUndoRedo());
    connect(chkCumulativeUndo, SIGNAL(toggled(bool)), btnAdvancedCumulativeUndo, SLOT(setEnabled(bool)));
    btnAdvancedCumulativeUndo->setEnabled(chkCumulativeUndo->isChecked());
    connect(btnAdvancedCumulativeUndo, SIGNAL(clicked()), SLOT(showAdvancedCumulativeUndoSettings()));
    m_cumulativeUndoData = cfg.cumulativeUndoData();

    chkShowRootLayer->setChecked(cfg.showRootLayer());

    chkRenameMergedLayers->setChecked(KisImageConfig(true).renameMergedLayers());
    chkRenamePastedLayers->setChecked(cfg.renamePastedLayers());
    chkRenameDuplicatedLayers->setChecked(KisImageConfig(true).renameDuplicatedLayers());

    KConfigGroup group = KSharedConfig::openConfig()->group("File Dialogs");
    bool dontUseNative = true;
#ifdef Q_OS_ANDROID
    dontUseNative = false;
#endif
#ifdef Q_OS_UNIX
    if (qgetenv("XDG_CURRENT_DESKTOP") == "KDE") {
        dontUseNative = false;
    }
#endif
#ifdef Q_OS_MACOS
    dontUseNative = false;
#endif
#ifdef Q_OS_WIN
    dontUseNative = false;
#endif
    m_chkNativeFileDialog->setChecked(!group.readEntry("DontUseNativeFileDialog", dontUseNative));

    if (!qEnvironmentVariable("APPIMAGE").isEmpty()) {
        // AppImages don't have access to platform plugins. BUG: 447805
        // Setting the checkbox to false is 
        m_chkNativeFileDialog->setChecked(false);
        m_chkNativeFileDialog->setEnabled(false);
    }

    intMaxBrushSize->setValue(KisImageConfig(true).maxBrushSize());
    chkIgnoreHighFunctionKeys->setChecked(cfg.ignoreHighFunctionKeys());
#ifndef Q_OS_WIN
    // we properly support ignoring high F-keys on Windows only. To support on other platforms
    // we should synchronize KisExtendedModifiersMatcher to ignore the keys as well.
    chkIgnoreHighFunctionKeys->setVisible(false);
#endif

    //
    // Resources
    //
    m_urlResourceFolder->setMode(KoFileDialog::OpenDirectory);
    m_urlResourceFolder->setConfigurationName("resource_directory");
    const QString resourceLocation = KoResourcePaths::getAppDataLocation();
    if (QFileInfo(resourceLocation).isWritable()) {
        m_urlResourceFolder->setFileName(resourceLocation);
    }
    else {
        m_urlResourceFolder->setFileName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }
    QValidator *writableValidator = new WritableLocationValidator(m_urlResourceFolder);
    txtBackupFileSuffix->setValidator(writableValidator);
    connect(m_urlResourceFolder, SIGNAL(textChanged(QString)), SLOT(checkResourcePath()));
    checkResourcePath();

    grpRestartMessage->setPixmap(
        grpRestartMessage->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(QSize(32, 32)));
    grpRestartMessage->setText(i18n("You will need to Restart Krita for the changes to take an effect."));

    grpAndroidWarningMessage->setVisible(false);
    grpAndroidWarningMessage->setPixmap(
        grpAndroidWarningMessage->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(QSize(32, 32)));
    grpAndroidWarningMessage->setText(
        i18n("Saving at a Location picked from the File Picker may slow down the startup!"));

#ifdef Q_OS_ANDROID
    m_urlResourceFolder->setVisible(false);

    m_resourceFolderSelector->setVisible(true);
    m_resourceFolderSelector->installEventFilter(new UnscrollableComboBox(this));

    const QList<QPair<QString, QString>> writableLocations = []() {
        QList<QPair<QString, QString>> writableLocationsAndText;
        // filters out the duplicates
        const QList<QString> locations = []() {
            QStringList filteredLocations;
            const QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
            Q_FOREACH(const QString &location, locations) {
                if (!filteredLocations.contains(location)) {
                    filteredLocations.append(location);
                }
            }
            return filteredLocations;
        }();

        bool isFirst = true;

        Q_FOREACH (QString location, locations) {
            QString text;
            QFileInfo fileLocation(location);
            // The first one that we get from is the "Default"
            if (isFirst) {
                text = i18n("Default");
                isFirst = false;
            } else if (location.startsWith("/data")) {
                text = i18n("Internal Storage");
            } else {
                text = i18n("SD-Card");
            }
            if (fileLocation.isWritable()) {
                writableLocationsAndText.append({text, location});
            }
        }
        return writableLocationsAndText;
    }();

    for (auto it = writableLocations.constBegin(); it != writableLocations.constEnd(); ++it) {
        m_resourceFolderSelector->addItem(it->first + " - " + it->second);
        // we need it to extract out the path
        m_resourceFolderSelector->setItemData(m_resourceFolderSelector->count() - 1, it->second, Qt::UserRole);
    }

    // if the user has selected a custom location, we add it to the list as well.
    if (resourceLocation.startsWith("content://")) {
        m_resourceFolderSelector->addItem(resourceLocation);
        int index = m_resourceFolderSelector->count() - 1;
        m_resourceFolderSelector->setItemData(index, resourceLocation, Qt::UserRole);
        m_resourceFolderSelector->setCurrentIndex(index);
        grpAndroidWarningMessage->setVisible(true);
    } else {
        // find the index of the current resource location in the writableLocation, so we can set our view to that
        auto iterator = std::find_if(writableLocations.constBegin(),
                                     writableLocations.constEnd(),
                                     [&resourceLocation](QPair<QString, QString> location) {
                                         return location.second == resourceLocation;
                                     });

        if (iterator != writableLocations.constEnd()) {
            int index = writableLocations.indexOf(*iterator);
            KIS_SAFE_ASSERT_RECOVER_NOOP(index < m_resourceFolderSelector->count());
            m_resourceFolderSelector->setCurrentIndex(index);
        }
    }

    // this should be the last item we add.
    m_resourceFolderSelector->addItem(i18n("Choose Manually"));

    connect(m_resourceFolderSelector, qOverload<int>(&QComboBox::activated), [this](int index) {
        const int previousIndex = m_resourceFolderSelector->currentIndex();

        // if it is the last item in the last item, then open file picker and set the name returned as the filename
        if (m_resourceFolderSelector->count() - 1 == index) {
            KoFileDialog dialog(this, KoFileDialog::OpenDirectory, "Select Directory");
            const QString selectedDirectory = dialog.filename();

            if (!selectedDirectory.isEmpty()) {
                // if the index above "Choose Manually" is a content Uri, then we just modify it, and then set that as
                // the index.
                if (m_resourceFolderSelector->itemData(index - 1, Qt::DisplayRole)
                        .value<QString>()
                        .startsWith("content://")) {
                    m_resourceFolderSelector->setItemText(index - 1, selectedDirectory);
                    m_resourceFolderSelector->setItemData(index - 1, selectedDirectory, Qt::UserRole);
                    m_resourceFolderSelector->setCurrentIndex(index - 1);
                } else {
                    // There isn't any content Uri in the ComboBox list, so just insert one, and set that as the index.
                    m_resourceFolderSelector->insertItem(index, selectedDirectory);
                    m_resourceFolderSelector->setItemData(index, selectedDirectory, Qt::UserRole);
                    m_resourceFolderSelector->setCurrentIndex(index);
                }
                // since we have selected the custom location, make the warning visible.
                grpAndroidWarningMessage->setVisible(true);
            } else {
                m_resourceFolderSelector->setCurrentIndex(previousIndex);
            }
        }

        // hide-unhide based on the selection of user.
        grpAndroidWarningMessage->setVisible(
            m_resourceFolderSelector->currentData(Qt::UserRole).value<QString>().startsWith("content://"));
    });

#else
    m_resourceFolderSelector->setVisible(false);
#endif

    grpWindowsAppData->setVisible(false);
#ifdef Q_OS_WIN
    QString folderInStandardAppData;
    QString folderInPrivateAppData;
    KoResourcePaths::getAllUserResourceFoldersLocationsForWindowsStore(folderInStandardAppData, folderInPrivateAppData);

    if (!folderInPrivateAppData.isEmpty()) {
        const auto pathToDisplay = [](const QString &path) {
            // Due to how Unicode word wrapping works, the string does not
            // wrap after backslashes in Qt 5.12. We don't want the path to
            // become too long, so we add a U+200B ZERO WIDTH SPACE to allow
            // wrapping. The downside is that we cannot let the user select
            // and copy the path because it now contains invisible unicode
            // code points.
            // See: https://bugreports.qt.io/browse/QTBUG-80892
            return QDir::toNativeSeparators(path).replace(QChar('\\'), QStringLiteral(u"\\\u200B"));
        };

        const QDir privateResourceDir(folderInPrivateAppData);
        const QDir appDataDir(folderInStandardAppData);
        grpWindowsAppData->setPixmap(
            grpWindowsAppData->style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(QSize(32, 32)));
        // Similar text is also used in KisViewManager.cpp
        grpWindowsAppData->setText(i18nc("@info resource folder",
                                         "<p>You are using the Microsoft Store package version of Krita. "
                                         "Even though Krita can be configured to place resources under the "
                                         "user AppData location, Windows may actually store the files "
                                         "inside a private app location.</p>\n"
                                         "<p>You should check both locations to determine where "
                                         "the files are located.</p>\n"
                                         "<p><b>User AppData</b> (<a href=\"copyuser\">Copy</a>):<br/>\n"
                                         "%1</p>\n"
                                         "<p><b>Private app location</b> (<a href=\"copyprivate\">Copy</a>):<br/>\n"
                                         "%2</p>",
                                         pathToDisplay(appDataDir.absolutePath()),
                                         pathToDisplay(privateResourceDir.absolutePath())));
        grpWindowsAppData->setVisible(true);

        connect(grpWindowsAppData,
                &KisWarningBlock::linkActivated,
                [userPath = appDataDir.absolutePath(),
                 privatePath = privateResourceDir.absolutePath()](const QString &link) {
                    if (link == QStringLiteral("copyuser")) {
                        qApp->clipboard()->setText(QDir::toNativeSeparators(userPath));
                    } else if (link == QStringLiteral("copyprivate")) {
                        qApp->clipboard()->setText(QDir::toNativeSeparators(privatePath));
                    } else {
                        qWarning() << "Unexpected link activated in lblWindowsAppDataNote:" << link;
                    }
                });
    }
#endif


    const int forcedFontDPI = cfg.readEntry("forcedDpiForQtFontBugWorkaround", -1);
    chkForcedFontDPI->setChecked(forcedFontDPI > 0);
    intForcedFontDPI->setValue(forcedFontDPI > 0 ? forcedFontDPI : qt_defaultDpi());
    intForcedFontDPI->setEnabled(forcedFontDPI > 0);
    connect(chkForcedFontDPI, SIGNAL(toggled(bool)), intForcedFontDPI, SLOT(setEnabled(bool)));

    m_pasteFormatGroup.addButton(btnDownload, KisClipboard::PASTE_FORMAT_DOWNLOAD);
    m_pasteFormatGroup.addButton(btnLocal, KisClipboard::PASTE_FORMAT_LOCAL);
    m_pasteFormatGroup.addButton(btnBitmap, KisClipboard::PASTE_FORMAT_CLIP);
    m_pasteFormatGroup.addButton(btnAsk, KisClipboard::PASTE_FORMAT_ASK);

    QAbstractButton *button = m_pasteFormatGroup.button(cfg.pasteFormat(false));

    Q_ASSERT(button);

    if (button) {
        button->setChecked(true);
    }
}

void GeneralTab::setDefault()
{
    KisConfig cfg(true);

    m_cmbCursorShape->setCurrentIndex(cfg.newCursorStyle(true));
    m_cmbOutlineShape->setCurrentIndex(cfg.newOutlineStyle(true));
    m_chkSeparateEraserCursor->setChecked(cfg.readEntry<bool>("separateEraserCursor", false));
    m_cmbEraserCursorShape->setCurrentIndex(cfg.eraserCursorStyle(true));
    m_cmbEraserOutlineShape->setCurrentIndex(cfg.eraserOutlineStyle(true));

    chkShowRootLayer->setChecked(cfg.showRootLayer(true));
    m_autosaveCheckBox->setChecked(cfg.autoSaveInterval(true) > 0);
    //convert to minutes
    m_autosaveSpinBox->setValue(cfg.autoSaveInterval(true) / 60);
    chkHideAutosaveFiles->setChecked(true);

    m_undoStackSize->setValue(cfg.undoStackLimit(true));
    chkCumulativeUndo->setChecked(cfg.useCumulativeUndoRedo(true));
    m_cumulativeUndoData = cfg.cumulativeUndoData(true);

    m_backupFileCheckBox->setChecked(cfg.backupFile(true));
    cmbBackupFileLocation->setCurrentIndex(0);
    txtBackupFileSuffix->setText("~");
    intNumBackupFiles->setValue(1);

    m_showOutlinePainting->setChecked(cfg.showOutlineWhilePainting(true));
    m_changeBrushOutline->setChecked(!cfg.forceAlwaysFullSizedOutline(true));
    m_showEraserOutlinePainting->setChecked(cfg.showEraserOutlineWhilePainting(true));
    m_changeEraserBrushOutline->setChecked(!cfg.forceAlwaysFullSizedEraserOutline(true));

#if defined Q_OS_ANDROID || defined Q_OS_MACOS || defined Q_OS_WIN
    m_chkNativeFileDialog->setChecked(true);
#else
    m_chkNativeFileDialog->setChecked(false);
#endif

    intMaxBrushSize->setValue(1000);

    chkIgnoreHighFunctionKeys->setChecked(cfg.ignoreHighFunctionKeys(true));


    chkUseCustomFont->setChecked(false);
    cmbCustomFont->setCurrentFont(qApp->font());
    intFontSize->setValue(qApp->font().pointSize());

        
    m_cmbMDIType->setCurrentIndex((int)QMdiArea::TabbedView);
    m_chkRubberBand->setChecked(cfg.useOpenGL(true));
    KoColor mdiColor;
    mdiColor.fromXML(cfg.getMDIBackgroundColor(true));
    m_mdiColor->setColor(mdiColor);
    m_backgroundimage->setText(cfg.getMDIBackgroundImage(true));
    m_chkCanvasMessages->setChecked(cfg.showCanvasMessages(true));
    m_chkCompressKra->setChecked(cfg.compressKra(true));
    m_chkTrimKra->setChecked(cfg.trimKra(true));
    m_chkTrimFramesImport->setChecked(cfg.trimFramesImport(true));
    chkZip64->setChecked(cfg.useZip64(true));
    m_chkHiDPI->setChecked(true);
#ifdef HAVE_HIGH_DPI_SCALE_FACTOR_ROUNDING_POLICY
    m_chkHiDPIFractionalScaling->setChecked(true);
#endif
    chkUsageLogging->setChecked(true);
    m_radioToolOptionsInDocker->setChecked(cfg.toolOptionsInDocker(true));
    cmbFlowMode->setCurrentIndex(0);
    chkEnableLongPress->setChecked(cfg.longPressEnabled(true));
    m_groupBoxKineticScrollingSettings->setChecked(cfg.kineticScrollingEnabled(true));
    m_cmbKineticScrollingGesture->setCurrentIndex(cfg.kineticScrollingGesture(true));
    spnZoomSteps->setValue(cfg.zoomSteps(true));
    m_kineticScrollingSensitivitySlider->setValue(cfg.kineticScrollingSensitivity(true));
    m_chkKineticScrollingHideScrollbars->setChecked(cfg.kineticScrollingHiddenScrollbars(true));
    intZoomMarginSize->setValue(cfg.zoomMarginSize(true));
    m_chkSwitchSelectionCtrlAlt->setChecked(cfg.switchSelectionCtrlAlt(true));
    cmbTouchPainting->setCurrentIndex(int(cfg.touchPainting(true)));
    chkTouchPressureSensitivity->setChecked(true);
    chkEnableTransformToolAfterPaste->setChecked(cfg.activateTransformToolAfterPaste(true));
    chkZoomHorizontally->setChecked(cfg.zoomHorizontal(true));
    m_chkConvertOnImport->setChecked(cfg.convertToImageColorspaceOnImport(true));

    KoColor cursorColor(KoColorSpaceRegistry::instance()->rgb8());
    cursorColor.fromQColor(cfg.getCursorMainColor(true));
    cursorColorButton->setColor(cursorColor);

    KoColor eraserCursorColor(KoColorSpaceRegistry::instance()->rgb8());
    eraserCursorColor.fromQColor(cfg.getEraserCursorMainColor(true));
    eraserCursorColorButton->setColor(eraserCursorColor);


    m_chkAutoPin->setChecked(cfg.autoPinLayersToTimeline(true));
    m_chkAdaptivePlaybackRange->setChecked(cfg.adaptivePlaybackRange(false));

    m_urlResourceFolder->setFileName(KoResourcePaths::getAppDataLocation());

    chkForcedFontDPI->setChecked(false);
    intForcedFontDPI->setValue(qt_defaultDpi());
    intForcedFontDPI->setEnabled(false);

    chkRenameMergedLayers->setChecked(KisImageConfig(true).renameMergedLayers(true));
    chkRenamePastedLayers->setChecked(cfg.renamePastedLayers(true));
    chkRenameDuplicatedLayers->setChecked(KisImageConfig(true).renameDuplicatedLayers(true));

    QAbstractButton *button = m_pasteFormatGroup.button(cfg.pasteFormat(true));
    Q_ASSERT(button);

    if (button) {
        button->setChecked(true);
    }
}

void GeneralTab::showAdvancedCumulativeUndoSettings()
{
    KisDlgConfigureCumulativeUndo dlg(m_cumulativeUndoData, m_undoStackSize->value(), this);
    if (dlg.exec() == KoDialog::Accepted) {
        m_cumulativeUndoData = dlg.cumulativeUndoData();
    }
}

CursorStyle GeneralTab::cursorStyle()
{
    return (CursorStyle)m_cmbCursorShape->currentIndex();
}

OutlineStyle GeneralTab::outlineStyle()
{
    return (OutlineStyle)m_cmbOutlineShape->currentIndex();
}

CursorStyle GeneralTab::eraserCursorStyle()
{
    return (CursorStyle)m_cmbEraserCursorShape->currentIndex();
}

OutlineStyle GeneralTab::eraserOutlineStyle()
{
    return (OutlineStyle)m_cmbEraserOutlineShape->currentIndex();
}

KisConfig::SessionOnStartup GeneralTab::sessionOnStartup() const
{
    return (KisConfig::SessionOnStartup)cmbStartupSession->currentIndex();
}

bool GeneralTab::saveSessionOnQuit() const
{
    return chkSaveSessionOnQuit->isChecked();
}

bool GeneralTab::showRootLayer()
{
    return chkShowRootLayer->isChecked();
}

int GeneralTab::autoSaveInterval()
{
    //convert to seconds
    return m_autosaveCheckBox->isChecked() ? m_autosaveSpinBox->value() * 60 : 0;
}

int GeneralTab::undoStackSize()
{
    return m_undoStackSize->value();
}

bool GeneralTab::showOutlineWhilePainting()
{
    return m_showOutlinePainting->isChecked();
}

bool GeneralTab::showEraserOutlineWhilePainting()
{
    return m_showEraserOutlinePainting->isChecked();
}

int GeneralTab::mdiMode()
{
    return m_cmbMDIType->currentIndex();
}

bool GeneralTab::showCanvasMessages()
{
    return m_chkCanvasMessages->isChecked();
}

bool GeneralTab::compressKra()
{
    return m_chkCompressKra->isChecked();
}

bool GeneralTab::trimKra()
{
    return m_chkTrimKra->isChecked();
}

bool GeneralTab::trimFramesImport()
{
    return m_chkTrimFramesImport->isChecked();
}

QString GeneralTab::exportMimeType()
{
    return cmbDefaultExportFileType->currentData().toString();
}

bool GeneralTab::useZip64()
{
    return chkZip64->isChecked();
}

bool GeneralTab::toolOptionsInDocker()
{
    return m_radioToolOptionsInDocker->isChecked();
}

int GeneralTab::zoomSteps()
{
    return spnZoomSteps->value();
}

bool GeneralTab::longPressEnabled()
{
    return chkEnableLongPress->isChecked();
}

bool GeneralTab::kineticScrollingEnabled()
{
    return m_groupBoxKineticScrollingSettings->isChecked();
}

int GeneralTab::kineticScrollingGesture()
{
    return m_cmbKineticScrollingGesture->currentIndex();
}

int GeneralTab::kineticScrollingSensitivity()
{
    return m_kineticScrollingSensitivitySlider->value();
}

bool GeneralTab::kineticScrollingHiddenScrollbars()
{
    return m_chkKineticScrollingHideScrollbars->isChecked();
}

int GeneralTab::zoomMarginSize()
{
    return intZoomMarginSize->value();
}

bool GeneralTab::switchSelectionCtrlAlt()
{
    return m_chkSwitchSelectionCtrlAlt->isChecked();
}

bool GeneralTab::convertToImageColorspaceOnImport()
{
    return m_chkConvertOnImport->isChecked();
}

bool GeneralTab::autopinLayersToTimeline()
{
    return m_chkAutoPin->isChecked();
}

bool GeneralTab::adaptivePlaybackRange()
{
    return m_chkAdaptivePlaybackRange->isChecked();
}

bool GeneralTab::autoZoomTimelineToPlaybackRange()
{
    return m_chkAutoZoom->isChecked();
}

int GeneralTab::forcedFontDpi()
{
    return chkForcedFontDPI->isChecked() ? intForcedFontDPI->value() : -1;
}

bool GeneralTab::renameMergedLayers()
{
    return chkRenameMergedLayers->isChecked();
}

bool GeneralTab::renamePastedLayers()
{
    return chkRenamePastedLayers->isChecked();
}

bool GeneralTab::renameDuplicatedLayers()
{
    return chkRenameDuplicatedLayers->isChecked();
}

void GeneralTab::getBackgroundImage()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "BackgroundImages");
    dialog.setCaption(i18n("Select a Background Image"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setImageFilters();

    QString fn = dialog.filename();
    // dialog box was canceled or somehow no file was selected
    if (fn.isEmpty()) {
        return;
    }

    QImage image(fn);
    if (image.isNull()) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("%1 is not a valid image file!", fn));
    }
    else {
        m_backgroundimage->setText(fn);
    }
}

void GeneralTab::clearBackgroundImage()
{
    // clearing the background image text will implicitly make the background color be used
    m_backgroundimage->setText("");
}

void GeneralTab::checkResourcePath()
{
    const QFileInfo fi(m_urlResourceFolder->fileName());
    if (!fi.isWritable()) {
        grpNonWritableLocation->setPixmap(
            grpNonWritableLocation->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(QSize(32, 32)));
        grpNonWritableLocation->setText(
            i18nc("@info resource folder", "<b>Warning:</b> this location is not writable."));
        grpNonWritableLocation->setVisible(true);
    } else {
        grpNonWritableLocation->setVisible(false);
    }
}

void GeneralTab::enableSubWindowOptions(int mdi_mode)
{
    group_subWinMode->setEnabled(mdi_mode == QMdiArea::SubWindowView);
}

void GeneralTab::updateTouchPressureSensitivityEnabled(int touchPainting)
{
    chkTouchPressureSensitivity->setEnabled(touchPainting != int(KisConfig::TOUCH_PAINTING_DISABLED));
}


#include "kactioncollection.h"
#include "KisActionsSnapshot.h"

ShortcutSettingsTab::ShortcutSettingsTab(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(name);

    QGridLayout * l = new QGridLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    m_page = new WdgShortcutSettings(this);
    l->addWidget(m_page, 0, 0);


    m_snapshot.reset(new KisActionsSnapshot);

    KisKActionCollection *collection =
        KisPart::instance()->currentMainwindow()->actionCollection();

    Q_FOREACH (QAction *action, collection->actions()) {
        m_snapshot->addAction(action->objectName(), action);
    }

    QMap<QString, KisKActionCollection*> sortedCollections =
        m_snapshot->actionCollections();

    for (auto it = sortedCollections.constBegin(); it != sortedCollections.constEnd(); ++it) {
        m_page->addCollection(it.value(), it.key());
    }
}

ShortcutSettingsTab::~ShortcutSettingsTab()
{
}

void ShortcutSettingsTab::setDefault()
{
    m_page->allDefault();
}

void ShortcutSettingsTab::saveChanges()
{
    m_page->save();
    KisActionRegistry::instance()->settingsPageSaved();
}

void ShortcutSettingsTab::cancelChanges()
{
    m_page->undo();
}

ColorSettingsTab::ColorSettingsTab(QWidget *parent, const char *name)
    : QWidget(parent)
    , m_proofModel(new KisProofingConfigModel())
{
    setObjectName(name);

    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    QGridLayout * l = new QGridLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    m_page = new WdgColorSettings(this);
    l->addWidget(m_page, 0, 0);

    KisConfig cfg(true);

    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    m_colorManagedByOS = mainWindow->managedSurfaceProfile();

    if (!m_colorManagedByOS) {
        m_page->chkUseSystemMonitorProfile->setChecked(cfg.useSystemMonitorProfile());
        connect(m_page->chkUseSystemMonitorProfile, SIGNAL(toggled(bool)), this, SLOT(toggleAllowMonitorProfileSelection(bool)));
    }
    m_page->chkUseSystemMonitorProfile->setVisible(!m_colorManagedByOS);

    m_page->useDefColorSpace->setChecked(cfg.useDefaultColorSpace());
    connect(m_page->useDefColorSpace, SIGNAL(toggled(bool)), this, SLOT(toggleUseDefaultColorSpace(bool)));
    QList<KoID> colorSpaces = KoColorSpaceRegistry::instance()->listKeys();
    for (QList<KoID>::iterator id = colorSpaces.begin(); id != colorSpaces.end(); /* nop */) {
        if (KoColorSpaceRegistry::instance()->colorSpaceColorModelId(id->id()) == AlphaColorModelID) {
            id = colorSpaces.erase(id);
        } else {
            ++id;
        }
    }
    m_page->cmbWorkingColorSpace->setIDList(colorSpaces);
    m_page->cmbWorkingColorSpace->setCurrent(cfg.workingColorSpace());
    m_page->cmbWorkingColorSpace->setEnabled(cfg.useDefaultColorSpace());

    if (!m_colorManagedByOS) {
        m_page->bnAddColorProfile->setIcon(koIcon("document-import-16"));
        connect(m_page->bnAddColorProfile, SIGNAL(clicked()), SLOT(installProfile()));
    }
    m_page->bnAddColorProfile->setVisible(!m_colorManagedByOS);

    {
        QStringList profiles;
        QMap<QString, const KoColorProfile *>  profileList;
        Q_FOREACH(const KoColorProfile *profile, KoColorSpaceRegistry::instance()->profilesFor(RGBAColorModelID.id())) {
            profileList[profile->name()] = profile;
            profiles.append(profile->name());
        }

        std::sort(profiles.begin(), profiles.end());
        Q_FOREACH (const QString profile, profiles) {
            m_page->cmbColorProfileForEXR->addSqueezedItem(profile);
        }

        const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID.id(), Float16BitsColorDepthID.id());
        const QString defaultProfile = KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId);
        const QString userProfile = cfg.readEntry("ExrDefaultColorProfile", defaultProfile);

        m_page->cmbColorProfileForEXR->setCurrent(profiles.contains(userProfile) ? userProfile : defaultProfile);
    }


    if (!m_colorManagedByOS) {
        QFormLayout *monitorProfileGrid = new QFormLayout(m_page->monitorprofileholder);
        monitorProfileGrid->setContentsMargins(0, 0, 0, 0);
        for(int i = 0; i < QGuiApplication::screens().count(); ++i) {
            QLabel *lbl = new QLabel(i18nc("The number of the screen (ordinal) and shortened 'name' of the screen (model + resolution)", "Screen %1 (%2):", i + 1, shortNameOfDisplay(i)));
            lbl->setWordWrap(true);
            m_monitorProfileLabels << lbl;
            KisSqueezedComboBox *cmb = new KisSqueezedComboBox();
            cmb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            monitorProfileGrid->addRow(lbl, cmb);
            m_monitorProfileWidgets << cmb;
        }

        // disable if not Linux as KisColorManager is not yet implemented outside Linux
#ifndef Q_OS_LINUX
        m_page->chkUseSystemMonitorProfile->setChecked(false);
        m_page->chkUseSystemMonitorProfile->setDisabled(true);
        m_page->chkUseSystemMonitorProfile->setHidden(true);
#endif

        refillMonitorProfiles(KoID("RGBA"));

        for(int i = 0; i < QApplication::screens().count(); ++i) {
            if (m_monitorProfileWidgets[i]->contains(cfg.monitorProfile(i))) {
                m_monitorProfileWidgets[i]->setCurrent(cfg.monitorProfile(i));
            }
        }
    } else {
        QVBoxLayout *vboxLayout = new QVBoxLayout(m_page->monitorprofileholder);
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        vboxLayout->addItem(new QSpacerItem(20,20));

        QGroupBox *groupBox = new QGroupBox(i18n("Display's color space is managed by the operating system"));
        vboxLayout->addWidget(groupBox);

        QFormLayout *monitorProfileGrid = new QFormLayout(groupBox);
        monitorProfileGrid->setContentsMargins(0, 0, 0, 0);

        m_chkEnableCanvasColorSpaceManagement =
            new QCheckBox(i18n("Enable canvas color management"), this);

        m_chkEnableCanvasColorSpaceManagement->setToolTip(
            i18n("<p>Enabling canvas color management automatically creates "
                 "a separate native surface for the canvas. It might cause "
                 "performance issues on some systems.</p>"
                 ""
                 "<p>If color management is disabled, Krita will render "
                 "the canvas into the surface of the main window, which "
                 "is considered sRGB. It will cause two limitations:"
                 ""
                 "<ol>"
                 "    <li>the color gamut will be limited to sRGB</li>"
                 "    <li>color proofing mode will be limited to \"use global display settings\", "
                 "        i.e. paper white proofing will become impossible</li>"
                 "</ol>"
                "</p>"));

        monitorProfileGrid->addRow(m_chkEnableCanvasColorSpaceManagement);

        m_canvasSurfaceColorSpace = new KisSqueezedComboBox();
            m_canvasSurfaceColorSpace->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        QLabel *canvasSurfaceColorSpaceLbl = new QLabel(i18n("Canvas surface color space:"), this);
        monitorProfileGrid->addRow(canvasSurfaceColorSpaceLbl, m_canvasSurfaceColorSpace);

        m_canvasSurfaceColorSpace->addSqueezedItem(i18n("Preferred by operating system"), QVariant::fromValue(CanvasSurfaceMode::Preferred));
        m_canvasSurfaceColorSpace->addSqueezedItem(i18n("Rec 709 Gamma 2.2"), QVariant::fromValue(CanvasSurfaceMode::Rec709g22));
        m_canvasSurfaceColorSpace->addSqueezedItem(i18n("Rec 709 Linear"), QVariant::fromValue(CanvasSurfaceMode::Rec709g10));
        m_canvasSurfaceColorSpace->addSqueezedItem(i18n("Unmanaged (testing only)"), QVariant::fromValue(CanvasSurfaceMode::Unmanaged));

        m_canvasSurfaceColorSpace->setToolTip(
            i18n("<p>Color space of the pixels that are transferred to the "
                 "window compositor. Use \"preferred\" space unless you know "
                 "what you are doing</p>"));

        vboxLayout->addItem(new QSpacerItem(20,20));

        QLabel *preferredLbl = new QLabel(i18n("Color space preferred by the operating system:\n%1", mainWindow->osPreferredColorSpaceReport()), this);
        vboxLayout->addWidget(preferredLbl);

        m_chkEnableCanvasColorSpaceManagement->setChecked(cfg.enableCanvasSurfaceColorSpaceManagement());
        auto mode = cfg.canvasSurfaceColorSpaceManagementMode();
        int index = m_canvasSurfaceColorSpace->findData(QVariant::fromValue(mode));
        KIS_SAFE_ASSERT_RECOVER(index >= 0) {
            index = 0;
        }
        m_canvasSurfaceColorSpace->setCurrentIndex(index);

        connect(m_chkEnableCanvasColorSpaceManagement, &QCheckBox::toggled, m_canvasSurfaceColorSpace, &QWidget::setEnabled);
        m_canvasSurfaceColorSpace->setEnabled(m_chkEnableCanvasColorSpaceManagement->isChecked());

        connect(m_chkEnableCanvasColorSpaceManagement, &QCheckBox::toggled, canvasSurfaceColorSpaceLbl, &QWidget::setEnabled);
        canvasSurfaceColorSpaceLbl->setEnabled(m_chkEnableCanvasColorSpaceManagement->isChecked());
    }

    m_page->chkBlackpoint->setChecked(cfg.useBlackPointCompensation());
    m_page->chkAllowLCMSOptimization->setChecked(cfg.allowLCMSOptimization());
    m_page->chkForcePaletteColor->setChecked(cfg.forcePaletteColors());
    KisImageConfig cfgImage(true);

    KisProofingConfigurationSP proofingConfig = cfgImage.defaultProofingconfiguration();
    m_proofModel->data.set(*proofingConfig.data());

    connect(m_page->chkBlackpoint, SIGNAL(toggled(bool)), this, SLOT(updateProofingDisplayInfo()));
    connect(m_page->cmbMonitorIntent, SIGNAL(currentIndexChanged(int)), this, SLOT(updateProofingDisplayInfo()));
    m_page->cmbMonitorIntent->setCurrentIndex(cfg.monitorRenderIntent());
    updateProofingDisplayInfo();

    m_page->sldAdaptationState->setMaximum(m_proofModel->adaptationRangeMax());
    m_page->sldAdaptationState->setMinimum(0);

    m_page->proofingSpaceSelector->showDepth(false);

    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Perceptual"), INTENT_PERCEPTUAL);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Relative Colorimetric"), INTENT_RELATIVE_COLORIMETRIC);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Saturation"), INTENT_SATURATION);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Absolute Colorimetric"), INTENT_ABSOLUTE_COLORIMETRIC);
    m_page->cmbProofingIntent->setModel(m_page->cmbDisplayIntent->model());

    m_page->cmbDisplayMode->addItem(i18nc("Display Mode", "Use global display settings"), int(KisProofingConfiguration::Monitor));
    m_page->cmbDisplayMode->addItem(i18nc("Display Mode", "Simulate paper white and black"), int(KisProofingConfiguration::Paper));
    m_page->cmbDisplayMode->addItem(i18nc("Display Mode", "Custom"), int(KisProofingConfiguration::Custom));

    const KoColorSpace *proofingSpace =  KoColorSpaceRegistry::instance()->colorSpace(m_proofModel->proofingModel(),
                                                                                      m_proofModel->proofingDepth(),
                                                                                      m_proofModel->proofingProfile());
    if (proofingSpace) {
        m_page->proofingSpaceSelector->setCurrentColorSpace(proofingSpace);
    }
    updateProofingWidgets();
    connect(m_page->cmbDisplayMode, SIGNAL(currentIndexChanged(int)), this, SLOT(proofingDisplayModeUpdated()));
    connect(m_page->cmbDisplayIntent, SIGNAL(currentIndexChanged(int)), this, SLOT(proofingDisplayIntentUpdated()));
    connect(m_page->cmbProofingIntent, SIGNAL(currentIndexChanged(int)), this, SLOT(proofingConversionIntentUpdated()));
    connect(m_page->chkDispBlackPoint, &QCheckBox::toggled, m_proofModel.data(), &KisProofingConfigModel::dispBlackPointCompensation);
    connect(m_page->sldAdaptationState, &QSlider::valueChanged, m_proofModel.data(), &KisProofingConfigModel::setadaptationState);
    KisWidgetConnectionUtils::connectControl(m_page->ckbProofBlackPoint, m_proofModel.data(), "convBlackPointCompensation");
    KisWidgetConnectionUtils::connectControl(m_page->gamutAlarm, m_proofModel.data(), "warningColor");
    KisWidgetConnectionUtils::connectWidgetEnabledToProperty(m_page->gbxDisplayTransform, m_proofModel.data(), "enableDisplayToggles");
    KisWidgetConnectionUtils::connectWidgetEnabledToProperty(m_page->sldAdaptationState, m_proofModel.data(), "enableAdaptationSlider");
    KisWidgetConnectionUtils::connectWidgetEnabledToProperty(m_page->chkDispBlackPoint, m_proofModel.data(), "enableDisplayBlackPointCompensation");

    m_pasteBehaviourGroup.addButton(m_page->radioPasteWeb, KisClipboard::PASTE_ASSUME_WEB);
    m_pasteBehaviourGroup.addButton(m_page->radioPasteMonitor, KisClipboard::PASTE_ASSUME_MONITOR);
    m_pasteBehaviourGroup.addButton(m_page->radioPasteAsk, KisClipboard::PASTE_ASK);

    QAbstractButton *button = m_pasteBehaviourGroup.button(cfg.pasteBehaviour());
    Q_ASSERT(button);

    if (button) {
        button->setChecked(true);
    }

    if (!m_colorManagedByOS) {
        toggleAllowMonitorProfileSelection(cfg.useSystemMonitorProfile());
    }
}

void ColorSettingsTab::installProfile()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_colorManagedByOS);

    KoFileDialog dialog(this, KoFileDialog::OpenFiles, "OpenDocumentICC");
    dialog.setCaption(i18n("Install Color Profiles"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setMimeTypeFilters(QStringList() << "application/vnd.iccprofile", "application/vnd.iccprofile");
    QStringList profileNames = dialog.filenames();

    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    Q_ASSERT(iccEngine);

    QString saveLocation = KoResourcePaths::saveLocation("icc_profiles");

    Q_FOREACH (const QString &profileName, profileNames) {
        if (!QFile::copy(profileName, saveLocation + QFileInfo(profileName).fileName())) {
            qWarning() << "Could not install profile!" << saveLocation + QFileInfo(profileName).fileName();
            continue;
        }
        iccEngine->addProfile(saveLocation + QFileInfo(profileName).fileName());
    }

    KisConfig cfg(true);
    refillMonitorProfiles(KoID("RGBA"));

    for(int i = 0; i < QApplication::screens().count(); ++i) {
        if (m_monitorProfileWidgets[i]->contains(cfg.monitorProfile(i))) {
            m_monitorProfileWidgets[i]->setCurrent(cfg.monitorProfile(i));
        }
    }

}

void ColorSettingsTab::toggleAllowMonitorProfileSelection(bool useSystemProfile)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_colorManagedByOS);

    KisConfig cfg(true);

    if (useSystemProfile) {
        QStringList devices = KisColorManager::instance()->devices();
        if (devices.size() == QApplication::screens().count()) {
            for(int i = 0; i < QApplication::screens().count(); ++i) {
                m_monitorProfileWidgets[i]->clear();
                QString monitorForScreen = cfg.monitorForScreen(i, devices[i]);
                Q_FOREACH (const QString &device, devices) {
                    m_monitorProfileLabels[i]->setText(i18nc("The number of the screen (ordinal) and shortened 'name' of the screen (model + resolution)", "Screen %1 (%2):", i + 1, shortNameOfDisplay(i)));
                    m_monitorProfileWidgets[i]->addSqueezedItem(KisColorManager::instance()->deviceName(device), device);
                    if (devices[i] == monitorForScreen) {
                        m_monitorProfileWidgets[i]->setCurrentIndex(i);
                    }
                }
            }
        }
    }
    else {
        refillMonitorProfiles(KoID("RGBA"));

        for(int i = 0; i < QApplication::screens().count(); ++i) {
            if (m_monitorProfileWidgets[i]->contains(cfg.monitorProfile(i))) {
                m_monitorProfileWidgets[i]->setCurrent(cfg.monitorProfile(i));
            }
        }
    }
}

void ColorSettingsTab::toggleUseDefaultColorSpace(bool useDefColorSpace)
{
    m_page->cmbWorkingColorSpace->setEnabled(useDefColorSpace);
}

void ColorSettingsTab::setDefault()
{
    m_page->cmbWorkingColorSpace->setCurrent("RGBA");

    const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID.id(), Float16BitsColorDepthID.id());
    const QString defaultProfile = KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId);
    m_page->cmbColorProfileForEXR->setCurrent(defaultProfile);

    KisConfig cfg(true);

    if (!m_colorManagedByOS) {
        refillMonitorProfiles(KoID("RGBA"));
    } else {
        m_chkEnableCanvasColorSpaceManagement->setChecked(cfg.enableCanvasSurfaceColorSpaceManagement(true));
        auto mode = cfg.canvasSurfaceColorSpaceManagementMode(true);
        int index = m_canvasSurfaceColorSpace->findData(QVariant::fromValue(mode));
        KIS_SAFE_ASSERT_RECOVER(index >= 0) {
            index = 0;
        }
        m_canvasSurfaceColorSpace->setCurrentIndex(index);
    }

    KisImageConfig cfgImage(true);
    KisProofingConfigurationSP proofingConfig =  cfgImage.defaultProofingconfiguration(true);
    m_proofModel->data.set(*proofingConfig.data());
    const KoColorSpace *proofingSpace =  KoColorSpaceRegistry::instance()->colorSpace(m_proofModel->proofingModel(),
                                                                                      m_proofModel->proofingDepth(),
                                                                                      m_proofModel->proofingProfile());
    if (proofingSpace) {
        m_page->proofingSpaceSelector->setCurrentColorSpace(proofingSpace);
    }
    updateProofingWidgets();

    m_page->chkBlackpoint->setChecked(cfg.useBlackPointCompensation(true));
    m_page->chkAllowLCMSOptimization->setChecked(cfg.allowLCMSOptimization(true));
    m_page->chkForcePaletteColor->setChecked(cfg.forcePaletteColors(true));
    m_page->cmbMonitorIntent->setCurrentIndex(cfg.monitorRenderIntent(true));
    if (!m_colorManagedByOS) {
        m_page->chkUseSystemMonitorProfile->setChecked(cfg.useSystemMonitorProfile(true));
    }
    QAbstractButton *button = m_pasteBehaviourGroup.button(cfg.pasteBehaviour(true));
    Q_ASSERT(button);
    if (button) {
        button->setChecked(true);
    }
}


void ColorSettingsTab::refillMonitorProfiles(const KoID & colorSpaceId)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_colorManagedByOS);

    for (int i = 0; i < QApplication::screens().count(); ++i) {
        m_monitorProfileWidgets[i]->clear();
    }

    QMap<QString, const KoColorProfile *>  profileList;
    Q_FOREACH(const KoColorProfile *profile, KoColorSpaceRegistry::instance()->profilesFor(colorSpaceId.id())) {
        profileList[profile->name()] = profile;
    }

    Q_FOREACH (const KoColorProfile *profile, profileList.values()) {
        //qDebug() << "Profile" << profile->name() << profile->isSuitableForDisplay() << csf->defaultProfile();
        if (profile->isSuitableForDisplay()) {
            for (int i = 0; i < QApplication::screens().count(); ++i) {
                m_monitorProfileWidgets[i]->addSqueezedItem(profile->name());
            }
        }
    }

    for (int i = 0; i < QApplication::screens().count(); ++i) {
        m_monitorProfileLabels[i]->setText(i18nc("The number of the screen (ordinal) and shortened 'name' of the screen (model + resolution)", "Screen %1 (%2):", i + 1, shortNameOfDisplay(i)));
        m_monitorProfileWidgets[i]->setCurrent(KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId.id()));
    }
}

void ColorSettingsTab::updateProofingWidgets() {
    m_page->cmbDisplayIntent->setCurrentIndex(m_page->cmbDisplayIntent->findData((int)m_proofModel->effectiveDisplayIntent(), Qt::UserRole));
    m_page->cmbProofingIntent->setCurrentIndex(m_page->cmbProofingIntent->findData((int)m_proofModel->conversionIntent(), Qt::UserRole));

    m_page->cmbDisplayMode->setCurrentIndex(m_page->cmbDisplayMode->findData(int(m_proofModel->displayTransformState()), Qt::UserRole));
    m_page->chkDispBlackPoint->setChecked(m_proofModel->effectiveDispBlackPointCompensation());
    m_page->sldAdaptationState->setValue(m_proofModel->effectiveAdaptationState());
}

void ColorSettingsTab::proofingDisplayModeUpdated() {
    m_proofModel->setdisplayTransformState(KisProofingConfiguration::DisplayTransformState(m_page->cmbDisplayMode->currentData(Qt::UserRole).toInt()));
    updateProofingWidgets();
}

void ColorSettingsTab::proofingConversionIntentUpdated() {
    m_proofModel->setconversionIntent(KoColorConversionTransformation::Intent(m_page->cmbProofingIntent->currentData(Qt::UserRole).toInt()));
    updateProofingWidgets();
}
void ColorSettingsTab::proofingDisplayIntentUpdated() {
    if (m_proofModel->displayTransformState() == KisProofingConfiguration::Custom) {
        m_proofModel->setdisplayIntent(KoColorConversionTransformation::Intent(m_page->cmbDisplayIntent->currentData(Qt::UserRole).toInt()));
        updateProofingWidgets();
    }
}

void ColorSettingsTab::updateProofingDisplayInfo() {
    KisDisplayConfig displayInfo;
    displayInfo.intent = KoColorConversionTransformation::Intent(m_page->cmbMonitorIntent->currentIndex());
    displayInfo.conversionFlags.setFlag(KoColorConversionTransformation::BlackpointCompensation, m_page->chkBlackpoint->isChecked());
    m_proofModel->updateDisplayConfig(displayInfo);
}

//---------------------------------------------------------------------------------------------------

void TabletSettingsTab::setDefault()
{
    KisConfig cfg(true);
    const KisCubicCurve curve(DEFAULT_CURVE_STRING);
    m_page->pressureCurve->setCurve(curve);

    m_page->chkUseRightMiddleClickWorkaround->setChecked(
        KisConfig(true).useRightMiddleTabletButtonWorkaround(true));

#if defined Q_OS_WIN && (defined QT5_HAS_WINTAB_SWITCH || QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)) 
        m_page->radioWintab->setChecked(!cfg.useWin8PointerInput(true));
        m_page->radioWin8PointerInput->setChecked(cfg.useWin8PointerInput(true));
#else
        m_page->grpTabletApi->setVisible(false);
#endif

    m_page->chkUseTimestampsForBrushSpeed->setChecked(false);
    m_page->intMaxAllowedBrushSpeed->setValue(30);
    m_page->intBrushSpeedSmoothing->setValue(3);
    m_page->tiltDirectionOffsetAngle->setAngle(0);
}

TabletSettingsTab::TabletSettingsTab(QWidget* parent, const char* name): QWidget(parent)
{
    setObjectName(name);

    QGridLayout * l = new QGridLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    m_page = new WdgTabletSettings(this);
    l->addWidget(m_page, 0, 0);

    KisConfig cfg(true);
    const KisCubicCurve curve(cfg.pressureTabletCurve());
    m_page->pressureCurve->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    m_page->pressureCurve->setCurve(curve);

    m_page->chkUseRightMiddleClickWorkaround->setChecked(
         cfg.useRightMiddleTabletButtonWorkaround());

#if defined Q_OS_WIN && (defined QT5_HAS_WINTAB_SWITCH || QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
# if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString actualTabletProtocol = "<unknown>";
    using QWindowsApplication = QNativeInterface::Private::QWindowsApplication;
    if (auto nativeWindowsApp = dynamic_cast<QWindowsApplication *>(QGuiApplicationPrivate::platformIntegration())) {
        actualTabletProtocol = nativeWindowsApp->isWinTabEnabled() ? "WinTab" : "Windows Ink";
    }
    m_page->grpTabletApi->setTitle(i18n("Tablet Input API (currently active API: \"%1\")", actualTabletProtocol));
# endif
    m_page->radioWintab->setChecked(!cfg.useWin8PointerInput());
    m_page->radioWin8PointerInput->setChecked(cfg.useWin8PointerInput());

    connect(m_page->btnResolutionSettings, SIGNAL(clicked()), SLOT(slotResolutionSettings()));
    connect(m_page->radioWintab, SIGNAL(toggled(bool)), m_page->btnResolutionSettings, SLOT(setEnabled(bool)));
    m_page->btnResolutionSettings->setEnabled(m_page->radioWintab->isChecked());
#else
    m_page->grpTabletApi->setVisible(false);
#endif
    connect(m_page->btnTabletTest, SIGNAL(clicked()), SLOT(slotTabletTest()));

#ifdef Q_OS_WIN
    m_page->chkUseTimestampsForBrushSpeed->setText(i18n("Use tablet driver timestamps for brush speed (may cause severe artifacts when using WinTab tablet API)"));
#else
    m_page->chkUseTimestampsForBrushSpeed->setText(i18n("Use tablet driver timestamps for brush speed"));
#endif
    m_page->chkUseTimestampsForBrushSpeed->setChecked(cfg.readEntry("useTimestampsForBrushSpeed", false));

    m_page->intMaxAllowedBrushSpeed->setRange(1, 100);
    m_page->intMaxAllowedBrushSpeed->setValue(cfg.readEntry("maxAllowedSpeedValue", 30));
    KisSpinBoxI18nHelper::install(m_page->intMaxAllowedBrushSpeed, [](int value) {
        // i18n: This is meant to be used in a spinbox so keep the {n} in the text
        //       and it will be substituted by the number. The text before will be
        //       used as the prefix and the text after as the suffix
        return i18np("Maximum brush speed: {n} px/ms", "Maximum brush speed: {n} px/ms", value);
    });

    m_page->intBrushSpeedSmoothing->setRange(3, 100);
    m_page->intBrushSpeedSmoothing->setValue(cfg.readEntry("speedValueSmoothing", 3));
    KisSpinBoxI18nHelper::install(m_page->intBrushSpeedSmoothing, [](int value) {
        // i18n: This is meant to be used in a spinbox so keep the {n} in the text
        //       and it will be substituted by the number. The text before will be
        //       used as the prefix and the text after as the suffix
        return i18np("Brush speed smoothing: {n} sample", "Brush speed smoothing: {n} samples", value);
    });

    m_page->tiltDirectionOffsetAngle->setDecimals(0);
    m_page->tiltDirectionOffsetAngle->setRange(-180, 180);
    // the angle is saved in clockwise direction to be consistent with Drawing Angle, so negate
    m_page->tiltDirectionOffsetAngle->setAngle(-cfg.readEntry("tiltDirectionOffset", 0.0));
    m_page->tiltDirectionOffsetAngle->setPrefix(i18n("Pen tilt direction offset: "));
    m_page->tiltDirectionOffsetAngle->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);
}

void TabletSettingsTab::slotTabletTest()
{
    TabletTestDialog tabletTestDialog(this);
    tabletTestDialog.exec();
}

#ifdef Q_OS_WIN
#include "KisDlgCustomTabletResolution.h"
#endif

void TabletSettingsTab::slotResolutionSettings()
{
#ifdef Q_OS_WIN
    KisDlgCustomTabletResolution dlg(this);
    dlg.exec();
#endif
}


//---------------------------------------------------------------------------------------------------
#include "kis_acyclic_signal_connector.h"

int getTotalRAM()
{
    return KisImageConfig(true).totalRAM();
}

int PerformanceTab::realTilesRAM()
{
    return intMemoryLimit->value() - intPoolLimit->value();
}

PerformanceTab::PerformanceTab(QWidget *parent, const char *name)
    : WdgPerformanceSettings(parent, name)
    , m_frameRateModel(new KisFrameRateLimitModel())
{
    KisImageConfig cfg(true);
    const double totalRAM = cfg.totalRAM();
    lblTotalMemory->setText(KFormat().formatByteSize(totalRAM * 1024 * 1024, 0, KFormat::IECBinaryDialect, KFormat::UnitMegaByte));

    KisSpinBoxI18nHelper::setText(sliderMemoryLimit, i18nc("{n} is the number value, % is the percent sign", "{n}%"));
    sliderMemoryLimit->setRange(1, 100, 2);
    sliderMemoryLimit->setSingleStep(0.01);

    KisSpinBoxI18nHelper::setText(sliderPoolLimit, i18nc("{n} is the number value, % is the percent sign", "{n}%"));
    sliderPoolLimit->setRange(0, 20, 2);
    sliderPoolLimit->setSingleStep(0.01);

    KisSpinBoxI18nHelper::setText(sliderUndoLimit, i18nc("{n} is the number value, % is the percent sign", "{n}%"));
    sliderUndoLimit->setRange(0, 50, 2);
    sliderUndoLimit->setSingleStep(0.01);

    intMemoryLimit->setMinimumWidth(80);
    intPoolLimit->setMinimumWidth(80);
    intUndoLimit->setMinimumWidth(80);

    {
        formLayout->takeRow(2);
        label_5->setVisible(false);
        intPoolLimit->setVisible(false);
        sliderPoolLimit->setVisible(false);
    }

    SliderAndSpinBoxSync *sync1 =
        new SliderAndSpinBoxSync(sliderMemoryLimit,
                                 intMemoryLimit,
                                 getTotalRAM);

    sync1->slotParentValueChanged();
    m_syncs << sync1;

    SliderAndSpinBoxSync *sync2 =
        new SliderAndSpinBoxSync(sliderPoolLimit,
                                 intPoolLimit,
                                 std::bind(&KisIntParseSpinBox::value,
                                             intMemoryLimit));


    connect(intMemoryLimit, SIGNAL(valueChanged(int)), sync2, SLOT(slotParentValueChanged()));
    sync2->slotParentValueChanged();
    m_syncs << sync2;

    SliderAndSpinBoxSync *sync3 =
        new SliderAndSpinBoxSync(sliderUndoLimit,
                                 intUndoLimit,
                                 std::bind(&PerformanceTab::realTilesRAM,
                                             this));


    connect(intPoolLimit, SIGNAL(valueChanged(int)), sync3, SLOT(slotParentValueChanged()));
    connect(intMemoryLimit, SIGNAL(valueChanged(int)), sync3, SLOT(slotParentValueChanged()));
    sync3->slotParentValueChanged();
    m_syncs << sync3;

    sliderSwapSize->setSuffix(i18n(" GiB"));
    sliderSwapSize->setRange(1, 64);
    intSwapSize->setRange(1, 64);


    KisAcyclicSignalConnector *swapSizeConnector = new KisAcyclicSignalConnector(this);

    swapSizeConnector->connectForwardInt(sliderSwapSize, SIGNAL(valueChanged(int)),
                                         intSwapSize, SLOT(setValue(int)));

    swapSizeConnector->connectBackwardInt(intSwapSize, SIGNAL(valueChanged(int)),
                                          sliderSwapSize, SLOT(setValue(int)));

    swapFileLocation->setMode(KoFileDialog::OpenDirectory);
    swapFileLocation->setConfigurationName("swapfile_location");
    swapFileLocation->setFileName(cfg.swapDir());

    sliderThreadsLimit->setRange(1, QThread::idealThreadCount());
    sliderFrameClonesLimit->setRange(1, QThread::idealThreadCount());

    sliderFrameTimeout->setRange(5, 600);
    sliderFrameTimeout->setSuffix(i18nc("suffix for \"seconds\"", " sec"));
    sliderFrameTimeout->setValue(cfg.frameRenderingTimeout() / 1000);

    sliderFpsLimit->setSuffix(i18n(" fps"));

    KisWidgetConnectionUtils::connectControlState(sliderFpsLimit, m_frameRateModel.data(), "frameRateState", "frameRate");
    KisWidgetConnectionUtils::connectControl(chkDetectFps, m_frameRateModel.data(), "detectFrameRate");

    connect(sliderThreadsLimit, SIGNAL(valueChanged(int)), SLOT(slotThreadsLimitChanged(int)));
    connect(sliderFrameClonesLimit, SIGNAL(valueChanged(int)), SLOT(slotFrameClonesLimitChanged(int)));

    intCachedFramesSizeLimit->setRange(256, 10000);
    intCachedFramesSizeLimit->setSuffix(i18n(" px"));
    intCachedFramesSizeLimit->setSingleStep(1);
    intCachedFramesSizeLimit->setPageStep(1000);

    intRegionOfInterestMargin->setRange(1, 100);
    KisSpinBoxI18nHelper::setText(intRegionOfInterestMargin,
                                  i18nc("{n} is the number value, % is the percent sign", "{n}%"));
    intRegionOfInterestMargin->setSingleStep(1);
    intRegionOfInterestMargin->setPageStep(10);

    connect(chkCachedFramesSizeLimit, SIGNAL(toggled(bool)), intCachedFramesSizeLimit, SLOT(setEnabled(bool)));
    connect(chkUseRegionOfInterest, SIGNAL(toggled(bool)), intRegionOfInterestMargin, SLOT(setEnabled(bool)));

    connect(chkTransformToolUseInStackPreview, SIGNAL(toggled(bool)), chkTransformToolForceLodMode, SLOT(setEnabled(bool)));

#ifndef Q_OS_WIN
    // AVX workaround is needed on Windows+GCC only
    chkDisableAVXOptimizations->setVisible(false);
#endif

    load(false);
}

PerformanceTab::~PerformanceTab()
{
    qDeleteAll(m_syncs);
}

void PerformanceTab::load(bool requestDefault)
{
    KisImageConfig cfg(true);

    sliderMemoryLimit->setValue(cfg.memoryHardLimitPercent(requestDefault));
    sliderPoolLimit->setValue(cfg.memoryPoolLimitPercent(requestDefault));
    sliderUndoLimit->setValue(cfg.memorySoftLimitPercent(requestDefault));

    chkPerformanceLogging->setChecked(cfg.enablePerfLog(requestDefault));
    chkProgressReporting->setChecked(cfg.enableProgressReporting(requestDefault));

    sliderSwapSize->setValue(cfg.maxSwapSize(requestDefault) / 1024);
    swapFileLocation->setFileName(cfg.swapDir(requestDefault));

    m_lastUsedThreadsLimit = cfg.maxNumberOfThreads(requestDefault);
    m_lastUsedClonesLimit = cfg.frameRenderingClones(requestDefault);

    sliderThreadsLimit->setValue(m_lastUsedThreadsLimit);
    sliderFrameClonesLimit->setValue(m_lastUsedClonesLimit);

#if KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    m_frameRateModel->data.set(std::make_tuple(cfg.detectFpsLimit(requestDefault), cfg.fpsLimit(requestDefault)));
#else
    m_frameRateModel->data.set(std::make_tuple(false, cfg.fpsLimit(requestDefault)));
    chkDetectFps->setVisible(false);
#endif
    {
        KisConfig cfg2(true);
        chkOpenGLFramerateLogging->setChecked(cfg2.enableOpenGLFramerateLogging(requestDefault));
        chkBrushSpeedLogging->setChecked(cfg2.enableBrushSpeedLogging(requestDefault));
        chkDisableVectorOptimizations->setChecked(cfg2.disableVectorOptimizations(requestDefault));
#ifdef Q_OS_WIN
        chkDisableAVXOptimizations->setChecked(cfg2.disableAVXOptimizations(requestDefault));
#endif
        chkBackgroundCacheGeneration->setChecked(cfg2.calculateAnimationCacheInBackground(requestDefault));
    }

    if (cfg.useOnDiskAnimationCacheSwapping(requestDefault)) {
        optOnDisk->setChecked(true);
    } else {
        optInMemory->setChecked(true);
    }

    chkCachedFramesSizeLimit->setChecked(cfg.useAnimationCacheFrameSizeLimit(requestDefault));
    intCachedFramesSizeLimit->setValue(cfg.animationCacheFrameSizeLimit(requestDefault));
    intCachedFramesSizeLimit->setEnabled(chkCachedFramesSizeLimit->isChecked());

    chkUseRegionOfInterest->setChecked(cfg.useAnimationCacheRegionOfInterest(requestDefault));
    intRegionOfInterestMargin->setValue(cfg.animationCacheRegionOfInterestMargin(requestDefault) * 100.0);
    intRegionOfInterestMargin->setEnabled(chkUseRegionOfInterest->isChecked());

    {
        KConfigGroup group = KSharedConfig::openConfig()->group("KisToolTransform");
        chkTransformToolUseInStackPreview->setChecked(!group.readEntry("useOverlayPreviewStyle", false));
        chkTransformToolForceLodMode->setChecked(group.readEntry("forceLodMode", true));
        chkTransformToolForceLodMode->setEnabled(chkTransformToolUseInStackPreview->isChecked());
    }

    {
        KConfigGroup group = KSharedConfig::openConfig()->group("KritaTransform/KisToolMove");
        chkMoveToolForceLodMode->setChecked(group.readEntry("forceLodMode", false));
    }

    {
        KConfigGroup group( KSharedConfig::openConfig(), "filterdialog");
        chkFiltersForceLodMode->setChecked(group.readEntry("forceLodMode", true));
    }
}

void PerformanceTab::save()
{
    KisImageConfig cfg(false);

    cfg.setMemoryHardLimitPercent(sliderMemoryLimit->value());
    cfg.setMemorySoftLimitPercent(sliderUndoLimit->value());
    cfg.setMemoryPoolLimitPercent(sliderPoolLimit->value());

    cfg.setEnablePerfLog(chkPerformanceLogging->isChecked());
    cfg.setEnableProgressReporting(chkProgressReporting->isChecked());

    cfg.setMaxSwapSize(sliderSwapSize->value() * 1024);

    cfg.setSwapDir(swapFileLocation->fileName());

    cfg.setMaxNumberOfThreads(sliderThreadsLimit->value());
    cfg.setFrameRenderingClones(sliderFrameClonesLimit->value());
    cfg.setFrameRenderingTimeout(sliderFrameTimeout->value() * 1000);
    cfg.setFpsLimit(std::get<int>(*m_frameRateModel->data));
#if KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    cfg.setDetectFpsLimit(std::get<bool>(*m_frameRateModel->data));
#endif

    {
        KisConfig cfg2(true);
        cfg2.setEnableOpenGLFramerateLogging(chkOpenGLFramerateLogging->isChecked());
        cfg2.setEnableBrushSpeedLogging(chkBrushSpeedLogging->isChecked());
        cfg2.setDisableVectorOptimizations(chkDisableVectorOptimizations->isChecked());
#ifdef Q_OS_WIN
        cfg2.setDisableAVXOptimizations(chkDisableAVXOptimizations->isChecked());
#endif
        cfg2.setCalculateAnimationCacheInBackground(chkBackgroundCacheGeneration->isChecked());
    }

    cfg.setUseOnDiskAnimationCacheSwapping(optOnDisk->isChecked());

    cfg.setUseAnimationCacheFrameSizeLimit(chkCachedFramesSizeLimit->isChecked());
    cfg.setAnimationCacheFrameSizeLimit(intCachedFramesSizeLimit->value());

    cfg.setUseAnimationCacheRegionOfInterest(chkUseRegionOfInterest->isChecked());
    cfg.setAnimationCacheRegionOfInterestMargin(intRegionOfInterestMargin->value() / 100.0);

    {
        KConfigGroup group = KSharedConfig::openConfig()->group("KisToolTransform");
        group.writeEntry("useOverlayPreviewStyle", !chkTransformToolUseInStackPreview->isChecked());
        group.writeEntry("forceLodMode", chkTransformToolForceLodMode->isChecked());
    }

    {
        KConfigGroup group = KSharedConfig::openConfig()->group("KritaTransform/KisToolMove");
        group.writeEntry("forceLodMode", chkMoveToolForceLodMode->isChecked());
    }

    {
        KConfigGroup group( KSharedConfig::openConfig(), "filterdialog");
        group.writeEntry("forceLodMode", chkFiltersForceLodMode->isChecked());
    }

}

void PerformanceTab::slotThreadsLimitChanged(int value)
{
    KisSignalsBlocker b(sliderFrameClonesLimit);
    sliderFrameClonesLimit->setValue(qMin(m_lastUsedClonesLimit, value));
    m_lastUsedThreadsLimit = value;
}

void PerformanceTab::slotFrameClonesLimitChanged(int value)
{
    KisSignalsBlocker b(sliderThreadsLimit);
    sliderThreadsLimit->setValue(qMax(m_lastUsedThreadsLimit, value));
    m_lastUsedClonesLimit = value;
}

//---------------------------------------------------------------------------------------------------

#include "KoColor.h"
#include "opengl/KisOpenGLModeProber.h"
#include "opengl/KisScreenInformationAdapter.h"
#include <QOpenGLContext>
#include <QScreen>

namespace {

QString colorSpaceString(const KisSurfaceColorSpaceWrapper &cs, int depth)
{
    const QString csString =
#ifdef HAVE_HDR
        cs == KisSurfaceColorSpaceWrapper::bt2020PQColorSpace ? "Rec. 2020 PQ" :
        cs == KisSurfaceColorSpaceWrapper::scRGBColorSpace ? "Rec. 709 Linear" :
#endif
        cs == KisSurfaceColorSpaceWrapper::sRGBColorSpace ? "sRGB" :
        cs == KisSurfaceColorSpaceWrapper::DefaultColorSpace ? "sRGB" :
        "Unknown Color Space";

    return QString("%1 (%2 bit)").arg(csString).arg(depth);
}

int formatToIndex(KisConfig::RootSurfaceFormat fmt)
{
    return fmt == KisConfig::BT2020_PQ ? 1 :
           fmt == KisConfig::BT709_G10 ? 2 :
           0;
}

KisConfig::RootSurfaceFormat indexToFormat(int value)
{
    return value == 1 ? KisConfig::BT2020_PQ :
           value == 2 ? KisConfig::BT709_G10 :
           KisConfig::BT709_G22;
}

int assistantDrawModeToIndex(KisConfig::AssistantsDrawMode mode)
{
    return mode == KisConfig::ASSISTANTS_DRAW_MODE_PIXMAP_CACHE ? 1 :
           mode == KisConfig::ASSISTANTS_DRAW_MODE_LARGE_PIXMAP_CACHE ? 2 :
           0;
}

KisConfig::AssistantsDrawMode indexToAssistantDrawMode(int value)
{
    return value == 1 ? KisConfig::ASSISTANTS_DRAW_MODE_PIXMAP_CACHE :
           value == 2 ? KisConfig::ASSISTANTS_DRAW_MODE_LARGE_PIXMAP_CACHE :
           KisConfig::ASSISTANTS_DRAW_MODE_DIRECT;
}

} // anonymous namespace

DisplaySettingsTab::DisplaySettingsTab(QWidget *parent, const char *name)
    : WdgDisplaySettings(parent, name)
{
    KisConfig cfg(true);

    const QString rendererOpenGLText = i18nc("canvas renderer", "OpenGL");
    const QString rendererSoftwareText = i18nc("canvas renderer", "Software Renderer (very slow)");
#ifdef Q_OS_WIN
    const QString rendererOpenGLESText =
        qEnvironmentVariable("QT_ANGLE_PLATFORM") != "opengl"
        ? i18nc("canvas renderer", "Direct3D 11 via ANGLE")
        : i18nc("canvas renderer", "OpenGL via ANGLE");
#else
    const QString rendererOpenGLESText = i18nc("canvas renderer", "OpenGL ES");
#endif

    const KisOpenGL::OpenGLRenderer renderer = KisOpenGL::getCurrentOpenGLRenderer();
    lblCurrentRenderer->setText(renderer == KisOpenGL::RendererOpenGLES ? rendererOpenGLESText :
                                renderer == KisOpenGL::RendererDesktopGL ? rendererOpenGLText :
                                renderer == KisOpenGL::RendererSoftware ? rendererSoftwareText :
                                i18nc("canvas renderer", "Unknown"));

    cmbPreferredRenderer->clear();

    const KisOpenGL::OpenGLRenderers supportedRenderers = KisOpenGL::getSupportedOpenGLRenderers();
    const bool onlyOneRendererSupported =
        supportedRenderers == KisOpenGL::RendererDesktopGL ||
        supportedRenderers == KisOpenGL::RendererOpenGLES ||
        supportedRenderers == KisOpenGL::RendererSoftware;


    if (!onlyOneRendererSupported) {
        QString qtPreferredRendererText;
        if (KisOpenGL::getQtPreferredOpenGLRenderer() == KisOpenGL::RendererOpenGLES) {
            qtPreferredRendererText = rendererOpenGLESText;
        } else if (KisOpenGL::getQtPreferredOpenGLRenderer() == KisOpenGL::RendererSoftware) {
            qtPreferredRendererText = rendererSoftwareText;
        } else {
            qtPreferredRendererText = rendererOpenGLText;
        }
        cmbPreferredRenderer->addItem(i18nc("canvas renderer", "Auto (%1)", qtPreferredRendererText), KisOpenGL::RendererAuto);
        cmbPreferredRenderer->setCurrentIndex(0);
    } else {
        cmbPreferredRenderer->setEnabled(false);
    }

    if (supportedRenderers & KisOpenGL::RendererDesktopGL) {
        cmbPreferredRenderer->addItem(rendererOpenGLText, KisOpenGL::RendererDesktopGL);
        if (KisOpenGL::getUserPreferredOpenGLRendererConfig() == KisOpenGL::RendererDesktopGL) {
            cmbPreferredRenderer->setCurrentIndex(cmbPreferredRenderer->count() - 1);
        }
    }

    if (supportedRenderers & KisOpenGL::RendererOpenGLES) {
        cmbPreferredRenderer->addItem(rendererOpenGLESText, KisOpenGL::RendererOpenGLES);
        if (KisOpenGL::getUserPreferredOpenGLRendererConfig() == KisOpenGL::RendererOpenGLES) {
            cmbPreferredRenderer->setCurrentIndex(cmbPreferredRenderer->count() - 1);
        }
    }

    if (supportedRenderers & KisOpenGL::RendererSoftware) {
        cmbPreferredRenderer->addItem(rendererSoftwareText, KisOpenGL::RendererSoftware);
        if (KisOpenGL::getUserPreferredOpenGLRendererConfig() == KisOpenGL::RendererSoftware) {
            cmbPreferredRenderer->setCurrentIndex(cmbPreferredRenderer->count() - 1);
        }
    }

    if (!(supportedRenderers &
          (KisOpenGL::RendererDesktopGL |
           KisOpenGL::RendererOpenGLES |
           KisOpenGL::RendererSoftware))) {

        grpOpenGL->setEnabled(false);
        grpOpenGL->setChecked(false);
        chkUseTextureBuffer->setEnabled(false);
        cmbAssistantsDrawMode->setEnabled(false);
        cmbFilterMode->setEnabled(false);
    } else {
        grpOpenGL->setEnabled(true);
        grpOpenGL->setChecked(cfg.useOpenGL());
        chkUseTextureBuffer->setEnabled(cfg.useOpenGL());
        chkUseTextureBuffer->setChecked(cfg.useOpenGLTextureBuffer());
        cmbAssistantsDrawMode->setEnabled(cfg.useOpenGL());
        cmbAssistantsDrawMode->setCurrentIndex(assistantDrawModeToIndex(cfg.assistantsDrawMode()));
        cmbFilterMode->setEnabled(cfg.useOpenGL());
        cmbFilterMode->setCurrentIndex(cfg.openGLFilteringMode());
        // Don't show the high quality filtering mode if it's not available
        if (!KisOpenGL::supportsLoD()) {
            cmbFilterMode->removeItem(3);
        }
    }

    lblCurrentDisplayFormat->setText("");
    lblCurrentRootSurfaceFormat->setText("");
    grpHDRWarning->setVisible(false);
    cmbPreferedRootSurfaceFormat->addItem(colorSpaceString(KisSurfaceColorSpaceWrapper::sRGBColorSpace, 8));
#ifdef HAVE_HDR
    cmbPreferedRootSurfaceFormat->addItem(colorSpaceString(KisSurfaceColorSpaceWrapper::bt2020PQColorSpace, 10));
    cmbPreferedRootSurfaceFormat->addItem(colorSpaceString(KisSurfaceColorSpaceWrapper::scRGBColorSpace, 16));
#endif
    cmbPreferedRootSurfaceFormat->setCurrentIndex(formatToIndex(KisConfig::BT709_G22));
    slotPreferredSurfaceFormatChanged(cmbPreferedRootSurfaceFormat->currentIndex());

    QOpenGLContext *context = QOpenGLContext::currentContext();

    if (!context) {
        context = QOpenGLContext::globalShareContext();
    }

    if (context) {
        QScreen *screen = KisPart::instance()->currentMainwindow()->screen();
        KisScreenInformationAdapter adapter(context);
        if (screen && adapter.isValid()) {
            KisScreenInformationAdapter::ScreenInfo info = adapter.infoForScreen(screen);
            if (info.isValid()) {
                QStringList toolTip;

                toolTip << i18n("Display Id: %1", info.screen->name());
                toolTip << i18n("Display Name: %1 %2", info.screen->manufacturer(), info.screen->model());
                toolTip << i18n("Min Luminance: %1", info.minLuminance);
                toolTip << i18n("Max Luminance: %1", info.maxLuminance);
                toolTip << i18n("Max Full Frame Luminance: %1", info.maxFullFrameLuminance);
                toolTip << i18n("Red Primary: %1, %2", info.redPrimary[0], info.redPrimary[1]);
                toolTip << i18n("Green Primary: %1, %2", info.greenPrimary[0], info.greenPrimary[1]);
                toolTip << i18n("Blue Primary: %1, %2", info.bluePrimary[0], info.bluePrimary[1]);
                toolTip << i18n("White Point: %1, %2", info.whitePoint[0], info.whitePoint[1]);

                lblCurrentDisplayFormat->setToolTip(toolTip.join('\n'));
                lblCurrentDisplayFormat->setText(colorSpaceString(info.colorSpace, info.bitsPerColor));
            } else {
                lblCurrentDisplayFormat->setToolTip("");
                lblCurrentDisplayFormat->setText(i18n("Unknown"));
            }
        } else {
            lblCurrentDisplayFormat->setToolTip("");
            lblCurrentDisplayFormat->setText(i18n("Unknown"));
            qWarning() << "Failed to fetch display info:" << adapter.errorString();
        }

        const QSurfaceFormat currentFormat = KisOpenGLModeProber::instance()->surfaceformatInUse();
        const auto colorSpace = KisSurfaceColorSpaceWrapper::fromQtColorSpace(currentFormat.colorSpace());
        lblCurrentRootSurfaceFormat->setText(colorSpaceString(colorSpace, currentFormat.redBufferSize()));
        cmbPreferedRootSurfaceFormat->setCurrentIndex(formatToIndex(cfg.rootSurfaceFormat()));
        connect(cmbPreferedRootSurfaceFormat, SIGNAL(currentIndexChanged(int)), SLOT(slotPreferredSurfaceFormatChanged(int)));
        slotPreferredSurfaceFormatChanged(cmbPreferedRootSurfaceFormat->currentIndex());
    }

#ifndef HAVE_HDR
    tabHDR->setEnabled(false);
#endif

    const QStringList openglWarnings = KisOpenGL::getOpenGLWarnings();
    if (openglWarnings.isEmpty()) {
        grpOpenGLWarnings->setVisible(false);
    } else {
        QString text = QString("<p><b>%1</b>").arg(i18n("Warning(s):"));
        text.append("<ul>");
        Q_FOREACH (const QString &warning, openglWarnings) {
            text.append("<li>");
            text.append(warning.toHtmlEscaped());
            text.append("</li>");
        }
        text.append("</ul></p>");
        grpOpenGLWarnings->setText(text);
        grpOpenGLWarnings->setPixmap(
            grpOpenGLWarnings->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(QSize(32, 32)));
        grpOpenGLWarnings->setVisible(true);
    }

    KisImageConfig imageCfg(false);

    KoColor c;
    c.fromQColor(imageCfg.selectionOverlayMaskColor());
    c.setOpacity(1.0);
    btnSelectionOverlayColor->setColor(c);
    sldSelectionOverlayOpacity->setRange(0.0, 1.0, 2);
    sldSelectionOverlayOpacity->setSingleStep(0.05);
    sldSelectionOverlayOpacity->setValue(imageCfg.selectionOverlayMaskColor().alphaF());

    sldSelectionOutlineOpacity->setRange(0.0, 1.0, 2);
    sldSelectionOutlineOpacity->setSingleStep(0.05);
    sldSelectionOutlineOpacity->setValue(imageCfg.selectionOutlineOpacity());

    intCheckSize->setValue(cfg.checkSize());
    chkMoving->setChecked(cfg.scrollCheckers());
    KoColor ck1(KoColorSpaceRegistry::instance()->rgb8());
    ck1.fromQColor(cfg.checkersColor1());
    colorChecks1->setColor(ck1);
    KoColor ck2(KoColorSpaceRegistry::instance()->rgb8());
    ck2.fromQColor(cfg.checkersColor2());
    colorChecks2->setColor(ck2);
    KoColor cb(KoColorSpaceRegistry::instance()->rgb8());
    cb.fromQColor(cfg.canvasBorderColor());
    canvasBorder->setColor(cb);
    hideScrollbars->setChecked(cfg.hideScrollbars());
    chkCurveAntialiasing->setChecked(cfg.antialiasCurves());
    chkSelectionOutlineAntialiasing->setChecked(cfg.antialiasSelectionOutline());
    chkChannelsAsColor->setChecked(cfg.showSingleChannelAsColor());
    chkHidePopups->setChecked(cfg.hidePopups());

    connect(grpOpenGL, SIGNAL(toggled(bool)), SLOT(slotUseOpenGLToggled(bool)));

    KoColor gridColor(KoColorSpaceRegistry::instance()->rgb8());
    gridColor.fromQColor(cfg.getPixelGridColor());
    pixelGridColorButton->setColor(gridColor);
    pixelGridDrawingThresholdBox->setValue(cfg.getPixelGridDrawingThreshold() * 100);
    KisSpinBoxI18nHelper::setText(pixelGridDrawingThresholdBox, i18nc("{n} is the number value, % is the percent sign", "{n}%"));
}

void DisplaySettingsTab::setDefault()
{
    KisConfig cfg(true);
    cmbPreferredRenderer->setCurrentIndex(0);
    if (!(KisOpenGL::getSupportedOpenGLRenderers() &
            (KisOpenGL::RendererDesktopGL | KisOpenGL::RendererOpenGLES))) {
        grpOpenGL->setEnabled(false);
        grpOpenGL->setChecked(false);
        chkUseTextureBuffer->setEnabled(false);
        cmbAssistantsDrawMode->setEnabled(false);
        cmbFilterMode->setEnabled(false);
    }
    else {
        grpOpenGL->setEnabled(true);
        grpOpenGL->setChecked(cfg.useOpenGL(true));
        chkUseTextureBuffer->setChecked(cfg.useOpenGLTextureBuffer(true));
        chkUseTextureBuffer->setEnabled(true);
        cmbAssistantsDrawMode->setEnabled(true);
        cmbAssistantsDrawMode->setCurrentIndex(assistantDrawModeToIndex(cfg.assistantsDrawMode(true)));
        cmbFilterMode->setEnabled(true);
        cmbFilterMode->setCurrentIndex(cfg.openGLFilteringMode(true));
    }

    chkMoving->setChecked(cfg.scrollCheckers(true));

    KisImageConfig imageCfg(false);

    KoColor c;
    c.fromQColor(imageCfg.selectionOverlayMaskColor(true));
    c.setOpacity(1.0);
    btnSelectionOverlayColor->setColor(c);
    sldSelectionOverlayOpacity->setValue(imageCfg.selectionOverlayMaskColor(true).alphaF());

    sldSelectionOutlineOpacity->setValue(imageCfg.selectionOutlineOpacity(true));

    intCheckSize->setValue(cfg.checkSize(true));
    KoColor ck1(KoColorSpaceRegistry::instance()->rgb8());
    ck1.fromQColor(cfg.checkersColor1(true));
    colorChecks1->setColor(ck1);
    KoColor ck2(KoColorSpaceRegistry::instance()->rgb8());
    ck2.fromQColor(cfg.checkersColor2(true));
    colorChecks2->setColor(ck2);
    KoColor cvb(KoColorSpaceRegistry::instance()->rgb8());
    cvb.fromQColor(cfg.canvasBorderColor(true));
    canvasBorder->setColor(cvb);
    hideScrollbars->setChecked(cfg.hideScrollbars(true));
    chkCurveAntialiasing->setChecked(cfg.antialiasCurves(true));
    chkSelectionOutlineAntialiasing->setChecked(cfg.antialiasSelectionOutline(true));
    chkChannelsAsColor->setChecked(cfg.showSingleChannelAsColor(true));
    chkHidePopups->setChecked(cfg.hidePopups(true));

    KoColor gridColor(KoColorSpaceRegistry::instance()->rgb8());
    gridColor.fromQColor(cfg.getPixelGridColor(true));
    pixelGridColorButton->setColor(gridColor);
    pixelGridDrawingThresholdBox->setValue(cfg.getPixelGridDrawingThreshold(true) * 100);
    KisSpinBoxI18nHelper::setText(pixelGridDrawingThresholdBox, i18nc("{n} is the number value, % is the percent sign", "{n}%"));

    cmbPreferedRootSurfaceFormat->setCurrentIndex(formatToIndex(KisConfig::BT709_G22));
    slotPreferredSurfaceFormatChanged(cmbPreferedRootSurfaceFormat->currentIndex());
}

void DisplaySettingsTab::slotUseOpenGLToggled(bool isChecked)
{
    chkUseTextureBuffer->setEnabled(isChecked);
    cmbFilterMode->setEnabled(isChecked);
    cmbAssistantsDrawMode->setEnabled(isChecked);
}

void DisplaySettingsTab::slotPreferredSurfaceFormatChanged(int index)
{
    Q_UNUSED(index);

    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (context) {
        QScreen *screen = KisPart::instance()->currentMainwindow()->screen();
        KisScreenInformationAdapter adapter(context);
        if (adapter.isValid()) {
            KisScreenInformationAdapter::ScreenInfo info = adapter.infoForScreen(screen);
            if (info.isValid()) {
                if (cmbPreferedRootSurfaceFormat->currentIndex() != formatToIndex(KisConfig::BT709_G22) &&
                    info.colorSpace == KisSurfaceColorSpaceWrapper::sRGBColorSpace) {
                    grpHDRWarning->setVisible(true);
                    grpHDRWarning->setPixmap(
                        grpHDRWarning->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(QSize(32, 32)));
                    grpHDRWarning->setText(i18n("<b>Warning:</b> current display doesn't support HDR rendering"));
                } else {
                    grpHDRWarning->setVisible(false);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------
FullscreenSettingsTab::FullscreenSettingsTab(QWidget* parent) : WdgFullscreenSettingsBase(parent)
{
    KisConfig cfg(true);

    chkDockers->setChecked(cfg.hideDockersFullscreen());
    chkMenu->setChecked(cfg.hideMenuFullscreen());
    chkScrollbars->setChecked(cfg.hideScrollbarsFullscreen());
    chkStatusbar->setChecked(cfg.hideStatusbarFullscreen());
    chkTitlebar->setChecked(cfg.hideTitlebarFullscreen());
    chkToolbar->setChecked(cfg.hideToolbarFullscreen());

}

void FullscreenSettingsTab::setDefault()
{
    KisConfig cfg(true);
    chkDockers->setChecked(cfg.hideDockersFullscreen(true));
    chkMenu->setChecked(cfg.hideMenuFullscreen(true));
    chkScrollbars->setChecked(cfg.hideScrollbarsFullscreen(true));
    chkStatusbar->setChecked(cfg.hideStatusbarFullscreen(true));
    chkTitlebar->setChecked(cfg.hideTitlebarFullscreen(true));
    chkToolbar->setChecked(cfg.hideToolbarFullscreen(true));
}


//---------------------------------------------------------------------------------------------------

namespace PopupPaletteTabPrivate {
static const QStringList allowedColorHistorySortingValues({"none", "hsv"});
}

PopupPaletteTab::PopupPaletteTab(QWidget *parent, const char *name)
    : WdgPopupPaletteSettingsBase(parent, name)
{
    using namespace PopupPaletteTabPrivate;

    load();

    connect(chkShowColorHistory, SIGNAL(toggled(bool)), cmbColorHistorySorting, SLOT(setEnabled(bool)));
    connect(chkShowColorHistory, SIGNAL(toggled(bool)), lblColorHistorySorting, SLOT(setEnabled(bool)));
    KIS_SAFE_ASSERT_RECOVER_NOOP(cmbColorHistorySorting->count() == allowedColorHistorySortingValues.size());
}

void PopupPaletteTab::load()
{
    using namespace PopupPaletteTabPrivate;

    KisConfig config(true);
    sbNumPresets->setValue(config.favoritePresets());
    sbPaletteSize->setValue(config.readEntry("popuppalette/size", 385));
    sbSelectorSize->setValue(config.readEntry("popuppalette/selectorSize", 140));
    cmbSelectorType->setCurrentIndex(config.readEntry<bool>("popuppalette/usevisualcolorselector", false) ? 1 : 0);
    chkShowColorHistory->setChecked(config.readEntry("popuppalette/showColorHistory", true));
    chkShowRotationTrack->setChecked(config.readEntry("popuppalette/showRotationTrack", true));
    chkUseDynamicSlotCount->setChecked(config.readEntry("popuppalette/useDynamicSlotCount", true));

    QString currentSorting = config.readEntry("popuppalette/colorHistorySorting", QString("hsv"));
    if (!allowedColorHistorySortingValues.contains(currentSorting)) {
        currentSorting = "hsv";
    }
    cmbColorHistorySorting->setCurrentIndex(allowedColorHistorySortingValues.indexOf(currentSorting));
    cmbColorHistorySorting->setEnabled(chkShowColorHistory->isChecked());
    lblColorHistorySorting->setEnabled(chkShowColorHistory->isChecked());
}

void PopupPaletteTab::save()
{
    using namespace PopupPaletteTabPrivate;

    KisConfig config(true);
    config.setFavoritePresets(sbNumPresets->value());
    config.writeEntry("popuppalette/size", sbPaletteSize->value());
    config.writeEntry("popuppalette/selectorSize", sbSelectorSize->value());
    config.writeEntry<bool>("popuppalette/usevisualcolorselector", cmbSelectorType->currentIndex() > 0);
    config.writeEntry<bool>("popuppalette/showColorHistory", chkShowColorHistory->isChecked());
    config.writeEntry<bool>("popuppalette/showRotationTrack", chkShowRotationTrack->isChecked());
    config.writeEntry<bool>("popuppalette/useDynamicSlotCount", chkUseDynamicSlotCount->isChecked());
    config.writeEntry("popuppalette/colorHistorySorting",
                      allowedColorHistorySortingValues[cmbColorHistorySorting->currentIndex()]);
}

void PopupPaletteTab::setDefault()
{
    KisConfig config(true);
    sbNumPresets->setValue(config.favoritePresets(true));
    sbPaletteSize->setValue(385);
    sbSelectorSize->setValue(140);
    cmbSelectorType->setCurrentIndex(0);
    chkShowColorHistory->setChecked(true);
    chkShowRotationTrack->setChecked(true);
    chkUseDynamicSlotCount->setChecked(true);
    cmbColorHistorySorting->setEnabled(chkShowColorHistory->isChecked());
    lblColorHistorySorting->setEnabled(chkShowColorHistory->isChecked());
}

//---------------------------------------------------------------------------------------------------

KisDlgPreferences::KisDlgPreferences(QWidget* parent, const char* name)
    : KPageDialog(parent)
{
    Q_UNUSED(name);
    setWindowTitle(i18n("Configure Krita"));
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);

    setFaceType(KPageDialog::List);

    // General
    KoVBox *vbox = new KoVBox();
    KPageWidgetItem *page = new KPageWidgetItem(vbox, i18n("General"));
    page->setObjectName("general");
    page->setHeader(i18n("General"));
    page->setIcon(KisIconUtils::loadIcon("config-general"));
    m_pages << page;
    addPage(page);
    m_general = new GeneralTab(vbox);

    // Shortcuts
    vbox = new KoVBox();
    page = new KPageWidgetItem(vbox, i18n("Keyboard Shortcuts"));
    page->setObjectName("shortcuts");
    page->setHeader(i18n("Shortcuts"));
    page->setIcon(KisIconUtils::loadIcon("config-keyboard"));
    m_pages << page;
    addPage(page);
    m_shortcutSettings = new ShortcutSettingsTab(vbox);
    connect(this, SIGNAL(accepted()), m_shortcutSettings, SLOT(saveChanges()));
    connect(this, SIGNAL(rejected()), m_shortcutSettings, SLOT(cancelChanges()));

    // Canvas input settings
    m_inputConfiguration = new KisInputConfigurationPage();
    page = addPage(m_inputConfiguration, i18n("Canvas Input Settings"));
    page->setHeader(i18n("Canvas Input"));
    page->setObjectName("canvasinput");
    page->setIcon(KisIconUtils::loadIcon("config-canvas-input"));
    m_pages << page;

    // Display
    vbox = new KoVBox();
    page = new KPageWidgetItem(vbox, i18n("Display"));
    page->setObjectName("display");
    page->setHeader(i18n("Display"));
    page->setIcon(KisIconUtils::loadIcon("config-display"));
    m_pages << page;
    addPage(page);
    m_displaySettings = new DisplaySettingsTab(vbox);

    // Color
    vbox = new KoVBox();
    page = new KPageWidgetItem(vbox, i18n("Color Management"));
    page->setObjectName("colormanagement");
    page->setHeader(i18nc("Label of color as in Color Management", "Color"));
    page->setIcon(KisIconUtils::loadIcon("config-color-manage"));
    m_pages << page;
    addPage(page);
    m_colorSettings = new ColorSettingsTab(vbox);

    // Performance
    vbox = new KoVBox();
    page = new KPageWidgetItem(vbox, i18n("Performance"));
    page->setObjectName("performance");
    page->setHeader(i18n("Performance"));
    page->setIcon(KisIconUtils::loadIcon("config-performance"));
    m_pages << page;
    addPage(page);
    m_performanceSettings = new PerformanceTab(vbox);

    // Tablet
    vbox = new KoVBox();
    page = new KPageWidgetItem(vbox, i18n("Tablet settings"));
    page->setObjectName("tablet");
    page->setHeader(i18n("Tablet"));
    page->setIcon(KisIconUtils::loadIcon("config-tablet"));
    m_pages << page;
    addPage(page);
    m_tabletSettings = new TabletSettingsTab(vbox);

    // full-screen mode
    vbox = new KoVBox();
    page = new KPageWidgetItem(vbox, i18n("Canvas-only settings"));
    page->setObjectName("canvasonly");
    page->setHeader(i18n("Canvas-only"));
    page->setIcon(KisIconUtils::loadIcon("config-canvas-only"));
    m_pages << page;
    addPage(page);
    m_fullscreenSettings = new FullscreenSettingsTab(vbox);

    // Pop-up Palette
    vbox = new KoVBox();
    page = new KPageWidgetItem(vbox, i18n("Pop-up Palette"));
    page->setObjectName("popuppalette");
    page->setHeader(i18n("Pop-up Palette"));
    page->setIcon(KisIconUtils::loadIcon("config-popup-palette"));
    m_pages << page;
    addPage(page);
    m_popupPaletteSettings = new PopupPaletteTab(vbox);

    // Author profiles
    m_authorPage = new KoConfigAuthorPage();
    page = addPage(m_authorPage, i18nc("@title:tab Author page", "Author" ));
    page->setObjectName("author");
    page->setHeader(i18n("Author"));
    page->setIcon(KisIconUtils::loadIcon("user-identity"));
    m_pages << page;

    KGuiItem::assign(button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    QPushButton *restoreDefaultsButton = button(QDialogButtonBox::RestoreDefaults);
    restoreDefaultsButton->setText(i18nc("@action:button", "Restore Defaults"));

    connect(this, SIGNAL(accepted()), m_inputConfiguration, SLOT(saveChanges()));
    connect(this, SIGNAL(rejected()), m_inputConfiguration, SLOT(revertChanges()));

    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    QStringList keys = preferenceSetRegistry->keys();
    keys.sort();
    Q_FOREACH(const QString &key, keys) {
        KisAbstractPreferenceSetFactory *preferenceSetFactory = preferenceSetRegistry->value(key);
        KisPreferenceSet* preferenceSet = preferenceSetFactory->createPreferenceSet();
        vbox = new KoVBox();
        page = new KPageWidgetItem(vbox, preferenceSet->name());
        page->setHeader(preferenceSet->header());
        page->setIcon(preferenceSet->icon());
        addPage(page);
        preferenceSet->setParent(vbox);
        preferenceSet->loadPreferences();

        connect(restoreDefaultsButton, SIGNAL(clicked(bool)), preferenceSet, SLOT(loadDefaultPreferences()), Qt::UniqueConnection);
        connect(this, SIGNAL(accepted()), preferenceSet, SLOT(savePreferences()), Qt::UniqueConnection);
    }

    connect(restoreDefaultsButton, SIGNAL(clicked(bool)), this, SLOT(slotDefault()));

    KisConfig cfg(true);
    QString currentPageName = cfg.readEntry<QString>("KisDlgPreferences/CurrentPage");
    Q_FOREACH(KPageWidgetItem *page, m_pages) {
        if (page->objectName() == currentPageName) {
            setCurrentPage(page);
            break;
        }
    }

    // TODO QT6: check what this code actually does?
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    {
        // HACK ALERT! Remove title widget background, thus making
        // it consistent across all systems
        const auto *titleWidget = findChild<KTitleWidget*>();
        if (titleWidget) {
            QLayoutItem *titleFrame = titleWidget->layout()->itemAt(0); // vboxLayout -> titleFrame
            if (titleFrame) {
                titleFrame->widget()->setBackgroundRole(QPalette::Window);
            }
        }
    }
#endif
}

KisDlgPreferences::~KisDlgPreferences()
{
    KisConfig cfg(true);
    cfg.writeEntry<QString>("KisDlgPreferences/CurrentPage", currentPage()->objectName());
}

void KisDlgPreferences::showEvent(QShowEvent *event){
    KPageDialog::showEvent(event);
    button(QDialogButtonBox::Cancel)->setAutoDefault(false);
    button(QDialogButtonBox::Ok)->setAutoDefault(false);
    button(QDialogButtonBox::RestoreDefaults)->setAutoDefault(false);
    button(QDialogButtonBox::Cancel)->setDefault(false);
    button(QDialogButtonBox::Ok)->setDefault(false);
    button(QDialogButtonBox::RestoreDefaults)->setDefault(false);
}

void KisDlgPreferences::slotButtonClicked(QAbstractButton *button)
{
    if (buttonBox()->buttonRole(button) == QDialogButtonBox::RejectRole) {
        m_cancelClicked = true;
    }
}

void KisDlgPreferences::slotDefault()
{
    if (currentPage()->objectName() == "general") {
        m_general->setDefault();
    }
    else if (currentPage()->objectName() == "shortcuts") {
        m_shortcutSettings->setDefault();
    }
    else if (currentPage()->objectName() == "display") {
        m_displaySettings->setDefault();
    }
    else if (currentPage()->objectName() == "colormanagement") {
        m_colorSettings->setDefault();
    }
    else if (currentPage()->objectName() == "performance") {
        m_performanceSettings->load(true);
    }
    else if (currentPage()->objectName() == "tablet") {
        m_tabletSettings->setDefault();
    }
    else if (currentPage()->objectName() == "canvasonly") {
        m_fullscreenSettings->setDefault();
    }
    else if (currentPage()->objectName() == "canvasinput") {
        m_inputConfiguration->setDefaults();
    }
    else if (currentPage()->objectName() == "popuppalette") {
        m_popupPaletteSettings->setDefault();
    }
}

bool KisDlgPreferences::editPreferences()
{
    connect(this->buttonBox(), SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotButtonClicked(QAbstractButton*)));

    int retval = exec();
    Q_UNUSED(retval);

    if (!m_cancelClicked) {
        // General settings
        KisConfig cfg(false);
        KisImageConfig cfgImage(false);

        cfg.setNewCursorStyle(m_general->cursorStyle());
        cfg.setNewOutlineStyle(m_general->outlineStyle());
        cfg.setSeparateEraserCursor(m_general->m_chkSeparateEraserCursor->isChecked());
        cfg.setEraserCursorStyle(m_general->eraserCursorStyle());
        cfg.setEraserOutlineStyle(m_general->eraserOutlineStyle());
        cfg.setShowRootLayer(m_general->showRootLayer());
        cfg.setShowOutlineWhilePainting(m_general->showOutlineWhilePainting());
        cfg.setForceAlwaysFullSizedOutline(!m_general->m_changeBrushOutline->isChecked());
        cfg.setShowEraserOutlineWhilePainting(m_general->showEraserOutlineWhilePainting());
        cfg.setForceAlwaysFullSizedEraserOutline(!m_general->m_changeEraserBrushOutline->isChecked());
        cfg.setSessionOnStartup(m_general->sessionOnStartup());
        cfg.setSaveSessionOnQuit(m_general->saveSessionOnQuit());

        KConfigGroup group = KSharedConfig::openConfig()->group("File Dialogs");
        group.writeEntry("DontUseNativeFileDialog", !m_general->m_chkNativeFileDialog->isChecked());

        cfgImage.setMaxBrushSize(m_general->intMaxBrushSize->value());
        cfg.setIgnoreHighFunctionKeys(m_general->chkIgnoreHighFunctionKeys->isChecked());

        cfg.writeEntry<bool>("use_custom_system_font", m_general->chkUseCustomFont->isChecked());
        if (m_general->chkUseCustomFont->isChecked()) {
            cfg.writeEntry<QString>("custom_system_font", m_general->cmbCustomFont->currentFont().family());
            cfg.writeEntry<int>("custom_font_size", m_general->intFontSize->value());
        }
        else {
            cfg.writeEntry<QString>("custom_system_font", "");
            cfg.writeEntry<int>("custom_font_size", -1);
        }

        cfg.writeEntry<int>("mdi_viewmode", m_general->mdiMode());
        cfg.setMDIBackgroundColor(m_general->m_mdiColor->color().toXML());
        cfg.setMDIBackgroundImage(m_general->m_backgroundimage->text());
        cfg.writeEntry<int>("mdi_rubberband", m_general->m_chkRubberBand->isChecked());
        cfg.setAutoSaveInterval(m_general->autoSaveInterval());
        cfg.writeEntry("autosavefileshidden", m_general->chkHideAutosaveFiles->isChecked());

        cfg.setBackupFile(m_general->m_backupFileCheckBox->isChecked());
        cfg.writeEntry("backupfilelocation", m_general->cmbBackupFileLocation->currentIndex());
        cfg.writeEntry("backupfilesuffix", m_general->txtBackupFileSuffix->text());
        cfg.writeEntry("numberofbackupfiles", m_general->intNumBackupFiles->value());


        cfg.setShowCanvasMessages(m_general->showCanvasMessages());
        cfg.setCompressKra(m_general->compressKra());
        cfg.setTrimKra(m_general->trimKra());
        cfg.setTrimFramesImport(m_general->trimFramesImport());
        cfg.setExportMimeType(m_general->exportMimeType());
        cfg.setUseZip64(m_general->useZip64());
        cfg.setPasteFormat(m_general->m_pasteFormatGroup.checkedId());

        const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
        QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
        kritarc.setValue("EnableHiDPI", m_general->m_chkHiDPI->isChecked());
#ifdef HAVE_HIGH_DPI_SCALE_FACTOR_ROUNDING_POLICY
        kritarc.setValue("EnableHiDPIFractionalScaling", m_general->m_chkHiDPIFractionalScaling->isChecked());
#endif
        kritarc.setValue("LogUsage", m_general->chkUsageLogging->isChecked());

        cfg.setToolOptionsInDocker(m_general->toolOptionsInDocker());

        cfg.writeEntry<bool>("useCreamyAlphaDarken", (bool)!m_general->cmbFlowMode->currentIndex());
        cfg.writeEntry<bool>("useSubtractiveBlendingForCmykColorSpaces", (bool)!m_general->cmbCmykBlendingMode->currentIndex());

        cfg.setZoomSteps(m_general->zoomSteps());
        cfg.setLongPressEnabled(m_general->longPressEnabled());
        cfg.setKineticScrollingEnabled(m_general->kineticScrollingEnabled());
        cfg.setKineticScrollingGesture(m_general->kineticScrollingGesture());
        cfg.setKineticScrollingSensitivity(m_general->kineticScrollingSensitivity());
        cfg.setKineticScrollingHideScrollbars(m_general->kineticScrollingHiddenScrollbars());

        cfg.setZoomMarginSize(m_general->zoomMarginSize());

        cfg.setSwitchSelectionCtrlAlt(m_general->switchSelectionCtrlAlt());
        cfg.setTouchPainting(KisConfig::TouchPainting(m_general->cmbTouchPainting->currentIndex()));
        cfg.writeEntry("useTouchPressureSensitivity", m_general->chkTouchPressureSensitivity->isChecked());
        cfg.setActivateTransformToolAfterPaste(m_general->chkEnableTransformToolAfterPaste->isChecked());
        cfg.setZoomHorizontal(m_general->chkZoomHorizontally->isChecked());
        cfg.setSelectionActionBar(m_general->chkEnableSelectionActionBar->isChecked());
        cfg.setConvertToImageColorspaceOnImport(m_general->convertToImageColorspaceOnImport());
        cfg.setUndoStackLimit(m_general->undoStackSize());
        cfg.setCumulativeUndoRedo(m_general->chkCumulativeUndo->isChecked());
        cfg.setCumulativeUndoData(m_general->m_cumulativeUndoData);

        // Animation..
        cfg.setAutoPinLayersToTimeline(m_general->autopinLayersToTimeline());
        cfg.setAdaptivePlaybackRange(m_general->adaptivePlaybackRange());
        cfg.setAutoZoomTimelineToPlaybackRange(m_general->autoZoomTimelineToPlaybackRange());

#ifdef Q_OS_ANDROID
        QFileInfo fi(m_general->m_resourceFolderSelector->currentData(Qt::UserRole).value<QString>());
#else
        QFileInfo fi(m_general->m_urlResourceFolder->fileName());
#endif
        if (fi.isWritable()) {
            cfg.writeEntry(KisResourceLocator::resourceLocationKey, fi.filePath());
        }

        KisImageConfig(true).setRenameMergedLayers(m_general->renameMergedLayers());
        cfg.setRenamePastedLayers(m_general->renamePastedLayers());
        KisImageConfig(true).setRenameDuplicatedLayers(m_general->renameDuplicatedLayers());

        // Color settings
        if (!m_colorSettings->m_colorManagedByOS) {
            cfg.setUseSystemMonitorProfile(m_colorSettings->m_page->chkUseSystemMonitorProfile->isChecked());
            for (int i = 0; i < QApplication::screens().count(); ++i) {
                if (m_colorSettings->m_page->chkUseSystemMonitorProfile->isChecked()) {
                    int currentIndex = m_colorSettings->m_monitorProfileWidgets[i]->currentIndex();
                    QString monitorid = m_colorSettings->m_monitorProfileWidgets[i]->itemData(currentIndex).toString();
                    cfg.setMonitorForScreen(i, monitorid);
                } else {
                    cfg.setMonitorProfile(i,
                                          m_colorSettings->m_monitorProfileWidgets[i]->currentUnsqueezedText(),
                                          m_colorSettings->m_page->chkUseSystemMonitorProfile->isChecked());
                }
            }
        } else {
            cfg.setEnableCanvasSurfaceColorSpaceManagement(m_colorSettings->m_chkEnableCanvasColorSpaceManagement->isChecked());
            cfg.setCanvasSurfaceColorSpaceManagementMode(m_colorSettings->m_canvasSurfaceColorSpace->currentData().value<ColorSettingsTab::CanvasSurfaceMode>());
        }
        cfg.setUseDefaultColorSpace(m_colorSettings->m_page->useDefColorSpace->isChecked());
        if (cfg.useDefaultColorSpace())
        {
            KoID currentWorkingColorSpace = m_colorSettings->m_page->cmbWorkingColorSpace->currentItem();
            cfg.setWorkingColorSpace(currentWorkingColorSpace.id());
            cfg.defColorModel(KoColorSpaceRegistry::instance()->colorSpaceColorModelId(currentWorkingColorSpace.id()).id());
            cfg.setDefaultColorDepth(KoColorSpaceRegistry::instance()->colorSpaceColorDepthId(currentWorkingColorSpace.id()).id());
        }

        cfg.writeEntry("ExrDefaultColorProfile", m_colorSettings->m_page->cmbColorProfileForEXR->currentText());

        cfgImage.setDefaultProofingConfig(m_colorSettings->m_page->proofingSpaceSelector->currentColorSpace(),
                                          int(m_colorSettings->m_proofModel->conversionIntent()),
                                          m_colorSettings->m_proofModel->convBlackPointCompensation(),
                                          m_colorSettings->m_proofModel->warningColor(),
                                          m_colorSettings->m_proofModel->adaptationState()*0.05,
                                          m_colorSettings->m_proofModel->dispBlackPointCompensation(),
                                          int(m_colorSettings->m_proofModel->displayIntent()),
                                          m_colorSettings->m_proofModel->displayTransformState());
        cfg.setUseBlackPointCompensation(m_colorSettings->m_page->chkBlackpoint->isChecked());
        cfg.setAllowLCMSOptimization(m_colorSettings->m_page->chkAllowLCMSOptimization->isChecked());
        cfg.setForcePaletteColors(m_colorSettings->m_page->chkForcePaletteColor->isChecked());
        cfg.setPasteBehaviour(m_colorSettings->m_pasteBehaviourGroup.checkedId());
        cfg.setRenderIntent(m_colorSettings->m_page->cmbMonitorIntent->currentIndex());

        // Tablet settings
        cfg.setPressureTabletCurve( m_tabletSettings->m_page->pressureCurve->curve().toString() );
        cfg.setUseRightMiddleTabletButtonWorkaround(
            m_tabletSettings->m_page->chkUseRightMiddleClickWorkaround->isChecked());

#if defined Q_OS_WIN && (defined QT5_HAS_WINTAB_SWITCH || QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)) 
        cfg.setUseWin8PointerInput(m_tabletSettings->m_page->radioWin8PointerInput->isChecked());
        
#  if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Qt6 supports switching the tablet API on the fly
        using QWindowsApplication = QNativeInterface::Private::QWindowsApplication;
        if (auto nativeWindowsApp = dynamic_cast<QWindowsApplication *>(QGuiApplicationPrivate::platformIntegration())) {
            nativeWindowsApp->setWinTabEnabled(!cfg.useWin8PointerInput());
        }
#  endif
#endif
        cfg.writeEntry<bool>("useTimestampsForBrushSpeed", m_tabletSettings->m_page->chkUseTimestampsForBrushSpeed->isChecked());
        cfg.writeEntry<int>("maxAllowedSpeedValue", m_tabletSettings->m_page->intMaxAllowedBrushSpeed->value());
        cfg.writeEntry<int>("speedValueSmoothing", m_tabletSettings->m_page->intBrushSpeedSmoothing->value());
        // the angle is saved in clockwise direction to be consistent with Drawing Angle, so negate
        cfg.writeEntry<int>("tiltDirectionOffset", -m_tabletSettings->m_page->tiltDirectionOffsetAngle->angle());

        m_performanceSettings->save();

        if (!cfg.useOpenGL() && m_displaySettings->grpOpenGL->isChecked())
            cfg.setCanvasState("TRY_OPENGL");

        if (m_displaySettings->grpOpenGL->isChecked()) {
            KisOpenGL::OpenGLRenderer renderer = static_cast<KisOpenGL::OpenGLRenderer>(
                    m_displaySettings->cmbPreferredRenderer->itemData(
                            m_displaySettings->cmbPreferredRenderer->currentIndex()).toInt());
            KisOpenGL::setUserPreferredOpenGLRendererConfig(renderer);
        } else {
            KisOpenGL::setUserPreferredOpenGLRendererConfig(KisOpenGL::RendererNone);
        }

        cfg.setUseOpenGLTextureBuffer(m_displaySettings->chkUseTextureBuffer->isChecked());
        cfg.setOpenGLFilteringMode(m_displaySettings->cmbFilterMode->currentIndex());
        cfg.setRootSurfaceFormat(&kritarc, indexToFormat(m_displaySettings->cmbPreferedRootSurfaceFormat->currentIndex()));
        cfg.setAssistantsDrawMode(indexToAssistantDrawMode(m_displaySettings->cmbAssistantsDrawMode->currentIndex()));

        cfg.setCheckSize(m_displaySettings->intCheckSize->value());
        cfg.setScrollingCheckers(m_displaySettings->chkMoving->isChecked());
        cfg.setCheckersColor1(m_displaySettings->colorChecks1->color().toQColor());
        cfg.setCheckersColor2(m_displaySettings->colorChecks2->color().toQColor());
        cfg.setCanvasBorderColor(m_displaySettings->canvasBorder->color().toQColor());
        cfg.setHideScrollbars(m_displaySettings->hideScrollbars->isChecked());
        KoColor c = m_displaySettings->btnSelectionOverlayColor->color();
        c.setOpacity(m_displaySettings->sldSelectionOverlayOpacity->value());
        cfgImage.setSelectionOverlayMaskColor(c.toQColor());
        cfgImage.setSelectionOutlineOpacity(m_displaySettings->sldSelectionOutlineOpacity->value());
        cfg.setAntialiasCurves(m_displaySettings->chkCurveAntialiasing->isChecked());
        cfg.setAntialiasSelectionOutline(m_displaySettings->chkSelectionOutlineAntialiasing->isChecked());
        cfg.setShowSingleChannelAsColor(m_displaySettings->chkChannelsAsColor->isChecked());
        cfg.setHidePopups(m_displaySettings->chkHidePopups->isChecked());

        cfg.setHideDockersFullscreen(m_fullscreenSettings->chkDockers->checkState());
        cfg.setHideMenuFullscreen(m_fullscreenSettings->chkMenu->checkState());
        cfg.setHideScrollbarsFullscreen(m_fullscreenSettings->chkScrollbars->checkState());
        cfg.setHideStatusbarFullscreen(m_fullscreenSettings->chkStatusbar->checkState());
        cfg.setHideTitlebarFullscreen(m_fullscreenSettings->chkTitlebar->checkState());
        cfg.setHideToolbarFullscreen(m_fullscreenSettings->chkToolbar->checkState());

        cfg.setCursorMainColor(m_general->cursorColorButton->color().toQColor());
        cfg.setEraserCursorMainColor(m_general->eraserCursorColorButton->color().toQColor());
        cfg.setPixelGridColor(m_displaySettings->pixelGridColorButton->color().toQColor());
        cfg.setPixelGridDrawingThreshold(m_displaySettings->pixelGridDrawingThresholdBox->value() / 100);

        m_popupPaletteSettings->save();
        m_authorPage->apply();

        cfg.logImportantSettings();
        cfg.writeEntry("forcedDpiForQtFontBugWorkaround", m_general->forcedFontDpi());
    }

    return !m_cancelClicked;
}
