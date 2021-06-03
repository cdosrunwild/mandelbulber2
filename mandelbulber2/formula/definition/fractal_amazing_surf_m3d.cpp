/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.         ______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,      / ____/ __    __
 *                                        \><||i|=>>%)     / /   __/ /___/ /_
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    / /__ /_  __/_  __/
 * The project is licensed under GPLv3,   -<>>=|><|||`    \____/ /_/   /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * amazing surf from Mandelbulber3D. Formula proposed by Kali, with features added by DarkBeam
 *
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */

#include "all_fractal_definitions.h"

cFractalAmazingSurfM3d::cFractalAmazingSurfM3d() : cAbstractFractal()
{
	nameInComboBox = "Amazing Surf M3D";
	internalName = "amazing_surf_m3d";
	internalID = fractal::amazingSurfM3d;
	DEType = analyticDEType;
	DEFunctionType = linearDEFunction;
	cpixelAddition = cpixelDisabledByDefault;
	defaultBailout = 100.0;
	DEAnalyticFunction = analyticFunctionLinear;
	coloringFunction = coloringFunctionABox;
}

void cFractalAmazingSurfM3d::FormulaCode(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations1)
	{
		if (fractal->transformCommon.functionEnabledxFalse) z.x = -z.x;
		if (fractal->transformCommon.functionEnabledyFalse) z.y = -z.y;
		if (fractal->transformCommon.functionEnabledzFalse) z.z = -z.z;
	}

	// update aux.actualScale
	aux.actualScale =
			fractal->transformCommon.scale015
				+ fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);
	CVector4 oldZ = z;
	z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
				- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
	z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
				- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
	CVector4 zCol = z;
	// no z fold

	double rr;
	if (!fractal->transformCommon.functionEnabledFalse) rr = z.Dot(z);
	else rr = sqrt(z.x * z.x + z.y * z.y + z.z * z.z);

	double m = aux.actualScale;
	double MinR = fractal->mandelbox.mR2;
	if (rr < MinR) m = m / MinR;
	else if (rr < 1.0f) m = m / rr;

	z *= m;
	aux.DE = aux.DE * fabs(m) + fractal->analyticDE.offset1;

	if (fractal->transformCommon.addCpixelEnabled)
		z += aux.const_c * fractal->transformCommon.constantMultiplier111;

	z += fractal->transformCommon.offsetA000;

	double temp = fractal->transformCommon.rotation.z * M_PI_180;
	double sinan = sin(temp);
	double cosan = cos(temp);
	temp = z.x;
	z.x = z.x * cosan - z.y * sinan;
	z.y = temp * sinan + z.y * cosan;

	temp = fractal->transformCommon.rotation.y * M_PI_180;
	sinan = sin(temp);
	cosan = cos(temp);
	temp = z.z;
	z.z = z.z * cosan - z.x * sinan;
	z.x = temp * sinan + z.x * cosan;

	temp = fractal->transformCommon.rotation.x * M_PI_180;
	sinan = sin(temp);
	cosan = cos(temp);
	temp = z.y;
	z.y = z.y * cosan - z.z * sinan;
	z.z = temp * sinan + z.z * cosan;

	if (fractal->foldColor.auxColorEnabledFalse)
	{
		double colorAdd = 0.0;
		if (aux.i >= fractal->foldColor.startIterationsA
				&& aux.i < fractal->foldColor.stopIterationsA)
		{
			zCol = fabs(zCol - oldZ);
			if (zCol.x > 0.0)
				colorAdd += fractal->foldColor.difs0000.x * zCol.x;
			if (zCol.y > 0.0)
				colorAdd += fractal->foldColor.difs0000.y * zCol.y;
			colorAdd += fractal->foldColor.difs0000.z * fabs(z.z);
			colorAdd += fractal->foldColor.difs0000.w * m;
		}
		aux.color += colorAdd;
	}
}