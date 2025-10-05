import math


class Wedge:
    def __init__(self, z_angle_min, z_angle_max, x_angle_min, x_angle_max, interaction_region_width, max_abs_z=3100, max_abs_xy=1100):
        self.z_angle_min = z_angle_min
        self.z_angle_max = z_angle_max
        self.x_angle_min = x_angle_min
        self.x_angle_max = x_angle_max
        self.interaction_region_width = interaction_region_width
        self.max_abs_z = max_abs_z
        self.max_abs_xy = max_abs_xy

    def get_outermost_points(self):
        def angles_to_xyz(z_angle, x_angle):
            x_plane_direction_z = math.cos(x_angle)
            x_plane_direction_y = math.sin(x_angle)

            x_plane_scale_to_z_limit = abs(x_plane_direction_z) / self.max_abs_z
            x_plane_scale_to_y_limit = abs(x_plane_direction_y) / self.max_abs_xy
            scale = 1 / max(x_plane_scale_to_z_limit, x_plane_scale_to_y_limit)
            
            return {
                'x': x_plane_direction_y * math.cos(z_angle) * scale,
                'y': x_plane_direction_y * math.sin(z_angle) * scale,
                'z': x_plane_direction_z * scale
            }

        return {
            'z_angle_min_x_angle_min': angles_to_xyz(self.z_angle_min, self.x_angle_min),
            'z_angle_min_x_angle_max': angles_to_xyz(self.z_angle_min, self.x_angle_max),
            'z_angle_max_x_angle_min': angles_to_xyz(self.z_angle_max, self.x_angle_min),
            'z_angle_max_x_angle_max': angles_to_xyz(self.z_angle_max, self.x_angle_max)
        }
    
    def get_interaction_region_zs(self):
        return {
            'x_angle_min': (1 if self.x_angle_min <= math.pi else -1) * self.interaction_region_width / 2,
            'x_angle_max': (-1 if self.x_angle_max <= math.pi else 1) * self.interaction_region_width / 2,
        }