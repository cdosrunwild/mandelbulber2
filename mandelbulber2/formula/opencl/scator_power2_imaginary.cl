/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * quadratic iteration in imaginary scator algebra
 * Use stop at maximum iteration (at maxiter)for the image to rendered correctly
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * ix-possibly-the-holy-grail-fractal-%28in-fff-lore%29
 * https://luz.izt.uam.mx/drupal/en/fractals/ix
 * @author Manuel Fernandez-Guasti

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_scator_power2_imaginary.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 ScatorPower2ImaginaryIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	REAL x2 = z.x * z.x; // + 1e-030f;
	REAL y2 = z.y * z.y;
	REAL z2 = z.z * z.z;

	REAL newx = x2 - y2 - z2 + (y2 * z2) / x2;
	REAL newy = 2.0f * z.x * z.y * (1.0f - z2 / x2);
	REAL newz = 2.0f * z.x * z.z * (1.0f - y2 / x2);

	z.x = newx;
	z.y = newy;
	z.z = newz;
	return z;
}