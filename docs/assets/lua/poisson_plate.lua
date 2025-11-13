local spec = PoissonDiskSpec.new()
spec.width = 1000
spec.height = 500
spec.margin = 20
spec.target_points = 100
spec.min_gap = 20
spec.r_min = 10
spec.r_max = 50
spec.seed = 42
spec.density_falloff = 1.0
spec.radius_falloff = 1.0

local plate = poisson_plate(spec, 5)
emit(plate)
