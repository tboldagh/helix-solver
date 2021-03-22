#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib.pyplot as plt

def pol2cart(r, a):
    x = r * np.cos(a)
    y = r * np.sin(a)
    return (x, y)

if __name__ == '__main__':
    r_detector = 600
    if len(sys.argv) < 3:
        print("Args must be: <path_to_points> <path_to_circles>")
        sys.exit(1)
    points = np.loadtxt(sys.argv[1])
    circles = np.loadtxt(sys.argv[2])
    axes = plt.gca()
    for p in points:
        plt.plot(p[0], p[1], '.', color='black')
    for c in circles:
        r, phi = c[0], c[1]
        x, y = pol2cart(r, phi)
        track = plt.Circle((x, y), r, color='g', fill=False)
        axes.add_artist(track)
    axes.set_xlim([-r_detector, r_detector])
    axes.set_ylim([-r_detector, r_detector])
    plt.ylabel('Y')
    plt.xlabel('X')
    detector_bound = plt.Circle((0, 0), r_detector, color='b', fill=False)
    axes.add_artist(detector_bound)
    plt.savefig('plot.png')
