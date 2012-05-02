from __future__ import absolute_import

from mom.protocol.frame_base import FrameBase

class Pong(FrameBase):
    TYPE = 0x02

    fields = ["sequence"]

    def __init__(self):
        self.sequence = 0
