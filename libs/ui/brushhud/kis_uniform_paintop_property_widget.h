/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_UNIFORM_PAINTOP_PROPERTY_WIDGET_H
#define __KIS_UNIFORM_PAINTOP_PROPERTY_WIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "kis_uniform_paintop_property.h"


class KisUniformPaintOpPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    KisUniformPaintOpPropertyWidget(KisUniformPaintOpPropertySP property, QWidget *parent);
    ~KisUniformPaintOpPropertyWidget() override;

protected:
    KisUniformPaintOpPropertySP property() const;

protected Q_SLOTS:
    virtual void setValue(const QVariant &value) = 0;

Q_SIGNALS:
    void valueChanged(const QVariant &value);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

class KisSliderSpinBox;
class KisDoubleSliderSpinBox;
class QCheckBox;

class KisUniformPaintOpPropertyIntSlider : public KisUniformPaintOpPropertyWidget
{
    Q_OBJECT
public:
    KisUniformPaintOpPropertyIntSlider(KisUniformPaintOpPropertySP property, QWidget *parent);

    void setValue(const QVariant &value) override;

private Q_SLOTS:
    void slotSliderChanged(int value);

private:
    KisSliderSpinBox *m_slider;
};

class KisUniformPaintOpPropertyDoubleSlider : public KisUniformPaintOpPropertyWidget
{
    Q_OBJECT
public:
    KisUniformPaintOpPropertyDoubleSlider(KisUniformPaintOpPropertySP property, QWidget *parent);

    void setValue(const QVariant &value) override;

private Q_SLOTS:
    void slotSliderChanged(qreal value);

private:
    KisDoubleSliderSpinBox *m_slider;
};

class KisUniformPaintOpPropertyCheckBox : public KisUniformPaintOpPropertyWidget
{
    Q_OBJECT
public:
    KisUniformPaintOpPropertyCheckBox(KisUniformPaintOpPropertySP property, QWidget *parent);

    void setValue(const QVariant &value) override;

private Q_SLOTS:
    void slotCheckBoxChanged(bool value);

private:
    QCheckBox *m_checkBox;
};

class QComboBox;

class KisUniformPaintOpPropertyComboBox : public KisUniformPaintOpPropertyWidget
{
    Q_OBJECT
public:
    KisUniformPaintOpPropertyComboBox(KisUniformPaintOpPropertySP property, QWidget *parent);

    void setValue(const QVariant &value) override;

private Q_SLOTS:
    void slotComboBoxChanged(int value);

private:
    QComboBox *m_comboBox;
};

#endif /* __KIS_UNIFORM_PAINTOP_PROPERTY_WIDGET_H */
