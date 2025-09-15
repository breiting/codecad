-- M20-ish printable rod: 60 mm long, 45 mm threaded section
local ts = ThreadSpec.new()
ts.fitDiameter = 20.0
ts.pitch = 2.0
ts.depth = 0.9
ts.clearance = 0.20
ts.segmentsPerTurn = 48
ts.handed = "right"
ts.tip = "cut"
ts.tipCutRatio = 0.4
ts:normalize()

local tr, major = threaded_rod(15, 10, ts)
emit(tr)
