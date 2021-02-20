;
;  SPDX-License-Identifier: GPL-3.0-or-later
;

!define CURRENT_LANG ${LANG_SIMPCHINESE}

# Strings to show in the installation log:
LangString RemovingShellEx ${CURRENT_LANG} "正在删除 Krita 文件资源管理器插件..."
LangString RemoveShellExFailed ${CURRENT_LANG} "无法删除 Krita 文件资源管理器插件。"
LangString RemoveShellExDone ${CURRENT_LANG} "成功删除 Krita 文件资源管理器插件。"
LangString RemovingOldVer ${CURRENT_LANG} "正在卸载旧版软件..."
LangString RemoveOldVerFailed ${CURRENT_LANG} "无法卸载旧版 Krita 软件。"
LangString RemoveOldVerDone ${CURRENT_LANG} "成功卸载旧版 Krita 软件。"

# Strings for the component selection dialog:
LangString SectionRemoveOldVer ${CURRENT_LANG} "卸载旧版软件"
LangString SectionRemoveOldVerDesc ${CURRENT_LANG} "卸载之前安装的 Krita $KritaNsisVersion ($KritaNsisBitness-bit)."
LangString SectionShellEx ${CURRENT_LANG} "文件资源管理器插件"
LangString SectionShellExDesc ${CURRENT_LANG} "安装此插件后，Windows 文件资源管理器即可显示 Krita 文件的缩略图和属性信息。$\r$\n$\r$\n版本: ${KRITASHELLEX_VERSION}"
LangString SectionMainDesc ${CURRENT_LANG} "${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}$\r$\n$\r$\n版本: ${KRITA_VERSION}"
# We don't actually bundle FFmpeg so these are not shown.
LangString SectionBundledFfmpeg ${CURRENT_LANG} "FFmpeg 软件包"
LangString SectionBundledFfmpegDesc ${CURRENT_LANG} "安装本安装程序自带的 FFmpeg 软件包，用于导出动画。"

# Main dialog strings:
LangString SetupLangPrompt ${CURRENT_LANG} "请选择安装程序显示的语言:"
LangString ShellExLicensePageHeader ${CURRENT_LANG} "许可证协议 (Krita 文件资源管理器插件)"
LangString ConfirmInstallPageHeader ${CURRENT_LANG} "确认安装"
LangString ConfirmInstallPageDesc ${CURRENT_LANG} "确认安装 ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}。"
LangString DesktopIconPageDesc2 ${CURRENT_LANG} "选择是否在桌面上创建启动 Krita 的图标快捷方式:"
LangString DesktopIconPageCheckbox ${CURRENT_LANG} "创建桌面图标快捷方式"
LangString ConfirmInstallPageDesc2 ${CURRENT_LANG} "安装程序已经准备就绪，即将安装 ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}。在继续前可返回检查安装选项，确保一切无误。$\r$\n$\r$\n$_CLICK"

# Misc. message prompts:
LangString MsgRequireWin7 ${CURRENT_LANG} "${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY} 只支持 Windows 7 以及更高版本。"
LangString Msg64bitOn32bit ${CURRENT_LANG} "本机正在运行 32 位版本的 Windows，但本安装程序提供的是 64 位版本的 Krita，它只能在 64 位版本的 Windows 下运行。请前往 https://krita.org/ 网站，下载 32 位版本的 Krita 安装程序。"
LangString Msg32bitOn64bit ${CURRENT_LANG} "你正在 64 位版本的 Windows 下面安装 32 位版本的 Krita，这将降低程序性能。我们强烈建议你改而安装 64 位版本的 Krita。$\n请前往 https://krita.org/ 网站，下载 64 位版本的 Krita。$\n$\n仍要继续安装 32 位版本的 Krita 吗？"
# These prompts are used for when Krita 2.9 or earlier, or the 3.0 alpha 1 MSI version is installed.
LangString MsgAncientVerMustBeRemoved ${CURRENT_LANG} "检测到本机已经安装了旧版本的 Krita。本安装程序将在安装前卸载旧版软件。$\n确定要继续吗？"
LangString MsgKrita3alpha1RemoveFailed ${CURRENT_LANG} "无法卸载 Krita 3.0 Alpha 1。"
LangString MsgKrita2msi32bitRemoveFailed ${CURRENT_LANG} "无法卸载旧版 Krita (32 位)。"
LangString MsgKrita2msi64bitRemoveFailed ${CURRENT_LANG} "无法卸载旧版 Krita (64 位)。"
#
LangString MsgKritaSameVerReinstall ${CURRENT_LANG} "检测到本机已经安装了 ${KRITA_PRODUCTNAME} ${KRITA_VERSION_DISPLAY}。$\n本安装程序将重新安装此版本的 Krita。"
LangString MsgKrita3264bitSwap ${CURRENT_LANG} "检测到本机已经安装了$KritaNsisBitness位版本的 Krita ($KritaNsisVersion)。本安装程序将用${KRITA_INSTALLER_BITNESS}位版本的 Krita ${KRITA_VERSION_DISPLAY} 将其覆盖。"
LangString MsgKritaNewerAlreadyInstalled ${CURRENT_LANG} "检测到本机已经安装了$KritaNsisBitness位版本的 Krita ($KritaNsisVersion)。如需安装旧版的 Krita (${KRITA_VERSION_DISPLAY})，请先手动卸载已有版本，然后在此运行此安装程序。"
LangString MsgKritaRunning ${CURRENT_LANG} "检测到 Krita 正在运行。请在关闭 Krita 后再次运行此安装程序。"
LangString MsgUninstallKritaRunning ${CURRENT_LANG} "检测到 Krita 正在运行。请在关闭 Krita 后再次运行此卸载程序。"

!undef CURRENT_LANG
