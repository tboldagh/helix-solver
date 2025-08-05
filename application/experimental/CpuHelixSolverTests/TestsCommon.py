import json
import os
import matplotlib.pyplot as plt
import numpy as np

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

def draw_heat_map(values_map, x_axis_values, y_axis_values, title="", x_label="", y_label="", figure_size=None, rotate_x_ticks=False, x_ticks_label_func=None, log_scale=False):
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
    plt.colorbar()
    plt.show()