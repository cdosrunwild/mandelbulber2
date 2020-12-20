/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.         ______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,      / ____/ __    __
 *                                        \><||i|=>>%)     / /   __/ /___/ /_
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    / /__ /_  __/_  __/
 * The project is licensed under GPLv3,   -<>>=|><|||`    \____/ /_/   /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * JosLeys-Kleinian V2 formula
 * @reference
 * http://www.fractalforums.com/3d-fractal-generation/an-escape-tim-algorithm-for-kleinian-group-limit-sets/msg98248/#msg98248
 * This formula contains aux.color and aux.DE
 */

#include "all_fractal_definitions.h"

cFractalJosKleinianV3::cFractalJosKleinianV3() : cAbstractFractal()
{
	nameInComboBox = "JosLeys-Kleinian V3";
	internalName = "jos_kleinian_v3";
	internalID = fractal::josKleinianV3;
	DEType = analyticDEType;
	DEFunctionType = customDEFunction;
	cpixelAddition = cpixelDisabledByDefault;
	defaultBailout = 10.0;
	DEAnalyticFunction = analyticFunctionCustomDE;
	coloringFunction = coloringFunctionDefault;
}

void cFractalJosKleinianV3::FormulaCode(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	// polyfold
	if (fractal->transformCommon.functionEnabledPFalse
			&& aux.i >= fractal->transformCommon.startIterationsP
			&& aux.i < fractal->transformCommon.stopIterationsP1)
	{
		if (fractal->transformCommon.functionEnabledx) z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledy) z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledz) z.z = fabs(z.z);

		if (fractal->transformCommon.functionEnabledCx)
		{
			//if (fractal->transformCommon.functionEnabledAxFalse && z.y < 0.0) z.x = -z.x;
			double psi = M_PI / fractal->transformCommon.int8X;
			psi = fabs(fmod(atan2(z.y, z.x) + psi, 2.0 * psi) - psi);
			double len = sqrt(z.x * z.x + z.y * z.y);
			z.x = cos(psi) * len;
			z.y = sin(psi) * len;
		}

		if (fractal->transformCommon.functionEnabledCyFalse)
		{
			//if (fractal->transformCommon.functionEnabledAyFalse && z.z < 0.0) z.y = -z.y;
			double psi = M_PI / fractal->transformCommon.int8Y;
			psi = fabs(fmod(atan2(z.z, z.y) + psi, 2.0 * psi) - psi);
			double len = sqrt(z.y * z.y + z.z * z.z);
			z.y = cos(psi) * len;
			z.z = sin(psi) * len;
		}

		if (fractal->transformCommon.functionEnabledCzFalse)
		{
			//if (fractal->transformCommon.functionEnabledAzFalse && z.x < 0.0) z.z = -z.z;
			double psi = M_PI / fractal->transformCommon.int8Z;
			psi = fabs(fmod(atan2(z.x, z.z) + psi, 2.0 * psi) - psi);
			double len = sqrt(z.z * z.z + z.x * z.x);
			z.z = cos(psi) * len;
			z.x = sin(psi) * len;
		}

		// addition constant
		z += fractal->transformCommon.offsetF000;

	}


	// sphere inversion
	if (fractal->transformCommon.sphereInversionEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsD1)
	{
		double rr = 1.0;
		z += fractal->transformCommon.offset000;
		rr = z.Dot(z);
		z *= fractal->transformCommon.maxR2d1 / rr;
		z += fractal->transformCommon.additionConstant000 - fractal->transformCommon.offset000;
		z *= fractal->transformCommon.scaleA1;
		// double r = sqrt(rr);
		aux.DE *= (fractal->transformCommon.maxR2d1 / rr) * fractal->analyticDE.scale1
							* fractal->transformCommon.scaleA1;
	}

	if (fractal->transformCommon.functionEnabledCyFalse
			&& aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC1)
	{
		CVector4 oldZ = z;
		CVector4 trigZ = CVector4(0.0, 0.0, 0.0, 0.0);
		CVector4 scaleZ = z * fractal->transformCommon.constantMultiplierC111;

		if (fractal->transformCommon.functionEnabledAx)
		{
			if (!fractal->transformCommon.functionEnabledAxFalse)
				trigZ.x = sin(scaleZ.x);
			else
				trigZ.x = cos(scaleZ.x); // scale =0, cos = 1
		}
		if (fractal->transformCommon.functionEnabledAy)
		{
			if (!fractal->transformCommon.functionEnabledAyFalse)
				trigZ.y = sin(scaleZ.y);
			else
				trigZ.y = cos(scaleZ.y);
		}
		if (fractal->transformCommon.functionEnabledAz)
		{
			if (!fractal->transformCommon.functionEnabledAzFalse)
				trigZ.z = sin(scaleZ.z);
			else
				trigZ.z = cos(scaleZ.z);
		}

		z = trigZ * fractal->transformCommon.scale;
		if (fractal->transformCommon.functionEnabledFalse)
		{
			z.x = z.x * fractal->transformCommon.scale / (fabs(oldZ.x) + 1.0);
			z.y = z.y * fractal->transformCommon.scale / (fabs(oldZ.y) + 1.0);
			z.z = z.z * fractal->transformCommon.scale / (fabs(oldZ.z) + 1.0);
			// aux.DE = aux.DE * z.Length() / oldZ.Length();
		}
	}

	if (fractal->transformCommon.functionEnabledJFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		if (z.z > z.x) swap(z.x, z.z);
	}

	// kleinian
	if (aux.i >= fractal->transformCommon.startIterationsF
			&& aux.i < fractal->transformCommon.stopIterationsF)
	{
		double a = fractal->transformCommon.foldingValue;
		double b = fractal->transformCommon.offset;
		//double c = fractal->transformCommon.offsetA0;
		double f = sign(b);

		// wrap
		CVector4 box_size = fractal->transformCommon.offset111;
		//CVector3 box1 = CVector3(2.0 * box_size.x, a * box_size.y, 2.0 * box_size.z);
		//CVector3 box2 = CVector3(-box_size.x, -box_size.y + 1.0, -box_size.z);
		//CVector3 wrapped = wrap(z.GetXYZ(), box1, box2);

		//z = CVector4(wrapped.x, wrapped.y, wrapped.z, z.w);
		{
			z.x += box_size.x;
			z.y += box_size.y;
			z.x = z.x - 2.0 * box_size.x * floor(z.x / 2.0 * box_size.x) - box_size.x;
			z.y = z.y - 2.0 * box_size.y * floor(z.y / 2.0 * box_size.y) - box_size.y;
			z.z += box_size.z - 1.0;
			z.z = z.z - a * box_size.z * floor(z.z / a * box_size.z);
			z.z -= (box_size.z - 1.0);
		}

		if (z.z >= a * (0.5 + 0.2 * sin(f * M_PI * (z.x + b * 0.5) / box_size.x)))
		{
			z.x = -z.x - b;
			z.z = -z.z + a;
			//z.z = -z.z - c;
		}

		double rr = z.Dot(z);

		CVector4 colorVector = CVector4(z.x, z.y, z.z, rr);
		aux.color = min(aux.color, colorVector.Length()); // For coloring

		double iR = 1.0 / rr;
		z *= -iR; // invert and mirror
		z.x = -z.x - b;
		z.z = a + z.z;
		//z.z = -z.z - c;

		aux.DE *= fabs(iR);
	}

	if (fractal->transformCommon.functionEnabledEFalse
			&& aux.i >= fractal->transformCommon.startIterationsE
			&& aux.i < fractal->transformCommon.stopIterationsE)
	{
		z.z = sign(z.z)
					* (fractal->transformCommon.offset1 - fabs(z.z)
						 + fabs(z.z) * fractal->transformCommon.scale0);
	}
	double Ztemp = z.z;
	if (fractal->transformCommon.spheresEnabled)
		Ztemp = min(z.z, fractal->transformCommon.foldingValue - z.z);

	aux.dist =
		min(Ztemp, fractal->analyticDE.tweak005)
		/ max(aux.DE, fractal->analyticDE.offset1);

}