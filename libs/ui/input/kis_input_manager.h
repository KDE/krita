/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_INPUTMANAGER_H
#define KIS_INPUTMANAGER_H

#include <QObject>
#include <QPointer>

#include <kritaui_export.h>

#include <kis_tool_proxy.h>

class QPointF;
class QTouchEvent;
class KisCanvas2;
/**
 * \brief Central object to manage canvas input.
 *
 * The Input Manager class manages all canvas input. It is created
 * by KisCanvas2 and processes all events related to input sent to the
 * canvas.
 *
 * The Input Manager keeps track of a set of actions and a set of
 * shortcuts. The actions are pre-defined while the shortcuts are
 * set from configuration.
 *
 * For each event, it will try to determine if there is a shortcut that
 * matches the input. It will then activate this action and pass all
 * consecutive events on to this action.
 *
 * \sa KisAbstractInputAction
 */
class KRITAUI_EXPORT KisInputManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    KisInputManager(QObject *parent);

    /**
     * Destructor.
     */
    ~KisInputManager() override;

    void addTrackedCanvas(KisCanvas2 *canvas);
    void removeTrackedCanvas(KisCanvas2 *canvas);

    void registerPopupWidget(KisPopupWidgetInterface *popupWidget);

    void toggleTabletLogger();

    /**
     * Installs the input manager as an event filter for \p receiver.
     * Please note that KisInputManager is supposed to handle events
     * for a single receiver only. This is defined by the fact that it
     * resends some of the events back through the Qt's queue to the
     * receiver. That is why the input manager will assert when it gets
     * an event with wrong destination.
     */
    void setupAsEventFilter(QObject *receiver);

    /**
     * Event filter method. Overridden from QObject.
     */
    bool eventFilter(QObject* object, QEvent* event ) override;

    /**
     * @brief attachPriorityEventFilter
     * @param filter
     * @param priority
     */
    void attachPriorityEventFilter(QObject *filter, int priority = 0);

    /**
     * @brief detachPriorityEventFilter
     * @param filter
     */
    void detachPriorityEventFilter(QObject *filter);

    /**
     * Return the canvas this input manager is associated with.
     */
    KisCanvas2 *canvas() const;

    /**
     * The tool proxy of the current application.
     */
    QPointer<KisToolProxy> toolProxy() const;

private Q_SLOTS:
    void slotAboutToChangeTool();
    void slotToolChanged();
    void profileChanged();
    void slotCompressedMoveEvent();
    void deregisterPopupWidget();

private:
    bool startTouch(bool &retval);
    void endTouch();

    bool eventFilterImpl(QEvent * event);
    template <class Event>
        bool compressMoveEventCommon(Event *event);

private:
    class Private;
    Private* const d;
};

#endif // KIS_INPUTMANAGER_H
