/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISQMLPOPUP_H
#define KISQMLPOPUP_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>

/**
 * @brief The KisQmlPopupWidgetManager class
 * This encapsulates a KisQQuickPopupWidget.h class, so it can be used from QML.
 *
 * QQuickWidgets don't allow popups to go outside their bounds, so this class is
 * essentially working around that by using a KisQQuickPopupWidget to wrap the
 * content item in.
 */
class KisQmlPopupWidgetManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)

    Q_PROPERTY(QQuickItem* parent READ itemParent WRITE setItemParent NOTIFY itemParentChanged)
    Q_PROPERTY(QQuickItem* rootControl READ rootControl NOTIFY rootControlChanged)

    Q_PROPERTY(qreal margins READ margins WRITE setMargins NOTIFY marginsChanged)
    Q_PROPERTY(qreal topMargin READ topMargin WRITE setTopMargin RESET resetTopMargin NOTIFY topMarginChanged)
    Q_PROPERTY(qreal leftMargin READ leftMargin WRITE setLeftMargin RESET resetLeftMargin NOTIFY leftMarginChanged)
    Q_PROPERTY(qreal rightMargin READ rightMargin WRITE setRightMargin RESET resetRightMargin NOTIFY rightMarginChanged)
    Q_PROPERTY(qreal bottomMargin READ bottomMargin WRITE setBottomMargin RESET resetBottomMargin NOTIFY bottomMarginChanged)

    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)

    QML_NAMED_ELEMENT(PopupWidget)
public:
    explicit KisQmlPopupWidgetManager(QObject *parent = nullptr);
    ~KisQmlPopupWidgetManager();

    qreal x() const;
    void setX(const qreal value);

    qreal y() const;
    void setY(const qreal value);

    QQuickItem *itemParent() const;
    void setItemParent(QQuickItem *item);

    QQuickItem *rootControl() const;

    /**
     * @brief margins
     * Popup qml component has margins, so we implement those in here too.
     */
    qreal margins() const;
    void setMargins(const qreal value);

    qreal topMargin() const;
    qreal bottomMargin() const;
    qreal leftMargin() const;
    qreal rightMargin() const;

    void setTopMargin(const qreal value);
    void setLeftMargin(const qreal value);
    void setRightMargin(const qreal value);
    void setBottomMargin(const qreal value);

    void resetTopMargin();
    void resetLeftMargin();
    void resetRightMargin();
    void resetBottomMargin();

    bool visible() const;

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void releaseKeyboard();
Q_SIGNALS:
    void xChanged();
    void yChanged();

    void itemParentChanged();
    void rootControlChanged();

    void visibleChanged();
    void marginsChanged();
    void topMarginChanged();
    void rightMarginChanged();
    void leftMarginChanged();
    void bottomMarginChanged();
private Q_SLOTS:
    void updateMargins();
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KISQMLPOPUP_H
