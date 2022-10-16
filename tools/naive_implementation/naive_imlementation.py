# Naive implementation of the algorithm intended for OneAPI implementation correctness tests.

import json
import math
import os
import sys
import numpy as np


def cartesian_to_polar(x, y):
    radius = math.sqrt(x ** 2 + y ** 2)
    angle = math.atan2(y, x)
    return (radius, angle)


def polar_to_cartesian(radius, angle):
    x = radius * math.cos(angle)
    y = radius * math.sin(angle)
    return (x, y)


def lerp(begin, end, value):
    return begin + (end - begin) * value


def load_config():
    if len(sys.argv) < 2:
        print('Args must be: <config_file>', file=sys.stderr)
        exit(-1)
    try:
        with open(sys.argv[1], 'r') as config_file:
            config = json.load(config_file)
    except Exception as e:
        print(e, file=sys.stderr)
        exit(-1)

    return config


def load_points(config):
    try:
        return np.loadtxt(config["inputFile"])
    except Exception as e:
        print(e, file=sys.stderr)
        exit(-1)


def fill_accumulator(accumulator, points):
    def fill_one_point(point_r, point_phi):
        for q_over_pt_index in range(x_dpi):
            for phi_index in range(y_dpi):
                q_over_pt = lerp(x_begin, x_end, q_over_pt_index / x_dpi)
                phi = lerp(y_begin, y_end, phi_index / y_dpi)
                q_over_pt_left = q_over_pt - delta_x * 0.5
                q_over_pt_right = q_over_pt + delta_x * 0.5
                phi_bottom = phi - delta_y * 0.5
                phi_top = phi + delta_y * 0.5
                phi_left = -point_r * q_over_pt_left + point_phi
                phi_right = -point_r * q_over_pt_right + point_phi

                if phi_left >= phi_bottom and phi_right <= phi_top:
                    # print(f'Hit: q_ver_pt: {q_over_pt}\t phi: {phi}\t q_over_pt_left: {q_over_pt_left}\t q_over_pt_right: {q_over_pt_right}\t phi_top: {phi_top}\t phi_bottom: {phi_bottom}')
                    accumulator[phi_index][q_over_pt_index] += 1

    for point in points:
        x = point[0]
        y = point[1]
        r, phi = cartesian_to_polar(x, y)
        r *= 0.001
        fill_one_point(r, phi)


if __name__ == '__main__':
    config = load_config()
    r_detector = config['R']
    B = config['B']
    points = load_points(config)

    x_begin = config['main_accumulator_config']['x_begin']
    x_end = config['main_accumulator_config']['x_end']
    x_dpi = config['main_accumulator_config']['x_dpi']
    y_begin = config['main_accumulator_config']['y_begin']
    y_end = config['main_accumulator_config']['y_end']
    y_dpi = config['main_accumulator_config']['y_dpi']
    layers = config['main_accumulator_config']['layers']

    x_linspace = np.linspace(x_begin, x_end, x_dpi)
    delta_x = x_linspace[1] - x_linspace[0]
    y_linspace = np.linspace(y_begin, y_end, y_dpi)
    delta_y = y_linspace[1] - y_linspace[0]

    accumulator = np.zeros((y_dpi, x_dpi), int)
    fill_accumulator(accumulator, points)

    with open("python_accumulator_dump.log", 'w') as file:
        for phi_index in range(y_dpi):
            for q_over_pt_index in range(x_dpi):
                file.write(f'{accumulator[phi_index][q_over_pt_index]} ')
            file.write('\n')

