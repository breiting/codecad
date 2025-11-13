local cube = box(40, 40, 40)
local smooth = fillet_all(cube, 1)

smooth = scale(center_xy(smooth), 0.2)
emit(smooth)
