/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Hybrid Color Trial2
 *
 * for folds the aux.color is updated each iteration
 * depending on which slots have formulas that use it
 *
 *
 * bailout may need to be adjusted with some formulas

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "TransfHybridColor2Iteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TransfHybridColor2Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL auxColor = 0.0f;
	REAL R2 = 0.0f;

	REAL distEst = 0.0f;
	REAL XYZbias = 0.0f;
	REAL planeBias = 0.0f;
	// REAL divideByIter = 0.0f;
	REAL radius = 0.0f;
	REAL linearOffset = 0.0f;
	// REAL factorR = fractal->mandelbox.color.factorR;
	REAL componentMaster = 0.0f;
	REAL minValue = 0.0f;
	// REAL4 lastPoint = aux->old_z;
	REAL lengthIter = 0.0f;
	REAL boxTrap = 0.0f;
	REAL sphereTrap = 0.0f;
	float sumDist = 0.0f;
	float addI = 0.0f;

	// used to turn off or mix with old hybrid color and orbit traps
	aux->oldHybridFactor *= fractal->foldColor.oldScale1;
	aux->minRFactor = fractal->foldColor.scaleC0; // orbit trap weight

	/*{ // length of last movement before termination
		REAL4 vecIter =  fabs(z - aux->old_z);
		lengthIter = length(vecIter) * aux->i; // (aux->i + 1.0f);
		aux->old_z = z;
	}*/

	{
		// radius
		if (fractal->transformCommon.functionEnabledCyFalse)
		{
			radius = length(z);
			radius *= fractal->foldColor.scaleG0;

			if (fractal->transformCommon.functionEnabledxFalse)
			{
				radius *= native_recip(fabs(aux->DE));
				// if (radius > 20) radius = 20;
			}
		}

		// radius squared components
		if (fractal->transformCommon.functionEnabledRFalse)
		{
			REAL temp0 = 0.0f;
			REAL temp1 = 0.0f;
			REAL4 c = aux->c;
			temp0 = dot(c, c) * fractal->foldColor.scaleA0; // initial R2
			temp1 = dot(z, z) * fractal->foldColor.scaleB0;
			R2 = temp0 + temp1;
		}

		// total distance squared
		if (fractal->foldColor.distanceEnabledFalse)
		{
			/*REAL4 subVs = z - aux->old_z;
			aux->addDist += dot(subVs, subVs) * fractal->foldColor.scaleB1;
			sumDist = aux->addDist;*/

			/*aux->sum_z +=(z); // fabs
			REAL4 sumZ = aux->sum_z;
			sumDist = dot(sumZ, sumZ) * fractal->foldColor.scaleB1;*/

			REAL4 subV = z - aux->old_z;
			// sumDist = dot(subV, subV) * native_divide(fractal->foldColor.scaleB1, 1000.0f);
			subV = fabs(subV);
			// sumDist = max(max(subV.x, subV.y), subV.z)  * fraboxMod11 invC ab4actal->foldColor.scaleB1
			// / 1000.0f;
			sumDist = min(min(subV.x, subV.y), subV.z) * native_divide(fractal->foldColor.scaleB1, 10.0f);

			// update
			aux->old_z = z;
		}

		// DE component
		if (fractal->transformCommon.functionEnabledDFalse)
		{
			if (fractal->transformCommon.functionEnabledBxFalse)
				distEst = aux->r_dz;
			else
				distEst = aux->DE;
			REAL temp5 = 0.0f;
			temp5 = distEst * fractal->foldColor.scaleD0;
			if (fractal->transformCommon.functionEnabledByFalse) temp5 *= native_recip((aux->i + 1.0f));
			if (fractal->transformCommon.functionEnabledBzFalse)
				temp5 *= native_recip((mad(aux->i, aux->i, 1.0f)));
			distEst = temp5;
		}

		// aux->color fold component
		if (fractal->transformCommon.functionEnabledAxFalse)
		{
			auxColor = aux->color;
			REAL temp8 = 0.0f;
			temp8 = auxColor * fractal->foldColor.scaleF0;
			auxColor = temp8;
		}

		// max linear offset
		if (fractal->transformCommon.functionEnabledMFalse)
		{
			REAL temp30 = 0.0f;
			REAL4 temp31 = z;
			if (fractal->transformCommon.functionEnabledM) temp31 = fabs(temp31);

			temp30 = max(max(temp31.x, temp31.y), temp31.z);
			temp30 *= fractal->foldColor.scaleA1;
			linearOffset = temp30;
		}

		// box trap
		if (fractal->surfBox.enabledX2False)
		{
			REAL4 box = fractal->transformCommon.scale3D444;
			REAL4 temp35 = z;
			REAL temp39 = 0.0f;

			if (fractal->transformCommon.functionEnabledCx) temp35 = fabs(temp35);

			temp35 = box - temp35;
			REAL temp36 = max(max(temp35.x, temp35.y), temp35.z);
			REAL temp37 = min(min(temp35.x, temp35.y), temp35.z);
			temp36 = mad(fractal->transformCommon.offsetB0, temp37, temp36);
			temp36 *= fractal->transformCommon.scaleC1;

			if (fractal->surfBox.enabledY2False)
			{
				REAL4 temp38 = aux->c;

				if (fractal->transformCommon.functionEnabledCz) temp38 = fabs(temp38);
				temp38 = box - temp38;

				temp39 = max(max(temp38.x, temp38.y), temp38.z);
				REAL temp40 = min(min(temp38.x, temp38.y), temp38.z);
				temp39 = mad(fractal->transformCommon.offsetA0, temp40, temp39);
				temp39 *= fractal->transformCommon.scaleE1;
			}
			boxTrap = temp36 + temp39;
		}

		// sphere trap
		if (fractal->transformCommon.functionEnabledzFalse)
		{
			REAL sphereR2 = fractal->transformCommon.maxR2d1;
			REAL temp45 = dot(z, z);
			REAL temp46 = sphereR2 - temp45;
			REAL temp47 = temp46;
			REAL temp51 = temp46;
			if (fractal->transformCommon.functionEnabledAx) temp51 = fabs(temp51);
			temp51 *= fractal->transformCommon.scaleF1;

			if (fractal->transformCommon.functionEnabledyFalse && temp45 > sphereR2)
			{
				temp46 *= temp46 * fractal->transformCommon.scaleG1;
			}
			if (fractal->transformCommon.functionEnabledPFalse && temp45 < sphereR2)
			{
				temp47 *= temp47 * fractal->transformCommon.scaleB1;
			}
			sphereTrap = temp51 + temp47 + temp46;
		}

		// XYZ bias
		if (fractal->transformCommon.functionEnabledCxFalse)
		{
			REAL4 temp10 = z;
			if (fractal->transformCommon.functionEnabledSFalse)
			{
				temp10.x *= temp10.x;
			}
			else
			{
				temp10.x = fabs(temp10.x);
			}
			if (fractal->transformCommon.functionEnabledSwFalse)
			{
				temp10.y *= temp10.y;
			}
			else
			{
				temp10.y = fabs(temp10.y);
			}

			if (fractal->transformCommon.functionEnabledXFalse)
			{
				temp10.z *= temp10.z;
			}
			else
			{
				temp10.z = fabs(temp10.z);
			}
			temp10 = temp10 * fractal->transformCommon.additionConstantA000;

			XYZbias = temp10.x + temp10.y + temp10.z;
		}

		// plane bias
		if (fractal->transformCommon.functionEnabledAzFalse)
		{
			REAL4 tempP = z;
			if (fractal->transformCommon.functionEnabledEFalse)
			{
				tempP.x = tempP.x * tempP.y;
				tempP.x *= tempP.x;
			}
			else
			{
				tempP.x = fabs(tempP.x * tempP.y);
			}
			if (fractal->transformCommon.functionEnabledFFalse)
			{
				tempP.y = tempP.y * tempP.z;
				tempP.y *= tempP.y;
			}
			else
			{
				tempP.y = fabs(tempP.y * tempP.z);
			}

			if (fractal->transformCommon.functionEnabledKFalse)
			{
				tempP.z = tempP.z * tempP.x;
				tempP.z *= tempP.z;
			}
			else
			{
				tempP.z = fabs(tempP.z * tempP.x);
			}

			tempP = tempP * fractal->transformCommon.scale3D000;
			planeBias = tempP.x + tempP.y + tempP.z;
		}

		// build  componentMaster
		componentMaster = (fractal->foldColor.colorMin + R2 + distEst + auxColor + XYZbias + planeBias
											 + radius + lengthIter + linearOffset + boxTrap + sphereTrap + sumDist);
	}

	// divide by i option
	if (fractal->transformCommon.functionEnabledCzFalse
			&& aux->i >= fractal->transformCommon.startIterationsT
			&& aux->i < fractal->transformCommon.stopIterationsT)
	{
		componentMaster +=
			componentMaster * (1.0f + native_divide(fractal->transformCommon.scale, (aux->i + 1.0f)));
	}

	// non-linear palette options
	if (fractal->foldColor.parabEnabledFalse)
	{ // parabolic
		componentMaster += (componentMaster * componentMaster * fractal->foldColor.parabScale0);
	}
	if (fractal->foldColor.cosEnabledFalse)
	{ // trig
		REAL trig =
			128 * -fractal->foldColor.trigAdd1
			* (native_cos(componentMaster * 2.0f * native_divide(M_PI_F, fractal->foldColor.period1))
					- 1.0f);
		componentMaster += trig;
	}
	if (fractal->transformCommon.functionEnabledAyFalse)
	{ // log
		REAL logCurve = log(componentMaster + 1.0f) * fractal->foldColor.scaleE0;
		componentMaster += logCurve;
	}

	// limit componentMaster
	if (componentMaster < fractal->foldColor.limitMin0)
		componentMaster = fractal->foldColor.limitMin0;
	if (componentMaster > fractal->foldColor.limitMax9999)
		componentMaster = fractal->foldColor.limitMax9999;

	// final component value + cumulative??
	{
		// aux->colorHybrid =
		//	(componentMaster * 256.0f) ; //+ (lastColorValue );ppppppppppppppppppp
	}
	// aux->temp100 *= fractal->transformCommon.scale0;

	aux->colorHybrid = componentMaster;

	// if (aux->i >= fractal->transformCommon.startIterationsD
	//		&& aux->i < fractal->transformCommon.stopIterationsD)
	//{
	addI = aux->i * fractal->foldColor.scaleC1;
	//}

	aux->colorHybrid += addI;

	if (fractal->surfBox.enabledZ2False)
	{
		if (componentMaster < aux->temp100 * fractal->transformCommon.scaleA1)
		{
			aux->temp100 = componentMaster;
		}
		minValue = aux->temp100;

		aux->colorHybrid += (minValue - aux->colorHybrid) * fractal->surfBox.scale1Z1;
		//	mad(aux->colorHybrid, (1.0f - fractal->surfBox.scale1Z1), (minValue *
		// fractal->surfBox.scale1Z1));
	}

	aux->colorHybrid *= fractal->foldColor.newScale0 * 256.0f;

	// master controls color
	// aux->foldFactor = fractal->foldColor.compFold; // fold group weight

	// REAL scaleColor =
	//	 +  fabs(aux->actualScaleA);
	// scaleColor += fabs(fractal->mandelbox.scale);
	// aux->scaleFactor = scaleColor * fractal->foldColor.compScale;*/
	return z;
}