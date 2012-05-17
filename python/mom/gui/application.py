from __future__ import absolute_import

import pkg_resources
import threading

from gi.repository import Gtk
from gi.repository import GObject

from mom.client import Client

class NetworkThread(threading.Thread):
    def __init__(self, client, gobject):
        threading.Thread.__init__(self)
        self.client = client
        self.gobject = gobject

    def run(self):
        self.client.ping()

class Application(GObject.GObject):
    """This is an Hello World GTK application"""

    __gsignals__ = {
        'my_signal': (GObject.SIGNAL_RUN_FIRST, None,
                      (object,))
    }

    def __init__(self):
        GObject.GObject.__init__(self)

        self.client = None
        self.network_thread = None

        self.glade_main = pkg_resources.resource_filename(__name__, "main.glade")

        #Set the Glade file
        self.builder = Gtk.Builder()
        self.builder.add_from_file(self.glade_main)
        self.builder.connect_signals(self)

        self.main_window = self.builder.get_object("MainWindow")

    def play(self, *args):
        self.client.play()

    def pause(self, *args):
        self.client.pause()

    def quit(self, *args):
        Gtk.main_quit()

    def run(self):
        GObject.threads_init()

        self.client = Client()
        self.network_thread = NetworkThread(self.client, self)
        self.network_thread.start()

        self.main_window.show()
        Gtk.main()
