# Keep code aligned with `../SplitterNotebook.ipynb!

import math
import json
import argparse


class DetectorProperties:
    max_abs_xy = None
    max_abs_z = None


class Wedge:
    def __init__(self, z_angle_min, z_angle_max, x_angle_min, x_angle_max, interaction_region_width, id=None):
        self.z_angle_min = z_angle_min
        self.z_angle_max = z_angle_max
        self.x_angle_min = x_angle_min
        self.x_angle_max = x_angle_max
        self.interaction_region_width = interaction_region_width
        self.id = id


class PoleRegion:
    def __init__(self, x_angle, interaction_region_width, id=None):
        self.x_angle = x_angle
        self.interaction_region_width = interaction_region_width
        self.id = id


def angle_wrap(angle):
    return math.fmod(angle + math.pi, 2 * math.pi) + (-1 + 2 * (1 if angle + math.pi < 0 else 0)) * math.pi

def angle_wrap_2pi(angle):
    return math.fmod(angle, 2 * math.pi) if angle >= 0 else (2 * math.pi + math.fmod(angle, 2 * math.pi))

# point[0] == min_value, point[-1] == max_value
def uniform_range(min_value, max_value, num_points):
    return [min_value + i * (max_value - min_value) / (num_points - 1) for i in range(num_points)]

def atan2_2pi(y, x):
    return angle_wrap_2pi(math.atan2(y, x))

def uniform_range_split(num_ranges, min_range, max_range, margin):
    boundaries = uniform_range(min_range, max_range, num_ranges + 1)
    return [(boundaries[i] - margin, boundaries[i + 1] + margin) for i in range(num_ranges)]

def point_in_wedge_z_angle(point, wedge):
    # Assumes wedge's z_angle_min and z_angle_max are in [0, 2 * math.pi)
    # Assumes wedge's z angle range is < math.pi

    z_angle = atan2_2pi(point['y'], point['x'])
    if wedge.z_angle_min < wedge.z_angle_max:
        if z_angle < wedge.z_angle_min or z_angle > wedge.z_angle_max:
            return False
    else:
        # wedge crosses z_angle = 0
        if (z_angle <= math.pi and z_angle > wedge.z_angle_max) or (z_angle > math.pi and z_angle < wedge.z_angle_min):
            return False

    return True

def point_in_wedge_x_angle(point, wedge):
    # Assumes wedge's x_angle_min and x_angle_max are in [0, math.pi)

    def outermost_point(x_angle):
        x_plane_direction_z = math.cos(x_angle)
        x_plane_direction_y = math.sin(x_angle)

        x_plane_scale_to_z_limit = abs(x_plane_direction_z) / DetectorProperties.max_abs_z
        x_plane_scale_to_y_limit = abs(x_plane_direction_y) / DetectorProperties.max_abs_xy
        scale = 1 / max(x_plane_scale_to_z_limit, x_plane_scale_to_y_limit)

        x = x_plane_direction_y * math.cos(wedge.z_angle_min) * scale
        y = x_plane_direction_y * math.sin(wedge.z_angle_min) * scale
        z = x_plane_direction_z * scale
        return x, y, z

    def to_z_angle(x, y, z, interaction_region_shift):
        distance_to_z = math.sqrt(x ** 2 + y ** 2)
        return math.atan2(distance_to_z, z - interaction_region_shift)

    interaction_region_shift = wedge.interaction_region_width / 2
    outermost_x, outermost_y, outermost_z = outermost_point(wedge.x_angle_min)
    boundary_to_z_angle = to_z_angle(outermost_x, outermost_y, outermost_z, interaction_region_shift)
    point_to_z_angle = to_z_angle(point['x'], point['y'], point['z'], interaction_region_shift)
    if point_to_z_angle < boundary_to_z_angle:
        return False
    
    interaction_region_shift = -wedge.interaction_region_width / 2
    outermost_x, outermost_y, outermost_z = outermost_point(wedge.x_angle_max)
    boundary_to_z_angle = to_z_angle(outermost_x, outermost_y, outermost_z, interaction_region_shift)
    point_to_z_angle = to_z_angle(point['x'], point['y'], point['z'], interaction_region_shift)
    if point_to_z_angle > boundary_to_z_angle:
        return False

    return True

def point_in_wedge(point, wedge):
    # Assumes wedge's z_angle_min and z_angle_max are in [0, 2 * math.pi)
    # Assumes wedge's z angle range is < math.pi
    # Assumes wedge's x_angle_min and x_angle_max are in [0, math.pi)
    return point_in_wedge_z_angle(point, wedge) and point_in_wedge_x_angle(point, wedge)

def point_in_pole_region(point, region):
    # Assumes region's x_angle is in [0, math.pi)

    def x_plane_outermost_yz(x_angle):
        x_plane_direction_z = math.cos(x_angle)
        x_plane_direction_y = math.sin(x_angle)

        x_plane_scale_to_z_limit = abs(x_plane_direction_z) / DetectorProperties.max_abs_z
        x_plane_scale_to_y_limit = abs(x_plane_direction_y) / DetectorProperties.max_abs_xy
        scale = 1 / max(x_plane_scale_to_z_limit, x_plane_scale_to_y_limit)

        return x_plane_direction_y * scale, x_plane_direction_z * scale
    
    def to_z_angle(point, interaction_region_shift):
        distance_to_z = math.sqrt(point['x'] ** 2 + point['y'] ** 2)
        z = point['z'] - interaction_region_shift
        return math.atan2(distance_to_z, z)
    
    x_plane_outermost_y, x_plane_outermost_z = x_plane_outermost_yz(region.x_angle)
    interaction_region_shift = (-1 if region.x_angle < math.pi / 2 else 1) * region.interaction_region_width / 2
    boundary_to_z_angle = math.atan2(x_plane_outermost_y, x_plane_outermost_z - interaction_region_shift)
    point_to_z_angle = to_z_angle(point, interaction_region_shift)

    if (region.x_angle < math.pi / 2 and point_to_z_angle > boundary_to_z_angle) or (region.x_angle > math.pi / 2 and point_to_z_angle < boundary_to_z_angle):
        return False

    return True


class SplitterSettings:
    wedges = []
    pole_regions = []


class CommandLineArguments:
    splitter_settings_path = None
    points_path = None
    points_output_path = None

def parse_command_line_arguments():

    parser = argparse.ArgumentParser(description='Splitter settings and spacepoints generator')
    parser.add_argument('--splitter_settings', type=str, help='Path to the splitter settings JSON file')
    parser.add_argument('--points', type=str, help='Path to the spacepoints CSV file')
    parser.add_argument('--points_output', type=str, help='Path to the output spacepoints with region IDs CSV file')

    args = parser.parse_args()
    CommandLineArguments.splitter_settings_path = args.splitter_settings
    CommandLineArguments.points_path = args.points
    CommandLineArguments.points_output_path = args.points_output


def load_splitter_settings(path):
    with open(path, 'r') as file:
        settings = json.load(file)

    detector_properties = settings['detector_properties']
    DetectorProperties.max_abs_xy = detector_properties['max_abs_xy']
    DetectorProperties.max_abs_z = detector_properties['max_abs_z']
        
    wedges = []
    for wedge in settings['wedges']:
        wedges.append(Wedge(wedge['z_angle_min'], wedge['z_angle_max'], wedge['x_angle_min'], wedge['x_angle_max'], wedge['interaction_region_width'], wedge['id']))
    SplitterSettings.wedges = wedges

    pole_regions = []
    for region in settings['pole_regions']:
        pole_regions.append(PoleRegion(region['x_angle'], region['interaction_region_width'], region['id']))
    SplitterSettings.pole_regions = pole_regions


def print_splitter_settings():
    print("Detector properties:")
    print("\tmax_abs_xy:", DetectorProperties.max_abs_xy)
    print("\tmax_abs_z:", DetectorProperties.max_abs_z)
    print()
    print("Splitter settings:")
    for wedge in SplitterSettings.wedges:
        print(f"\tWedge {wedge.id}:")
        print("\t\tz_angle_min:", wedge.z_angle_min)
        print("\t\tz_angle_max:", wedge.z_angle_max)
        print("\t\tx_angle_min:", wedge.x_angle_min)
        print("\t\tx_angle_max:", wedge.x_angle_max)
        print("\t\tinteraction_region_width:", wedge.interaction_region_width)
    for region in SplitterSettings.pole_regions:
        print(f"\tPole region {region.id}:")
        print("\t\tx_angle:", region.x_angle)
        print("\t\tinteraction_region_width:", region.interaction_region_width)
    print()


def read_spacepoints(path):
    with open(path, 'r') as file:
        lines = file.readlines()

    points = []
    for line in lines[1:]:
        point_variables = line.strip().split(',')
        point = {
            'x': float(point_variables[2]),
            'y': float(point_variables[3]),
            'z': float(point_variables[4]),
        }
        points.append(point)

    return points


def points_with_region_ids(points):
    for point in points:
        region_ids = []
        for wedge in SplitterSettings.wedges:
            if point_in_wedge(point, wedge):
                region_ids.append(wedge.id)
        for region in SplitterSettings.pole_regions:
            if point_in_pole_region(point, region):
                region_ids.append(region.id)
        point['region_ids'] = region_ids if len(region_ids) > 0 else [0]

    return points


def print_points_by_region_id(points):
    points_by_region_id = {}
    for point in points:
        region_ids = point['region_ids']
        for region_id in region_ids:
            if region_id not in points_by_region_id:
                points_by_region_id[region_id] = []
            points_by_region_id[region_id].append(point)
    print("Points by region id:")
    region_ids = list(points_by_region_id.keys())
    region_ids.sort()
    for region_id in region_ids:
        print(f"\tRegion {region_id}: {len(points_by_region_id[region_id])}\tpoints")


def write_points_with_region_ids(points, path):
    with open(path, 'w') as file:
        file.write('x, y, z, region_ids\n')
        for point in points:
            region_ids = point['region_ids']
            file.write(f"{point['x']}, {point['y']}, {point['z']}, {', '.join(map(str, region_ids))}\n")


def main():
    parse_command_line_arguments()

    load_splitter_settings(CommandLineArguments.splitter_settings_path)
    print_splitter_settings()

    points = read_spacepoints(CommandLineArguments.points_path)
    points = points_with_region_ids(points)
    print_points_by_region_id(points)

    write_points_with_region_ids(points, CommandLineArguments.points_output_path)


if __name__ == '__main__':
    main()