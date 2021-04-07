/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_POPUP_WIDGET_ACTION_H
#define KIS_POPUP_WIDGET_ACTION_H

#include "kis_abstract_input_action.h"

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QMainWindow>

#include "kis_global.h"
#include "kis_debug.h"
#include "kis_input_manager.h"
#include "kis_canvas2.h"
class QMenu;

/**
 * @brief The PopupWidgetInterface class
 */
struct PopupWidgetInterface {
    virtual ~PopupWidgetInterface() {}
    virtual void popup(const QPoint& position) = 0;
};


/**
 * @brief The PopupWidgetHolder class
 */
class PopupWidgetHolder : public QObject, PopupWidgetInterface
{
public:
    PopupWidgetHolder(QWidget* toPopUp, KisInputManager* inputManager)
        : m_toPopUp(toPopUp)
        , m_inputManager(inputManager)
    {
        m_toPopUp->setParent(m_inputManager->canvas()->canvasWidget());
    }

    ~PopupWidgetHolder() {}

    void popup(const QPoint& position) override {
        m_toPopUp->setVisible(!m_toPopUp->isVisible());
        m_inputManager->registerPopupWidget(m_toPopUp);
        adjustPopupLayout(position);
    }

private:
    void adjustPopupLayout(const QPoint& postition) {
        if (m_toPopUp->isVisible() && m_toPopUp->parentWidget())  {
            const float widgetMargin = -20.0f;
            const QRect fitRect = kisGrowRect(m_toPopUp->parentWidget()->rect(), widgetMargin);
            const QPoint paletteCenterOffset(m_toPopUp->sizeHint().width() / 2, m_toPopUp->sizeHint().height() / 2);

            QRect paletteRect = m_toPopUp->rect();

            paletteRect.moveTo(postition - paletteCenterOffset);

            paletteRect = kisEnsureInRect(paletteRect, fitRect);
            m_toPopUp->move(paletteRect.topLeft());
        }
    }

    QWidget* m_toPopUp;
    KisInputManager* m_inputManager;
};


/**
 * \brief Get the current tool's popup widget and display it.
 */
class KisPopupWidgetAction : public QObject, public KisAbstractInputAction
{
    Q_OBJECT

public:
    explicit KisPopupWidgetAction();
    ~KisPopupWidgetAction() override;

    int priority() const override {return 1;}

    void begin(int, QEvent *) override;

private:
    QScopedPointer<PopupWidgetHolder> m_activePopup;
    bool m_requestedWithStylus;
};

#endif // KIS_POPUP_WIDGET_ACTION_H
