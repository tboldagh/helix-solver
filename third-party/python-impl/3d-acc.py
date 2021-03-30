#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from copy import deepcopy
import random
import threading
import math

R = 600
r_range = np.arange(1, 1000, 1)
multiplier = math.ceil(2 * math.pi)

points = np.loadtxt('../out/points.out')


def plot_points(points, map_size):
    plt.plot(for_plot[0], for_plot[1], '.', color='black')
    plt.ylabel('Y')
    plt.xlabel('X')
    axes = plt.gca()
    axes.set_xlim([0, sp_size])
    axes.set_ylim([0, sp_size])
    plt.show()


def calc_for_r(r, th):
    print('Th: {} calculating r: {}'.format(th, r))
    d_angles = np.linspace(0., np.pi / 2, int(r * multiplier))

    sins = np.sin(d_angles)
    coss = np.cos(d_angles)
    rsins = np.flip(sins)
    rcoss = np.flip(coss)

    sin_range = np.concatenate((sins, rsins, -sins, -rsins))
    cos_range = np.concatenate((coss, -rcoss, -coss, rcoss))

    sp_size = int(2 * (R + r))

    space = np.zeros((sp_size, sp_size), dtype=int)

    fixed_points = points + (sp_size / 2)

    for point in fixed_points:
        x = int(point[0])
        y = int(point[1])

        x_prev = int(x + r)
        y_prev = int(y)

        for idx in np.arange(0, len(sin_range)):
            x_circle = int(x + cos_range[idx] * r)
            y_circle = int(y + sin_range[idx] * r)

            if (x_circle != x_prev or y_circle != y_prev):
                space[y_circle][x_circle] += 1
                x_prev = x_circle
                y_prev = y_circle

    idx = np.unravel_index(space.argmax(), space.shape)
    print("From th: {} for r: {}: ".format(th, r) + " : " + str(idx) + " : " + str(space[idx[0]][idx[1]]))


def runAlgotithm(radius_list, id):
    for r in radius_list:
        calc_for_r(r, id)


if __name__ == '__main__':

    num_of_threads = 8

    jobs = []
    random.shuffle(r_range)

    r_len = len(r_range)
    one_range_len = math.ceil(r_len / num_of_threads)

    for i in range(0, num_of_threads):
        begin = i * one_range_len
        end = (i + 1) * one_range_len

        if end > r_len:
            end = r_len

        print('{} to {}'.format(begin, end))
        thread = threading.Thread(target=runAlgotithm, args=(r_range[begin:end], i))
        jobs.append(thread)

    for idx, j in enumerate(jobs):
        print(idx)
        j.start()

    for j in jobs:
        j.join()
