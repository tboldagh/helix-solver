import json
import os
import matplotlib.pyplot as plt
import numpy as np
import math

class SolverTestParams:
    def __init__(self):
        self.testType = None
        self.eventId = None
        self.multipleEventIds = None
        self.numRuns = None
        self.inputSpacepointsFile = None
        self.inputParticlesInitialFile = None
        self.inputSpacepointsGenerationParamsFile = None
        self.outputResultsFile = None
        self.maxAbsXy = None
        self.maxAbsZ = None
        self.minZAngle = None
        self.maxZAngle = None
        self.minXAngle = None
        self.maxXAngle = None
        self.poleRegionAngle = None
        self.interactionRegionMin = None
        self.interactionRegionMax = None
        self.zAngleMargin = None
        self.xAngleMargin = None
        self.numZRanges = None
        self.numXRanges = None
        self.filterOutCenterR = None
        self.filterOutCenterZ = None
        self.hitsThresholds = None
        self.linesCrossingsThresholds = None

    def as_dict(self):
        return {
            "testType": self.testType,
            "eventId": self.eventId,
            "multipleEventIds": self.multipleEventIds,
            "numRuns": self.numRuns,
            "inputSpacepointsFile": self.inputSpacepointsFile,
            "inputParticlesInitialFile": self.inputParticlesInitialFile,
            "inputSpacepointsGenerationParamsFile": self.inputSpacepointsGenerationParamsFile,
            "outputResultsFile": self.outputResultsFile,
            "maxAbsXy": self.maxAbsXy,
            "maxAbsZ": self.maxAbsZ,
            "minZAngle": self.minZAngle,
            "maxZAngle": self.maxZAngle,
            "minXAngle": self.minXAngle,
            "maxXAngle": self.maxXAngle,
            "poleRegionAngle": self.poleRegionAngle,
            "interactionRegionMin": self.interactionRegionMin,
            "interactionRegionMax": self.interactionRegionMax,
            "zAngleMargin": self.zAngleMargin,
            "xAngleMargin": self.xAngleMargin,
            "numZRanges": self.numZRanges,
            "numXRanges": self.numXRanges,
            "filterOutCenterR": self.filterOutCenterR,
            "filterOutCenterZ": self.filterOutCenterZ,
            "hitsThresholds": self.hitsThresholds,
            "linesCrossingsThresholds": self.linesCrossingsThresholds
        }


def write_config(params, test_base_dir_relative, config_file_relative):
    os.makedirs(os.path.dirname(config_file_relative), exist_ok=True)
    with open(config_file_relative, 'w') as f:
        json.dump(params.as_dict(), f, indent=4)

def add_test_to_list(config_path):
    with open("../../../test_results/test_list.txt", "r") as file:
        if config_path in file.read():
            return

    with open("../../../test_results/test_list.txt", "a") as file:
        file.write(config_path + "\n")

def erase_test_list():
    with open("../../../test_results/test_list.txt", "w") as file:
        file.write("")

def wilson_score_interval(k, n, z = 1.96):
    if n == 0:
        return (0, 1)

    p = float(k) / n
    denominator = 1 + z**2/n
    centre_adjusted_probability = p + z*z / (2*n)
    adjusted_standard_deviation = math.sqrt((p*(1 - p) + z*z / (4*n)) / n)

    lower_bound = (centre_adjusted_probability - z*adjusted_standard_deviation) / denominator
    upper_bound = (centre_adjusted_probability + z*adjusted_standard_deviation) / denominator
    return (lower_bound, upper_bound)

def draw_bar_chart(bins, values, title, figure_size=(5, 5), max_xticks=None, draw_values=True, x_label=None, y_label=None, color_red=None, y_log=False):
    plt.figure(figsize=figure_size)

    colors = ['tab:blue' if color_red is None or not color_red[i] else 'tab:red' for i in range(len(bins))]
    bar_values = [values[i] if color_red is None or not color_red[i] else 0.5 for i in range(len(bins))]
    for i in range(len(bins)):
        plt.bar([f"{bins[i][0]:.2f}-{bins[i][1]:.2f}"], bar_values[i], color=colors[i])

    plt.xticks(rotation=90)

    if y_log:
        plt.yscale('log')

    if x_label is not None:
        plt.xlabel(x_label)
    if y_label is not None:
        plt.ylabel(y_label)

    if max_xticks is not None:
        plt.xticks(range(0, len(bins), len(bins) // max_xticks))

    if draw_values:
        for i in range(len(bins)):
            plt.text(i, values[i], f"{values[i]:.2f}", ha='center', va='bottom', rotation=90)

    plt.title(title)
    plt.show()

def draw_errorbar_chart(data, title, figure_size=(5, 5), max_xticks=None, draw_values=True, x_label=None, y_label=None, color_red=None, y_log=False, capsize=4):
    bins = data[0]
    values = data[1]
    counts = data[2] if len(data) > 2 else None
    sums = data[3] if len(data) > 3 else None

    colors = ['tab:blue' if color_red is None or not color_red[i] else 'tab:red' for i in range(len(bins))]
    bar_values = [values[i] if color_red is None or not color_red[i] else 0.5 for i in range(len(bins))]

    if counts is not None and sums is not None:
        wilson_intervals = [wilson_score_interval(sums[i], counts[i]) for i in range(len(bins))]
        wilson_errors = [([max(0, value - wilson[0])], [max(0, wilson[1] - value)]) for value, wilson in zip(bar_values, wilson_intervals)]
    else:
        wilson_errors = None

    plt.figure(figsize=figure_size)

    uplims = wilson_errors is not None
    lolims = uplims
    for i in range(len(bins)):
        plt.errorbar([f"{bins[i][0]:.2f}-{bins[i][1]:.2f}"], bar_values[i], yerr=wilson_errors[i], color=colors[i], marker='_', markeredgecolor='red', markersize=capsize * 2, capsize=capsize)

    plt.xticks(rotation=90)

    if y_log:
        plt.yscale('log')

    if x_label is not None:
        plt.xlabel(x_label)
    if y_label is not None:
        plt.ylabel(y_label)

    if max_xticks is not None:
        plt.xticks(range(0, len(bins), len(bins) // max_xticks))

    if draw_values:
        for i in range(len(bins)):
            plt.text(i, values[i], f"{values[i]:.2f}", ha='center', va='bottom', rotation=90)

    plt.title(title)
    plt.show()

def draw_heat_map(values_map, x_axis_values, y_axis_values, title="", x_label="", y_label="", figure_size=None, rotate_x_ticks=False, x_ticks_label_func=None, log_scale=False, clim=None, aspect=20, shrink=1.0):
    if x_ticks_label_func is None:
        x_ticks_values = x_axis_values
    else:
        x_ticks_values = [x_ticks_label_func(x_value) for x_value in x_axis_values]
    
    heat_map = np.zeros((len(y_axis_values), len(x_axis_values)))
    for i, x_value in enumerate(x_axis_values):
        for j, y_value in enumerate(y_axis_values):
            heat_map[j, i] = values_map[(x_value, y_value)]
            if log_scale:
                if heat_map[j, i] == 0:
                    heat_map[j, i] = 1
                heat_map[j, i] = np.log10(heat_map[j, i])

    plt.imshow(heat_map, cmap="hot")
    if figure_size is not None:
        plt.gcf().set_size_inches(figure_size[0], figure_size[1])

    plt.gca().invert_yaxis()
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(title)
    if rotate_x_ticks:
        plt.xticks(np.arange(len(x_axis_values)), x_ticks_values, rotation=90)
    else:
        plt.xticks(np.arange(len(x_axis_values)), x_ticks_values)
    plt.yticks(np.arange(len(y_axis_values)), y_axis_values)
    plt.colorbar(aspect=aspect, shrink=shrink)
    if clim is not None:
        plt.clim(clim[0], clim[1])
    plt.show()