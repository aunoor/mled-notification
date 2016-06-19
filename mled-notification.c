/*
 * Led-notification
 * Copyright (C) 2006  Simo Mattila
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Simo Mattila <simo.h.mattila@gmail.com>
 *
 * set ts=4 ;)
 *
 * Dmitry Komissarov <aunoor@gmail.com>
 */

#define PURPLE_PLUGINS

#define VERSION "0.1"

#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <mce/dbus-names.h>


#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "debug.h"
#include "cmds.h"
#include "gtkconv.h"
#include "prefs.h"
#include "gtkprefs.h"
#include "gtkutils.h"
#include "gtkplugin.h"
#include "gtkblist.h"

static DBusConnection *dbus_sysConnection(void);



void fire_vibration(gboolean enable) {
	const char * pattern = "PatternIncomingMessage";

	DBusConnection *connection = dbus_sysConnection();

	DBusMessage *message = dbus_message_new_method_call(MCE_SERVICE,
                                    			    MCE_REQUEST_PATH,
                                    			    MCE_REQUEST_IF,
                                    			    enable ? MCE_ACTIVATE_VIBRATOR_PATTERN : MCE_DEACTIVATE_VIBRATOR_PATTERN);
	DBusMessage *reply = NULL;

	if (message) {
		//purple_debug_info("|-P- *** fire_vibration: Making dbus call\n");
		dbus_message_append_args(message, DBUS_TYPE_STRING, &pattern, DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(connection, message, -1, NULL);

		if (reply == NULL) {
		    //purple_debug_info(("|-P- *** fire_vibration: dbus send error \n");
				}
		dbus_message_unref(message);
	} else {
	 purple_debug_info("|-P-"," *** fire_vibration: create dbus signal failed \n");
	}

	if (reply)
		dbus_message_unref(reply);
}

void fire_led_notify(gboolean enable) {

const char * pattern = "PatternCommunicationIM";

DBusConnection *connection = dbus_sysConnection();
DBusMessage *message = dbus_message_new_method_call(MCE_SERVICE,
																						MCE_REQUEST_PATH,
																						MCE_REQUEST_IF,
																						enable ? MCE_ACTIVATE_LED_PATTERN : MCE_DEACTIVATE_LED_PATTERN);
DBusMessage *reply = NULL;

if (message) {
	//purple_debug_info("|-P- *** fire_vibration: Making dbus call\n");
	dbus_message_append_args(message, DBUS_TYPE_STRING, &pattern, DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(connection, message, -1, NULL);

	if (reply == NULL) {
			//purple_debug_info(("|-P- *** fire_vibration: dbus send error \n");
			}
	dbus_message_unref(message);
} else {
 purple_debug_info("|-P-", " *** fire_vibration: create dbus signal failed \n");
}

if (reply)
	dbus_message_unref(reply);
}



void led_set(gboolean state) {


	if(state) {
		fire_led_notify(TRUE);
		fire_vibration(TRUE);
	} else {
		fire_led_notify(FALSE);
		fire_vibration(FALSE);
	}
}

GList *get_pending_list(guint max) {
	const char *im=purple_prefs_get_string("/plugins/gtk/gtk-maemonot/im");
	const char *chat=purple_prefs_get_string(
	                                     "/plugins/gtk/gtk-maemonot/chat");
	GList *l_im = NULL;
	GList *l_chat = NULL;


	if (im != NULL && strcmp(im, "always") == 0) {
		l_im = pidgin_conversations_find_unseen_list(PURPLE_CONV_TYPE_IM,
		                                             PIDGIN_UNSEEN_TEXT,
		                                             FALSE, max);
	} else if (im != NULL && strcmp(im, "hidden") == 0) {
		l_im = pidgin_conversations_find_unseen_list(PURPLE_CONV_TYPE_IM,
		                                             PIDGIN_UNSEEN_TEXT,
		                                             TRUE, max);
	}

	if (chat != NULL && strcmp(chat, "always") == 0) {
		l_chat = pidgin_conversations_find_unseen_list(PURPLE_CONV_TYPE_CHAT,
		                                               PIDGIN_UNSEEN_TEXT,
		                                               FALSE, max);
	} else if (chat != NULL && strcmp(chat, "nick") == 0) {
		l_chat = pidgin_conversations_find_unseen_list(PURPLE_CONV_TYPE_CHAT,
		                                               PIDGIN_UNSEEN_NICK,
		                                               FALSE, max);
	}

	if (l_im != NULL && l_chat != NULL)
		return g_list_concat(l_im, l_chat);
	else if (l_im != NULL)
		return l_im;
	else
		return l_chat;
}

static void lednot_conversation_updated(PurpleConversation *conv,
                                        PurpleConvUpdateType type) {
	GList *list;

	if( type != PURPLE_CONV_UPDATE_UNSEEN ) {
		return;
	}

#if 0
	purple_debug_info("Led-notification", "Change in unseen conversations\n");
#endif

	list=get_pending_list(1);

	if(list==NULL) {
		led_set(FALSE);
	} else if(list!=NULL) {
		led_set(TRUE);
	}
	g_list_free(list);
}

static GtkWidget *plugin_config_frame(PurplePlugin *plugin) {
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkSizeGroup *sg;
	GtkWidget *dd;
	//GtkWidget *ent;

	frame = gtk_vbox_new(FALSE, 18);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 12);

	vbox = pidgin_make_frame(frame, "Inform about unread...");
	sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	dd = pidgin_prefs_dropdown(vbox, "Instant Messages:",
	                           PURPLE_PREF_STRING,
	                           "/plugins/gtk/gtk-maemonot/im",
	                           "Never", "never",
	                           "In hidden conversations", "hidden",
	                           "Always", "always",
	                           NULL);
	gtk_size_group_add_widget(sg, dd);

	dd = pidgin_prefs_dropdown(vbox, "Chat Messages:",
	                        PURPLE_PREF_STRING,
	                        "/plugins/gtk/gtk-maemonot/chat",
	                        "Never", "never",
	                        "When my nick is said", "nick",
	                        "Always", "always",
	                        NULL);
	gtk_size_group_add_widget(sg, dd);

	gtk_widget_show_all(frame);
	return frame;
}

static void init_plugin(PurplePlugin *plugin) {
	purple_prefs_add_none("/plugins/gtk/gtk-maemonot");
	purple_prefs_add_string("/plugins/gtk/gtk-maemonot/im", "always");
	purple_prefs_add_string("/plugins/gtk/gtk-maemonot/chat", "nick");
}

static DBusConnection *
dbus_sysConnection()
{
	static DBusConnection *sysConn = NULL;

	if (G_UNLIKELY(NULL == sysConn)) {
		sysConn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
		dbus_connection_setup_with_g_main(sysConn, NULL);
//		dbus_connection_add_filter(sysConn, (DBusHandleMessageFunction)message_filter, NULL, NULL);
//		dbus_bus_add_match(sysConn, "type='signal',sender='org.freedesktop.DBus',interface='org.freedesktop.DBus',path='/org/freedesktop/DBus',member='NameOwnerChanged'", NULL);
//		debug_print("|-P- *** dbus_connection: My unique name is %s\n", dbus_bus_get_unique_name(conn));
	}

	return sysConn;
}

static gboolean plugin_load(PurplePlugin *plugin) {
	purple_signal_connect(purple_conversations_get_handle(),
	                      "conversation-updated", plugin,
	                      PURPLE_CALLBACK(lednot_conversation_updated), NULL);
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin) {
	led_set(FALSE); /* Turn the led off */
	purple_signal_disconnect(purple_conversations_get_handle(),
	                         "conversation-updated", plugin,
	                         PURPLE_CALLBACK(lednot_conversation_updated));
    return TRUE;
}

static PidginPluginUiInfo ui_info = {
	plugin_config_frame,
	0 /* page_num (Reserved) */
};

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    PIDGIN_PLUGIN_TYPE,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    "gtk-maemonot",
    "MLed-notification",
    VERSION,

    "Led notification on Nokia N900",
    "Informs for new messages with led",
    "",
    "",

    plugin_load,   /* load */
    plugin_unload, /* unload */
    NULL,          /* destroy */

    &ui_info,
    NULL,
    NULL,
    NULL
};

PURPLE_INIT_PLUGIN(lednot, init_plugin, info);
