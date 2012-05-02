from __future__ import absolute_import

import zmq
import random
import sys

from mpdpp.protocol import FrameBase
from mpdpp.protocol import Error
from mpdpp.protocol import Ok
from mpdpp.protocol import Ping
from mpdpp.protocol import Pong
from mpdpp.protocol import Status
from mpdpp.protocol import RequestStatus
from mpdpp.protocol import Play
from mpdpp.protocol import Pause

from mpdpp.exceptions import RemoteError
from mpdpp.exceptions import LocalError

class Client(object):
    def __init__(self, address="tcp://localhost:5555"):
        self.context = zmq.Context(1)
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect(address)
        self.ping_sequence = random.randint(0, 2**16)

    def send(self, frame):
        """
        Send a single frame
        """

        try:
            frame_string = frame.serialize()
        except:
            _, _, tb = sys.exc_info()
            raise LocalError("Serialization failed"), None, tb

        try:
            self.socket.send(frame_string)
        except:
            _, _, tb = sys.exc_info()
            raise RemoteError("Send failed"), None, tb

    def recv(self, expected=None):
        """
        Receive a single frame.

        expected - Enable recv type checking and raise error if expectation is not
        fullfilled.
        """

        try:
            frame_string = self.socket.recv()
        except:
            _, _, tb = sys.exc_info()
            raise RemoteError("Recv failed"), None, tb

        try:
            frame = FrameBase.deserialize(frame_string)
        except:
            _, _, tb = sys.exc_info()
            raise LocalError("Deserialization failed"), None, tb

        if isinstance(frame, Error):
            raise RemoteError(frame.message)

        if expected is not None and not isinstance(frame, expected):
            raise LocalError("Expected frame of type "
                    "" + repr(expected) + " "
                    "but got " + repr(frame.__class__) + "")

        return frame

    def sendrecv(self, frame, expected=None):
        """
        Sugar coating for the normal send-recv operation.

        expected - Passed into recv if recv type checking should be enabled.
        """

        self.send(frame)
        return self.recv(expected=expected)

    def ping(self):
        ping = Ping(sequence=self.ping_sequence)

        pong = self.sendrecv(ping, expected=Pong)

        if pong.sequence != ping.sequence:
            raise RemoteError("Remote side returned invalid sequence")

        self.ping_sequence += 1

    def status(self):
        request = RequestStatus()
        return self.sendrecv(request, expected=Status)

    def play(self):
        play = Play()
        self.sendrecv(play, expected=Ok)

    def pause(self):
        pause = Pause()
        self.sendrecv(pause, expected=Ok)
