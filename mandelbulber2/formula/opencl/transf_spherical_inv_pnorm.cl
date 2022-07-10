/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2021 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * TransfSphericalInvPnorm using p-norm

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_transf_spherical_inv_pnorm.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfSphericalInvPnormIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 lpN = fabs(z);
	REAL pr = fractal->transformCommon.scale2;
	lpN.x = pow(lpN.x, pr);
	lpN.y = pow(lpN.y, pr);
	lpN.z = pow(lpN.z, pr);

	REAL pNorm = lpN.x + lpN.y + lpN.z;
	if (fractal->transformCommon.functionEnabledFalse) pNorm += pow(lpN.w, pr);
	pNorm = pow(pNorm, 1.0f / pr);

	pNorm = pow(pNorm, fractal->transformCommon.scaleA2);
	pNorm = max(pNorm, fractal->transformCommon.offset0);

	REAL useScale = fractal->transformCommon.scale1 - aux->actualScaleA;
	if (fractal->transformCommon.functionEnabledKFalse) // update actualScaleA
		aux->actualScaleA = fractal->transformCommon.scaleVary0 * (fabs(aux->actualScaleA) + 1.0f);
	pNorm = useScale / pNorm;
	z *= pNorm;
	aux->DE *= fabs(pNorm);

	if (fractal->analyticDE.enabledFalse)
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	return z;
}