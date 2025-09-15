local block = box(50, 40, 20)

-- Select edges lying on the top face (zmax)
local top_edges = edges(block):on_box_side("zmax"):collect()

-- Apply chamfer to those edges
local beveled = chamfer(block, top_edges, 3)
beveled = scale(center_xy(beveled), 0.2)

emit(beveled)
