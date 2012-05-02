from __future__ import absolute_import

import zmq
import msgpack

class FrameBaseMetaClass(type):
    frames = dict()

    def __init__(self, name, bases, dct):
        if bases != (object,):
            self.frames[self.TYPE] = self
        super(FrameBaseMetaClass, self).__init__(name, bases, dct)

    def __new__(cls, name, bases, dct):
        return type.__new__(cls, name, bases, dct)

class FrameBase(object):
    __metaclass__ = FrameBaseMetaClass

    fields = []

    def serialize(self):
        fields = list()

        for field_name in self.fields:
            fields.append(getattr(self, field_name))

        frame_bytes = msgpack.packb(fields)

        return msgpack.packb([self.TYPE, frame_bytes])

    @classmethod
    def deserialize(self, s):
        frame_type, frame_bytes = msgpack.unpackb(s)

        frame_class = self.frames.get(frame_type)

        if frame_class is None:
            raise ValueError("Not a valid frame")

        frame_fields = msgpack.unpackb(frame_bytes)

        frame_instance = frame_class()

        if frame_fields is None:
            return frame_instance

        for field_name, field_value in zip(frame_class.fields, frame_fields):
            setattr(frame_instance, field_name, field_value)

        return frame_instance
