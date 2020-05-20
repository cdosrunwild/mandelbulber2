/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * DarkBeam's MixPinski4 from M3D
 * A strange but intriguing fractal, that mixes Sierpinski and Menger folds.
 * The amazing thing is that in 3D it does not work so well! LUCA GN 2011
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_mix_pinski4d.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MixPinski4dIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	if (aux->i >= fractal->transformCommon.startIterationsS
			&& aux->i < fractal->transformCommon.stopIterationsS)
	{
		REAL temp;
		if (z.x + z.y < 0.0f)
		{
			temp = z.x;
			z.x = -z.y;
			z.y = -temp;
		}

		if (z.x + z.z < 0.0f)
		{
			temp = z.x;
			z.x = -z.z;
			z.z = -temp;
		}

		if (z.y + z.z < 0.0f)
		{
			temp = z.z;
			z.z = -z.y;
			z.y = -temp;
		}

		if (z.x + z.w < 0.0f)
		{
			temp = z.x;
			z.x = -z.w;
			z.w = -temp;
		}

		if (z.y + z.w < 0.0f)
		{
			temp = z.y;
			z.y = -z.w;
			z.w = -temp;
		}

		if (z.z + z.w < 0.0f)
		{
			temp = z.z;
			z.z = -z.w;
			z.w = -temp;
		}
		z *= fractal->transformCommon.scale1;
		aux->DE *= fabs(fractal->transformCommon.scale1);
	}

	if (aux->i >= fractal->transformCommon.startIterationsC
			&& aux->i < fractal->transformCommon.stopIterationsC)
	{
		z += fractal->transformCommon.additionConstant0000; // offset
	}
	// 6 plane rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
	{
		REAL4 tp;
		if (fractal->transformCommon.rotation44a.x != 0)
		{
			tp = z;
			REAL alpha = fractal->transformCommon.rotation44a.x * M_PI_180_F;
			z.x = tp.x * native_cos(alpha) + tp.y * native_sin(alpha);
			z.y = tp.x * -native_sin(alpha) + tp.y * native_cos(alpha);
		}
		if (fractal->transformCommon.rotation44a.y != 0)
		{
			tp = z;
			REAL beta = fractal->transformCommon.rotation44a.y * M_PI_180_F;
			z.y = tp.y * native_cos(beta) + tp.z * native_sin(beta);
			z.z = tp.y * -native_sin(beta) + tp.z * native_cos(beta);
		}
		if (fractal->transformCommon.rotation44a.z != 0)
		{
			tp = z;
			REAL gamma = fractal->transformCommon.rotation44a.z * M_PI_180_F;
			z.x = tp.x * native_cos(gamma) + tp.z * native_sin(gamma);
			z.z = tp.x * -native_sin(gamma) + tp.z * native_cos(gamma);
		}
		if (fractal->transformCommon.rotation44b.x != 0)
		{
			tp = z;
			REAL delta = fractal->transformCommon.rotation44b.x * M_PI_180_F;
			z.x = tp.x * native_cos(delta) + tp.w * native_sin(delta);
			z.w = tp.x * -native_sin(delta) + tp.w * native_cos(delta);
		}
		if (fractal->transformCommon.rotation44b.y != 0)
		{
			tp = z;
			REAL epsilon = fractal->transformCommon.rotation44b.y * M_PI_180_F;
			z.y = tp.y * native_cos(epsilon) + tp.w * native_sin(epsilon);
			z.w = tp.y * -native_sin(epsilon) + tp.w * native_cos(epsilon);
		}
		if (fractal->transformCommon.rotation44b.z != 0)
		{
			tp = z;
			REAL zeta = fractal->transformCommon.rotation44b.z * M_PI_180_F;
			z.z = tp.z * native_cos(zeta) + tp.w * native_sin(zeta);
			z.w = tp.z * -native_sin(zeta) + tp.w * native_cos(zeta);
		}
	}
	if (aux->i >= fractal->transformCommon.startIterationsM
			&& aux->i < fractal->transformCommon.stopIterationsM)
	{
		REAL scaleM = fractal->transformCommon.scale2;
		REAL4 offsetM = fractal->transformCommon.additionConstant111d5;
		z.x = scaleM * z.x - offsetM.x * (scaleM - 1.0f);
		z.y = scaleM * z.y - offsetM.y * (scaleM - 1.0f);
		z.w = scaleM * z.w - offsetM.w * (scaleM - 1.0f);
		z.z -= 0.5f * offsetM.z * (scaleM - 1.0f) / scaleM;
		z.z = -fabs(-z.z);
		z.z += 0.5f * offsetM.z * (scaleM - 1.0f) / scaleM;
		z.z = scaleM * z.z;
		aux->DE *= fabs(scaleM) * fractal->analyticDE.scale1;
	}
	return z;
}