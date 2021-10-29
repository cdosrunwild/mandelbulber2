/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * poly fold atan2
 * @reference
 * https://fractalforums.org/fragmentarium/17/polyfoldsym-pre-transform/2684

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_transf_poly_fold_atan2.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfPolyFoldXYV1Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	if (aux->i >= fractal->transformCommon.startIterationsD
			&& aux->i < fractal->transformCommon.stopIterationsD1)
	{
		if (fractal->transformCommon.functionEnabledx) z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledy) z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledzFalse) z.z = fabs(z.z);
		z += fractal->transformCommon.offset000;
		REAL psi = M_PI_F / fractal->transformCommon.int8X;
		psi = fabs(fmod(atan2(z.y, z.x) + psi, 2.0f * psi) - psi);
		REAL len = native_sqrt(z.y * z.y + z.x * z.x);
		REAL cz = native_cos(psi) * len;
		REAL sz = native_sin(psi) * len;
		z.x = sz + (cz - sz) * fractal->transformCommon.scaleF1;
		z.y = cz + (sz - cz) * fractal->transformCommon.scaleG1;
	}

	if (fractal->transformCommon.rotation2EnabledFalse
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
	{
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	}
	if (aux->i >= fractal->transformCommon.startIterationsA
			&& aux->i < fractal->transformCommon.stopIterationsA)
	{
		z += fractal->transformCommon.additionConstant000;
	}

	// DE tweaks
	if (fractal->analyticDE.enabledFalse)
	{
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	}
	return z;
}
