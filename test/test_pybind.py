import pmtv
from pmtv import pmt
import numpy as np

# x = pmt(7)
# xp = x()
# print(type(xp))

# x = pmt(777)
# xp = x()
# print(type(xp))

# x = pmt(int(777))
# xp = x()
# print(type(xp))
# x = pmt(np.array([1,2,3],dtype=np.int8))
# print(x)

# x = pmt([1,2,3])
# print(x)

# x = pmt(np.array([1,2,3],dtype=np.float32))
x = pmt([pmt(12345), pmt(67890)])
# print(type(x))
# print(x)
print(type(x()))
# print(type(x()[0]))
# print(type(x()[0]()))

# print(x()[0]())
# print(x()[1]())

# x = pmt(np.int16(777))
# print(x)
# xp = x()
# print(type(xp))
