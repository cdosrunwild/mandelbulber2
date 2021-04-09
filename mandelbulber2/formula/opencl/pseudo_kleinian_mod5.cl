/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Pseudo Kleinian, Knighty - Theli-at's Pseudo Kleinian (Scale 1 JuliaBox + Something
 * @reference https://github.com/Syntopia/Fragmentarium/blob/master/
 * Fragmentarium-Source/Examples/Knighty%20Collection/PseudoKleinian.frag
 * rec fold from darkbeam
 * implementation of pow() from gaz at https://www.shadertoy.com/view/3l2yDd

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_pseudo_kleinian_mod4.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 PseudoKleinianMod5Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	// sphere inversion
	if (fractal->transformCommon.sphereInversionEnabledFalse
			&& aux->i >= fractal->transformCommon.startIterationsX
			&& aux->i < fractal->transformCommon.stopIterations1)
	{
		z += fractal->transformCommon.offset000;
		REAL rr = dot(z, z);
		z *= fractal->transformCommon.scaleG1 / rr;
		aux->DE *= (fractal->transformCommon.scaleG1 / rr);
		z += fractal->transformCommon.additionConstantP000 - fractal->transformCommon.offset000;
		z *= fractal->transformCommon.scaleA1;
		aux->DE *= fractal->transformCommon.scaleA1;
	}

	// box offset
	if (aux->i >= fractal->transformCommon.startIterationsM
			&& aux->i < fractal->transformCommon.stopIterationsM)
	{
		z.x -= fractal->transformCommon.constantMultiplier000.x * sign(z.x);
		z.y -= fractal->transformCommon.constantMultiplier000.y * sign(z.y);
		z.z -= fractal->transformCommon.constantMultiplier000.z * sign(z.z);
	}

	// Pseudo kleinian
	if (fractal->transformCommon.functionEnabledCx)
		z.x = z.x - fractal->transformCommon.additionConstant222.x
			* round(z.x / fractal->transformCommon.additionConstant222.x);
	if (fractal->transformCommon.functionEnabledCy)
		z.y = z.y - fractal->transformCommon.additionConstant222.y
			* round(z.y / fractal->transformCommon.additionConstant222.y);
	if (fractal->transformCommon.functionEnabledCz)
		z.z = z.z - fractal->transformCommon.additionConstant222.z
			* round(z.z / fractal->transformCommon.additionConstant222.z);

	if (fractal->transformCommon.functionEnabledFFalse
			&& aux->i >= fractal->transformCommon.startIterationsF
			&& aux->i < fractal->transformCommon.stopIterationsF)
	{
		if (fractal->transformCommon.functionEnabledCxFalse)
			z.x = fabs(z.x + fractal->transformCommon.offset111.x)
				- fabs(z.x - fractal->transformCommon.offset111.x) - z.x;
		if (fractal->transformCommon.functionEnabledCyFalse)
			z.y = fabs(z.y + fractal->transformCommon.offset111.y)
				- fabs(z.y - fractal->transformCommon.offset111.y) - z.y;
		if (fractal->transformCommon.functionEnabledCzFalse)
			z.z = fabs(z.z + fractal->transformCommon.offset111.z)
				- fabs(z.z - fractal->transformCommon.offset111.z) - z.z;
	}

	REAL4 lpN = fabs(z);
	REAL pr = fractal->transformCommon.scale2;
	lpN.x = pow(lpN.x, pr);
	lpN.y = pow(lpN.y, pr);
	lpN.z = pow(lpN.z, pr);
	REAL pNorm = pow((lpN.x + lpN.y + lpN.z), 1.0 / pr);

	pNorm = pow(pNorm, fractal->transformCommon.scaleA2);
	pNorm = max(pNorm, fractal->transformCommon.offset02);
	pNorm = fractal->transformCommon.scale1p1 / pNorm;
	z *= pNorm;
	aux->DE *= fabs(pNorm);

	if (fractal->transformCommon.functionEnabledGFalse
			&& aux->i >= fractal->transformCommon.startIterationsG
			&& aux->i < fractal->transformCommon.stopIterationsG)
	{
		z.x += aux->pos_neg * fractal->transformCommon.additionConstantA000.x;
		z.y += aux->pos_neg * fractal->transformCommon.additionConstantA000.y;
		z.z += aux->pos_neg * fractal->transformCommon.additionConstantA000.z;

		aux->pos_neg *= fractal->transformCommon.scaleNeg1;
	}


	if (fractal->transformCommon.functionEnabledNFalse
			&& aux->i >= fractal->transformCommon.startIterationsN
			&& aux->i < fractal->transformCommon.stopIterationsN)
	{
		REAL foldX = fractal->transformCommon.offset1;
		REAL foldY = fractal->transformCommon.offsetA1;

		REAL t;
		z.x = fabs(z.x);
		z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAFalse)
		{
			t = z.x;
			z.x = z.y;
			z.y = t;
		}
		t = z.x;
		z.x = z.x + z.y - fractal->transformCommon.offset0;
		z.y = t - z.y - fractal->transformCommon.offsetA0;
		if (fractal->transformCommon.functionEnabledBxFalse
				&& aux->i >= fractal->transformCommon.startIterationsO
				&& aux->i < fractal->transformCommon.stopIterationsO)
			z.x = -fabs(z.x);
		if (fractal->transformCommon.functionEnabledBx
				&& aux->i >= fractal->transformCommon.startIterationsP
				&& aux->i < fractal->transformCommon.stopIterationsP)
			z.y = -fabs(z.y);

		t = z.x;
		z.x = z.x + z.y;
		z.y = t - z.y;
		z.x *= 0.5f;
		z.y *= 0.5f;
		if (fractal->transformCommon.functionEnabledAx
				&& aux->i >= fractal->transformCommon.startIterationsR
				&& aux->i < fractal->transformCommon.stopIterationsR)
			z.x = foldX - fabs(z.x + foldX);
		if (fractal->transformCommon.functionEnabledAxFalse
				&& aux->i >= fractal->transformCommon.startIterationsRV
				&& aux->i < fractal->transformCommon.stopIterationsRV)
			z.y = foldY - fabs(z.y + foldY);
	}


	// DE options

	REAL len = length(z);
	if (fractal->transformCommon.functionEnabledCFalse)
	{
		REAL k = max(fractal->transformCommon.minR05 / dot(z, z), 1.0f);
		z *= k;
		aux->DE *= k;
	}

	aux->DE *= 1.0 + fractal->analyticDE.tweak005;

	if (fractal->transformCommon.functionEnabledDFalse)
	{
		len = min(len, fractal->transformCommon.foldingValue - len);
	}

	if (fractal->transformCommon.functionEnabledBFalse)
	{
		if (!fractal->transformCommon.functionEnabledXFalse)
		{
			len -= fractal->transformCommon.offsetD0;
			if (!fractal->transformCommon.functionEnabledXFalse)
			{
				aux->DE0= len / aux->DE;
			}
			else
			{
				aux->DE0 = min(len, fractal->analyticDE.offset1)
						/ max(aux->DE, fractal->analyticDE.offset0);
			}
		}
	}
	if (fractal->transformCommon.functionEnabledJFalse)
	{
		REAL rxy = length(z.xy);
		aux->DE0 = max(rxy - fractal->analyticDE.scale1, fabs(rxy * z.z) / len) / aux->DE;
	}
	if (!fractal->transformCommon.functionEnabledYFalse) aux->dist = aux->DE0;
	else aux->dist = min(aux->dist, aux->DE0);

	aux->pseudoKleinianDE = fractal->analyticDE.scale1; // for pK DE

	// color
	if (fractal->foldColor.auxColorEnabledFalse)
	{
		REAL colorAdd = 0.0f;
		colorAdd += fractal->foldColor.difs0000.x * fabs(z.x);
		colorAdd += fractal->foldColor.difs0000.y * fabs(z.y);
		colorAdd += fractal->foldColor.difs0000.z * fabs(z.z);
		colorAdd += fractal->foldColor.difs0000.w * pNorm;

		aux->color += colorAdd;
	}
	return z;
}
