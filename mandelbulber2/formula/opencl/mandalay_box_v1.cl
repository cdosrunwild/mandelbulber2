/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2020 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Based on a DarkBeam fold formula adapted by Knighty
 * http://www.fractalforums.com/amazing-box-amazing-surf-and-variations/'new'-fractal-type-mandalay/msg81348/#msg81348

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_mandalay_box_v1.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 MandalayBoxV1Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	// tglad fold
	if (fractal->transformCommon.functionEnabledAFalse)
		z = fabs(z + fractal->transformCommon.additionConstant111)
				- fabs(z - fractal->transformCommon.additionConstant111) - z;

	REAL signX = sign(z.x);
	REAL signY = sign(z.y);
	REAL signZ = sign(z.z);

	z = fabs(z);

	REAL4 fo = fractal->transformCommon.additionConstant0555;
	REAL4 g = fractal->transformCommon.offsetA000;
	REAL4 p = z;
	REAL4 q = z;

	REAL t1, t2, v, v1;

	if (p.z > p.y)
	{
		REAL temp = p.y;
		p.y = p.z;
		p.z = temp;
	}
	t1 = p.x - 2.0f * fo.x;
	t2 = p.y - 4.0f * fo.x;
	v = max(fabs(t1 + fo.x) - fo.x, t2);
	v1 = max(t1 - g.x, p.y);
	v = min(v, v1);
	q.x = min(v, p.x);

	if (!fractal->transformCommon.functionEnabledSwFalse)
		p = z;
	else
		p = q;

	if (p.x > p.z)
	{
		REAL temp = p.z;
		p.z = p.x;
		p.x = temp;
	}
	t1 = p.y - 2.0f * fo.y;
	t2 = p.z - 4.0f * fo.y;
	v = max(fabs(t1 + fo.y) - fo.y, t2);
	v1 = max(t1 - g.y, p.z);
	v = min(v, v1);
	q.y = min(v, p.y);

	if (!fractal->transformCommon.functionEnabledSwFalse)
		p = z;
	else
		p = q;

	if (p.y > p.x)
	{
		REAL temp = p.x;
		p.x = p.y;
		p.y = temp;
	}
	t1 = p.z - 2.0f * fo.z;
	t2 = p.x - 4.0f * fo.z;
	v = max(fabs(t1 + fo.z) - fo.z, t2);
	v1 = max(t1 - g.z, p.x);
	v = min(v, v1);
	q.z = min(v, p.z);

	z = q;

	z.x *= signX;
	z.y *= signY;
	z.z *= signZ;

	// spherical fold and scale
	REAL rr = dot(z, z);
	REAL t = min(1.0f / fractal->transformCommon.minR2p25, max(1.0f, 1.0f / rr));
	z *= fractal->transformCommon.scale2 * t;
	aux->DE = aux->DE * fabs(fractal->transformCommon.scale2) * t + 1.0f;

	// rotation
	z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);

	if (fractal->analyticDE.enabledFalse)
		aux->DE = aux->DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;

	// temp code
	p = fabs(z);
	aux->dist = max(p.x, max(p.y, p.z));
	aux->dist = aux->dist / aux->DE;

	return z;
}