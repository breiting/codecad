local cube = box(40, 40, 40)
local beveled = chamfer_all(cube, 2)
beveled = scale(center_xy(beveled), 0.2)

emit(beveled)
