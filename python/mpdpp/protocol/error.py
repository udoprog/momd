from __future__ import absolute_import

from mpdpp.protocol.frame_base import FrameBase

class Error(FrameBase):
    TYPE = 0x80

    fields = ["message"]

    def __init__(self, message=0):
        self.message = message
