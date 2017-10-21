/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Inverse cylindrical coordinates, very easy transform
 * Formula by Luca GN 2011
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "TransfInvCylindricalIteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfInvCylindricalIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL newZx = z.x;
	REAL newZy = z.y;

	if (fractal->transformCommon.functionEnabledFalse) newZx = z.x * native_cos(z.y);
	if (fractal->transformCommon.functionEnabledxFalse) newZy = z.x * native_sin(z.y);

	z = (REAL4){z.x * native_cos(newZy * fractal->transformCommon.scale1),
				newZx * native_sin(z.y * fractal->transformCommon.scale1),
				z.z * fractal->transformCommon.scaleC1, z.w}
			* fractal->transformCommon.scaleA1;

	aux->DE = mad(aux->DE * fabs(fractal->transformCommon.scaleA1), fractal->transformCommon.scaleB1,
		fractal->transformCommon.offset1);
	aux->r_dz *= fabs(fractal->transformCommon.scaleA1) * fractal->transformCommon.scaleB1;
	return z;
}