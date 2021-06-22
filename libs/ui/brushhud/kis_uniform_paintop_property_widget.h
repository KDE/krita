/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    void slotThemeChanged(QPalette pal);

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
    QWidget *m_slider;
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
    QWidget *m_slider;
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
