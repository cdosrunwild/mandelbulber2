/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Classic Mandelbulb fractal.
 * @reference http://www.fractalforums.com/3d-fractal-generation/true-3d-mandlebrot-type-fractal/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_mandelbulb_plus_z.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MandelbulbPlusZIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 zTmp = (REAL4){0.0f, 0.0f, 0.0f, 0.0f};
	if (fractal->transformCommon.functionEnabledFalse)
	{
		if (fractal->transformCommon.functionEnabledAxFalse) z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledAyFalse) z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAzFalse) z.z = fabs(z.z);
	}

	if (aux->i == fractal->transformCommon.startIterations) aux->c = zTmp;

	if (aux->i >= fractal->transformCommon.startIterationsA) zTmp = z;

	REAL theta = (asin(z.z / aux->r) + fractal->bulb.betaAngleOffset) * fractal->transformCommon.int2;
	REAL phi = (atan2(z.y, z.x) + fractal->bulb.alphaAngleOffset) * fractal->transformCommon.int2;
	REAL rp = pow(aux->r, fractal->transformCommon.int2 - 1.0f);
	aux->DE = rp * aux->DE * fractal->transformCommon.int2 + 1.0f;

	rp *= aux->r;

	if (!fractal->transformCommon.functionEnabledByFalse)
	{
		REAL sinth = native_sin(theta);
		z = rp * (REAL4){sinth * native_cos(phi), native_sin(phi) * sinth, native_cos(theta), 0.0f};
	}
	else
	{
		REAL costh = native_cos(theta);
		z = rp * (REAL4){costh * native_sin(phi), native_cos(phi) * costh, native_sin(theta), 0.0f};
	}
	z += aux->c * fractal->transformCommon.constantMultiplierC111;

	aux->c = zTmp;

	// offset or juliaC
	if (aux->i >= fractal->transformCommon.startIterationsG
			&& aux->i < fractal->transformCommon.stopIterationsG)
	{
		z += fractal->transformCommon.offset000;
	}

	// z.z scale
	z.z *= fractal->transformCommon.scale1;

	// Analytic DE tweak
	if (fractal->analyticDE.enabledFalse)
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	return z;
}