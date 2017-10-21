/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * scale variation v1. Scale varies based on iteration parameters.

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "TransfScaleVaryV1Iteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfScaleVaryV1Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL tempVC = fractal->transformCommon.scale; // constant to be varied

	if (aux->i >= fractal->transformCommon.startIterations250
			&& aux->i < fractal->transformCommon.stopIterations
			&& (fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250
					 != 0))
	{
		int iterationRange =
			fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250;
		int currentIteration = (aux->i - fractal->transformCommon.startIterations250);
		tempVC +=
			fractal->transformCommon.offset0 * native_divide((1.0f * currentIteration), iterationRange);
	}
	if (aux->i >= fractal->transformCommon.stopIterations)
	{
		tempVC = (tempVC + fractal->transformCommon.offset0);
	}
	z *= tempVC;
	aux->DE = mad(aux->DE, fabs(tempVC), 1.0f); // prepared for future analytic DE for hybrids
	aux->r_dz *= fabs(tempVC);
	return z;
}