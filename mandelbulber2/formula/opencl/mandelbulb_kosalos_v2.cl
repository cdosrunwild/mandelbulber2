/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * based on formula by kosalos
 * https://fractalforums.org/fractal-mathematics-and-new-theories/28
 * /julia-sets-and-altering-the-iterate-afterwards/2871/msg16342#msg16342
 * This formula contains aux.const_c, and uses aux.c for oldZ

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_mandelbulb_kosalos_v2.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MandelbulbKosalosV2Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 c = aux->const_c;
	REAL power = fractal->bulb.power;

	REAL4 diffVec = (REAL4){0.001f, 0.001f, 0.001f, 0.0f} + aux->c - z;
	aux->c = z;
	REAL diffLen = length(diffVec); // > 3.16e-5f
	REAL thetaTweak = fractal->transformCommon.scaleA1 * 0.01f
										/ (diffLen + fractal->transformCommon.offsetA0 * 0.01f);

	if (!fractal->transformCommon.functionEnabledxFalse)
	{
		thetaTweak = (1.0f - thetaTweak);
	}
	else // mode2
	{
		thetaTweak = thetaTweak + (1.0f - thetaTweak) * fractal->transformCommon.scaleB1;
	}

	REAL theta;
	if (!fractal->transformCommon.functionEnabledAzFalse)
	{
		REAL xyL = native_sqrt(z.x * z.x + z.y * z.y);
		theta = atan2(xyL * thetaTweak, z.z);
	}
	else
	{
		theta = asin(z.z / aux->r * thetaTweak);
	}
	theta = (theta + fractal->bulb.betaAngleOffset) * power;

	REAL phi = atan2(z.y, z.x) * power;
	REAL pwr = pow(aux->r, power);

	if (!fractal->transformCommon.addCpixelEnabledFalse) // z = old z + new z
	{
		if (!fractal->transformCommon.functionEnabledzFalse)
		{
			REAL ss = native_sin(theta) * pwr;
			z.x += ss * native_cos(phi);
			z.y += ss * native_sin(phi);
			z.z += pwr * native_cos(theta);
		}
		else
		{
			REAL cs = native_cos(theta) * pwr;
			z.x += cs * native_sin(phi);
			z.y += cs * native_cos(phi);
			z.z += pwr * native_sin(theta);
		}
		aux->DE += (pow(aux->r, power - 1.0f) * power * aux->DE);
		z.z += fractal->transformCommon.offset0;
	}
	else // z = f(z) + c
	{
		if (!fractal->transformCommon.functionEnabledzFalse)
		{
			REAL ss = native_sin(theta * power) * pwr;
			z.x = ss * native_cos(phi);
			z.y = ss * native_sin(phi);
			z.z = pwr * native_cos(theta * power);
		}
		else
		{
			REAL cs = native_cos(theta * power) * pwr;
			z.x += cs * native_sin(phi);
			z.y += cs * native_cos(phi);
			z.z += pwr * native_sin(theta * power);
		}
		aux->DE = (pow(aux->r, power - 1.0f) * power * aux->DE);
		z.z += fractal->transformCommon.offset0;
		c *= fractal->transformCommon.constantMultiplierC111;
		if (!fractal->transformCommon.functionEnabledyFalse)
			z += c;
		else
			z += (REAL4){c.y, c.x, c.z, 0.0f};
	}
	aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset1;
	return z;
}