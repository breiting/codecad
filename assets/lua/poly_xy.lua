local plate = extrude(
	poly_xy({
		{ -30, 0 },
		{ 30,  0 },
		{ 0,   30 },
	}),
	5
)
plate = scale(plate, 0.5)
emit(plate)
