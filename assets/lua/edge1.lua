local base = box(40, 30, 20)

-- Select edges parallel to Z
local vertical_edges = edges(base):parallel("z"):collect()

-- Apply fillet to those edges only
local softened = fillet(base, vertical_edges, 2)

softened = scale(center_xy(softened), 0.2)

emit(softened)
