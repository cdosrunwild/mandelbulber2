/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.         ______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,      / ____/ __    __
 *                                        \><||i|=>>%)     / /   __/ /___/ /_
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    / /__ /_  __/_  __/
 * The project is licensed under GPLv3,   -<>>=|><|||`    \____/ /_/   /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * 3D pow3 bulb by gannjondal
 * refrence:
 * https://fractalforums.org/fractal-mathematics-and-new-theories/28/revisiting-the-3d-newton/1026
 */

#include "all_fractal_definitions.h"

cFractalNewtonPow3::cFractalNewtonPow3() : cAbstractFractal()
{
	nameInComboBox = "Newton Pow3";
	internalName = "newton_pow3";
	internalID = fractal::newtonPow3;
	DEType = deltaDEType;
	DEFunctionType = logarithmicDEFunction;
	cpixelAddition = cpixelDisabledByDefault;
	defaultBailout = 10.0;
	DEAnalyticFunction = analyticFunctionLogarithmic;
	coloringFunction = coloringFunctionDefault;
}

void cFractalNewtonPow3::FormulaCode(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	// abs()
	if (fractal->transformCommon.functionEnabledAFalse)
	{
		if (fractal->transformCommon.functionEnabledAxFalse) z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledAyFalse) z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAzFalse) z.z = fabs(z.z);
	}
	// Preparation operations
	double fac_eff = 0.6666666666;

	// MB2 needs a potentially diverging value of z to work correctly.
	// However the Newton method for z^3-1 is so far always converging.
	// Fortunately there is a one-to-one relation between these values.
	// Below the z from MB2 is transformed
	// to the z used for the Newton calculations
	double sq_r = fractal->transformCommon.scaleA1 / (aux.r * aux.r);
	//aux.DE *= (sq_r);
	z.x = z.x * sq_r + 1.0;
	z.y = -z.y * sq_r; // 0.0
	z.z = -z.z * sq_r; // 0.0

	// Calculate the inverse power of z=(z.x,z.y,z.z),
	// and use it for the Newton method calculations for z^3 + 1 = 0
	// i.e. z(n+1) = 2*z(n)/3 - 1/3*z(n)^2
	CVector4 tp = z * z;
	sq_r = tp.x + tp.y + tp.z; // dot
	sq_r = 1.0 / (3.0 * sq_r * sq_r);

	double r_xy = tp.x + tp.y;
	double h1 = 1.0 - tp.z / r_xy;

	double tmpx = h1 * (tp.x - tp.y) * sq_r;
	double tmpy = -2.0 * h1 * z.x * z.y * sq_r;
	double tmpz = 2.0 * z.z * sqrt(r_xy) * sq_r;

	tp.x = -tmpx;
	tp.y = -tmpy;
	tp.z = tmpz;

	z = fac_eff * z - tp;

	// Below the z used for (converging) Newton calculation
	// is transformed back to the potentially diverging z used by MB2
	// (see also the notes above)
	tp.x = z.x - 1.0;
	tp.y = z.y;
	tp.z = z.z;
	sq_r = fractal->transformCommon.scaleB1 / tp.Dot(tp);
	z.x = tp.x * sq_r;
	z.y = -tp.y * sq_r;
	z.z = -tp.z * sq_r;

	//Below translation is equivalent to the usage of c in Julia mode
	//However, in hybrids this setting can be used as a variable only for this fractal formula
	z += fractal->transformCommon.offset000;

	aux.DE *= aux.r * 2.0;
	if (fractal->analyticDE.enabledFalse)
	{
		aux.DE = aux.DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset1;
	}

	if (fractal->transformCommon.functionEnabledOFalse)
	{
		CVector4 q = z;
		q.x -= fractal->transformCommon.offsetA1;
		double r = q.Length() + fractal->transformCommon.offsetA1;

		if (!fractal->transformCommon.functionEnabledYFalse)
			aux.dist = min(aux.dist, 0.5f * log(r) * r / aux.DE);
		else
			aux.dist = 0.5f * log(r) * r / aux.DE;
	}
}
