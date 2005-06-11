KDE_OPTIONS = nofinal

INCLUDES = -I$(srcdir)/../core/filters -I$(srcdir)/../core \
	-I$(srcdir)/../core/resources/ -I$(srcdir)/../core/color_strategy -I$(srcdir)/../core/tiles \
	-I$(srcdir)/../core/tool -I$(srcdir)/../core/paintop $(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) \
	$(all_includes)

noinst_LTLIBRARIES = libkisui.la
libkisui_la_LIBADD = ./labels/libkislabel.la ./widgets/libkiswidgets.la
libkisui_la_SOURCES = \
    kis_cmb_idlist.cc \
    kis_dlg_preferences.cc \
    kis_dlg_progress.cc \
    kis_canvas.cc \
    kis_cmb_composite.cc \
    kis_layerbox.cc \
    kis_dlg_create_img.cc \
    kis_dlg_image_properties.cc \
    kis_dlg_new_layer.cc \
    kis_dlg_paint_properties.cc \
    kis_dlg_transform.cc \
    kis_gray_widget.cc \
    kis_icon_item.cc \
    kis_ruler.cc \
    kis_iconwidget.cc \
    kis_itemchooser.cc \
    kis_previewwidget.cc \
    kis_rgb_widget.cc \
    kis_hsv_widget.cc \
    kis_colorwheel.cc \
    kis_progress_subject.cc \
    kis_double_widget.cc \
    kis_dlg_apply_profile.cc \
    kis_previewview.cc \
    kis_previewdialog.cc \
    kis_selection_options.cc \
    kis_guide.cc \
    wdgmatrix.ui \
    kis_previewwidgetbase.ui \
    wdgselectionoptions.ui \
    wdgshapeoptions.ui \
    wdgnewimage.ui \
    wdgcolorsettings.ui \
    wdgapplyprofile.ui \
    wdglayerbox.ui


noinst_HEADERS = kis_dlg_preferences.h \
    kis_dlg_progress.h \
    kis_canvas.h \
    kis_layerbox.h \
    kis_dlg_create_img.h \
    kis_dlg_new_layer.h \
    kis_dlg_paint_properties.h \
    kis_dlg_image_properties.h \
    kis_dlg_transform.h \
    kis_gray_widget.h \
    kis_icon_item.h \
    kis_ruler.h \
    kis_iconwidget.h \
    kis_itemchooser.h \
    kis_previewwidget.h \
    kis_rgb_widget.h \
    kis_hsv_widget.h \
    kis_colorwheel.h \
    kis_progress_subject.h \
    kis_double_widget.h \
    kis_dlg_apply_profile.h \
    kis_guide.h \
    wdgmatrix.h \
    kis_previewwidgetbase.h \
    kis_previewdialog.h \
    wdgnewimage.h \
    wdgcolorsettings.h \
    wdgapplyprofile.h \
    wdglayerbox.h 


libkisui_la_METASOURCES = AUTO
SUBDIRS = labels widgets


