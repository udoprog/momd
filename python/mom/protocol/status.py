from __future__ import absolute_import

from mom.protocol.frame_base import FrameBase

class Status(FrameBase):
    TYPE = 0x05

    fields = [
        "playing",
        "decoder_ready",
        "output_pending_frames"
    ]

    def __init__(self):
        self.playing = False
        self.decoder_ready = False
        self.output_pending_frames = 0
