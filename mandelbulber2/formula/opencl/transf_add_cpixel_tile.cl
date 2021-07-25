/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * spherical fold MBox
 * This formula contains aux.color

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_transf_spherical_fold.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfAddCpixelTileIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 cv = aux->const_c;
	if (aux->i == fractal->transformCommon.startIterations)
	{
		REAL4 p = aux->const_c;
		REAL4 rs = 1.0f / fractal->transformCommon.scale3D444;
		cv.x = round(p.x * rs.x);
		cv.y = round(p.y * rs.y);
		cv.z = round(p.z * rs.z);
		z = (p * rs - cv) * fractal->transformCommon.scale3D444;

		cv = cv * fractal->transformCommon.constantMultiplier111
				+ fractal->transformCommon.offset000;

		if (fractal->transformCommon.functionEnabledBFalse)
		{
			cv.x *= sign(aux->const_c.x);
			cv.y *= sign(aux->const_c.y);
			cv.z *= sign(aux->const_c.z);
		}
		aux->c = cv;
	}

	if (aux->i >= fractal->transformCommon.startIterationsA)
	{
		cv = aux->c;
		if (fractal->transformCommon.functionEnabledCFalse)
		{
			cv.x *= sign(z.x);
			cv.y *= sign(z.y);
			cv.z *= sign(z.z);
		}
		z += cv;
		aux->c = cv;
	}

	// Analytic DE tweak
	if (fractal->analyticDE.enabledFalse)
			aux->DE = aux->DE * fractal->analyticDE.scale1
								+ fractal->analyticDE.offset0;

	// aux->color
	if (fractal->foldColor.auxColorEnabledFalse)
	{
		aux->color += fabs(cv.x * cv.y) * fractal->foldColor.difs0000.x;
		aux->color += (cv.x * cv.x + cv.y * cv.y) * fractal->foldColor.difs0000.y;
	}

	return z;
}
