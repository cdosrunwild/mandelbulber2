/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2021 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * amazing surf Mod4 based on Mandelbulber3D. Formula proposed by Kali, with features added by
 * DarkBeam
 * This formula has a c.x c.y SWAP
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_amazing_surf_mod4.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 AmazingIfsIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{

	REAL colorAdd = 0.0f;

	// sphere inversion
	if (fractal->transformCommon.sphereInversionEnabledFalse
			&& aux->i >= fractal->transformCommon.startIterationsX
			&& aux->i < fractal->transformCommon.stopIterations1)
	{
		z += fractal->transformCommon.offset000;
		REAL rr = dot(z, z);
		z *= fractal->transformCommon.scaleG1 / rr;
		aux->DE *= (fractal->transformCommon.scaleG1 / rr);
		z += fractal->transformCommon.additionConstant000 - fractal->transformCommon.offset000;
		z *= fractal->transformCommon.scaleA1;
		aux->DE *= fractal->transformCommon.scaleA1;
	}

	z += fractal->transformCommon.offsetA000;

	REAL4 oldZ = z;
	z.x = fabs(z.x + fractal->transformCommon.additionConstant0555.x)
				- fabs(z.x - fractal->transformCommon.additionConstant0555.x) - z.x;
	z.y = fabs(z.y + fractal->transformCommon.additionConstant0555.y)
				- fabs(z.y - fractal->transformCommon.additionConstant0555.y) - z.y;
	if (fractal->transformCommon.functionEnabledJFalse)
		z.z = fabs(z.z + fractal->transformCommon.additionConstant0555.z)
					- fabs(z.z - fractal->transformCommon.additionConstant0555.z) - z.z;
	REAL4 zCol = z;

	z += fractal->transformCommon.additionConstantA000;
	REAL rr = dot(z, z);
	REAL rrCol = rr;
	REAL MinRR = fractal->transformCommon.minR2p25;
	REAL dividend = rr < MinRR ? MinRR : min(rr, 1.0f);

	// scale
	REAL useScale = 1.0f;
	if (aux->i >= fractal->transformCommon.startIterationsB
			&& aux->i < fractal->transformCommon.stopIterationsB)
	{
		useScale = (aux->actualScaleA + fractal->transformCommon.scale1) / dividend;
		z *= useScale;
		aux->DE = aux->DE * fabs(useScale) + fractal->analyticDE.offset0;
		if (fractal->transformCommon.functionEnabledKFalse)
		{
			// update actualScaleA for next iteration
			REAL vary = fractal->transformCommon.scaleVary0
									* (fabs(aux->actualScaleA) - fractal->transformCommon.scaleC1);
			aux->actualScaleA -= vary;
		}
	}

	// offset
	z += fractal->transformCommon.offset001;

	if (fractal->transformCommon.rotation2EnabledFalse)
	{
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	}




	z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix2, z);

	if (fractal->analyticDE.enabled)
	{
		if (!fractal->analyticDE.enabledFalse) aux->DE0 = length(z) / aux->DE;
		else aux->DE0 = min(aux->dist, length(z) / aux->DE);
		aux->dist = aux->DE0;
	}


	if (fractal->foldColor.auxColorEnabledFalse)
	{
		if (zCol.x != oldZ.x)
			colorAdd += fractal->mandelbox.color.factor.x
									* (fabs(zCol.x) - fractal->transformCommon.additionConstant111.x);
		if (zCol.y != oldZ.y)
			colorAdd += fractal->mandelbox.color.factor.y
									* (fabs(zCol.y) - fractal->transformCommon.additionConstant111.y);
		if (zCol.z != oldZ.z)
			colorAdd += fractal->mandelbox.color.factor.z
									* (fabs(zCol.z) - fractal->transformCommon.additionConstant111.z);
		if (rrCol > fractal->transformCommon.minR2p25)
			colorAdd +=
				fractal->mandelbox.color.factorSp2 * (rrCol - fractal->transformCommon.minR2p25) / 100.0f;
		aux->color += colorAdd;
	}
	return z;
}
