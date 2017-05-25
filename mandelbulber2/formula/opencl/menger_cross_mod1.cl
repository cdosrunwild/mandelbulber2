/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Menger Cross Mod1
 * from code by Knighty
 * http://www.fractalforums.com/fragmentarium/
 * cross-menger!-can-anyone-do-this/msg93972/#new
 */

/* ### This file has been autogenerated. Remove this line, to prevent override. ### */

#ifndef DOUBLE_PRECISION
void MengerCrossMod1Iteration(float4 *z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	float4 gap = fractal->transformCommon.constantMultiplier000;

	if (fractal->transformCommon.functionEnabledx
			&& aux->i >= fractal->transformCommon.startIterations
			&& aux->i < fractal->transformCommon.stopIterations1)
	{
		z->y = fabs(z->y);
		z->z = fabs(z->z);
		if (fractal->transformCommon.functionEnabledFFalse) z->x = fabs(z->x);
		float dot1 = (mad(z->x, -SQRT_3_4, z->y * 0.5f)) * fractal->transformCommon.scale;
		float t = max(0.0f, dot1);
		z->x -= mad(t, -SQRT_3, -(0.5f * SQRT_3_4));

		z->y = fabs(z->y - t);

		if (z->y > z->z)
		{
			float temp = z->y;
			z->y = z->z;
			z->z = temp;
		}
		z->y -= 1.5f;
		*z -= gap * (float4){SQRT_3_4, -1.5f, 1.5f, 0.0f};

		if (z->z > z->x)
		{
			float temp = z->z;
			z->z = z->x;
			z->x = temp;
		}
		if (fractal->transformCommon.functionEnabledyFalse)
		{
			if (z->x >= 0.0f)
			{
				z->y = max(0.0f, z->y) * fractal->transformCommon.scaleA1;
				z->z = max(0.0f, z->z) * fractal->transformCommon.scaleB1;
			}
		}
		*z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, *z);
		aux->DE *= fractal->analyticDE.scale1; // tweak
	}

	if (fractal->transformCommon.functionEnabledy
			&& aux->i >= fractal->transformCommon.startIterationsA
			&& aux->i < fractal->transformCommon.stopIterationsA)
	{ // CrossMengerTrick
		z->y = fabs(z->y);
		z->z = fabs(z->z);
		if (fractal->transformCommon.functionEnabledzFalse) z->x = fabs(z->x);

		float dot1 = (mad(z->x, -SQRT_3_4, z->y * 0.5f));
		float t = 1.0f * max(0.0f, dot1);
		z->x -= t * -SQRT_3;
		if (fractal->transformCommon.functionEnabledXFalse)
			z->y = fabs(z->y) - t;
		else
		{
			z->y = fabs(z->y - t);
		}
		z->x -= SQRT_3_4;

		// Choose nearest corner/edge to get translation symmetry (all y & *z code)
		float dy = 0.0f;
		float dz = 0.0f;
		if (z->y > 0.5f && z->z > 0.5f) // if both y & *z > 0.5f  then =1.5
		{
			dy = 1.5f;
			dz = 1.5f;
		}
		else if (z->z < z->y)
		{
			dy = 1.5f; // and dz is unchanged
		}
		else
			dz = 1.5f; // and dy is unchanged

		z->y -= dy;
		z->z -= dz;
		*z *= fractal->transformCommon.scale3;
		aux->DE *= fractal->transformCommon.scale3;
		z->y += dy;
		z->z += dz;

		z->x += SQRT_3_4;

		if (fractal->transformCommon.functionEnabledPFalse)
		{
			z->x = fabs(z->x + fractal->transformCommon.offset) + fractal->transformCommon.offset0;
		}
		if (fractal->transformCommon.functionEnabledRFalse
				&& aux->i >= fractal->transformCommon.startIterationsR
				&& aux->i < fractal->transformCommon.stopIterationsR)
		{
			*z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix2, *z);
		}
	}
}
#else
void MengerCrossMod1Iteration(double4 *z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	double4 gap = fractal->transformCommon.constantMultiplier000;

	if (fractal->transformCommon.functionEnabledx
			&& aux->i >= fractal->transformCommon.startIterations
			&& aux->i < fractal->transformCommon.stopIterations1)
	{
		z->y = fabs(z->y);
		z->z = fabs(z->z);
		if (fractal->transformCommon.functionEnabledFFalse) z->x = fabs(z->x);
		double dot1 = (z->x * -SQRT_3_4 + z->y * 0.5) * fractal->transformCommon.scale;
		double t = max(0.0, dot1);
		z->x -= mad(t, -SQRT_3, -(0.5 * SQRT_3_4));

		z->y = fabs(z->y - t);

		if (z->y > z->z)
		{
			double temp = z->y;
			z->y = z->z;
			z->z = temp;
		}
		z->y -= 1.5;
		*z -= gap * (double4){SQRT_3_4, -1.5, 1.5, 0.0};

		if (z->z > z->x)
		{
			double temp = z->z;
			z->z = z->x;
			z->x = temp;
		}
		if (fractal->transformCommon.functionEnabledyFalse)
		{
			if (z->x >= 0.0)
			{
				z->y = max(0.0, z->y) * fractal->transformCommon.scaleA1;
				z->z = max(0.0, z->z) * fractal->transformCommon.scaleB1;
			}
		}
		*z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, *z);
		aux->DE *= fractal->analyticDE.scale1; // tweak
	}

	if (fractal->transformCommon.functionEnabledy
			&& aux->i >= fractal->transformCommon.startIterationsA
			&& aux->i < fractal->transformCommon.stopIterationsA)
	{ // CrossMengerTrick
		z->y = fabs(z->y);
		z->z = fabs(z->z);
		if (fractal->transformCommon.functionEnabledzFalse) z->x = fabs(z->x);

		double dot1 = (z->x * -SQRT_3_4 + z->y * 0.5);
		double t = 1.0 * max(0.0, dot1);
		z->x -= t * -SQRT_3;
		if (fractal->transformCommon.functionEnabledXFalse)
			z->y = fabs(z->y) - t;
		else
		{
			z->y = fabs(z->y - t);
		}
		z->x -= SQRT_3_4;

		// Choose nearest corner/edge to get translation symmetry (all y & *z code)
		double dy = 0.0;
		double dz = 0.0;
		if (z->y > 0.5 && z->z > 0.5) // if both y & *z > 0.5  then =1.5
		{
			dy = 1.5;
			dz = 1.5;
		}
		else if (z->z < z->y)
		{
			dy = 1.5; // and dz is unchanged
		}
		else
			dz = 1.5; // and dy is unchanged

		z->y -= dy;
		z->z -= dz;
		*z *= fractal->transformCommon.scale3;
		aux->DE *= fractal->transformCommon.scale3;
		z->y += dy;
		z->z += dz;

		z->x += SQRT_3_4;

		if (fractal->transformCommon.functionEnabledPFalse)
		{
			z->x = fabs(z->x + fractal->transformCommon.offset) + fractal->transformCommon.offset0;
		}
		if (fractal->transformCommon.functionEnabledRFalse
				&& aux->i >= fractal->transformCommon.startIterationsR
				&& aux->i < fractal->transformCommon.stopIterationsR)
		{
			*z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix2, *z);
		}
	}
}
#endif
