/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2022 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Mandelbulb fractal.
 * @reference http://www.fractalforums.com/3d-fractal-generation/true-3d-mandlebrot-type-fractal/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_mandelbulb_sin_cos_v2.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MandelbulbSinCosV2Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL temp;
	REAL th = z.z / aux->r;
	if (!fractal->transformCommon.functionEnabledBFalse)
	{
		if (!fractal->transformCommon.functionEnabledAFalse)
			th = asin(th);
		else
			th = acos(th);
	}
	else
	{
		REAL acth = acos(th);
		th = acth + (asin(th) - acth) * fractal->transformCommon.scale1;
	}
	REAL ph = atan2(z.y, z.x);

	th = (th + fractal->bulb.betaAngleOffset) * fractal->bulb.power;
	ph = (ph + fractal->bulb.alphaAngleOffset) * fractal->bulb.power;
	REAL rp = pow(aux->r, fractal->bulb.power - 1.0f);
	aux->DE = rp * aux->DE * fractal->bulb.power + 1.0f;
	rp *= aux->r;

	// polar to cartesian
	REAL cth = native_cos(th);
	REAL sth = native_sin(th);

	REAL4 trg = CVector4{0.0f, 0.0f, 0.0f, 0.0f};
	if (!fractal->transformCommon.functionEnabledFFalse)
	{
		trg.x = cth * native_cos(ph);
		trg.y = cth * native_sin(ph);
		trg.z = sth;
	}
	else
	{
		trg.x = sth * native_sin(ph);
		trg.y = sth * native_cos(ph);
		trg.z = cth;
	}

	if (fractal->transformCommon.functionEnabledAx
			&& aux->i >= fractal->transformCommon.startIterationsT
			&& aux->i < fractal->transformCommon.stopIterationsT)
	{
		if (!fractal->transformCommon.functionEnabledDFalse)
		{
			z = trg;
		}
		else
		{
			temp = trg.y;
			trg.y = trg.z;
			trg.z = temp;
			z = trg;
		}
	}

	if (fractal->transformCommon.functionEnabledGFalse
			&& aux->i >= fractal->transformCommon.startIterationsG
			&& aux->i < fractal->transformCommon.stopIterationsG)
	{
		z.x += (trg.x - z.x) * fractal->transformCommon.scaleC1;
		z.y += (trg.y - z.y) * fractal->transformCommon.scaleF1;
		if (!fractal->transformCommon.functionEnabledwFalse)
			z.z = sth;
		else
			z.z = cth;
	}

	if (fractal->transformCommon.functionEnabledJFalse
			&& aux->i >= fractal->transformCommon.startIterationsJ
			&& aux->i < fractal->transformCommon.stopIterationsJ)
	{
		z.x = native_cos(ph);
		z.y = native_sin(ph);
		z.z = cth;
		if (fractal->transformCommon.functionEnabledKFalse) z.x *= sth;
		if (fractal->transformCommon.functionEnabledMFalse) z.y *= sth;
		if (fractal->transformCommon.functionEnabledNFalse) z.z *= sth;
	}

	z *= rp;

	z += fractal->transformCommon.offsetA000;
	z += aux->const_c * fractal->transformCommon.constantMultiplierA111;
	z.z *= fractal->transformCommon.scaleA1;

	if (fractal->analyticDE.enabledFalse)
	{
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	}

	if (fractal->transformCommon.functionEnabledCFalse)
	{
		aux->DE0 = length(z);
		if (aux->DE0 > 1.0f)
			aux->DE0 = 0.5f * log(aux->DE0) * aux->DE0 / aux->DE;
		else
			aux->DE0 = 0.0f;

		if (aux->i >= fractal->transformCommon.startIterationsO
				&& aux->i < fractal->transformCommon.stopIterationsO)
			aux->dist = min(aux->dist, aux->DE0);
		else
			aux->dist = aux->DE0;
	}
	return z;
}