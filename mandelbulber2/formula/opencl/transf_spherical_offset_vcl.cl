/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * spherical radial offset Curvilinear.
 * This formula contains analytic aux.DE and aux.r-dz

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_transf_spherical_offset_vcl.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfSphericalOffsetVCLIteration(
	REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL para = fractal->Cpara.para00;
	REAL paraAdd = 0.0f;
	REAL paraAddP0 = 0.0f;
	// curvilinear mode
	if (fractal->transformCommon.functionEnabled)
	{
		if (fractal->Cpara.enabledLinear)
		{
			para = fractal->Cpara.para00; // parameter value at iter 0
			REAL temp0 = para;
			REAL tempA = fractal->Cpara.paraA0;
			REAL tempB = fractal->Cpara.paraB0;
			REAL tempC = fractal->Cpara.paraC0;
			REAL lengthAB = fractal->Cpara.iterB - fractal->Cpara.iterA;
			REAL lengthBC = fractal->Cpara.iterC - fractal->Cpara.iterB;
			REAL grade1 = (tempA - temp0) / fractal->Cpara.iterA;
			REAL grade2 = (tempB - tempA) / lengthAB;
			REAL grade3 = (tempC - tempB) / lengthBC;

			// slopes
			if (aux->i < fractal->Cpara.iterA)
			{
				para = temp0 + (aux->i * grade1);
			}
			if (aux->i < fractal->Cpara.iterB && aux->i >= fractal->Cpara.iterA)
			{
				para = tempA + (aux->i - fractal->Cpara.iterA) * grade2;
			}
			if (aux->i >= fractal->Cpara.iterB)
			{
				para = tempB + (aux->i - fractal->Cpara.iterB) * grade3;
			}

			// Curvi part on "true"
			if (fractal->Cpara.enabledCurves)
			{
				REAL paraIt;
				if (lengthAB > 2.0f * fractal->Cpara.iterA) // stop  error, todo fix.
				{
					REAL curve1 = (grade2 - grade1) / (4.0f * fractal->Cpara.iterA);
					REAL tempL = lengthAB - fractal->Cpara.iterA;
					REAL curve2 = (grade3 - grade2) / (4.0f * tempL);
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
				}
				para += paraAdd;
			}
		}
	}
	// Parabolic
	if (fractal->Cpara.enabledParabFalse)
	{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
		paraAddP0 = fractal->Cpara.parabOffset0 + (aux->i * fractal->Cpara.parabSlope)
								+ (aux->i * aux->i * 0.001f * fractal->Cpara.parabScale);
		para += paraAddP0;
	}
	// para offset
	para += fractal->transformCommon.offset0;

	REAL div = 0.0f;
	// dot mode
	if (fractal->transformCommon.functionEnabledFalse)
	{
		div = dot(z, z);
	}
	else
	{
		div = length(z);
	}

	// using the parameter
	z *= 1.0f + para / -div;

	// post scale
	z *= fractal->transformCommon.scale;
	aux->DE = aux->DE * fractal->transformCommon.scale + 1.0f;

	// DE tweak
	aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	return z;
}