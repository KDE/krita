#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include "wetpix.h"
#include "wetpaint.h"
#include "wettexture.h"
#include "wetphysics.h"
WetPack *pack;			/* The global wet pack */

double lastx, lasty;
double dist;
double spacing = 2;

WetPix paint = { 707, 0, 707, 0, 707, 0, 240, 0 };

/* colors from Curtis et al, Siggraph 97 */

WetPix paintbox[] = {
	{496, 0, 16992, 0, 3808, 0, 0, 0},
	{16992, 9744, 21712, 6400, 25024, 3296, 0, 0},
	{6512, 6512, 6512, 4880, 11312, 0, 0, 0},
	{16002, 0, 2848, 0, 16992, 0, 0, 0},
	{22672, 0, 5328, 2272, 4288, 2640, 0, 0},
	{8000, 0, 16992, 0, 28352, 0, 0, 0},
	{5696, 5696, 12416, 2496, 28352, 0, 0, 0},
	{0, 0, 5136, 0, 28352, 0, 0, 0},
	{2320, 1760, 7344, 4656, 28352, 0, 0, 0},
	{8000, 0, 3312, 0, 5504, 0, 0, 0},
	{13680, 0, 16992, 0, 3312, 0, 0, 0},
	{5264, 5136, 1056, 544, 6448, 6304, 0, 0},
	{11440, 11440, 11440, 11440, 11440, 11440, 0, 0},
	{11312, 0, 11312, 0, 11312, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}
};

int n_paints = 15;

char *paintstr = "select paint";

GtkWidget *paintname;

char *paintnames[] = {
	"Quinacridone Rose",
	"Indian Red",
	"Cadmium Yellow",
	"Hookers Green",
	"Cerulean Blue",
	"Burnt Umber",
	"Cadmium Red",
	"Brilliant Orange",
	"Hansa Yellow",
	"Phthalo Green",
	"French Ultramarine",
	"Interference Lilac",
	"Titanium White",
	"Ivory Black",
	"Pure Water"
};

GtkWidget *autodryb;
int timo = 0;
int adsorb_cnt;

GtkObject *brushsize_adjust;

GtkObject *wetness_adjust;

GtkObject *strength_adjust;

static void stop_drying(void)
{
	timo = 0;
	gtk_label_set_text(GTK_LABEL(paintname), paintstr);
}

static double strength_func(double strength, double pressure)
{
	return strength * (strength + pressure) * 0.5;
}

static gint wet_button_press(GtkWidget * widget, GdkEventButton * event)
{
#define noVERBOSE
#ifdef VERBOSE
	g_print("button press %f %f %f\n", event->x, event->y,
		event->pressure);

#endif
	wet_dab(pack->layers[1],
		&paint,
		event->x,
		event->y,
		((GtkAdjustment *) brushsize_adjust)->value *
		event->pressure, 0.75 + 0.25 * event->pressure,
		strength_func(((GtkAdjustment *) strength_adjust)->value,
			      event->pressure));

	lastx = event->x;
	lasty = event->y;
	dist = 0;

	stop_drying();

	gtk_widget_queue_draw(widget);
	return TRUE;
}

static gint wet_motion(GtkWidget * widget, GdkEventMotion * event)
{
	double delta;
#ifdef VERBOSE
	g_print("motion %f %f %f %d\n", event->x, event->y,
		event->pressure, event->state);

#endif
	stop_drying();

	if (!(event->state & 256))
		return TRUE;

	delta = sqrt((event->x - lastx) * (event->x - lastx) +
		     (event->y - lasty) * (event->y - lasty));

	dist += delta;

	if (dist >= spacing) {
		/* todo: interpolate position and pressure of the dab */
		wet_dab(pack->layers[1],
			&paint,
			event->x,
			event->y,
			((GtkAdjustment *) brushsize_adjust)->value *
			event->pressure, 0.75 + 0.25 * event->pressure,
			strength_func(((GtkAdjustment *) strength_adjust)->
				      value, event->pressure));
		gtk_widget_queue_draw(widget);
		dist -= spacing;
	}

	lastx = event->x;
	lasty = event->y;

	return TRUE;
}

static void dry(GtkWidget * da)
{
	g_print("drying...");
	gtk_label_set_text(GTK_LABEL(paintname), "drying...");
	gtk_widget_draw(paintname, NULL);
	gdk_flush();
	wet_flow(pack->layers[1]);
	adsorb_cnt++;
	if (adsorb_cnt == 2) {
		wet_adsorb(pack->layers[1], pack->layers[0]);
		wet_dry(pack->layers[1]);
		adsorb_cnt = 0;
	}

	gtk_widget_draw(da, NULL);
#if 0
	gtk_label_set_text(GTK_LABEL(paintname), paintstr);
#endif
	g_print("done\n");
}

static gint wet_dry_button_press(GtkWidget * widget, GtkWidget * da)
{
	dry(da);

	timo = 0;

	return TRUE;
}

static gint clear_button_press(GtkWidget * widget, GtkWidget * da)
{
	wet_layer_clear(pack->layers[0]);
	wet_layer_clone_texture(pack->layers[0], pack->layers[1]);
	wet_layer_clear(pack->layers[1]);
	wet_layer_clone_texture(pack->layers[1], pack->layers[0]);

	gtk_widget_draw(da, NULL);

	stop_drying();

	return TRUE;
}

static gint dry_timer(gpointer * dummy)
{
	GtkWidget *da = (GtkWidget *) dummy;

	timo++;
	if (timo >= 10) {
		if (gtk_toggle_button_get_active
		    (GTK_TOGGLE_BUTTON(autodryb))) {
			dry(da);
		}

		timo -= 2;
	}
	return TRUE;
}

static gint
wet_expose(GtkWidget * widget, GdkEventExpose * event, WetPack * pack)
{
	byte *rgb;
	int rowstride;

#ifdef VERBOSE
	g_print("expose: %d layers\n", pack->n_layers);
#endif

	rowstride = event->area.width * 3;
	rowstride = (rowstride + 3) & -4;	/* align to 4-byte boundary */
	rgb = g_new(byte, event->area.height * rowstride);

	wet_pack_render(rgb, rowstride,
			pack,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	gdk_draw_rgb_image(widget->window,
			   widget->style->black_gc,
			   event->area.x, event->area.y,
			   event->area.width, event->area.height,
			   GDK_RGB_DITHER_MAX, rgb, rowstride);

	g_free(rgb);
	return FALSE;
}


static void init_input(void)
{
	GList *tmp_list;
	GdkDeviceInfo *info;

	tmp_list = gdk_input_list_devices();

	info = NULL;
	while (tmp_list) {
		info = (GdkDeviceInfo *) tmp_list->data;
#ifdef VERBOSE
		g_print("device: %s\n", info->name);
#endif
		if (!g_strcasecmp(info->name, "wacom") ||
		    !g_strcasecmp(info->name, "stylus") ||
		    !g_strcasecmp(info->name, "eraser")) {
			gdk_input_set_mode(info->deviceid,
					   GDK_MODE_SCREEN);
		}
		tmp_list = tmp_list->next;
	}
	if (!info)
		return;
}

static gint
pselect_expose(GtkWidget * widget, GdkEventExpose * event, WetPack * pack)
{
	byte *rgb;
	int x;
	int paint_quad, paint_num;
	int last_pn;
	int bg;

#ifdef VERBOSE
	g_print("expose: %d layers\n", pack->n_layers);
#endif

	rgb = g_new(byte, pack->layers[0]->width * 3);

	last_pn = 0;
	for (x = 0; x < pack->layers[0]->width; x++) {
		paint_quad =
		    floor(4 * x * n_paints / pack->layers[0]->width + 0.5);
		paint_num = paint_quad >> 2;
		if (last_pn != paint_num) {
			rgb[x * 3] = 255;
			rgb[x * 3 + 1] = 255;
			rgb[x * 3 + 2] = 255;
			last_pn = paint_num;
		} else {
			if ((paint_quad & 3) > 0 && (paint_quad & 3) < 3)
				bg = 0;
			else
				bg = 255;
			rgb[x * 3] = bg;
			rgb[x * 3 + 1] = bg;
			rgb[x * 3 + 2] = bg;
			wet_composite(&rgb[x * 3], 0, &paintbox[paint_num],
				      0, 1, 1);
		}
	}

	gdk_draw_rgb_image(widget->window,
			   widget->style->black_gc,
			   event->area.x, event->area.y,
			   event->area.width, event->area.height,
			   GDK_RGB_DITHER_MAX,
			   rgb + (event->area.x) * 3, 0);

	g_free(rgb);
	return FALSE;
}

static gint
pselect_button_press(GtkWidget * widget, GdkEventButton * event)
{
	int paint_num;
	int wet;

#ifdef VERBOSE
	g_print("pselect button press %f %f %f\n", event->x, event->y,
		event->pressure);

#endif
	paint_num = floor((event->x * n_paints) / pack->layers[0]->width);

	/* preserve wetness */
	wet = paint.w;
	paint = paintbox[paint_num];
	paint.w = wet;
	paintstr = paintnames[paint_num];
	/*
	   gtk_adjustment_set_value (GTK_ADJUSTMENT (wetness_adjust), paint.w);
	 */
	gtk_label_set_text(GTK_LABEL(paintname), paintstr);

	stop_drying();

	return TRUE;
}

static void wetness_update(GtkAdjustment * adj, gpointer data)
{
	paint.w = floor(15 * adj->value + 0.5);
}

int main(int argc, char **argv)
{
	GtkWidget *w;
	GtkWidget *v;
	GtkWidget *eb;
	GtkWidget *da;
	GtkWidget *peb;
	GtkWidget *pda;
	GtkWidget *h;
	GtkWidget *b;
	GtkWidget *db;
	GtkWidget *h2;
	GtkWidget *l;
	GtkWidget *brushsize;
	GtkWidget *wetness;
	GtkWidget *strength;
	int xs = 512;
	int ys = 512;

	gtk_init(&argc, &argv);

	if (argc >= 3) {
		xs = atoi(argv[1]);
		ys = atoi(argv[2]);
		if (xs == 0)
			xs = 512;
		if (ys == 0)
			ys = 512;
	}


	init_input();

	gdk_rgb_init();

	gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
	gtk_widget_set_default_visual(gdk_rgb_get_visual());

	pack = wet_pack_new(xs, ys);

	wet_pack_maketexture(pack, 1, 0.7, 0.5);

	w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect(GTK_OBJECT(w), "destroy",
			   (GtkSignalFunc) gtk_main_quit, NULL);

	v = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(w), v);
	gtk_widget_show(v);

	eb = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(v), eb);
	gtk_widget_show(eb);

	gtk_widget_set_extension_events(eb, GDK_EXTENSION_EVENTS_ALL);

	gtk_widget_set_events(eb, GDK_EXPOSURE_MASK
			      | GDK_LEAVE_NOTIFY_MASK
			      | GDK_BUTTON_PRESS_MASK
			      | GDK_KEY_PRESS_MASK
			      | GDK_POINTER_MOTION_MASK
			      | GDK_PROXIMITY_OUT_MASK);

	gtk_signal_connect(GTK_OBJECT(eb), "button_press_event",
			   (GtkSignalFunc) wet_button_press, NULL);
	gtk_signal_connect(GTK_OBJECT(eb), "motion_notify_event",
			   (GtkSignalFunc) wet_motion, NULL);

	da = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(da), xs, ys);
	gtk_container_add(GTK_CONTAINER(eb), da);
	gtk_widget_show(da);

	gtk_signal_connect(GTK_OBJECT(da), "expose_event",
			   (GtkSignalFunc) wet_expose, pack);

	peb = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(v), peb);
	gtk_widget_show(peb);

	gtk_widget_set_extension_events(peb, GDK_EXTENSION_EVENTS_ALL);

	gtk_widget_set_events(peb, GDK_EXPOSURE_MASK
			      | GDK_LEAVE_NOTIFY_MASK
			      | GDK_BUTTON_PRESS_MASK
			      | GDK_KEY_PRESS_MASK
			      | GDK_PROXIMITY_OUT_MASK);

	gtk_signal_connect(GTK_OBJECT(peb), "button_press_event",
			   (GtkSignalFunc) pselect_button_press, NULL);

	pda = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(pda), xs, 16);
	gtk_container_add(GTK_CONTAINER(peb), pda);
	gtk_widget_show(pda);

	gtk_signal_connect(GTK_OBJECT(pda), "expose_event",
			   (GtkSignalFunc) pselect_expose, pack);

	paintname = gtk_label_new(paintstr);
	gtk_container_add(GTK_CONTAINER(v), paintname);
	gtk_widget_show(paintname);

	h = gtk_hbox_new(TRUE, 5);
	gtk_container_add(GTK_CONTAINER(v), h);
	gtk_widget_show(h);

	b = gtk_button_new_with_label("Dry");
	gtk_container_add(GTK_CONTAINER(h), b);
	gtk_widget_show(b);

	gtk_signal_connect(GTK_OBJECT(b), "clicked",
			   (GtkSignalFunc) wet_dry_button_press, da);

	autodryb = gtk_toggle_button_new_with_label("Auto Dry");
	gtk_container_add(GTK_CONTAINER(h), autodryb);
	gtk_widget_show(autodryb);

	db = gtk_button_new_with_label("Clear");
	gtk_container_add(GTK_CONTAINER(h), db);
	gtk_widget_show(db);

	gtk_signal_connect(GTK_OBJECT(db), "clicked",
			   (GtkSignalFunc) clear_button_press, da);

	h2 = gtk_hbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(v), h2);
	gtk_widget_show(h2);

	l = gtk_label_new("Brush size: ");
	gtk_container_add(GTK_CONTAINER(h2), l);
	gtk_widget_show(l);

	brushsize_adjust = gtk_adjustment_new(10, 0, 32, 0.1, 0.1, 0);
	brushsize = gtk_hscale_new(GTK_ADJUSTMENT(brushsize_adjust));
	gtk_container_add(GTK_CONTAINER(h2), brushsize);
	gtk_widget_show(brushsize);

	h2 = gtk_hbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(v), h2);
	gtk_widget_show(h2);

	l = gtk_label_new("Wetness: ");
	gtk_container_add(GTK_CONTAINER(h2), l);
	gtk_widget_show(l);

	wetness_adjust = gtk_adjustment_new(16, 0, 16, 1.0, 1.0, 0);
	wetness = gtk_hscale_new(GTK_ADJUSTMENT(wetness_adjust));
	gtk_container_add(GTK_CONTAINER(h2), wetness);
	gtk_widget_show(wetness);
	gtk_signal_connect(GTK_OBJECT(wetness_adjust), "value_changed",
			   (GtkSignalFunc) wetness_update, NULL);

	h2 = gtk_hbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(v), h2);
	gtk_widget_show(h2);

	l = gtk_label_new("Strength: ");
	gtk_container_add(GTK_CONTAINER(h2), l);
	gtk_widget_show(l);

	strength_adjust = gtk_adjustment_new(1, 0, 2, 0.1, 0.1, 0);
	strength = gtk_hscale_new(GTK_ADJUSTMENT(strength_adjust));
	gtk_scale_set_digits(GTK_SCALE(strength), 2);
	gtk_container_add(GTK_CONTAINER(h2), strength);
	gtk_widget_show(strength);

	gtk_widget_show(w);

	gtk_timeout_add(50, (GtkFunction) dry_timer, da);

	gtk_main();

	return 0;
}
