/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SHADE_SELECTOR_LINE_EDITOR_H
#define __KIS_SHADE_SELECTOR_LINE_EDITOR_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QLabel>

#include <klocalizedstring.h>

#include "kis_shade_selector_line.h"

class KisDoubleParseSpinBox;

class KisShadeSelectorLineEditor : public KisShadeSelectorLineBase {
    Q_OBJECT
public:
    KisShadeSelectorLineEditor(QWidget* parent, KisShadeSelectorLine *preview);

    QString toString() const override;
    void fromString(const QString &string) override;

    void mousePressEvent(QMouseEvent* e) override;

private:
    void updatePreview();

private Q_SLOTS:
    void valueChanged();

Q_SIGNALS:
    void requestActivateLine(QWidget *widget);

private:
    KisShadeSelectorLine* m_line_preview;

    KisDoubleParseSpinBox* m_hueDelta;
    KisDoubleParseSpinBox* m_saturationDelta;
    KisDoubleParseSpinBox* m_valueDelta;
    KisDoubleParseSpinBox* m_hueShift;
    KisDoubleParseSpinBox* m_saturationShift;
    KisDoubleParseSpinBox* m_valueShift;
};

#endif /* __KIS_SHADE_SELECTOR_LINE_EDITOR_H */
