/* -*- Mode: C; c-basic-offset: 4 -*-
 * geyes.c - A cheap xeyes ripoff.
 * Copyright (C) 1999 Dave Camp
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config.h>
#include <math.h>
#include <stdlib.h>
#include <mate-panel-applet.h>
#include <mate-panel-applet-gsettings.h>
#include "geyes.h"

#ifdef HAVE_WAYLAND
#include <gdk/gdkwayland.h>
#include <wayland-client.h>
#include "marco-pointer-position-v1-client.h"

static struct marco_pointer_position_manager_v1 *pointer_manager = NULL;
static struct marco_pointer_position_v1 *pointer_position = NULL;
static int wayland_x = 0;
static int wayland_y = 0;
static gboolean has_wayland_pointer = FALSE;
static gboolean wayland_pointer_initialized = FALSE;

static void
pointer_position_handle_motion (void *data,
                                struct marco_pointer_position_v1 *pp,
                                int32_t x,
                                int32_t y)
{
    wayland_x = x;
    wayland_y = y;
    has_wayland_pointer = TRUE;
}

static const struct marco_pointer_position_v1_listener pointer_position_listener = {
    .motion = pointer_position_handle_motion,
};

static void
registry_handle_global (void *data,
                        struct wl_registry *registry,
                        uint32_t id,
                        const char *interface,
                        uint32_t version)
{
    if (strcmp (interface, "marco_pointer_position_manager_v1") == 0) {
        pointer_manager = wl_registry_bind (registry, id, &marco_pointer_position_manager_v1_interface, 1);
        if (pointer_manager) {
            pointer_position = marco_pointer_position_manager_v1_get_pointer_position (pointer_manager);
            if (pointer_position) {
                marco_pointer_position_v1_add_listener (pointer_position, &pointer_position_listener, NULL);
            }
        }
    }
}

static void
registry_handle_global_remove (void *data,
                               struct wl_registry *registry,
                               uint32_t id)
{
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void
setup_wayland_pointer (void)
{
    if (wayland_pointer_initialized)
        return;
    wayland_pointer_initialized = TRUE;

    GdkDisplay *display = gdk_display_get_default ();
    if (GDK_IS_WAYLAND_DISPLAY (display)) {
        struct wl_display *wl_disp = gdk_wayland_display_get_wl_display (display);
        if (wl_disp) {
            struct wl_registry *registry = wl_display_get_registry (wl_disp);
            if (registry) {
                wl_registry_add_listener (registry, &registry_listener, NULL);
                wl_display_roundtrip (wl_disp);
            }
        }
    }
}
#endif

#define UPDATE_TIMEOUT 100

static gfloat
gtk_align_to_gfloat (GtkAlign align)
{
    switch (align) {
        case GTK_ALIGN_START:
            return 0.0f;
        case GTK_ALIGN_END:
            return 1.0f;
        case GTK_ALIGN_CENTER:
        case GTK_ALIGN_FILL:
            return 0.5f;
        default:
            return 0.0f;
    }
}

/* TODO - Optimize this a bit */
static void
calculate_pupil_xy (EyesApplet *eyes_applet,
                    gint        x,
                    gint        y,
                    gint       *pupil_x,
                    gint       *pupil_y,
                    GtkWidget  *widget)
{
    GtkAllocation allocation;
    float sina;
    float cosa;
    float h;
    float temp;
    float nx, ny;
    float xalign, yalign;
    float eye_width_half, eye_height_half;

    gtk_widget_get_allocation (GTK_WIDGET (widget), &allocation);
    xalign = gtk_align_to_gfloat (gtk_widget_get_halign (widget));
    yalign = gtk_align_to_gfloat (gtk_widget_get_valign (widget));
    eye_width_half = (float) eyes_applet->eye_width * 0.5f;
    eye_height_half = (float) eyes_applet->eye_height * 0.5f;

    nx = (float) x - (float) MAX (allocation.width - eyes_applet->eye_width, 0) * xalign
         - eye_width_half;
    ny = (float) y - (float) MAX (allocation.height - eyes_applet->eye_height, 0) * yalign
         - eye_height_half;

    h = hypotf (nx, ny);
    if ((h < 0.5f) ||
        (fabs (h) < (fabs (hypotf (eye_height_half, eye_width_half)
                           - (float) eyes_applet->wall_thickness
                           - (float) eyes_applet->pupil_height)))) {
        *pupil_x = (gint)(nx + eye_width_half);
        *pupil_y = (gint)(ny + eye_height_half);
        return;
    }

    sina = nx / h;
    cosa = ny / h;

    temp  = hypotf ((float) eyes_applet->eye_width * sina,
                    (float) eyes_applet->eye_height * cosa);
    temp -= hypotf ((float) eyes_applet->pupil_width * sina,
                    (float) eyes_applet->pupil_height * cosa);
    temp -= (float) eyes_applet->wall_thickness * hypotf (sina, cosa);
    temp *= 0.5f;

    *pupil_x = (gint)(temp * sina + eye_width_half);
    *pupil_y = (gint)(temp * cosa + eye_height_half);
}

static void
draw_eye (EyesApplet *eyes_applet,
          gsize       eye_num,
          gint        pupil_x,
          gint        pupil_y)
{
    GdkPixbuf *pixbuf;
    GdkRectangle rect, r1, r2;

    pixbuf = gdk_pixbuf_copy (eyes_applet->eye_image);
    r1.x = pupil_x - eyes_applet->pupil_width / 2;
    r1.y = pupil_y - eyes_applet->pupil_height / 2;
    r1.width = eyes_applet->pupil_width;
    r1.height = eyes_applet->pupil_height;
    r2.x = 0;
    r2.y = 0;
    r2.width = eyes_applet->eye_width;
    r2.height = eyes_applet->eye_height;
    gdk_rectangle_intersect (&r1, &r2, &rect);
    gdk_pixbuf_composite (eyes_applet->pupil_image, pixbuf,
                          rect.x, rect.y,
                          rect.width, rect.height,
                          pupil_x - eyes_applet->pupil_width / 2,
                          pupil_y - eyes_applet->pupil_height / 2,
                          1.0, 1.0,
                          GDK_INTERP_BILINEAR,
                          255);
    gtk_image_set_from_pixbuf (GTK_IMAGE (eyes_applet->eyes[eye_num]), pixbuf);
    g_object_unref (pixbuf);

}

#ifdef HAVE_WAYLAND
static void
calibrate_offsets (EyesApplet *eyes_applet, GtkWidget *event_widget, double x, double y)
{
    if (has_wayland_pointer) {
        gsize i;
        for (i = 0; i < eyes_applet->num_eyes; i++) {
            gint dx = 0, dy = 0;
            if (gtk_widget_translate_coordinates (event_widget, eyes_applet->eyes[i], (gint)x, (gint)y, &dx, &dy)) {
                eyes_applet->wayland_offset_x[i] = wayland_x - dx;
                eyes_applet->wayland_offset_y[i] = wayland_y - dy;
                eyes_applet->wayland_offset_calibrated[i] = TRUE;
            }
        }
    }
}

static gboolean
applet_motion_notify_cb (GtkWidget      *widget,
                         GdkEventMotion *event,
                         EyesApplet     *eyes_applet)
{
    GtkWidget *event_widget = gtk_get_event_widget ((GdkEvent *)event);
    if (!event_widget) {
        event_widget = widget;
    }
    calibrate_offsets (eyes_applet, event_widget, event->x, event->y);
    return FALSE;
}

#endif

static gint
timer_cb (EyesApplet *eyes_applet)
{
    GdkDisplay *display;
    GdkSeat *seat;
    GdkDevice *pointer_device;
    gint x, y;
    gint pupil_x, pupil_y;
    gsize i;
#ifdef ENABLE_IN_PROCESS
    GtkAllocation allocation;
#else
    int applet_x,applet_y;
    gint dx, dy;
#endif

    display = gtk_widget_get_display (GTK_WIDGET (eyes_applet->applet));
    seat = gdk_display_get_default_seat (display);
    if (seat == NULL)
        return TRUE;

    pointer_device = gdk_seat_get_pointer (seat);
    if (pointer_device == NULL)
        return TRUE;

#ifdef HAVE_WAYLAND
    if (has_wayland_pointer && eyes_applet->motion_handler_id == 0) {
        GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (eyes_applet->applet));
        if (toplevel && GTK_IS_WINDOW (toplevel) && gtk_widget_get_realized (toplevel)) {
            gtk_widget_add_events (toplevel, GDK_POINTER_MOTION_MASK);
            eyes_applet->motion_toplevel = toplevel;
            eyes_applet->motion_handler_id =
                g_signal_connect (toplevel, "motion-notify-event",
                                  G_CALLBACK (applet_motion_notify_cb), eyes_applet);
        }
    }
#endif

    for (i = 0; i < eyes_applet->num_eyes; i++)
    {
        if (gtk_widget_get_realized (eyes_applet->eyes[i]))
        {
#ifdef HAVE_WAYLAND
            if (has_wayland_pointer)
            {
                gint gdk_x, gdk_y;
                if (gdk_window_get_device_position (gtk_widget_get_window (eyes_applet->eyes[i]),
                                                    pointer_device,
                                                    &gdk_x, &gdk_y, NULL) != NULL)
                {
#ifdef ENABLE_IN_PROCESS
                    gtk_widget_get_allocation(GTK_WIDGET(eyes_applet->eyes[i]), &allocation);
                    gdk_x -= i * allocation.width;
#else
                    gtk_widget_translate_coordinates (eyes_applet->eyes[i],
                                                      gtk_widget_get_toplevel(eyes_applet->eyes[i]),
                                                      0, 0, &dx, &dy);
                    gdk_x -= dx;
                    gdk_y -= dy;
#endif
                    eyes_applet->wayland_offset_x[i] = wayland_x - gdk_x;
                    eyes_applet->wayland_offset_y[i] = wayland_y - gdk_y;
                    eyes_applet->wayland_offset_calibrated[i] = TRUE;
                    x = gdk_x;
                    y = gdk_y;
                }
                else if (eyes_applet->wayland_offset_calibrated[i])
                {
                    x = wayland_x - eyes_applet->wayland_offset_x[i];
                    y = wayland_y - eyes_applet->wayland_offset_y[i];
                }
                else
                {
                    x = eyes_applet->eye_width / 2;
                    y = eyes_applet->eye_height / 2;
                }
            }
            else
            {
#endif
                gdk_window_get_device_position (gtk_widget_get_window (eyes_applet->eyes[i]),
                                                pointer_device,
                                                &x, &y, NULL);

                /*correct for the positon of each eye, this is done differently in-process or out*/
#ifdef ENABLE_IN_PROCESS
                gtk_widget_get_allocation(GTK_WIDGET(eyes_applet->eyes[i]), &allocation);
                x -= i * allocation.width;
#else
                gtk_widget_translate_coordinates (eyes_applet->eyes[i],
                                                  gtk_widget_get_toplevel(eyes_applet->eyes[i]),
                                                  0, 0, &dx, &dy);
                x -= dx;
                y -= dy;
#endif
#ifdef HAVE_WAYLAND
            }
#endif

            if ((x != eyes_applet->pointer_last_x[i]) ||
                (y != eyes_applet->pointer_last_y[i])) {

                calculate_pupil_xy (eyes_applet, x, y, &pupil_x, &pupil_y,
                                    eyes_applet->eyes[i]);
                draw_eye (eyes_applet, i, pupil_x, pupil_y);

                eyes_applet->pointer_last_x[i] = x;
                eyes_applet->pointer_last_y[i] = y;
            }
        }
    }
    return TRUE;
}

static void
about_cb (GtkAction  *action,
          EyesApplet *eyes_applet)
{
    static const gchar *authors[] = {
        "Dave Camp <campd@oit.edu>",
        NULL
    };

    const gchar *documenters[] = {
        "Arjan Scherpenisse <acscherp@wins.uva.nl>",
        "Telsa Gwynne <hobbit@aloss.ukuu.org.uk>",
        N_("Sun GNOME Documentation Team <gdocteam@sun.com>"),
        N_("MATE Documentation Team"),
        NULL
    };

#ifdef ENABLE_NLS
    const char **p;
    for (p = documenters; *p; ++p)
        *p = _(*p);
#endif

    gtk_show_about_dialog (NULL,
                           "title",              _("About Eyes"),
                           "version",            VERSION,
                           "comments",           _("A goofy set of eyes for the MATE "
                                                   "panel. They follow your mouse."),
                           "copyright",          _("Copyright \xC2\xA9 1999 Dave Camp\n"
                                                   "Copyright \xc2\xa9 2012-2021 MATE developers"),
                           "authors",            authors,
                           "documenters",        documenters,
                           "translator-credits", _("translator-credits"),
                           "logo-icon-name",     "mate-eyes-applet",
                           NULL);
}

static gboolean
properties_load (EyesApplet *eyes_applet)
{
    gchar *theme_path = NULL;
    gboolean result;

    theme_path = g_settings_get_string (eyes_applet->settings,
                                        GEYES_SETTINGS_THEME_PATH_KEY);

    if (theme_path == NULL)
        theme_path = g_strdup (GEYES_THEMES_DIR "Default-tiny");

    result = load_theme (eyes_applet, theme_path);
    g_free (theme_path);

    return result;
}

void
setup_eyes (EyesApplet *eyes_applet)
{
    gsize i;

    eyes_applet->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (eyes_applet->vbox), eyes_applet->hbox, TRUE,
                        TRUE, 0);

    eyes_applet->eyes = g_new0 (GtkWidget *, eyes_applet->num_eyes);
    eyes_applet->pointer_last_x = g_new0 (gint, eyes_applet->num_eyes);
    eyes_applet->pointer_last_y = g_new0 (gint, eyes_applet->num_eyes);
#ifdef HAVE_WAYLAND
    eyes_applet->wayland_offset_x = g_new0 (gint, eyes_applet->num_eyes);
    eyes_applet->wayland_offset_y = g_new0 (gint, eyes_applet->num_eyes);
    eyes_applet->wayland_offset_calibrated = g_new0 (gboolean, eyes_applet->num_eyes);

    for (i = 0; i < eyes_applet->num_eyes; i++) {
        eyes_applet->wayland_offset_calibrated[i] = FALSE;
    }
#endif

    for (i = 0; i < eyes_applet->num_eyes; i++) {
        if ((eyes_applet->eyes[i] = gtk_image_new ()) == NULL)
            g_error ("Error creating geyes\n");

        gtk_widget_set_size_request (GTK_WIDGET (eyes_applet->eyes[i]),
                                     eyes_applet->eye_width,
                                     eyes_applet->eye_height);

        gtk_widget_show (eyes_applet->eyes[i]);

        gtk_box_pack_start (GTK_BOX (eyes_applet->hbox), eyes_applet->eyes[i],
                            TRUE, TRUE, 0);

        if ((eyes_applet->num_eyes != 1) && (i == 0)) {
            gtk_widget_set_halign (eyes_applet->eyes[i], GTK_ALIGN_END);
            gtk_widget_set_valign (eyes_applet->eyes[i], GTK_ALIGN_CENTER);
        } else if ((eyes_applet->num_eyes != 1) &&
                   (i == eyes_applet->num_eyes - 1)) {
            gtk_widget_set_halign (eyes_applet->eyes[i], GTK_ALIGN_START);
            gtk_widget_set_valign (eyes_applet->eyes[i], GTK_ALIGN_CENTER);
        } else {
            gtk_widget_set_halign (eyes_applet->eyes[i], GTK_ALIGN_CENTER);
            gtk_widget_set_valign (eyes_applet->eyes[i], GTK_ALIGN_CENTER);
        }

        eyes_applet->pointer_last_x[i] = G_MAXINT;
        eyes_applet->pointer_last_y[i] = G_MAXINT;

        draw_eye (eyes_applet, i, eyes_applet->eye_width / 2,
                  eyes_applet->eye_height / 2);

    }
    gtk_widget_show (eyes_applet->hbox);
}

void
destroy_eyes (EyesApplet *eyes_applet)
{
    gtk_widget_destroy (eyes_applet->hbox);
    eyes_applet->hbox = NULL;

    g_free (eyes_applet->eyes);
    g_free (eyes_applet->pointer_last_x);
    g_free (eyes_applet->pointer_last_y);
#ifdef HAVE_WAYLAND
    g_free (eyes_applet->wayland_offset_x);
    g_free (eyes_applet->wayland_offset_y);
    g_free (eyes_applet->wayland_offset_calibrated);
#endif
}

static EyesApplet*
create_eyes (MatePanelApplet *applet)
{
    EyesApplet *eyes_applet = g_new0 (EyesApplet, 1);

    eyes_applet->applet = applet;
    eyes_applet->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    eyes_applet->settings =
        mate_panel_applet_settings_new (applet,
                                        GEYES_SETTINGS_SCHEMA);
    gtk_container_add (GTK_CONTAINER (applet), eyes_applet->vbox);
    return eyes_applet;
}

static void
destroy_cb (GObject    *object,
            EyesApplet *eyes_applet)
{
    g_return_if_fail (eyes_applet);

    g_source_remove (eyes_applet->timeout_id);

#ifdef HAVE_WAYLAND
    if (eyes_applet->motion_handler_id != 0) {
        g_signal_handler_disconnect (eyes_applet->motion_toplevel,
                                     eyes_applet->motion_handler_id);
        eyes_applet->motion_handler_id = 0;
        eyes_applet->motion_toplevel = NULL;
    }
#endif

    if (eyes_applet->hbox)
        destroy_eyes (eyes_applet);
    eyes_applet->timeout_id = 0;

    if (eyes_applet->eye_image)
        g_object_unref (eyes_applet->eye_image);
    eyes_applet->eye_image = NULL;

    if (eyes_applet->pupil_image)
        g_object_unref (eyes_applet->pupil_image);
    eyes_applet->pupil_image = NULL;

    g_free (eyes_applet->theme_dir);
    eyes_applet->theme_dir = NULL;

    g_free (eyes_applet->theme_name);
    eyes_applet->theme_name = NULL;

    g_free (eyes_applet->eye_filename);
    eyes_applet->eye_filename = NULL;

    g_free (eyes_applet->pupil_filename);
    eyes_applet->pupil_filename = NULL;

    if (eyes_applet->prop_box.pbox)
        gtk_widget_destroy (eyes_applet->prop_box.pbox);

    if (eyes_applet->settings)
        g_object_unref (eyes_applet->settings);
    eyes_applet->settings = NULL;

    g_free (eyes_applet);
}

static void
help_cb (GtkAction  *action,
         EyesApplet *eyes_applet)
{
    GError *error = NULL;

    gtk_show_uri_on_window (NULL, "help:mate-geyes",
                            gtk_get_current_event_time (),
                            &error);

    if (error) {
        GtkWidget *dialog
            = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_CLOSE,
                                      _("There was an error displaying help: %s"),
                                      error->message);
        g_signal_connect (dialog, "response",
                          G_CALLBACK (gtk_widget_destroy),
                          NULL);
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
        gtk_window_set_screen (GTK_WINDOW (dialog),
                               gtk_widget_get_screen (GTK_WIDGET (eyes_applet->applet)));
        gtk_widget_show (dialog);
        g_clear_error (&error);
    }
}

static const GtkActionEntry geyes_applet_menu_actions[] = {
        { "Props", "document-properties", N_("_Preferences"),
          NULL, NULL, G_CALLBACK (properties_cb) },
        { "Help", "help-browser", N_("_Help"),
          NULL, NULL, G_CALLBACK (help_cb) },
        { "About", "help-about", N_("_About"),
          NULL, NULL, G_CALLBACK (about_cb) }
};

static void
set_atk_name_description (GtkWidget   *widget,
                          const gchar *name,
                          const gchar *description)
{
    AtkObject *aobj;

    aobj = gtk_widget_get_accessible (widget);

    /* Check if gail is loaded */
    if (GTK_IS_ACCESSIBLE (aobj) == FALSE)
        return;

    atk_object_set_name (aobj, name);
    atk_object_set_description (aobj, description);
}

static gboolean
geyes_applet_fill (MatePanelApplet *applet)
{
    EyesApplet *eyes_applet;
    GtkActionGroup *action_group;
    gboolean result;

#ifndef ENABLE_IN_PROCESS
    g_set_application_name (_("Eyes"));
#endif
    gtk_window_set_default_icon_name ("mate-eyes-applet");
    mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_EXPAND_MINOR);

    eyes_applet = create_eyes (applet);

#ifdef HAVE_WAYLAND
    setup_wayland_pointer ();
#endif

    eyes_applet->timeout_id = g_timeout_add (UPDATE_TIMEOUT,
                                             (GSourceFunc) timer_cb,
                                             eyes_applet);

    action_group = gtk_action_group_new ("Geyes Applet Actions");
    gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

    gtk_action_group_add_actions (action_group, geyes_applet_menu_actions,
                                  G_N_ELEMENTS (geyes_applet_menu_actions),
                                  eyes_applet);

    mate_panel_applet_setup_menu_from_resource (eyes_applet->applet,
                                                GEYES_RESOURCE_PATH "geyes-applet-menu.xml",
                                                action_group);

    if (mate_panel_applet_get_locked_down (eyes_applet->applet)) {
        GtkAction *action;

        action = gtk_action_group_get_action (action_group, "Props");
        gtk_action_set_visible (action, FALSE);
    }
    g_object_unref (action_group);

    gtk_widget_set_tooltip_text (GTK_WIDGET (eyes_applet->applet), _("Eyes"));

    set_atk_name_description (GTK_WIDGET (eyes_applet->applet), _("Eyes"),
                              _("The eyes look in the direction of the mouse pointer"));

    g_signal_connect (eyes_applet->vbox, "destroy",
                      G_CALLBACK (destroy_cb),
                      eyes_applet);

    gtk_widget_show_all (GTK_WIDGET (eyes_applet->applet));

    /* setup here and not in create eyes so the destroy signal is set so
     * that when there is an error within loading the theme
     * we can emit this signal */
    if ((result = properties_load (eyes_applet)) == TRUE)
        setup_eyes (eyes_applet);

    return result;
}

static gboolean
geyes_applet_factory (MatePanelApplet *applet,
                      const gchar     *iid,
                      gpointer         data)
{
    gboolean retval = FALSE;

    theme_dirs_create ();

    if (!strcmp (iid, "GeyesApplet"))
        retval = geyes_applet_fill (applet);

    if (retval == FALSE) {
        exit (-1);
    }

    return retval;
}

PANEL_APPLET_FACTORY ("GeyesAppletFactory",
                      PANEL_TYPE_APPLET,
                      "geyes",
                      geyes_applet_factory,
                      NULL)
