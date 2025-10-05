class Solution:
    def __init__(self, event_id, index, particle_id, q, r, phi, hit_count, x_angle_min, x_angle_max):
        self.event_id = event_id
        self.index = index
        self.particle_id = particle_id
        self.q = q
        self.r = r
        self.phi = phi
        self.hit_count = hit_count
        self.x_angle_min = x_angle_min
        self.x_angle_max = x_angle_max

    def __str__(self):
        return f"Solution(event_id={self.event_id}, index={self.index}, particle_id={self.particle_id}, q={self.q}, r={self.r}, phi={self.phi}, hit_count={self.hit_count}, x_angle_min={self.x_angle_min}, x_angle_max={self.x_angle_max})"

    @staticmethod
    def read_from_csv(path):
        with open(path, 'r') as file:
            lines = file.readlines()
        
        solutions = []
        for line in lines[1:]:
            line = [i.strip() for i in line.split(",")]
            event_id = int(line[0])
            index = int(line[1])
            q = float(line[2])
            r = float(line[3])
            phi = float(line[4])
            hit_count = int(line[5])
            x_angle_min = float(line[6])
            x_angle_max = float(line[7])
            particle_id = int(line[8]) if line[8] != "null" else None
            solutions.append(Solution(event_id, index, particle_id, q, r, phi, hit_count, x_angle_min, x_angle_max))

        return solutions