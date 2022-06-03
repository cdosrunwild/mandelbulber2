/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2021 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * sin and cos

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_transf_sincos.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfSincosV2Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	if (fractal->transformCommon.functionEnabledDFalse
			&& aux->i >= fractal->transformCommon.startIterationsD
			&& aux->i < fractal->transformCommon.stopIterationsD1)
	{
		if (fractal->transformCommon.functionEnabledAx) z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledAy) z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAzFalse) z.z = fabs(z.z);
	}

	if (fractal->transformCommon.rotationEnabledFalse
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR1)
	{
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	}

	REAL ang = atan2(z.y, z.x) / M_PI_2x_F;

	if (fractal->transformCommon.functionEnabledAFalse)
	{	REAL Voff = fractal->transformCommon.scale2;
		REAL g = z.z - 2.0f * Voff * ang + Voff;
		z.z = g - 2.0f * Voff * floor(g / (2.0f * Voff)) - Voff;
	}

	z.y = sqrt(z.x * z.x + z.y * z.y) - fractal->transformCommon.radius1;
	if (fractal->transformCommon.functionEnabledM)
	{
		z.x = (fractal->transformCommon.scaleA2 * ang + 1.0f)
				- 2.0f * floor((fractal->transformCommon.scaleA2 * ang + 1.0f) / 2.0f) - 1.0f;
	}
	ang = fractal->transformCommon.int6 * M_PI_2_F * ang;
	REAL cosA = native_cos(ang);
	REAL sinB = native_sin(ang);
	REAL temp = z.z;
	z.z = z.y * cosA + z.z * sinB;
	z.y = temp * cosA + z.y * -sinB;
	if (fractal->transformCommon.functionEnabledFalse)
	{
		z = fractal->transformCommon.offset000 - fabs(z);
		//z += fractal->transformCommon.offset000;
	}

	if (fractal->analyticDE.enabledFalse)
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset1;

	/*
	// aux->color
	if (aux->i >= fractal->foldColor.startIterationsA && aux->i < fractal->foldColor.stopIterationsA)
	{
		REAL addColor = 0.0f;
		if (aux->dist == colDist) addColor += fractal->foldColor.difs0000.x;
		if (aux->dist != colDist) addColor += fractal->foldColor.difs0000.y;
		aux->color += addColor;
	}*/
	return z;

}