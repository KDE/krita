/*
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisSpinBoxPluralHelper.h"

#include <QDebug>
#include <QSpinBox>

namespace KisSpinBoxPluralHelper
{
    namespace
    {
        const char *const HANDLER_PROPERTY_NAME = "_kis_KisSpinBoxPluralHelper_handler";

        struct HandlerWrapper
        {
            std::function<void(int)> m_handler;

            HandlerWrapper() {}

            explicit HandlerWrapper(std::function<void(int)> handler)
                : m_handler(handler)
            {}
        };

    } /* namespace */

} /* namespace KisSpinBoxPluralHelper */

Q_DECLARE_METATYPE(KisSpinBoxPluralHelper::HandlerWrapper)

namespace KisSpinBoxPluralHelper
{
    void install(QSpinBox *spinBox, std::function<QString(int)> messageFn)
    {
        const auto changeHandler = [messageFn, spinBox](int value) {
            const QString text = messageFn(value);
            const QString placeholder = QStringLiteral("{n}");
            const int idx = text.indexOf(placeholder);
            if (idx >= 0) {
                spinBox->setPrefix(text.left(idx));
                spinBox->setSuffix(text.mid(idx + placeholder.size()));
            } else {
                spinBox->setPrefix(QString());
                spinBox->setSuffix(text);
            }
        };
        // Apply prefix/suffix with existing value immediately.
        changeHandler(spinBox->value());
        QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), changeHandler);
        spinBox->setProperty(HANDLER_PROPERTY_NAME, QVariant::fromValue(HandlerWrapper(changeHandler)));
    }

    bool update(QSpinBox *spinBox)
    {
        const QVariant handlerVariant = spinBox->property(HANDLER_PROPERTY_NAME);
        if (!handlerVariant.isValid()) {
            qWarning() << "KisSpinBoxPluralHelper::update called with" << spinBox
                       << "but it does not have the property" << HANDLER_PROPERTY_NAME;
            return false;
        }
        if (!handlerVariant.canConvert<HandlerWrapper>()) {
            qWarning() << "KisSpinBoxPluralHelper::update called with" << spinBox
                       << "but its property" << HANDLER_PROPERTY_NAME << "is invalid";
            return false;
        }
        const HandlerWrapper handler = handlerVariant.value<HandlerWrapper>();
        handler.m_handler(spinBox->value());
        return true;
    }
} /* namespace KisSpinBoxPluralHelper */
