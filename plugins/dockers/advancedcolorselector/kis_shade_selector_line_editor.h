/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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
