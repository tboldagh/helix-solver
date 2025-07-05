import math


def angle_wrap(angle):
    return math.fmod(angle + math.pi, 2 * math.pi) + (-1 + 2 * (1 if angle + math.pi < 0 else 0)) * math.pi

def angle_wrap_2pi(angle):
    return angle - 2.0 * math.pi * math.floor(angle / (2.0 * math.pi))

def uniform_range(min_value, max_value, num_points):
    return [min_value + i * (max_value - min_value) / (num_points - 1) for i in range(num_points)]

def atan2_2pi(y, x):
    return angle_wrap_2pi(math.atan2(y, x))

def uniform_range_split(num_ranges, min_range, max_range, margin):
    boundaries = uniform_range(min_range, max_range, num_ranges + 1)
    return [(boundaries[i] - margin, boundaries[i + 1] + margin) for i in range(num_ranges)]

def lerp(min_value, max_value, t):
    return min_value + t * (max_value - min_value)