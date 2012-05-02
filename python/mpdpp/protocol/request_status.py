from __future__ import absolute_import

from mpdpp.protocol.frame_base import FrameBase

class RequestStatus(FrameBase):
    TYPE = 0x04
