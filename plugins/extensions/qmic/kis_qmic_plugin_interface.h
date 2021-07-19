/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_GMIC_PLUGIN_INTERFACE
#define KRITA_GMIC_PLUGIN_INTERFACE

#include <memory>
#include <QObject>

#include "kis_qmic_interface.h"
#include "kritaqmicinterface_export.h"

#define KRITA_GMIC_PLUGIN_INTERFACE_IID "org.kde.krita.KritaGmicPluginInterface"

class KRITAQMICINTERFACE_EXPORT KisQmicPluginInterface
{
public:
  KisQmicPluginInterface();
  virtual ~KisQmicPluginInterface();
  virtual int launch(std::shared_ptr<KisImageInterface> iface, bool headless = false) = 0;
};

Q_DECLARE_INTERFACE(KisQmicPluginInterface, KRITA_GMIC_PLUGIN_INTERFACE_IID)

#endif
