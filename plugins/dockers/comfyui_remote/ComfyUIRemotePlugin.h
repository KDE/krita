/*
 * SPDX-FileCopyrightText: 2025 Krita Project
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMFYUI_REMOTE_PLUGIN_H_
#define COMFYUI_REMOTE_PLUGIN_H_

#include <QObject>
#include <QVariant>

class ComfyUIRemotePlugin : public QObject
{
    Q_OBJECT
public:
    ComfyUIRemotePlugin(QObject *parent, const QVariantList &);
    ~ComfyUIRemotePlugin() override;
};

#endif
