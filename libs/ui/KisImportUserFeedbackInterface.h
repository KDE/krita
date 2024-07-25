/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISIMPORTUSERFEEDBACKINTERFACE_H
#define KISIMPORTUSERFEEDBACKINTERFACE_H

#include <QtGlobal>
#include <functional>

class QWidget;

/**
 * Sometimes the importing filter may face some werd issue that needs
 * user's input/decision.
 */
class KisImportUserFeedbackInterface
{
public:
    using AskCallback = std::function<bool(QWidget*)>;

    enum Result {
        Success = 0,
        UserCancelled,
        SuppressedByBatchMode
    };

public:
    KisImportUserFeedbackInterface() = default;


    virtual ~KisImportUserFeedbackInterface();

    /**
     * @brief ask the user a question about the loading process
     *
     * @param callback a functor that actually asks the user
     * @return the result of the operation
     */
    virtual Result askUser(AskCallback callback) = 0;

private:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    Q_DISABLE_COPY_MOVE(KisImportUserFeedbackInterface);
#else
    KisImportUserFeedbackInterface(const KisImportUserFeedbackInterface&) = delete;
    KisImportUserFeedbackInterface(KisImportUserFeedbackInterface&&) = delete;
#endif
};

#endif // KISIMPORTUSERFEEDBACKINTERFACE_H
