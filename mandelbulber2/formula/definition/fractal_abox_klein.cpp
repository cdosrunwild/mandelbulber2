/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.         ______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,      / ____/ __    __
 *                                        \><||i|=>>%)     / /   __/ /___/ /_
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    / /__ /_  __/_  __/
 * The project is licensed under GPLv3,   -<>>=|><|||`    \____/ /_/   /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Mandelbulber modification of fractal known as AmazingBox or Mandelbox, invented by Tom Lowe in 2010
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/amazing-fractal/msg12467/#msg12467
 */

#include "all_fractal_definitions.h"

cFractalAboxKlein::cFractalAboxKlein() : cAbstractFractal()
{
	nameInComboBox = "Abox- Klein";
	internalName = "abox_klein";
	internalID = fractal::aboxKlein;
	DEType = analyticDEType;
	DEFunctionType = linearDEFunction;
	cpixelAddition = cpixelDisabledByDefault;
	defaultBailout = 100.0;
	DEAnalyticFunction = analyticFunctionLinear;
	coloringFunction = coloringFunctionDefault;
}

void cFractalAboxKlein::FormulaCode(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	// sphere inversion
	if (fractal->transformCommon.sphereInversionEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsX
			&& aux.i < fractal->transformCommon.stopIterations1)
	{
		z += fractal->transformCommon.offset000;
		double rr = z.Dot(z);
		z *= fractal->transformCommon.scaleG1 / rr;
		aux.DE *= (fractal->transformCommon.scaleG1 / rr);
		z += fractal->transformCommon.additionConstant000 - fractal->transformCommon.offset000;
		z *= fractal->transformCommon.scaleA1;
		aux.DE *= fractal->transformCommon.scaleA1;
	}

	if (aux.i < fractal->transformCommon.stopIterations15)
	{
		CVector4 oldZ = z;
		z = fabs(z + fractal->transformCommon.additionConstant111)
					- fabs(z - fractal->transformCommon.additionConstant111) - z;

		if (fractal->transformCommon.functionEnabledSwFalse)
		{
			double tt = z.x;
			z.x = z.y;
			z.y = tt;
		}
		CVector4 zCol = z;

		double rr = z.Dot(z);
		double MinRR = fractal->transformCommon.minR0;
		double dividend = rr < MinRR ? MinRR : min(rr, 1.0);

		// scale
		double useScale = 1.0;

		useScale = (aux.actualScaleA + fractal->transformCommon.scale1) / dividend;
		z *= useScale;
		aux.DE = aux.DE * fabs(useScale) + fractal->analyticDE.offset1;
		if (fractal->transformCommon.functionEnabledKFalse)
		{
			// update actualScaleA for next iteration
			double vary = fractal->transformCommon.scaleVary0
										* (fabs(aux.actualScaleA) - fractal->transformCommon.scaleC1);
			aux.actualScaleA -= vary;
		}

		if (fractal->transformCommon.rotation2EnabledFalse)
		{
			z = fractal->transformCommon.rotationMatrix.RotateVector(z);
		}

		z += fractal->transformCommon.additionConstantA000;

		if (fractal->foldColor.auxColorEnabledFalse)
		{
			double colorAdd = 0.0;
			if (zCol.x != oldZ.x)
				colorAdd += fractal->foldColor.difs0000.x
										* (fabs(zCol.x) - fractal->transformCommon.additionConstant111.x);
			if (zCol.y != oldZ.y)
				colorAdd += fractal->foldColor.difs0000.y
										* (fabs(zCol.y) - fractal->transformCommon.additionConstant111.y);
			if (zCol.z != oldZ.z)
				colorAdd += fractal->foldColor.difs0000.z
										* (fabs(zCol.z) - fractal->transformCommon.additionConstant111.z);
			colorAdd += fractal->foldColor.difs0000.w * useScale;

			aux.color += colorAdd;
		}
	}
	else
	{
		if (fractal->transformCommon.functionEnabled)
		{
			z = fabs(z + fractal->transformCommon.offset110)
					- fabs(z - fractal->transformCommon.offset110) - z;

			z *= fractal->transformCommon.scale2;
			aux.DE *= fractal->transformCommon.scale2;

			z += fractal->transformCommon.offsetA000;
		}
	}
}
