from __future__ import absolute_import

import zmq
import random
import sys

from mom.protocol import FrameBase
from mom.protocol import Error
from mom.protocol import Ok
from mom.protocol import Ping
from mom.protocol import Kill
from mom.protocol import Pong
from mom.protocol import Status
from mom.protocol import RequestStatus
from mom.protocol import Play
from mom.protocol import Pause

from mom.exceptions import RemoteError
from mom.exceptions import LocalError

class Client(object):
    DEFAULT_TIMEOUT=5000

    def __init__(self, address="tcp://localhost:5555"):
        self.address = address
        self.context = zmq.Context(1)
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect(self.address)
        self.ping_sequence = random.randint(0, 2**16)
        self.recv_poller = zmq.Poller()
        self.recv_poller.register(self.socket, zmq.POLLIN)
        self.send_poller = zmq.Poller()
        self.send_poller.register(self.socket, zmq.POLLOUT)

    def close(self):
        self.socket.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, tb):
        self.close()
        return False

    def send(self, frame, timeout=DEFAULT_TIMEOUT):
        """
        Send a single frame
        """

        result = self.send_poller.poll(timeout)

        if not result:
            raise RemoteError("send timeout")

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

    def recv(self, expected=None, timeout=DEFAULT_TIMEOUT):
        """
        Receive a single frame.

        expected - Enable recv type checking and raise error if expectation is not
        fullfilled.
        """

        result = self.recv_poller.poll(timeout)

        if not result:
            raise RemoteError("recv timeout")

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

    def kill(self):
        kill = Kill()
        self.send(kill)
