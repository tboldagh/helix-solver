import math


class PoleRegion:
    def __init__(self, x_angle, interaction_region_width, max_abs_z=3100, max_abs_xy=1100):
        self.x_angle = x_angle
        self.interaction_region_width = interaction_region_width
        self.max_abs_z = max_abs_z
        self.max_abs_xy = max_abs_xy

    def get_outermost_points(self, num_points):
        x_plane_direction_z = math.cos(self.x_angle)
        x_plane_direction_y = math.sin(self.x_angle)

        x_plane_scale_to_z_limit = abs(x_plane_direction_z) / self.max_abs_z
        x_plane_scale_to_y_limit = abs(x_plane_direction_y) / self.max_abs_xy
        scale = 1 / max(x_plane_scale_to_z_limit, x_plane_scale_to_y_limit)

        points = []
        for z_angle in [i / num_points * 2 * math.pi for i in range(num_points)]:
            points.append({
                'x': x_plane_direction_y * math.cos(z_angle) * scale,
                'y': x_plane_direction_y * math.sin(z_angle) * scale,
                'z': x_plane_direction_z * scale
            })
        return points
    
    def get_interaction_region_z(self):
        return (1 if self.x_angle > math.pi / 2 else -1) * self.interaction_region_width / 2