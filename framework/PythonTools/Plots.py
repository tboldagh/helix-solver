from ParticleInitial import *
from Spacepoint import *
from Wedge import *
from PoleRegion import *
import matplotlib.pyplot as plt
import math


class DetectorPlot:
    def __init__(self, figure=None, ax=None, figure_size=(10, 10), max_abs_z=3100, max_abs_xy=1100, keep_aspect_ratio=False, perspective=None):
        self.figure = figure if figure is not None else plt.figure(figsize=figure_size)
        self.ax = ax if ax is not None else plt.axes(projection='3d')

        self.max_abs_z = max_abs_z
        self.max_abs_xy = max_abs_xy
        self.max_abs_xyz = math.sqrt(self.max_abs_z ** 2 + self.max_abs_xy ** 2)

        self.ax.set_xlim([-max_abs_z, max_abs_z] if keep_aspect_ratio else [-max_abs_xy, max_abs_xy])
        self.ax.set_ylim([-max_abs_z, max_abs_z])
        self.ax.set_zlim([-max_abs_z, max_abs_z] if keep_aspect_ratio else [-max_abs_xy, max_abs_xy])
        self.ax.set_xlabel('x')
        self.ax.set_ylabel('z')
        self.ax.set_zlabel('y')
        self.ax.set_facecolor('black')
        self.ax.xaxis.pane.fill = False
        self.ax.yaxis.pane.fill = False
        self.ax.zaxis.pane.fill = False
        self.ax.tick_params(colors='white')
        self.ax.xaxis.label.set_color('white')
        self.ax.yaxis.label.set_color('white')
        self.ax.zaxis.label.set_color('white')

        if perspective is not None:
            self.ax.view_init(elev=perspective['elev'], azim=perspective['azim'], roll=perspective['roll'])

    def plot_spacepoints(self, spacepoints, size=4, color='red'):
        xs = [spacepoint.x for spacepoint in spacepoints]
        ys = [spacepoint.y for spacepoint in spacepoints]
        zs = [spacepoint.z for spacepoint in spacepoints]
        self.ax.scatter3D(xs, zs, ys, c=color, s=size)

    def plot_particles(self, particles, size=1, color='green'):
        def get_far_point(particle):
            scale_x = (self.max_abs_xy - abs(particle.vx)) / max(abs(particle.direction_x), 1e-6)
            scale_y = (self.max_abs_xy - abs(particle.vy)) / max(abs(particle.direction_y), 1e-6)
            scale_z = (self.max_abs_z - abs(particle.vz)) / max(abs(particle.direction_z), 1e-6)
            scale = min(scale_x, scale_y, scale_z)
            return particle.vx + particle.direction_x * scale, particle.vy + particle.direction_y * scale, particle.vz + particle.direction_z * scale

        far_points = [get_far_point(particle) for particle in particles]
        line_xs = [[particle.vx, far_point[0]] for particle, far_point in zip(particles, far_points)]
        line_ys = [[particle.vy, far_point[1]] for particle, far_point in zip(particles, far_points)]
        line_zs = [[particle.vz, far_point[2]] for particle, far_point in zip(particles, far_points)]

        for line_x, line_y, line_z in zip(line_xs, line_ys, line_zs):
            self.ax.plot3D(line_x, line_z, line_y, c=color, linewidth=size)

    def plot_wedge(self, wedge, color='yellow'):
        outermost_points = wedge.get_outermost_points()
        interaction_region_zs = wedge.get_interaction_region_zs()

        xs = []
        xs.append([0, outermost_points['z_angle_min_x_angle_min']['x']])
        xs.append([0, outermost_points['z_angle_min_x_angle_max']['x']])
        xs.append([0, outermost_points['z_angle_max_x_angle_min']['x']])
        xs.append([0, outermost_points['z_angle_max_x_angle_max']['x']])
        xs.append([outermost_points['z_angle_min_x_angle_min']['x'], outermost_points['z_angle_min_x_angle_max']['x']])
        xs.append([outermost_points['z_angle_max_x_angle_min']['x'], outermost_points['z_angle_max_x_angle_max']['x']])
        xs.append([outermost_points['z_angle_min_x_angle_min']['x'], outermost_points['z_angle_max_x_angle_min']['x']])
        xs.append([outermost_points['z_angle_min_x_angle_max']['x'], outermost_points['z_angle_max_x_angle_max']['x']])

        ys = []
        ys.append([0, outermost_points['z_angle_min_x_angle_min']['y']])
        ys.append([0, outermost_points['z_angle_min_x_angle_max']['y']])
        ys.append([0, outermost_points['z_angle_max_x_angle_min']['y']])
        ys.append([0, outermost_points['z_angle_max_x_angle_max']['y']])
        ys.append([outermost_points['z_angle_min_x_angle_min']['y'], outermost_points['z_angle_min_x_angle_max']['y']])
        ys.append([outermost_points['z_angle_max_x_angle_min']['y'], outermost_points['z_angle_max_x_angle_max']['y']])
        ys.append([outermost_points['z_angle_min_x_angle_min']['y'], outermost_points['z_angle_max_x_angle_min']['y']])
        ys.append([outermost_points['z_angle_min_x_angle_max']['y'], outermost_points['z_angle_max_x_angle_max']['y']])

        zs = []
        zs.append([interaction_region_zs['x_angle_min'], outermost_points['z_angle_min_x_angle_min']['z']])
        zs.append([interaction_region_zs['x_angle_max'], outermost_points['z_angle_min_x_angle_max']['z']])
        zs.append([interaction_region_zs['x_angle_min'], outermost_points['z_angle_max_x_angle_min']['z']])
        zs.append([interaction_region_zs['x_angle_max'], outermost_points['z_angle_max_x_angle_max']['z']])
        zs.append([outermost_points['z_angle_min_x_angle_min']['z'], outermost_points['z_angle_min_x_angle_max']['z']])
        zs.append([outermost_points['z_angle_max_x_angle_min']['z'], outermost_points['z_angle_max_x_angle_max']['z']])
        zs.append([outermost_points['z_angle_min_x_angle_min']['z'], outermost_points['z_angle_max_x_angle_min']['z']])
        zs.append([outermost_points['z_angle_min_x_angle_max']['z'], outermost_points['z_angle_max_x_angle_max']['z']])

        for i in range(len(xs)):
            self.ax.plot(xs[i], zs[i], ys[i], color=color)

    def plot_pole_region(self, region, num_rays=16, color='yellow'):
        outermost_points = region.get_outermost_points(num_rays)
        interaction_region_z = region.get_interaction_region_z()

        xs = []
        ys = []
        zs = []
        for point in outermost_points:
            xs.append([0, point['x']])
            ys.append([0, point['y']])
            zs.append([interaction_region_z, point['z']])

        for i in range(len(outermost_points) - 1):
            xs.append([outermost_points[i]['x'], outermost_points[i + 1]['x']])
            ys.append([outermost_points[i]['y'], outermost_points[i + 1]['y']])
            zs.append([outermost_points[i]['z'], outermost_points[i + 1]['z']])

        xs.append([outermost_points[-1]['x'], outermost_points[0]['x']])
        ys.append([outermost_points[-1]['y'], outermost_points[0]['y']])
        zs.append([outermost_points[-1]['z'], outermost_points[0]['z']])

        for i in range(len(xs)):
            self.ax.plot(xs[i], zs[i], ys[i], color=color)