/*
 *  SPDX-FileCopyrightText: 2018 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KRITADESIGNERPLUGINCOLLECTION_H_
#define _KRITADESIGNERPLUGINCOLLECTION_H_

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>


class KritaDesignerPluginCollection : public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    KritaDesignerPluginCollection(QObject *parent = nullptr);

    QList<QDesignerCustomWidgetInterface*> customWidgets() const override;

private:
    QList<QDesignerCustomWidgetInterface*> m_widgets;
};

#endif // _KRITADESIGNERPLUGINCOLLECTION_H_
