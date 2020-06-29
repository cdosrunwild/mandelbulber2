/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * based on formula by pupukuusikko
 * @reference http://www.fractalforums.com/the-3d-mandelbulb/a-new-3d-mandelbrot-variant-mandelcup/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_mandelcup.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MandelcupIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 c = aux->const_c;
	if (fractal->transformCommon.functionEnabledNFalse)
	{
		if (fractal->transformCommon.functionEnabledAxFalse
				&& aux->i >= fractal->transformCommon.startIterationsX
				&& aux->i < fractal->transformCommon.stopIterationsX)
			z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledAyFalse
				&& aux->i >= fractal->transformCommon.startIterationsY
				&& aux->i < fractal->transformCommon.stopIterationsY)
			z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAzFalse
				&& aux->i >= fractal->transformCommon.startIterationsZ
				&& aux->i < fractal->transformCommon.stopIterationsZ)
			z.z = fabs(z.z);
	}

	REAL r = aux->r;
	aux->DE = r * 2.0f * aux->DE + 1.0f;
	REAL Scale = fractal->transformCommon.scale1;
	REAL Shape = fractal->transformCommon.offset0;
	REAL temp = 1.0f;

	// stereographic projection, modified a bit
	if (fractal->transformCommon.functionEnabledx)
	{
		z.x /= r;
		z.y /= r;
	}
	if (fractal->transformCommon.functionEnabledz) z.z = (z.z / r) + Shape;

	if (fractal->transformCommon.functionEnabledBFalse)
	{
		z.z *= z.z;
		Shape *= Shape;
	}
	if (fractal->transformCommon.functionEnabledy)
	{
		z.x /= z.z;
		z.y /= z.z;
	}
	if (fractal->transformCommon.functionEnabledDFalse)
	{
		temp = 1.0f / (z.z * z.z * 1.0f);
		z.x *= temp;
		z.y *= temp;
	}
	if (fractal->transformCommon.functionEnabledEFalse)
	{
		z.x *= z.z;
		z.y *= z.z;
	}

	// complex multiplication
	temp = z.x * z.x - z.y * z.y;
	z.y = 2.0f * z.x * z.y;
	z.x = temp;
	temp = Scale * (1.0f + (Shape * Shape));

	z.x *= temp;
	z.y *= temp;

	// inverse stereographic
	REAL mag1 = z.x * z.x + z.y * z.y - 1.0f;
	REAL mag2 = 1.0f / (mag1 + 2.0f);
	z.x = z.x * 2.0f * mag2;
	z.y = z.y * 2.0f * mag2;
	z.z = mag1 * mag2;

	z *= r * r;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		REAL4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux->c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZCl_xyz:
				default: tempC = (REAL4){tempC.x, tempC.y, tempC.z, tempC.w}; break;
				case multi_OrderOfXYZCl_xzy: tempC = (REAL4){tempC.x, tempC.z, tempC.y, tempC.w}; break;
				case multi_OrderOfXYZCl_yxz: tempC = (REAL4){tempC.y, tempC.x, tempC.z, tempC.w}; break;
				case multi_OrderOfXYZCl_yzx: tempC = (REAL4){tempC.y, tempC.z, tempC.x, tempC.w}; break;
				case multi_OrderOfXYZCl_zxy: tempC = (REAL4){tempC.z, tempC.x, tempC.y, tempC.w}; break;
				case multi_OrderOfXYZCl_zyx: tempC = (REAL4){tempC.z, tempC.y, tempC.x, tempC.w}; break;
			}
			aux->c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZCl_xyz:
				default: tempC = (REAL4){c.x, c.y, c.z, c.w}; break;
				case multi_OrderOfXYZCl_xzy: tempC = (REAL4){c.x, c.z, c.y, c.w}; break;
				case multi_OrderOfXYZCl_yxz: tempC = (REAL4){c.y, c.x, c.z, c.w}; break;
				case multi_OrderOfXYZCl_yzx: tempC = (REAL4){c.y, c.z, c.x, c.w}; break;
				case multi_OrderOfXYZCl_zxy: tempC = (REAL4){c.z, c.x, c.y, c.w}; break;
				case multi_OrderOfXYZCl_zyx: tempC = (REAL4){c.z, c.y, c.x, c.w}; break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplierC111;
	}

	z += fractal->transformCommon.additionConstant000;

	if (fractal->analyticDE.enabledFalse)
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	return z;
}