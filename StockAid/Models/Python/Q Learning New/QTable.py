from ctypes import Structure
from multiprocessing import sharedctypes


class QTable(Structure):
    _fields_ = [("value", (((sharedctypes.ctypes.c_float * 3) * 11) * 11) * 11)]