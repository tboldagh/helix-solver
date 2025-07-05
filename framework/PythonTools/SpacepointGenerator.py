from ParticleInitial import *
from Spacepoint import *
from MathFunctions import *
import math


def generate_spacepoints(x_angle=None, z_angle=None, interaction_z=None, r=None, particle_initial=None, counter_clockwise=True, num_points=10, max_abs_z=3100, max_abs_xy=1100):
    def direction_to_angles(dir_x, dir_y, dir_z):
        x_angle = math.atan2(math.sqrt(dir_y ** 2 + dir_x ** 2), dir_z)
        z_angle = math.atan2(dir_y, dir_x)
        return x_angle, z_angle
    
    if x_angle is None:
        x_angle, z_angle = direction_to_angles(particle_initial.direction_x, particle_initial.direction_y, particle_initial.direction_z)
        interaction_z = particle_initial.vz
        
    phi = z_angle + 0.5 * math.pi
    bend_direction = 1 if counter_clockwise else -1

    direction_z = math.cos(x_angle)
    direction_xy = math.sin(x_angle)

    scale = min(abs(max_abs_xy / max(abs(direction_xy), 1e-6)), abs((max_abs_z - interaction_z) / max(abs(direction_z), 1e-6)))
    far_z = direction_z * scale + interaction_z
    far_xy = direction_xy * scale
    far_alpha = far_xy / r * bend_direction

    center_x = r * math.cos(phi) * bend_direction
    center_y = r * math.sin(phi) * bend_direction

    spacepoints = []
    event_id = particle_initial.event_id if particle_initial is not None else None
    measurement_id = None
    geometry_id = None
    var_r = None
    var_z = None
    for i in range(num_points):
        t = (i + 1) / num_points

        alpha = lerp(0, far_alpha, t)
        z = lerp(interaction_z, far_z, t)

        x = (-center_x * math.cos(alpha) - (-center_y * math.sin(alpha))) + center_x
        y = -center_y * math.cos(alpha) + (-center_x * math.sin(alpha)) + center_y

        spacepoints.append(Spacepoint(event_id, measurement_id, geometry_id, x, y, z, var_r, var_z))

    return spacepoints