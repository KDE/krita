/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <KisDialogStateSaver.h>
#include <KoColor.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <filter/kis_filter_configuration.h>
#include <SeExpr2/UI/ErrorMessages.h>

#include "generator.h"
#include "kis_wdg_seexpr.h"
#include "SeExprExpressionContext.h"
#include "ui_wdgseexpr.h"

KisWdgSeExpr::KisWdgSeExpr(QWidget *parent)
    : KisConfigWidget(parent),
      updateCompressor(1000, KisSignalCompressor::Mode::POSTPONE)
{
    m_widget = new Ui_WdgSeExpr();
    m_widget->setupUi(this);
    KisDialogStateSaver::restoreState(m_widget->txtEditor, "krita/generators/seexpr");
    // Manually restore SeExpr state. KisDialogStateSaver uses setPlainText, not text itself
    m_widget->txtEditor->setExpr(m_widget->txtEditor->exprTe->toPlainText());

    m_widget->txtEditor->registerExtraVariable("$u", i18nc("SeExpr variable", "Normalized X axis coordinate of the image from its top-left corner"));
    m_widget->txtEditor->registerExtraVariable("$v", i18nc("SeExpr variable", "Normalized Y axis coordinate of the image from its top-left corner"));
    m_widget->txtEditor->registerExtraVariable("$w", i18nc("SeExpr variable", "Image width"));
    m_widget->txtEditor->registerExtraVariable("$h", i18nc("SeExpr variable", "Image height"));

    m_widget->txtEditor->updateCompleter();

    connect(m_widget->txtEditor, SIGNAL(apply()), &updateCompressor, SLOT(start()));
    connect(m_widget->txtEditor, SIGNAL(preview()), &updateCompressor, SLOT(start()));

    connect(&updateCompressor, SIGNAL(timeout()), this, SLOT(isValid()));
}

KisWdgSeExpr::~KisWdgSeExpr()
{
    KisDialogStateSaver::saveState(m_widget->txtEditor, "krita/generators/seexpr");
    delete m_widget;
}

inline const Ui_WdgSeExpr *KisWdgSeExpr::widget() const {
    return m_widget;
}

void KisWdgSeExpr::setConfiguration(const KisPropertiesConfigurationSP config)
{
    Q_ASSERT(!config->getString("script").isEmpty());
    QString script = config->getString("script");

    widget()->txtEditor->setExpr(script, true);
}

KisPropertiesConfigurationSP KisWdgSeExpr::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("seexpr", 1);
    QVariant v(widget()->txtEditor->getExpr());

    config->setProperty("script", v);

    return config;
}

void KisWdgSeExpr::isValid()
{
    SeExprExpressionContext expression(widget()->txtEditor->getExpr());

    expression.setDesiredReturnType(SeExpr2::ExprType().FP(3));

    expression.m_vars["u"] = new SeExprVariable();
    expression.m_vars["v"] = new SeExprVariable();
    expression.m_vars["w"] = new SeExprVariable();
    expression.m_vars["h"] = new SeExprVariable();

    widget()->txtEditor->clearErrors();

    if (!expression.isValid())
    {
        auto errors = expression.getErrors();

        for (auto occurrence: errors)
        {
            QString message = ErrorMessages::message(occurrence.error);
            for (auto arg : occurrence.ids) {
                message = message.arg(QString::fromStdString(arg));
            }
            widget()->txtEditor->addError(occurrence.startPos, occurrence.endPos, message);
        }
    }
    // Should not happen now, but I've left it for completeness's sake
    else if (!expression.returnType().isFP(3))
    {
        QString type = QString::fromStdString(expression.returnType().toString());
        widget()->txtEditor->addError(1, 1, tr2i18n("Expected this script to output color, got '%1'").arg(type));
    }
    else
    {
        widget()->txtEditor->clearErrors();
        emit sigConfigurationUpdated();
    }
}
