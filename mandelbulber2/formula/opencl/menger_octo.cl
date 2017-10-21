/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Menger Sponge and octo
 * from code by Knighty

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "MengerOctoIteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MengerOctoIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{ // octo
	if (aux->i >= fractal->transformCommon.startIterationsE
			&& aux->i < fractal->transformCommon.stopIterationsE)
	{
		if (z.x + z.y < 0.0f) z = (REAL4){-z.y, -z.x, z.z, z.w};

		if (z.x + z.z < 0.0f) // z.xz = -z.zx;
			z = (REAL4){-z.z, z.y, -z.x, z.w};

		if (z.x - z.y < 0.0f) // z.xy = z.yx;
			z = (REAL4){z.y, z.x, z.z, z.w};

		if (z.x - z.z < 0.0f) // z.xz = z.zx;
			z = (REAL4){z.z, z.y, z.x, z.w};

		z.x = fabs(z.x);
		z = mad(z, fractal->transformCommon.scale2,
			-fractal->transformCommon.offset100 * (fractal->transformCommon.scale2 - 1.0f));

		aux->DE *= fractal->transformCommon.scale2;
	}
	// box offset
	if (fractal->transformCommon.functionEnabledxFalse
			&& aux->i >= fractal->transformCommon.startIterationsA
			&& aux->i < fractal->transformCommon.stopIterationsA)
	{
		REAL4 temp = z;
		z.x = mad(sign(z.x), fractal->transformCommon.additionConstantA000.x, z.x);
		z.y = mad(sign(z.y), fractal->transformCommon.additionConstantA000.y, z.y);
		z.z = mad(sign(z.z), fractal->transformCommon.additionConstantA000.z, z.z);

		if (fractal->transformCommon.functionEnabledzFalse)
		{
			REAL tempL = length(temp);
			// if (tempL < 1e-21f) tempL = 1e-21f;
			REAL avgScale = native_divide(length(z), tempL);
			aux->DE = mad(aux->DE, avgScale, 1.0f);
		}
	}
	// spherical fold
	if (fractal->transformCommon.functionEnabledSFalse
			&& aux->i >= fractal->transformCommon.startIterationsS
			&& aux->i < fractal->transformCommon.stopIterationsS)
	{
		REAL para = 0.0f;
		REAL paraAddP0 = 0.0f;
		if (fractal->transformCommon.functionEnabledyFalse)
		{
			// para += paraAddP0 + fractal->transformCommon.minR2p25;
			if (fractal->Cpara.enabledLinear)
			{
				para = fractal->Cpara.para00; // parameter value at iter 0
				REAL temp0 = para;
				REAL tempA = fractal->Cpara.paraA;
				REAL tempB = fractal->Cpara.paraB;
				REAL tempC = fractal->Cpara.paraC;
				REAL lengthAB = fractal->Cpara.iterB - fractal->Cpara.iterA;
				REAL lengthBC = fractal->Cpara.iterC - fractal->Cpara.iterB;
				REAL grade1 = native_divide((tempA - temp0), fractal->Cpara.iterA);
				REAL grade2 = native_divide((tempB - tempA), lengthAB);
				REAL grade3 = native_divide((tempC - tempB), lengthBC);

				// slopes
				if (aux->i < fractal->Cpara.iterA)
				{
					para = temp0 + (aux->i * grade1);
				}
				if (aux->i < fractal->Cpara.iterB && aux->i >= fractal->Cpara.iterA)
				{
					para = mad(grade2, (aux->i - fractal->Cpara.iterA), tempA);
				}
				if (aux->i >= fractal->Cpara.iterB)
				{
					para = mad(grade3, (aux->i - fractal->Cpara.iterB), tempB);
				}

				// Curvi part on "true"
				if (fractal->Cpara.enabledCurves)
				{
					REAL paraAdd = 0.0f;
					REAL paraIt;
					if (lengthAB > 2.0f * fractal->Cpara.iterA) // stop  error, todo fix.
					{
						REAL curve1 = native_divide((grade2 - grade1), (4.0f * fractal->Cpara.iterA));
						REAL tempL = lengthAB - fractal->Cpara.iterA;
						REAL curve2 = native_divide((grade3 - grade2), (4.0f * tempL));
						if (aux->i < 2 * fractal->Cpara.iterA)
						{
							paraIt = tempA - fabs(tempA - aux->i);
							paraAdd = paraIt * paraIt * curve1;
						}
						if (aux->i >= 2 * fractal->Cpara.iterA && aux->i < fractal->Cpara.iterB + tempL)
						{
							paraIt = tempB - fabs(tempB * aux->i);
							paraAdd = paraIt * paraIt * curve2;
						}
						para += paraAdd;
					}
				}
			}
			paraAddP0 = 0.0f;
			if (fractal->Cpara.enabledParabFalse)
			{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
				paraAddP0 = fractal->Cpara.parabOffset0 + (aux->i * fractal->Cpara.parabSlope)
										+ (aux->i * aux->i * 0.001f * fractal->Cpara.parabScale);
			}
		}
		para += paraAddP0 + fractal->transformCommon.minR2p25;
		// spherical fold
		REAL rr = dot(z, z);

		z += fractal->mandelbox.offset;

		// if (r2 < 1e-21f) r2 = 1e-21f;
		if (rr < para)
		{
			REAL tglad_factor1 = native_divide(fractal->transformCommon.maxR2d1, para);
			z *= tglad_factor1;
			aux->DE *= tglad_factor1;
			aux->color += fractal->mandelbox.color.factorSp1;
		}
		else if (rr < fractal->transformCommon.maxR2d1)
		{
			REAL tglad_factor2 = native_divide(fractal->transformCommon.maxR2d1, rr);
			z *= tglad_factor2;
			aux->DE *= tglad_factor2;
			aux->color += fractal->mandelbox.color.factorSp2;
		}
		z -= fractal->mandelbox.offset;
		z *= fractal->transformCommon.scale08;
		aux->DE = aux->DE * fabs(fractal->transformCommon.scale08);
	}
	// rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
	{
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	}
	// menger
	if (fractal->transformCommon.functionEnabledM
			&& aux->i >= fractal->transformCommon.startIterationsM
			&& aux->i < fractal->transformCommon.stopIterationsM)
	{
		z = fabs(z);
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
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0f * fractal->transformCommon.constantMultiplier111.x;
		z.y -= 2.0f * fractal->transformCommon.constantMultiplier111.y;
		if (z.z > 1.0f) z.z -= 2.0f * fractal->transformCommon.constantMultiplier111.z;
		aux->DE *= fractal->transformCommon.scale3;
		z += fractal->transformCommon.additionConstant000;
	}
	// iter weight
	if (fractal->transformCommon.functionEnabledFalse)
	{
		REAL4 zA = (aux->i == fractal->transformCommon.intA) ? z : (REAL4){};
		REAL4 zB = (aux->i == fractal->transformCommon.intB) ? z : (REAL4){};

		z = (z * fractal->transformCommon.scale1) + (zA * fractal->transformCommon.offsetA0)
				+ (zB * fractal->transformCommon.offsetB0);
		aux->DE *= fractal->transformCommon.scale1;
		aux->r_dz *= fractal->transformCommon.scale1;
	}
	return z;
}