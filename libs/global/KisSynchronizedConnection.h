/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSYNCHRONIZEDCONNECTION_H
#define KISSYNCHRONIZEDCONNECTION_H

#include <kritaglobal_export.h>

#include <QObject>
#include <QEvent>

#include <KisMpl.h>
#include <functional>
#include <queue>
#include <boost/bind.hpp>
#include <kis_assert.h>
#include <QPointer>

/**
 * @brief Event type used for synchronizing connection in KisSynchronizedConnection
 *
 * KisApplication will recognize this event type and postpone it until the
 * recursion state is over
 */
struct KRITAGLOBAL_EXPORT KisSynchronizedConnectionEvent : public QEvent
{
    KisSynchronizedConnectionEvent(QObject *_destination);
    KisSynchronizedConnectionEvent(const KisSynchronizedConnectionEvent &rhs);
    ~KisSynchronizedConnectionEvent();

    const QPointer<QObject> destination;
};

/**
 * @brief A base class for KisSynchronizedConnection
 *
 * This class implements QEvent logic for KisSynchronizedConnection. Since
 * KisSynchronizedConnection is templated, it should be implemented fully
 * inline, but we don't want to expose our interactions with KisApplication.
 * Therefore we implement this logic in a separate non-templated class that
 * will be hidden in `kritaglobal`.
 */
class KRITAGLOBAL_EXPORT KisSynchronizedConnectionBase : public QObject
{
public:
    static int eventType();

protected:
    bool event(QEvent *event);

protected:
    virtual void deliverEventToReceiver() = 0;
    void postEvent();
};

/**
 * A "simple" class for ensuring a queued connection is never executed in
 * a recursive event processing loop.
 *
 * In several places in Krita we use queued signals for synchronizing
 * image chages to the GUI. In such cases we use Qt::DirectConnection
 * to fetch some data from the image, wrap that into the signal
 * parameters and post at the events queue as a queued signal. Obviously,
 * we expect this queued signal to be executed "after all the currently
 * processed GUI actions are finished". But that is not always true in Qt...
 *
 * In Qt the queued signal will be executed "as soon as execution path
 * returns to the event loop". And it can also happen when a nested
 * event loop started (by opening a QDialog) or QApplication::processEvent()
 * is called. It means that the processing of a queued signal can start
 * before the currently running GUI action is finished (because the current
 * task has been recursively overridden by KisBusyWaitBroker.
 *
 * KisSynchronizedConnection is workaround to this problem. Every connection
 * made via KisSynchronizedConnection ensures that the target slot
 * is executed without any recursion. The class tried to resemble new
 * member-function-pointer-based API of QObject::connect.
 *
 * In case the signal is emitted from the GUI thread, KisSynchronizedConnection
 * behaves as Qt::AutoConnection, that is, delivers event right away, skipping
 * the event loop.
 *
 * Under the hood the class uses a custom event (KisSynchronizedConnectionEvent),
 * which is recognized by KisApplication and postponed until the recursion state
 * is over.
 *
 * @param Args... the list of arguments that are passed through the signal
 *
 * Usage:
 *
 *        \code{.cpp}
 *
 *        class KisImage
 *        {
 *            // ...
 *        Q_SIGNALS:
 *            void sigRequestNodeReselection(KisNodeSP activeNode, const KisNodeList &selectedNodes);
 *        };
 *
 *        KisSynchronizedConnection<KisNodeSP, KisNodeList> connection;
 *
 *        // if you want connect input and output separately
 *        connection.connectInputSignal(image, &KisImage::sigRequestNodeReselection);
 *        connection.connectOutputSlot(nodeManager, &KisNodeManager::slotImageRequestNodeReselection)
 *
 *        // if you want to connect them in one call (in QObject style)
 *        connection.connectSync(image, &KisImage::sigRequestNodeReselection,
 *                               nodeManager, &KisNodeManager::slotImageRequestNodeReselection);
 *
 *        \endcode
 */
template <typename... Args>
class KisSynchronizedConnection : public KisSynchronizedConnectionBase
{
public:
public:
    using ArgsTuple = std::tuple<Args...>;
    using CallbackFunction = std::function<void (Args...)>;

public:
    KisSynchronizedConnection() = default;
    KisSynchronizedConnection(CallbackFunction callback)
        : m_callback(callback)
    {}

    /**
     * Triggers the delivery of the signal to the destination slot manualy
     */
    void start(const Args &...argsTuple) {
        m_queue.emplace(std::make_tuple(argsTuple...));
        this->postEvent();
    }

    /**
     * Sets an arbitrary callback as a destination slot in the connection.
     * The callback should have a signature `void (Args...)`
     */
    void setCallback(CallbackFunction callback) {
        m_callback = callback;
    }

    /**
     * Connect input signal to the connection
     *
     * This part of the connection is based on Qt-signal mechanism, therefore
     * @p object should be convertible into `const QObject*`.
     */
    template <typename Dptr, typename C, typename R, typename ...MemFnArgs>
    void connectInputSignal(Dptr object, R (C::* memfn)(MemFnArgs...)) {
        static_assert (std::is_convertible<Dptr, const C*>::value, "Source object should be convertible into the base of the member pointer");
        static_assert (std::is_convertible<Dptr, const QObject*>::value, "Source object should be convertible into QObject");

        QObject::connect(static_cast<const C*>(object), memfn,
                         this, &KisSynchronizedConnection::start);
    }

    /**
     * Connect output slot to the connection
     *
     * Since destination slot doesn't use Qt-signal machinery, the destination
     * object shouldn't necessarily be a QObject. It should just be a member
     * function with a compatible signature.
     */
    template <typename Dptr, typename C, typename R, typename ...MemFnArgs>
    void connectOutputSlot(Dptr object, R (C::* memfn)(MemFnArgs...)) {
        static_assert (std::is_convertible<Dptr, C*>::value, "Destination object should be convertible into the base of the member pointer");
        KIS_SAFE_ASSERT_RECOVER_RETURN(!m_callback);

        m_callback = bindToMemberFunction(object, memfn,
                                          kismpl::make_index_sequence_from_1<
                                              std::tuple_size<ArgsTuple>::value>());
    }

    /**
     * A convenience method for seting up input and output connections at
     * the same time
     */
    template <typename Dptr1, typename C1, typename R1, typename ...MemFnArgs1,
              typename Dptr2, typename C2, typename R2, typename ...MemFnArgs2>
    void connectSync(Dptr1 object1, R1 (C1::* memfn1)(MemFnArgs1...),
                     Dptr2 object2, R2 (C2::* memfn2)(MemFnArgs2...)) {

        connectInputSignal(object1, memfn1);
        connectOutputSlot(object2, memfn2);
   }

    void disconnectInputSignals() {
        this->disconnect();
    }

    void disconnectOutputSlot() {
        m_callback = CallbackFunction();
    }

    bool hasPendingSignals() const {
        return !m_queue.empty();
    }

private:

    template <typename Dptr, typename C, typename R, typename ...MemFnArgs, std::size_t ...Idx>
    CallbackFunction bindToMemberFunction(Dptr object, R (C::* memfn)(MemFnArgs...), std::index_sequence<Idx...>) {

        /// we cannot use std::bind here, because it doesn't support
        /// indexed iteration over the argument placeholders

        return boost::bind(memfn, object, boost::arg<Idx>()...);
    }

protected:
    void deliverEventToReceiver() override {
        kismpl::apply(m_callback, m_queue.front());
        m_queue.pop();
    }

private:
    CallbackFunction m_callback;
    std::queue<ArgsTuple> m_queue;
};

#endif // KISSYNCHRONIZEDCONNECTION_H
