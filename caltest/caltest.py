#!/usr/bin/env python3

'''
Some exemplary values for hp spectre x360 convertible
- screen flat: (0,0,-10)
- screen straight (landscape) (0.5,-9.4,-1.5)
- screen portrait left (-9.7,-0.4,-1.2)
- screen portrait right (9.5, 0.3 -0.5)
'''


import time

pacc = '/sys/bus/iio/devices/iio:device3'
txyz = 'in_accel_%s_raw'
tscale = 'in_accel_scale'
gscale = 1.0 # 1.0 leads to a bit more than a in m/s**2
scale = 1.0 # re-init later

def read_sens(s):
    f = open(f"{pacc}/{s}")
    r = f.read()
    f.close()
    return r

def get_gvec():
    r = tuple([v * gscale * scale for v in [float(read_sens(txyz%c)) for c in ['x','y','z']]])
    return r

if __name__ == '__main__':
    scale = float(read_sens(tscale))
    while 1:
        v = get_gvec()
        print(v)
        time.sleep(1)
