from __future__ import absolute_import

from mpdpp.protocol.frame_base import FrameBase

class Status(FrameBase):
    TYPE = 0x05

    fields = [
        "playing",
        "decoder_ready",
        "decoder_requested_frames"
    ]

    def __init__(self):
        self.playing = False
        self.decoder_ready = False
        self.decoder_requested_frames = 0
