-- Transition from a 20×26 pipe (ID×OD) to a 12×16 pipe over 40 mm
local adapter = pipe_adapter(
	10,
	13, -- rIn0, rOut0 (ID=20 -> rIn0=10; OD=26 -> rOut0=13)
	6,
	8, -- rIn1, rOut1 (ID=12 -> rIn1=6; OD=16 -> rOut1=8)
	40, -- length
	8 -- steepness (optional, 4..12; higher = gentler blend)
)
emit(scale(adapter, 0.2))
