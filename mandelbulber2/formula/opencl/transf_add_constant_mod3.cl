/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2019 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Adds c constant to z vector
 * This formula contains aux.pos_neg

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the file "fractal_transf_add_constant_mod2.cpp" in the folder formula/definition
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfAddConstantMod3Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL4 signs = (REAL4)(1.0, 1.0, 1.0, 1.0);
	signs.x *= sign(aux->const_c.x);
	signs.y *= sign(aux->const_c.y);
	signs.z *= sign(aux->const_c.z);

	REAL4 offset = fractal->transformCommon.additionConstantA000;
	if (fractal->transformCommon.functionEnabledDFalse) offset *= signs;

	REAL4 t;
	if (!fractal->transformCommon.functionEnabledBFalse) t = aux->const_c - offset;
	else t = z - offset;
	REAL r;
	if (!fractal->transformCommon.functionEnabledAFalse) r = length(t);
	else r = dot(t, t);

	REAL4 offset1 = fractal->transformCommon.offset111;
	if (fractal->transformCommon.functionEnabledCFalse) offset1 *= signs;

	REAL m = (1.0f - fractal->transformCommon.radius1 / r) * fractal->transformCommon.scale1;

	if (r > fractal->transformCommon.radius1)
	{
		 offset1 =  offset1 + t * m;
		z +=  offset1;
	}

	// plus iter control and alternate offset
	/*if (fractal->transformCommon.functionEnabledAxFalse)
	{
		if (aux->i >= fractal->transformCommon.startIterationsA
				&& aux->i < fractal->transformCommon.stopIterationsA)
		{
			if (fractal->transformCommon.functionEnabledBxFalse)
			{
				z.x += aux->pos_neg * fractal->transformCommon.additionConstant000.x;
			}
			else
			{
				z.x += fractal->transformCommon.additionConstant000.x;
			}
		}
	}

	if (fractal->transformCommon.functionEnabledAyFalse)
	{
		if (aux->i >= fractal->transformCommon.startIterationsB
				&& aux->i < fractal->transformCommon.stopIterationsB)
		{
			if (fractal->transformCommon.functionEnabledByFalse)
			{
				z.y += aux->pos_neg * fractal->transformCommon.additionConstant000.y;
			}
			else
			{
				z.y += fractal->transformCommon.additionConstant000.y;
			}
		}
	}

	if (fractal->transformCommon.functionEnabledAzFalse)
	{
		if (aux->i >= fractal->transformCommon.startIterationsC
				&& aux->i < fractal->transformCommon.stopIterationsC)
		{
			if (fractal->transformCommon.functionEnabledBzFalse)
			{
				z.z += aux->pos_neg * fractal->transformCommon.additionConstant000.z;
			}
			else
			{
				z.z += fractal->transformCommon.additionConstant000.z;
			}
		}
	}
	aux->pos_neg *= fractal->transformCommon.scaleNeg1;*/
	return z;
}
