/*
 * SPDX-FileCopyrightText: 2021-2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisSpinBoxI18nHelper.h"

#include <QDebug>
#include <QSpinBox>

namespace KisSpinBoxI18nHelper
{
    namespace
    {
        const char *const HANDLER_PROPERTY_NAME = "_kis_KisSpinBoxI18nHelper_handler";

        struct HandlerWrapper
        {
            std::function<void(int)> m_handler;

            HandlerWrapper() {}

            explicit HandlerWrapper(std::function<void(int)> handler)
                : m_handler(handler)
            {}
        };

    } /* namespace */

} /* namespace KisSpinBoxI18nHelper */

Q_DECLARE_METATYPE(KisSpinBoxI18nHelper::HandlerWrapper)

namespace KisSpinBoxI18nHelper
{
    void install(QSpinBox *spinBox, std::function<QString(int)> messageFn)
    {
        const auto changeHandler = [messageFn, spinBox](int value) {
            setText(spinBox, messageFn(value));
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
            qWarning() << "KisSpinBoxI18nHelper::update called with" << spinBox
                       << "but it does not have the property" << HANDLER_PROPERTY_NAME;
            return false;
        }
        if (!handlerVariant.canConvert<HandlerWrapper>()) {
            qWarning() << "KisSpinBoxI18nHelper::update called with" << spinBox
                       << "but its property" << HANDLER_PROPERTY_NAME << "is invalid";
            return false;
        }
        const HandlerWrapper handler = handlerVariant.value<HandlerWrapper>();
        handler.m_handler(spinBox->value());
        return true;
    }

    template<typename TSpinBox>
    static void setTextGeneric(TSpinBox *spinBox, const QStringView textTemplate)
    {
        const QLatin1String placeholder{"{n}"};
        const qsizetype idx = textTemplate.indexOf(placeholder);
        if (idx >= 0) {
            spinBox->setPrefix(textTemplate.left(idx).toString());
            spinBox->setSuffix(textTemplate.mid(idx + placeholder.size()).toString());
        } else {
            spinBox->setPrefix(QString());
            spinBox->setSuffix(textTemplate.toString());
        }
    }

    void setText(QSpinBox *spinBox, const QStringView textTemplate)
    {
        setTextGeneric(spinBox, textTemplate);
    }

    void setText(QDoubleSpinBox *spinBox, const QStringView textTemplate)
    {
        setTextGeneric(spinBox, textTemplate);
    }

} /* namespace KisSpinBoxI18nHelper */
