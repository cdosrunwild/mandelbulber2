/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Reciprocal3  based on Darkbeam's code from M3D,
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "TransfReciprocal3Iteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfReciprocal3Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 tempZ = z;

	if (fractal->transformCommon.functionEnabledx)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.x = (native_recip(fractal->transformCommon.offset111.x))
								- native_recip((fabs(z.x) + fractal->transformCommon.offset111.x));

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.x = (fractal->transformCommon.offsetA111.x)
								- native_recip((fabs(z.x) + fractal->transformCommon.offset111.x));

		if (fractal->transformCommon.functionEnabledBxFalse)
		{
			REAL M1 = fractal->transformCommon.scale1;
			REAL M2 = fractal->transformCommon.scaleA1;
			tempZ.x = (native_recip(fractal->transformCommon.offset111.x))
								+ (native_recip(fractal->transformCommon.offsetA111.x))
								- native_recip((fabs(z.x * M1) + fractal->transformCommon.offset111.x))
								- native_recip(((z.x * z.x * M2) + fractal->transformCommon.offsetA111.x));
		}
		if (fractal->transformCommon.functionEnabledCxFalse)
		{
			REAL M1 = fractal->transformCommon.scale1;
			REAL M2 = fractal->transformCommon.scaleA1;
			tempZ.x = fractal->transformCommon.offsetB111.x
								- native_recip((fabs(z.x * M1) + fractal->transformCommon.offset111.x))
								- native_recip(((z.x * z.x * M2) + fractal->transformCommon.offsetA111.x));
		}

		tempZ.x += fabs(z.x) * fractal->transformCommon.offset000.x; // function slope
		z.x = sign(z.x) * tempZ.x;
	}

	if (fractal->transformCommon.functionEnabledy)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.y = (native_recip(fractal->transformCommon.offset111.y))
								- native_recip((fabs(z.y) + fractal->transformCommon.offset111.y));

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.y = (fractal->transformCommon.offsetA111.y)
								- native_recip((fabs(z.y) + fractal->transformCommon.offset111.y));

		if (fractal->transformCommon.functionEnabledBxFalse)
		{
			REAL M1 = fractal->transformCommon.scale1;
			REAL M2 = fractal->transformCommon.scaleA1;
			tempZ.y = (native_recip(fractal->transformCommon.offset111.y))
								+ (native_recip(fractal->transformCommon.offsetA111.y))
								- native_recip((fabs(z.y * M1) + fractal->transformCommon.offset111.y))
								- native_recip(((z.y * z.y * M2) + fractal->transformCommon.offsetA111.y));
		}

		if (fractal->transformCommon.functionEnabledCxFalse)
		{
			REAL M1 = fractal->transformCommon.scale1;
			REAL M2 = fractal->transformCommon.scaleA1;
			tempZ.y = fractal->transformCommon.offsetB111.y
								- native_recip((fabs(z.y * M1) + fractal->transformCommon.offset111.y))
								- native_recip(((z.y * z.y * M2) + fractal->transformCommon.offsetA111.y));
		}
		tempZ.y += fabs(z.y) * fractal->transformCommon.offset000.y;
		z.y = sign(z.y) * tempZ.y;
	}

	if (fractal->transformCommon.functionEnabledz)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.z = (native_recip(fractal->transformCommon.offset111.z))
								- native_recip((fabs(z.z) + fractal->transformCommon.offset111.z));

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.z = (fractal->transformCommon.offsetA111.z)
								- native_recip((fabs(z.z) + fractal->transformCommon.offset111.z));

		if (fractal->transformCommon.functionEnabledBxFalse)
		{
			REAL M1 = fractal->transformCommon.scale1;
			REAL M2 = fractal->transformCommon.scaleA1;
			tempZ.z = (native_recip(fractal->transformCommon.offset111.z))
								+ (native_recip(fractal->transformCommon.offsetA111.z))
								- native_recip((fabs(z.z * M1) + fractal->transformCommon.offset111.z))
								- native_recip(((z.z * z.z * M2) + fractal->transformCommon.offsetA111.z));
		}
		if (fractal->transformCommon.functionEnabledCxFalse)
		{
			REAL M1 = fractal->transformCommon.scale1;
			REAL M2 = fractal->transformCommon.scaleA1;
			tempZ.z = fractal->transformCommon.offsetB111.z
								- native_recip((fabs(z.z * M1) + fractal->transformCommon.offset111.z))
								- native_recip(((z.z * z.z * M2) + fractal->transformCommon.offsetA111.z));
		}

		tempZ.z += fabs(z.z) * fractal->transformCommon.offset000.z;
		z.z = sign(z.z) * tempZ.z;
	}
	// aux->DE = aux->DE * l/L;
	aux->DE *= fractal->analyticDE.scale1; // DE tweak
	return z;
}