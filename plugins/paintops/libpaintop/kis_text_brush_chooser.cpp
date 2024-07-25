/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_text_brush_chooser.h"

#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QLineEdit>
#include <QString>
#include <QFontDialog>

#include "KisTextBrushModel.h"
#include "KisWidgetConnectionUtils.h"

using namespace KisBrushModel;
using namespace KisWidgetConnectionUtils;

struct KisTextBrushChooser::Private
{
    Private(KisTextBrushModel *_model)
        : model(_model)
    {
    }

    KisTextBrushModel *model;
};

KisTextBrushChooser::KisTextBrushChooser(KisTextBrushModel *model, QWidget *parent)
    : QWidget(parent),
      m_d(new Private(model))
{
    setObjectName("textbrush");
    setupUi(this);

    setWindowTitle(i18nc("Text Brush tip mode", "Text"));

    inputSpacing->setRange(0.0, 10, 2);
    inputSpacing->setSingleStep(0.01);
    inputSpacing->setValue(0.1);

    connectControl(pipeModeChbox, m_d->model, "usePipeMode");
    connectControl(inputSpacing, m_d->model, "spacing");
    connectControl(lineEdit, m_d->model, "text");

    connect((QObject*)bnFont, SIGNAL(clicked()), this, SLOT(getFont()));

    connect(m_d->model, &KisTextBrushModel::fontChanged,
            this, &KisTextBrushChooser::updateBrushPreview);
    updateBrushPreview();
}

KisTextBrushChooser::~KisTextBrushChooser()
{
}

void KisTextBrushChooser::getFont()
{
    bool ok = false;

    QFont f;
    f.fromString(m_d->model->font());
    f = QFontDialog::getFont(&ok, f);

    if (ok) {
        m_d->model->setfont(f.toString());
    }
}

void KisTextBrushChooser::updateBrushPreview()
{
    QFont f;
    f.fromString(m_d->model->font());

    lblFont->setText(QString(f.family() + ", %1").arg(f.pointSize()));
    lblFont->setFont(f);
}

