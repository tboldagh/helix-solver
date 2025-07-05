class ParticleInitial:
    def __init__(self, event_id, particle_id, particle_type, process, vx, vy, vz, vt, px, py, pz, m, q, eta, phi, pt, p, vertex_primary_id, vertex_secondary_id, generation, sub_particle_id):
        self.event_id = event_id
        self.particle_id = particle_id
        self.particle_type = particle_type
        self.process = process
        self.vx = vx
        self.vy = vy
        self.vz = vz
        self.vt = vt
        self.direction_x = px / p
        self.direction_y = py / p
        self.direction_z = pz / p
        self.px = px
        self.py = py
        self.pz = pz
        self.m = m
        self.q = q
        self.eta = eta
        self.phi = phi
        self.pt = pt
        self.p = p
        self.vertex_primary_id = vertex_primary_id
        self.vertex_secondary_id = vertex_secondary_id
        self.generation = generation
        self.sub_particle_id = sub_particle_id

    def __str__(self):
        return f"ParticleInitial(event_id={self.event_id}, particle_id={self.particle_id}, particle_type={self.particle_type}, process={self.process}, vx={self.vx}, vy={self.vy}, vz={self.vz}, vt={self.vt}, px={self.px}, py={self.py}, pz={self.pz}, m={self.m}, q={self.q}, eta={self.eta}, phi={self.phi}, pt={self.pt}, p={self.p}, vertex_primary_id={self.vertex_primary_id}, vertex_secondary_id={self.vertex_secondary_id}, generation={self.generation}, sub_particle_id={self.sub_particle_id})"

    @staticmethod
    def read_from_csv(path):
        with open(path, 'r') as file:
            lines = file.readlines()

        lines_split = []
        for line in lines:
            lines_split.append([i.strip() for i in line.split(',')])

        particles = []
        for line in lines_split[1:]:
            event_id = int(line[0])
            particle_id = int(line[1])
            particle_type = int(line[2])
            process = int(line[3])
            vx = float(line[4])
            vy = float(line[5])
            vz = float(line[6])
            vt = float(line[7])
            px = float(line[8])
            py = float(line[9])
            pz = float(line[10])
            m = float(line[11])
            q = float(line[12])
            eta = float(line[13])
            phi = float(line[14])
            pt = float(line[15])
            p = float(line[16])
            vertex_primary_id = int(line[17])
            vertex_secondary_id = int(line[18])
            generation = int(line[19])
            sub_particle_id = int(line[20])
            particles.append(ParticleInitial(event_id, particle_id, particle_type, process, vx, vy, vz, vt, px, py, pz, m, q, eta, phi, pt, p, vertex_primary_id, vertex_secondary_id, generation, sub_particle_id))

        return particles
    