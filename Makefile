# led-notification Makefile
# Simo Mattila <simo.h.mattila at gmail dot com>

GTK_PIDGIN_INCLUDES= `pkg-config --cflags gtk+-2.0 pidgin`

GTK_PREFIX=/usr/local
GTK_PREFIX2=/usr

CFLAGS= -O2 -Wall -fpic -g
LDFLAGS= -shared -ldbus-glib-1 -ldbus-1 -lpthread -lgobject-2.0 -lglib-2.0

INCLUDES = \
      -I$(GTK_PREFIX)/include \
      -I$(GTK_PREFIX)/include/gtk-2.0 \
      -I$(GTK_PREFIX)/include/glib-2.0 \
      -I$(GTK_PREFIX)/include/pango-1.0 \
      -I$(GTK_PREFIX)/include/atk-1.0 \
      -I$(GTK_PREFIX)/lib/glib-2.0/include \
      -I$(GTK_PREFIX)/lib/gtk-2.0/include \
      -I$(GTK_PREFIX2)/include \
      -I$(GTK_PREFIX2)/include/gtk-2.0 \
      -I$(GTK_PREFIX2)/include/glib-2.0 \
      -I$(GTK_PREFIX2)/include/pango-1.0 \
      -I$(GTK_PREFIX2)/include/atk-1.0 \
      -I$(GTK_PREFIX2)/lib/glib-2.0/include \
      -I$(GTK_PREFIX2)/lib/gtk-2.0/include \
      -I/usr/include/dbus-1.0 \
      -I/usr/lib/dbus-1.0/include \
      $(GTK_PIDGIN_INCLUDES)

mled-notification.so: mled-notification.c
	gcc mled-notification.c $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o mled-notification.so

install: mled-notification.so
	cp mled-notification.so /usr/lib/pidgin/

uninstall:
	rm -f /usr/lib/pidgin/mled-notification.so

clean:
	rm -f mled-notification.so
