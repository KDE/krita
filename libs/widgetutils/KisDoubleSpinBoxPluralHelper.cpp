/*
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisDoubleSpinBoxPluralHelper.h"

#include <QDebug>
#include <QDoubleSpinBox>

namespace KisDoubleSpinBoxPluralHelper
{
    namespace
    {
        const char *const HANDLER_PROPERTY_NAME = "_kis_KisDoubleSpinBoxPluralHelper_handler";

        struct HandlerWrapper
        {
            std::function<void(double)> m_handler;

            HandlerWrapper() {}

            explicit HandlerWrapper(std::function<void(double)> handler)
                : m_handler(handler)
            {}
        };

    } /* namespace */

} /* namespace KisDoubleSpinBoxPluralHelper */

Q_DECLARE_METATYPE(KisDoubleSpinBoxPluralHelper::HandlerWrapper)

namespace KisDoubleSpinBoxPluralHelper
{
    void install(QDoubleSpinBox *spinBox, std::function<QString(double)> messageFn)
    {
        const auto changeHandler = [messageFn, spinBox](double doubleValue) {
            const QString text = messageFn(doubleValue);
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

        // Apply prefix/suffix with the existing value immediately.
        changeHandler(spinBox->value());
        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), changeHandler);
        spinBox->setProperty(HANDLER_PROPERTY_NAME, QVariant::fromValue(HandlerWrapper(changeHandler)));
    }

    bool update(QDoubleSpinBox *spinBox)
    {
        const QVariant handlerVariant = spinBox->property(HANDLER_PROPERTY_NAME);
        if (!handlerVariant.isValid()) {
            qWarning() << "KisDoubleSpinBoxPluralHelper::update called with" << spinBox
                       << "but it does not have the property" << HANDLER_PROPERTY_NAME;
            return false;
        }
        if (!handlerVariant.canConvert<HandlerWrapper>()) {
            qWarning() << "KisDoubleSpinBoxPluralHelper::update called with" << spinBox
                       << "but its property" << HANDLER_PROPERTY_NAME << "is invalid";
            return false;
        }
        const HandlerWrapper handler = handlerVariant.value<HandlerWrapper>();
        handler.m_handler(spinBox->value());
        return true;
    }
} /* namespace KisDoubleSpinBoxPluralHelper */
