from __future__ import absolute_import

from mpdpp.protocol.frame_base import FrameBase

class Ping(FrameBase):
    TYPE = 0x01

    fields = ["sequence"]

    def __init__(self, sequence=0):
        self.sequence = sequence
