/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * spherical invert ( v2.17)
 * from M3D. Formula by Luca GN 2011, updated May 2012.
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_transf_spherical_fold_v2.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfSphericalFoldV3Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 signs = z;
	signs.x = sign(z.x);
	signs.y = sign(z.y);
	signs.z = sign(z.z);
	signs.w = sign(z.w);

	REAL4 out = (REAL4){0.0, 0.0, 0.0, 0.0};
	REAL4 in = out;
	if (aux->i >= fractal->transformCommon.startIterationsX
			&& aux->i < fractal->transformCommon.stopIterationsX)
	{
		out = fractal->transformCommon.offset000;
		if (fractal->transformCommon.functionEnabledAzFalse)
			out *= signs;
	}
	if (aux->i >= fractal->transformCommon.startIterationsY
			&& aux->i < fractal->transformCommon.stopIterationsY)
	{
		in = fractal->transformCommon.offsetF000;
		if (fractal->transformCommon.functionEnabledBzFalse)
			in *= signs;
	}
	if (fractal->transformCommon.functionEnabledAx) z -= out;

	REAL m = 0.0f;
	REAL rr = dot(z, z);

	if (fractal->transformCommon.functionEnabledByFalse) z -= in;

	if (rr < fractal->transformCommon.minR2p25)
	{
		m = fractal->transformCommon.maxR2d1 / fractal->transformCommon.minR2p25;
		z *= m;
		aux->DE *= m;
	}
	else if (rr < fractal->transformCommon.maxR2d1)
	{
		m = fractal->transformCommon.maxR2d1 / rr;
		z *= m;
		aux->DE *= m;
	}

	if (fractal->transformCommon.functionEnabledAy) z += out;
	if (fractal->transformCommon.functionEnabledByFalse) z += in;

	if (fractal->analyticDE.enabledFalse)
	{
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	}
	return z;
}
