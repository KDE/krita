/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2018 Scott Petrovic <scottpetrovic@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISWELCOMEPAGEWIDGET_H
#define KISWELCOMEPAGEWIDGET_H

#include "kritaui_export.h"
#include "KisViewManager.h"
#include "KisMainWindow.h"
#include <KisUpdaterBase.h>

#include <QAction>
#include <QWidget>
#include "ui_KisWelcomePage.h"
#include <QStandardItemModel>
#include <QScopedPointer>
#include "KisRecentDocumentsModelWrapper.h"

#include "config-updaters.h"
class RecentItemDelegate;

// Custom QAction to bridge a QLabel::linkActivated signal to a QAction::setChecked signal
class ShowNewsAction : public QAction
{
  Q_OBJECT
public:
    using QAction::QAction;
private Q_SLOTS:
    void enableFromLink(QString unused_url);
};

/// A widget for displaying if no documents are open. This will display in the MDI area
class KRITAUI_EXPORT KisWelcomePageWidget : public QWidget, public Ui::KisWelcomePage
{
    Q_OBJECT

    public:
    explicit KisWelcomePageWidget(QWidget *parent);
    ~KisWelcomePageWidget() override;

    void setMainWindow(KisMainWindow* m_mainWindow);

public Q_SLOTS:
    /// if a document is placed over this area, a dotted line will appear as an indicator
    /// that it is a droppable area. KisMainwindow is what triggers this
    void showDropAreaIndicator(bool show);

    void slotUpdateThemeColors();

    /**
     * Populate `recentDocumentsListView` with m_recentFilesModel
     * This could be called multiple times in a session
     */
    void populateRecentDocuments();

#ifdef ENABLE_UPDATERS
    void slotSetUpdateStatus(KisUpdaterStatus updateStatus);
    void slotShowUpdaterErrorDetails();
#endif

#ifdef Q_OS_ANDROID
    void slotStartDonationFlow();
#endif

protected:

    // QWidget overrides
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * event) override;
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dragLeaveEvent(QDragLeaveEvent * event) override;

    bool eventFilter(QObject *watched, QEvent *event) override;


private:
    void setupNewsLangSelection(QMenu *newsOptionMenu);
    void showDevVersionHighlight();

#ifdef ENABLE_UPDATERS
    void updateVersionUpdaterFrame();
#endif

    KisMainWindow *m_mainWindow;
    KisRecentDocumentsModelWrapper m_recentFilesModel;

    /// help us see how many people are clicking startup screen links
    /// you can see the results in Matomo (stats.kde.org)
    /// this will be listed in the "Acquisition" section of Matomo
    /// just append some text to this to associate it with an event/page
    const QString analyticsString = "pk_campaign=startup-sceen&pk_kwd=";


    // keeping track of link colors with theme change
    QColor textColor;
    QColor backgroundColor;
    QColor blendedColor;
    QString blendedStyle;

#ifdef ENABLE_UPDATERS
    QScopedPointer<KisUpdaterBase> m_versionUpdater;
    KisUpdaterStatus m_updaterStatus;
#endif
    bool m_checkUpdates {false};

#ifdef Q_OS_ANDROID
public:
    static QPushButton* donationLink;
    static QLabel* donationBannerImage;
#endif
    QScopedPointer<RecentItemDelegate> recentItemDelegate;

private Q_SLOTS:
    void slotNewFileClicked();
    void slotOpenFileClicked();

    void recentDocumentClicked(QModelIndex index);

    /**
     * Once all files in the recent documents model are checked, cleanup the UI if the model is empty
     */
    void slotRecentFilesModelIsUpToDate();
    /**
     * m_recentFilesModel reported an invalid recent document, we shall report it to m_mainWindow
     */
    void slotReportInvalidRecentDocument(QUrl url);

#ifdef ENABLE_UPDATERS
    void slotRunVersionUpdate();
    void slotToggleUpdateChecks(bool state);
#endif

    bool isDevelopmentBuild();
};

#endif // KISWELCOMEPAGEWIDGET_H
