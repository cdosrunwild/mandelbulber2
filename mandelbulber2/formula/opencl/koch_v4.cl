/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * KochV4Iteration
 * Based on Knighty's Kaleidoscopic IFS 3D Fractals, described here:
 * http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_koch_v2.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 KochV4Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 zc = z;
	if (fractal->transformCommon.functionEnabledAx) zc.x = fabs(zc.x);
	if (fractal->transformCommon.functionEnabledAy) zc.y = fabs(zc.y);
	if (fractal->transformCommon.functionEnabledAzFalse) zc.z = fabs(zc.z);
	if (fractal->transformCommon.functionEnabledCx)
		if (zc.y > zc.x)
		{
			REAL temp = zc.x;
			zc.x = zc.y;
			zc.y = temp;
		}

	// folds
	if (fractal->transformCommon.functionEnabledFalse)
	{
		// diagonal2
		if (fractal->transformCommon.functionEnabledCxFalse)
			if (zc.x > zc.y)
			{
				REAL temp = zc.x;
				zc.x = zc.y;
				zc.y = temp;
			}
		// polyfold
		if (fractal->transformCommon.functionEnabledPFalse)
		{
			zc.x = fabs(zc.x);
			REAL psi = M_PI_F / fractal->transformCommon.int6;
			psi = fabs(fmod(atan(zc.y / zc.x) + psi, 2.0f * psi) - psi);
			REAL len = native_sqrt(zc.x * zc.x + zc.y * zc.y);
			zc.x = native_cos(psi) * len;
			zc.y = native_sin(psi) * len;
		}
		// abs offsets
		if (fractal->transformCommon.functionEnabledCFalse)
		{
			REAL xOffset = fractal->transformCommon.offsetC0;
			if (zc.x < xOffset) zc.x = fabs(zc.x - xOffset) + xOffset;
		}
		if (fractal->transformCommon.functionEnabledDFalse)
		{
			REAL yOffset = fractal->transformCommon.offsetD0;
			if (zc.y < yOffset) zc.y = fabs(zc.y - yOffset) + yOffset;
		}
	}

	REAL YOff = FRAC_1_3_F * fractal->transformCommon.scale1;
	zc.y = YOff - fabs(zc.y - YOff);

	zc.x += FRAC_1_3_F;
	if (zc.zc > zc.x)
	{
		REAL temp = zc.x;
		zc.x = zc.z;
		zc.z = temp;
	}
	zc.x -= FRAC_1_3_F;

	zc.x -= FRAC_1_3_F;
	if (zc.z > zc.x)
	{
		REAL temp = zc.x;
		zc.x = zc.z;
		zc.z = temp;
	}
	zc.x += FRAC_1_3_F;

	REAL4 Offset = fractal->transformCommon.offset100;
	zc = fractal->transformCommon.scale3 * (zc - Offset) + Offset;
	aux->DE = aux->DE * fractal->transformCommon.scale3;

	// rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
	{
		zc = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	}
	zc += fractal->transformCommon.offset000;

	// aux.dist
	REAL4 c = aux.const_c;
	REAL e = fractal->transformCommon.offset2;
	REAL d = 10000.0;

	// clip
	if (!fractal->transformCommon.functionEnabledEFalse)
	{
		REAL4 f = fabs(c) - (REAL4){e, e, e, 0.0f};
		if (!fractal->transformCommon.functionEnabledIFalse)
			e = max(f.x, f.y); // sq
		else
			e = max(f.x, max(f.y, f.z)); // box
	}
	else
	{
		if (!fractal->transformCommon.functionEnabledIFalse)
			e = clamp(sqrt(c.x * c.x + c.y * c.y) - e, 0.0, 100.0); // circle
		else
			e = clamp(length(c) - e, 0.0f, 100.0f); // sphere
	}

	// plane
	REAL g = fabs(zc.z - fractal->transformCommon.offsetR0) - fractal->transformCommon.offsetF0;
	g = max(g, e);


	REAL a = fractal->transformCommon.offsetA0;
	if (!fractal->transformCommon.functionEnabledFFalse)
	{
		REAL4 b = fabs(zc) - (REAL4){a, a, a, 0.0f};
		d = max(b.x, max(b.y, b.z));
	}
	else
	{
		d = fabs(length(zc) - length(Offset) - a);
	}
	d = max(d, e);
	g = min(g, d) / aux->DE;
	aux->dist = min(g, aux->dist);

	// aux->color
	if (fractal->foldColor.auxColorEnabled)
	{
		if (aux->dist == g)
			aux->color = fractal->foldColor.difs0000.x;
		else
		{
			double addColor = fractal->foldColor.difs0000.y * fabs(zc.x)
					+ fractal->foldColor.difs0000.z * fabs(zc.z)
					+ fractal->foldColor.difs0000.w * d;
			if (!fractal->transformCommon.functionEnabledJFalse)
				aux=>color = addColor;
			else
				aux->color += addColor;
		}
	}


	z = zc;
	return z;
}
