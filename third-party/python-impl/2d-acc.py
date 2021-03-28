#!/usr/bin/env python3

import numpy as np
import math
import sys
import json
from copy import deepcopy

def cart2pol(x, y):
    r = np.sqrt(x**2 + y**2)
    a = np.arctan2(y, x)
    return (r, a)

def pol2cart(r, a):
    x = r * np.cos(a)
    y = r * np.sin(a)
    return (x, y)

def find_nearest(array, value):
    array = np.asarray(array)
    idx = (np.abs(array - value)).argmin()
    return idx

def prepare_y_idx(accumulator_config, Y, y_left, y_right):
    y_end = accumulator_config['y_end']
    y_begin = accumulator_config['y_begin']
    y_dpi = accumulator_config['y_dpi']

    if y_left > y_begin and y_left < y_end and y_right > y_begin and y_right < y_end:
        y_right_idx = find_nearest(Y, y_right)
        y_left_idx = find_nearest(Y, y_left)

    elif y_left > y_begin and y_right < y_begin:
        y_right_idx = 0
        y_left_idx = find_nearest(Y, y_left)
    
    elif y_left < y_begin and y_right > y_begin:
        y_right_idx = find_nearest(Y, y_right)
        y_left_idx = 0

    elif y_left > y_end and y_right < y_end:
        y_right_idx = find_nearest(Y, y_right)
        y_left_idx = y_dpi - 1

    elif y_left < y_end and y_right > y_end:
        y_right_idx = y_dpi - 1
        y_left_idx = find_nearest(Y, y_left)

    else:
        y_left_idx = None
        y_right_idx = None

    return y_left_idx, y_right_idx

class Accumulator:
    points = []

    def __init__(self, config):
        self._config = config
        self._X = np.linspace(config['x_begin'], config['x_end'], config['x_dpi'])
        self._dx = self._X[1] - self._X[0]
        self._dx_2 = self._dx / 2

        self._Y = np.linspace(config['y_begin'], config['y_end'], config['y_dpi'])
        self._dy = self._Y[1] - self._Y[0]
        self._map = np.zeros((config['y_dpi'], config['x_dpi']), dtype=np.ubyte)

    def fill(self):
        for point in points:
            (r, phi) = cart2pol(point[0], point[1])
            r = r / 1000 # to meters
            fun = lambda x: -r * x + phi

            for x_idx in range(self._config['x_dpi']):
                x = self._X[x_idx]
                x_left = x - self._dx_2
                x_right = x + self._dx_2

                y_left = fun(x_left)
                y_right = fun(x_right)
                y_left_idx, y_right_idx = prepare_y_idx(self._config, self._Y, y_left, y_right)

                if y_left_idx == None:
                    continue

                if y_left_idx > y_right_idx:
                    y_left_idx, y_right_idx = y_right_idx, y_left_idx
                
                for y_idx in range(y_left_idx, y_right_idx+1):
                    self._map[y_idx][x_idx] += 1

    def get_values_at_position(self, x, y):
        return [self._X[x], self._Y[y]]

    def get_X_at(self, idx):
        return self._X[idx]

    def get_Y_at(self, idx):
        return self._Y[idx]

    def get_map(self):
        return self._map

    def get_deltas(self):
        return self._dx, self._dy

    def dealloc(self):
        self._map = None
    
    def get_cells_above_threshold(self, threshold=5):
        maximums = self._map > threshold
        maximums_idx = np.where(maximums)
        return np.transpose(maximums_idx)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Args must be: <config_file> <points_file>', file=sys.stderr)
    try:
        with open(sys.argv[1], 'r') as config_file:
            config = json.load(config_file)
        points = np.loadtxt(sys.argv[2])
    except Exception as e:
        print(e, file=sys.stderr)
    r_detector = config['R']
    B = config['B']

    Accumulator.points = points

    main_acc = Accumulator(config['main_accumulator_config'])
    main_acc.fill()
    acc_map = main_acc.get_map()

    candidate_cells = main_acc.get_cells_above_threshold(3)
    
    for cand in candidate_cells:
        vals = main_acc.get_values_at_position(cand[1], cand[0])
        r = ((1 / vals[0]) / B) * 1000
        phi = vals[1] + math.pi / 2
        x, y = pol2cart(r, phi)

    main_acc_dx, main_acc_dy = main_acc.get_deltas()
    main_acc_dx_2 = main_acc_dx / 2
    main_acc_dy_2 = main_acc_dy / 2

    track_candidates = []

    for cell in candidate_cells:
        cell_accumulator_config = deepcopy(config['cell_accumulator_config'])
        cell_accumulator_config['x_begin'] = main_acc.get_X_at(cell[1]) - main_acc_dx_2
        cell_accumulator_config['x_end'] = main_acc.get_X_at(cell[1]) + main_acc_dx_2
        cell_accumulator_config['y_begin'] = main_acc.get_Y_at(cell[0]) - main_acc_dy_2
        cell_accumulator_config['y_end'] = main_acc.get_Y_at(cell[0]) + main_acc_dy_2

        cell_acc = Accumulator(cell_accumulator_config)
        cell_acc.fill()

        cand_idxes = cell_acc.get_cells_above_threshold(3)

        for cand in cand_idxes:
            track_candidates.append(cell_acc.get_values_at_position(cand[1], cand[0]))

    with open('detected_circles.txt', 'w') as f:
        for cand in track_candidates:
            r = ((1 / cand[0]) / B) * 1000
            phi = cand[1] + math.pi / 2

            x, y = pol2cart(r, phi)
            r, phi = cart2pol(x, y)

            f.write('{} {}\n'.format(r, phi))
