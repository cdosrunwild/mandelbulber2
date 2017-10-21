/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * rotation folding plane

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "TransfRotationFoldingPlaneIteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfRotationFoldingPlaneIteration(
	REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 zRot;
	// cast vector to array pointer for address taking of components in opencl
	REAL *zRotP = (REAL *)&zRot;
	__constant REAL *colP = (__constant REAL *)&fractal->mandelbox.color.factor;
	for (int dim = 0; dim < 3; dim++)
	{
		// handle each dimension x, y and z sequentially in pointer var dim
		REAL *rotDim = (dim == 0) ? &zRotP[0] : ((dim == 1) ? &zRotP[1] : &zRotP[2]);
		__constant REAL *colorFactor = (dim == 0) ? &colP[0] : ((dim == 1) ? &colP[1] : &colP[2]);

		zRot = Matrix33MulFloat4(fractal->mandelbox.rot[0][dim], z);
		if (*rotDim > fractal->mandelbox.foldingLimit)
		{
			*rotDim = fractal->mandelbox.foldingValue - *rotDim;
			z = Matrix33MulFloat4(fractal->mandelbox.rotinv[0][dim], zRot);
			aux->color += *colorFactor;
		}
		else
		{
			zRot = Matrix33MulFloat4(fractal->mandelbox.rot[1][dim], z);
			if (*rotDim < -fractal->mandelbox.foldingLimit)
			{
				*rotDim = -fractal->mandelbox.foldingValue - *rotDim;
				z = Matrix33MulFloat4(fractal->mandelbox.rotinv[1][dim], zRot);
				aux->color += *colorFactor;
			}
		}
	}
	return z;
}