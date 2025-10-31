/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISQUICKPOPUPWIDGET_H
#define KISQUICKPOPUPWIDGET_H

#include <QFrame>
#include <QQuickItem>

#include <kritaqmlwidgets_export.h>

/**
 * @brief The KisQQuickPopupWidget class
 *
 * This is a wrapper of a QFrame with Popup window flag, and a KisQQuickWidget
 * inside. This is used by KisQmlPopupWidgetManager to work around some
 * limitations with QML popups inside a QQuickWidget.
 */
class KRITAQMLWIDGETS_EXPORT KisQQuickPopupWidget: public QFrame
{
    Q_OBJECT
public:
    KisQQuickPopupWidget(QWidget *parent = nullptr);
    ~KisQQuickPopupWidget();

    /**
     * @brief rootObject
     * @return QQuickWidget::rootObject().
     */
    QQuickItem *rootObject() const;

    /**
     * @brief layoutContentMargins
     * @return layout()->contentMargins()
     */
    QMargins layoutContentMargins() const;
    /**
     * @brief setLayoutMargins
     * calls layout()->setContentMargins(margins);
     */
    void setLayoutMargins(const QMargins margins);
Q_SIGNALS:
    void signalRootObjectReady();
protected:
    ///Copied from KisPopupButton.cpp
    void keyPressEvent(QKeyEvent *event) override;

    bool event(QEvent *e) override;
private Q_SLOTS:
    void emitRootObjectReady();
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KISQUICKPOPUPWIDGET_H
