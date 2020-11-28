/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWarningWidget.h"

#include "klocalizedstring.h"
#include "kis_icon_utils.h"
#include <QHBoxLayout>
#include <QLabel>


KisWarningWidget::KisWarningWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    m_warningIcon = new QLabel(this);
    m_warningIcon->setPixmap(KisIconUtils::loadIcon("warning").pixmap(32, 32));
    m_warningIcon->setAlignment(Qt::AlignTop);
    hLayout->addWidget(m_warningIcon);

    m_warningText = new QLabel(this);
    m_warningText->setWordWrap(true);
    m_warningText->setOpenExternalLinks(true);
    hLayout->addWidget(m_warningText, 1);

    setLayout(hLayout);
}

void KisWarningWidget::setText(const QString &text)
{
    m_warningText->setText(text);
}

QString KisWarningWidget::changeImageProfileWarningText()
{
    return i18nc("warning message when changing image color space",
                 "<html><body>"
                 "<p><b>WARNING:</b> the image will look different after changing the color profile because it contains either:"
                 "<ul>"
                 "<li>more than one layer</li>"
                 "<li>one or more layers with transparent pixels</li>"
                 "<li>layers with blending modes other than \"Normal\"</li>"
                 "</ul></p>"
                 "<p>"
                 "<a href=\"https://docs.krita.org/en/general_concepts/colors/color_managed_workflow.html\">More information</a>"
                 "</p></body></html>");
}
