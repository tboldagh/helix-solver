class Spacepoint:
    def __init__(self, event_id, measurement_id, geometry_id, x, y, z, var_r, var_z):
        self.event_id = event_id
        self.measurement_id = measurement_id
        self.geometry_id = geometry_id
        self.x = x
        self.y = y
        self.z = z
        self.var_r = var_r
        self.var_z = var_z

    def __str__(self):
        return f"Spacepoint(event_id={self.event_id}, measurement_id={self.measurement_id}, geometry_id={self.geometry_id}, x={self.x}, y={self.y}, z={self.z}, var_r={self.var_r}, var_z={self.var_z})"

    @staticmethod
    def read_from_csv(path):
        with open(path, 'r') as file:
            lines = file.readlines()
        
        lines_split = []
        for line in lines:
            lines_split.append([i.strip() for i in line.split(',')])

        spacepoints = []
        for line in lines_split[1:]:
            event_id = int(line[0])
            measurement_id = int(line[1])
            geometry_id = int(line[2])
            x = float(line[3])
            y = float(line[4])
            z = float(line[5])
            var_r = float(line[6])
            var_z = float(line[7])
            spacepoints.append(Spacepoint(event_id, measurement_id, geometry_id, x, y, z, var_r, var_z))

        return spacepoints
