/*
 *  SPDX-FileCopyrightText: 2018 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KISGRADIENTSLIDERPLUGIN_H_
#define _KISGRADIENTSLIDERPLUGIN_H_

#include <QObject>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>

class KisGradientSliderPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    explicit KisGradientSliderPlugin(QObject *parent = nullptr);

    bool isContainer() const override;
    bool isInitialized() const override;
    QIcon icon() const override;
    QString domXml() const override;
    QString group() const override;
    QString includeFile() const override;
    QString name() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QWidget *createWidget(QWidget *parent) override;
    void initialize(QDesignerFormEditorInterface *core) override;

private:
    bool m_initialized;
};

#endif // _KISGRADIENTSLIDERPLUGIN_H_
