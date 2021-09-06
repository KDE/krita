/* This file is part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2003-2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_STATUSBAR_H
#define KIS_STATUSBAR_H

#include <QObject>
#include <QPointer>
#include <QIcon>
#include <QStatusBar>

#include <kis_types.h>
#include "KisView.h"

class QLabel;
class QToolButton;
class QPushButton;
class QBoxLayout;
class KSqueezedTextLabel;
class KisViewManager;
class KisProgressWidget;
class KoProgressUpdater;
class KisMemoryReportButton;
class KisAngleSelector;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisStatusBar : public QObject
{
    class StatusBarItem
    {
    public:
        StatusBarItem()
            : m_widget(0) {}

        StatusBarItem(QWidget * widget)
            : m_widget(widget) {}

        bool operator==(const StatusBarItem& rhs) {
            return m_widget == rhs.m_widget;
        }

        bool operator!=(const StatusBarItem& rhs) {
            return m_widget != rhs.m_widget;
        }

        QWidget * widget() const {
            return m_widget;
        }

        void show() const {
            m_widget->show();
        }
        void hide() const {
            m_widget->hide();
        }


    private:
        QPointer<QWidget> m_widget;
    };

    Q_OBJECT

public:

    explicit KisStatusBar(KisViewManager *view);
    ~KisStatusBar() override;

    void setup();
    void setView(QPointer<KisView> imageView);
    void hideAllStatusBarItems();
    void showAllStatusBarItems();

    KoProgressUpdater *progressUpdater();

    void addExtraWidget(QWidget *widget);
    void removeExtraWidget(QWidget *widget);

    void setStatusBarStatusLabelText(const QString &text);

public Q_SLOTS:

    void documentMousePositionChanged(const QPointF &p);
    void imageSizeChanged();
    void setSelection(KisImageWSP image);
    void setProfile(KisImageWSP image);
    void setHelp(const QString &t);
    void updateStatusBarProfileLabel();
    void updateSelectionToolTip();

private Q_SLOTS:
    void updateSelectionIcon();
    void showMemoryInfoToolTip();
    void slotCanvasAngleSelectorAngleChanged(qreal angle);
    void slotCanvasRotationChanged();

Q_SIGNALS:
    void sigCancellationRequested();

    /// tell the listener that the memory usage has changed
    /// and it needs to update its stats
    void memoryStatusUpdated();

private:

    void removeStatusBarItem(QWidget *widget);
    void addStatusBarItem(QWidget *widget, int stretch = 0, bool permanent = false);
    void updateMemoryStatus();

private:

    QPointer<KisViewManager> m_viewManager;
    QPointer<KisView> m_imageView;
    QPointer<QStatusBar> m_statusBar;
    KisProgressWidget *m_progress;
    QScopedPointer<KoProgressUpdater> m_progressUpdater;

    QToolButton *m_selectionStatus;
    KisMemoryReportButton *m_memoryReportBox;
    QLabel *m_pointerPositionLabel;
    QWidget *m_extraWidgetsParent;
    QBoxLayout *m_extraWidgetsLayout;
    KisAngleSelector *m_canvasAngleSelector;
    QToolButton *m_fullscreenToggle;

    KSqueezedTextLabel *m_statusBarStatusLabel;
    KSqueezedTextLabel *m_statusBarProfileLabel;

    QString m_shortMemoryTag;
    QString m_longMemoryTag;
    QIcon m_memoryStatusIcon;

    QVector<StatusBarItem> m_statusBarItems;

    bool m_memoryWarningLogged {false};
};

#endif
