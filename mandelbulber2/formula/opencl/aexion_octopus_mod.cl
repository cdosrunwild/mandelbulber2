/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * aexion octopus. Based on Aex_Octo from Mandelbulber3D
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "AexionOctopusModIteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 AexionOctopusModIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 c = aux->const_c;

	REAL4 tempN;
	tempN.x = z.x * z.z * fractal->transformCommon.scale3D111.x;
	tempN.y = (mad(z.x, z.x, -z.z * z.z)) * fractal->transformCommon.scale3D111.y;
	tempN.z = z.y;

	if (fractal->transformCommon.functionEnabledAx) tempN.x = fabs(tempN.x);
	if (fractal->transformCommon.functionEnabledAy) tempN.y = fabs(tempN.y);
	if (fractal->transformCommon.functionEnabledAz) tempN.z = fabs(tempN.z);

	z = tempN;

	if (fractal->transformCommon.rotationEnabled)
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);

	z += fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		REAL4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.constantMultiplier000;

		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}
	return z;
}