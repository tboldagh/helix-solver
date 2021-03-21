#!/usr/bin/env python3

# this script generates only circles for now (as current version of solver does not support helices detection)
# TODO: implement also Z dimention generation (now is always 0)

import sys
import numpy as np
import traceback

def cart2pol(x, y):
    rho = np.sqrt(x**2 + y**2)
    phi = np.arctan2(y, x)
    return (rho, phi)

def pol2cart(rho, phi):
    x = rho * np.cos(phi)
    y = rho * np.sin(phi)
    return (x, y)

def calculate_y(x, X, Y, r):
    y1 = y2 = None
    u = r**2 - (x - X)**2
    if u > 0:
        y1 = Y - np.sqrt(u)
        y2 = Y + np.sqrt(u)
    return (y1, y2)

def is_point_in_detector(x, y, layers):
    r = np.sqrt(x**2 + y**2)
    for l in layers:
        if r >= l[0] and r <= l[1]:
            return True
    return False

if __name__ == '__main__':
    r_detector = 600 # in millimeters
    detector_layers = np.array([[50, 60], [70, 75],  [270, 300], [350, 380], [460, 490], [550, 570]])
    if len(sys.argv) < 5:
        print('Args must be: begin end num_of_circles points_per_circle', file=sys.stderr)
        sys.exit(1)
    try:
        r_linspace = np.linspace(float(sys.argv[1]), float(sys.argv[2]), int(sys.argv[3]))
        points = []
        for r in r_linspace:
            phi = np.random.uniform(-np.pi, np.pi) # get random angle
            X, Y = pol2cart(r, phi)
            cnt = int(sys.argv[4])
            while cnt > 0:
                x = np.random.uniform(-r_detector, r_detector) # random x inside detector
                y = np.random.choice(calculate_y(x, X, Y, r))
                if y is None:
                    continue
                if is_point_in_detector(x, y, detector_layers):
                    points.append([x, y, 0])
                    cnt = cnt - 1
        with open('event-gen.txt', 'w') as f:
            for point in points:
                f.write('{} {} {}\n'.format(point[0], point[1], point[2]))
    except:
        print('Something went wrong - traceback below')
        traceback.print_exc()
