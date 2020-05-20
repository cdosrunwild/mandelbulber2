/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Menger Smooth
 * http://www.fractalforums.com/fragmentarium/help-t22583/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_menger_smooth.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MengerSmoothIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL sc1 = fractal->transformCommon.scale3 - 1.0f; // 3 - 1 = 2f, 2/3 = 0.6667f;
	REAL sc2 = sc1 / fractal->transformCommon.scale3;	 //  8 - 1 = 7f, 7/8 = 0.89ish;
	REAL OffsetS = fractal->transformCommon.offset0005;

	// the closer to origin the greater the effect of OffsetSQ
	z = (REAL4){native_sqrt(z.x * z.x + OffsetS), native_sqrt(z.y * z.y + OffsetS),
		native_sqrt(z.z * z.z + OffsetS), z.w};

	REAL t;
	REAL4 OffsetC = fractal->transformCommon.offset1105;

	t = z.x - z.y;
	t = 0.5f * (t - native_sqrt(t * t + OffsetS));
	z.x = z.x - t;
	z.y = z.y + t;

	t = z.x - z.z;
	t = 0.5f * (t - native_sqrt(t * t + OffsetS));
	z.x = z.x - t;
	z.z = z.z + t;

	t = z.y - z.z;
	t = 0.5f * (t - native_sqrt(t * t + OffsetS));
	z.y = z.y - t;
	z.z = z.z + t;

	z.z = z.z - OffsetC.z * sc2; // sc2 reduces C.z
	z.z = -native_sqrt(z.z * z.z + OffsetS);
	z.z = z.z + OffsetC.z * sc2;

	z.x = fractal->transformCommon.scale3 * z.x - OffsetC.x * sc1; // sc1 scales up C.x
	z.y = fractal->transformCommon.scale3 * z.y - OffsetC.y * sc1;
	z.z = fractal->transformCommon.scale3 * z.z;

	aux->DE *= fractal->transformCommon.scale3;

	if (fractal->transformCommon.rotationEnabled
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
	{
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	}

	if (fractal->transformCommon.functionEnabledzFalse)
	{
		REAL4 zA = (aux->i == fractal->transformCommon.intA) ? z : (REAL4){0, 0, 0, 0};
		REAL4 zB = (aux->i == fractal->transformCommon.intB) ? z : (REAL4){0, 0, 0, 0};
		z = (z * fractal->transformCommon.scale1) + (zA * fractal->transformCommon.offsetA0)
				+ (zB * fractal->transformCommon.offsetB0);
		aux->DE *= fractal->transformCommon.scale1;
	}
	return z;
}