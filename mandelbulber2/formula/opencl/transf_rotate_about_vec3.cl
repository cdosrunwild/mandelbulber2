/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * rotate about vec3

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_transf_rotate_about_vec3.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfRotateAboutVec3Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL useAngle = fractal->transformCommon.angle0;

	if (fractal->transformCommon.functionEnabledEFalse)
	{
		if (aux->i > fractal->transformCommon.startIterationsRV)
		{
			useAngle *= (1.0f
										- 1.0f
												/ (1.0f
													 + (aux->i - fractal->transformCommon.startIterationsRV)
															 / fractal->transformCommon.offset0))
									* fractal->transformCommon.scale1;
		}

		REAL4 v = fractal->transformCommon.vec111;
		v = v / length(v); // normalise
		REAL c = native_cos(useAngle * M_PI_180_F);
		REAL s = native_sin(useAngle * M_PI_180_F);
		REAL4 rotVec = (REAL4){0.0f, 0.0f, 0.0f, z.w};

		rotVec.x = z.x * (c + (1.0f - c) * v.x * v.x) + z.y * ((1.0f - c) * v.x * v.y + s * v.z)
							 + z.z * ((1.0f - c) * v.x * v.z - s * v.y);
		rotVec.y = z.x * ((1.0f - c) * v.x * v.y - s * v.z) + z.y * (c + (1.0f - c) * v.y * v.y)
							 + z.z * ((1.0f - c) * v.y * v.z + s * v.x);
		rotVec.z = z.x * ((1.0f - c) * v.x * v.z + s * v.y) + z.y * ((1.0f - c) * v.y * v.z - s * v.x)
							 + z.z * (c + (1.0f - c) * v.z * v.z);
		z = rotVec;
	}
	else
	{
		REAL4 v = fractal->transformCommon.vec111;
		v = v / length(v); // normalise
		REAL c = native_cos(useAngle * M_PI_180_F);
		REAL s = native_sin(useAngle * M_PI_180_F);
		REAL4 rotVec = (REAL4){0.0f, 0.0f, 0.0f, z.w};

		rotVec.x = z.x * (c + (1.0f - c) * v.x * v.x) + z.y * ((1.0f - c) * v.x * v.y + s * v.z)
							 + z.z * ((1.0f - c) * v.x * v.z - s * v.y);
		rotVec.y = z.x * ((1.0f - c) * v.x * v.y - s * v.z) + z.y * (c + (1.0f - c) * v.y * v.y)
							 + z.z * ((1.0f - c) * v.y * v.z + s * v.x);
		rotVec.z = z.x * ((1.0f - c) * v.x * v.z + s * v.y) + z.y * ((1.0f - c) * v.y * v.z - s * v.x)
							 + z.z * (c + (1.0f - c) * v.z * v.z);
		z = rotVec;

		/*	REAL4 v = fractal->transformCommon.vec111;
			v = v / length(v); // normalise
			REAL c = native_cos(useAngle * M_PI_180_F);
			REAL s = native_sin(useAngle * M_PI_180_F);
			REAL4 rotVec = (REAL4) {0.0f, 0.0f, 0.0f, z.w};

			CMatrix44 rotM = CMatrix44(c + (1.0f - c) * v.x * v.x, (1.0f - c) * v.x * v.y - s * v.z, (1.0f
			- c) * v.x * v.z + s * v.y, 0.0f, (1.0f - c) * v.x * v.y + s * v.z, c + (1.0f - c) * v.y *
			v.y, (1.0f
			- c) * v.y * v.z - s * v.x, 0.0f, (1.0f - c) * v.x * v.z - s * v.y, (1.0f - c) * v.y * v.z + s
			* v.x, c + (1.0f - c) * v.z * v.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
				);
			z *= rotM;*/
	}

	// DE tweak
	if (fractal->analyticDE.enabledFalse)
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	return z;
}