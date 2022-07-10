/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * mandelbulb vary scaleV1

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_mandelbulb_vary_power_v1.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MandelbulbVaryPowerV1Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL tempVC = fractal->bulb.power; // constant to be varied

	if (aux->i >= fractal->transformCommon.startIterations250
			&& aux->i < fractal->transformCommon.stopIterations
			&& (fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250
					!= 0))
	{
		int iterationRange =
			fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250;
		int currentIteration = (aux->i - fractal->transformCommon.startIterations250);
		tempVC += fractal->transformCommon.offset0 * (1.0f * currentIteration) / iterationRange;
	}
	if (aux->i >= fractal->transformCommon.stopIterations)
	{
		tempVC = (tempVC + fractal->transformCommon.offset0);
	}

	// if (aux->r < 1e-21f)
	//	aux->r = 1e-21f;
	REAL th0 = asin(z.z / aux->r) + fractal->bulb.betaAngleOffset;
	REAL ph0 = atan2(z.y, z.x) + fractal->bulb.alphaAngleOffset;
	REAL rp = pow(aux->r, tempVC - 1.0f);
	REAL th = th0 * tempVC;
	REAL ph = ph0 * tempVC;
	REAL cth = native_cos(th);
	aux->DE = rp * aux->DE * tempVC + 1.0f;
	rp *= aux->r;
	z = (REAL4){cth * native_cos(ph), cth * native_sin(ph), native_sin(th), 0.0f} * rp;
	return z;
}