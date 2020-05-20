/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * BoxBulb power 2  menger with scaling of z axis
 * This formula contains aux.color

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_box_fold_bulb_menger.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 BoxFoldBulbMengerIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 c = aux->const_c;
	REAL colorAdd = 0.0f;
	REAL rrCol = 0.0f;
	REAL4 zCol = z;
	REAL4 oldZ = z;
	// tglad fold
	if (aux->i >= fractal->transformCommon.startIterationsB
			&& aux->i < fractal->transformCommon.stopIterationsB)
	{
		z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
					- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
		z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
					- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
		z.z = fabs(z.z + fractal->transformCommon.additionConstant111.z)
					- fabs(z.z - fractal->transformCommon.additionConstant111.z) - z.z;
		zCol = z;
	}

	// spherical fold
	if (aux->i >= fractal->transformCommon.startIterationsS
			&& aux->i < fractal->transformCommon.stopIterationsS)
	{
		REAL rr = dot(z, z);
		rrCol = rr;
		z += fractal->mandelbox.offset;

		// if (r2 < 1e-21f) r2 = 1e-21f;
		if (rr < fractal->transformCommon.minR2p25)
		{
			REAL tglad_factor1 = fractal->transformCommon.maxMinR2factor;
			z *= tglad_factor1;
			aux->DE *= tglad_factor1;
		}
		else if (rr < fractal->transformCommon.maxR2d1)
		{
			REAL tglad_factor2 = fractal->transformCommon.maxR2d1 / rr;
			z *= tglad_factor2;
			aux->DE *= tglad_factor2;
		}
		z -= fractal->mandelbox.offset;
	}
	// scale
	if (aux->i >= fractal->transformCommon.startIterationsC
			&& aux->i < fractal->transformCommon.stopIterationsC)
	{
		z *= fractal->transformCommon.scale;
		aux->DE *= fabs(fractal->transformCommon.scale);
	}

	// addCpixel
	if (fractal->transformCommon.addCpixelEnabledFalse
			&& aux->i >= fractal->transformCommon.startIterationsE
			&& aux->i < fractal->transformCommon.stopIterationsE)
	{
		z += c * fractal->transformCommon.constantMultiplierC111;
	}

	if (fractal->transformCommon.functionEnabledXFalse
			&& aux->i >= fractal->transformCommon.startIterationsTM
			&& aux->i < fractal->transformCommon.stopIterationsTM1)
	{
		REAL tempXZ = z.x * SQRT_2_3_F - z.z * SQRT_1_3_F;
		z = (REAL4){(tempXZ - z.y) * SQRT_1_2_F, (tempXZ + z.y) * SQRT_1_2_F,
			z.x * SQRT_1_3_F + z.z * SQRT_2_3_F, z.w};
		REAL4 temp = z;
		REAL tempL = length(temp);
		z = fabs(z) * fractal->transformCommon.scale3D333;
		// if (tempL < 1e-21f) tempL = 1e-21f;
		REAL avgScale = length(z) / tempL;
		aux->DE = aux->DE * avgScale;
		z = (fabs(z + fractal->transformCommon.additionConstantA111)
				 - fabs(z - fractal->transformCommon.additionConstantA111) - z);
		tempXZ = (z.y + z.x) * SQRT_1_2_F;
		z = (REAL4){z.z * SQRT_1_3_F + tempXZ * SQRT_2_3_F, (z.y - z.x) * SQRT_1_2_F,
			z.z * SQRT_2_3_F - tempXZ * SQRT_1_3_F, z.w};
	}

	// octo
	if (fractal->transformCommon.functionEnabledDFalse
			&& aux->i >= fractal->transformCommon.startIterationsD
			&& aux->i < fractal->transformCommon.stopIterationsD)

	{
		if (z.x + z.y < 0.0f) z = (REAL4){-z.y, -z.x, z.z, z.w};

		if (z.x + z.z < 0.0f) // z.xz = -z.zx;
			z = (REAL4){-z.z, z.y, -z.x, z.w};

		if (z.x - z.y < 0.0f) // z.xy = z.yx;
			z = (REAL4){z.y, z.x, z.z, z.w};

		if (z.x - z.z < 0.0f) // z.xz = z.zx;
			z = (REAL4){z.z, z.y, z.x, z.w};

		z.x = fabs(z.x);
		z = z * fractal->transformCommon.scaleA2
				- fractal->transformCommon.offset100 * (fractal->transformCommon.scaleA2 - 1.0f);
		aux->DE *= fabs(fractal->transformCommon.scaleA2);
	}

	// rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);

	// pow2 bulb
	if (aux->i >= fractal->transformCommon.startIterationsA
			&& aux->i < fractal->transformCommon.stopIterationsA)
	{
		aux->r = length(z);

		if (fractal->analyticDE.enabledFalse)
		{
			aux->DE = aux->r * aux->DE * 5.0f * fractal->analyticDE.scale1
									* fractal->transformCommon.scaleG1
									* native_sqrt(fractal->foldingIntPow.zFactor * fractal->foldingIntPow.zFactor
																+ 2.0f + fractal->analyticDE.offset2)
								+ fractal->analyticDE.offset1;
		}
		else
		{
			aux->DE = aux->r * aux->DE * 8.0f * fractal->analyticDE.scale1
									* fractal->transformCommon.scaleG1
									* native_sqrt(fractal->foldingIntPow.zFactor * fractal->foldingIntPow.zFactor
																+ 2.0f + fractal->analyticDE.offset2)
									/ SQRT_3_F
								+ fractal->analyticDE.offset1;
		}
		z *= fractal->transformCommon.scaleG1;
		REAL x2 = z.x * z.x;
		REAL y2 = z.y * z.y;
		REAL z2 = z.z * z.z;
		REAL temp = 1.0f - z2 / (x2 + y2);
		REAL4 zTemp;
		zTemp.x = (x2 - y2) * temp;
		zTemp.y = 2.0f * z.x * z.y * temp;
		zTemp.z = -2.0f * z.z * native_sqrt(x2 + y2);
		zTemp.w = z.w;
		z = zTemp;
		z += fractal->transformCommon.offset000;
		z.z *= fractal->foldingIntPow.zFactor;
	}
	// menger sponge
	if (fractal->transformCommon.functionEnabledM
			&& aux->i >= fractal->transformCommon.startIterationsM
			&& aux->i < fractal->transformCommon.stopIterationsM)
	{
		z = fabs(z + fractal->transformCommon.additionConstantA000);
		if (z.x - z.y < 0.0f)
		{
			REAL temp = z.y;
			z.y = z.x;
			z.x = temp;
		}
		if (z.x - z.z < 0.0f)
		{
			REAL temp = z.z;
			z.z = z.x;
			z.x = temp;
		}
		if (z.y - z.z < 0.0f)
		{
			REAL temp = z.z;
			z.z = z.y;
			z.y = temp;
		}
		// menger scales and offsets
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0f * fractal->transformCommon.constantMultiplier111.x;
		z.y -= 2.0f * fractal->transformCommon.constantMultiplier111.y;
		if (fractal->transformCommon.functionEnabled)
		{
			if (z.z > 1.0f) z.z -= 2.0f * fractal->transformCommon.constantMultiplier111.z;
		}
		else
		{
			z.z -= 2.0f * fractal->transformCommon.constantMultiplier111.z;
		}
		aux->DE *= fractal->transformCommon.scale3;
	}
	// color
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

		if (rrCol < fractal->transformCommon.maxR2d1)
		{
			if (rrCol < fractal->transformCommon.minR2p25)
				colorAdd += fractal->mandelbox.color.factorSp1 * (fractal->transformCommon.minR2p25 - rrCol)
										+ fractal->mandelbox.color.factorSp2
												* (fractal->transformCommon.maxR2d1 - fractal->transformCommon.minR2p25);
			else
				colorAdd += fractal->mandelbox.color.factorSp2 * (fractal->transformCommon.maxR2d1 - rrCol);
		}

		aux->color += colorAdd;
	}
	return z;
}