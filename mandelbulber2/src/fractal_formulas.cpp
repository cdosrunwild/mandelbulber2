/**
 * Mandelbulber v2, a 3D fractal generator       ,=#MKNmMMKmmßMNWy,
 *                                             ,B" ]L,,p%%%,,,§;, "K
 * Copyright (C) 2014-17 Mandelbulber Team     §R-==%w["'~5]m%=L.=~5N
 *                                        ,=mm=§M ]=4 yJKA"/-Nsaj  "Bw,==,,
 * This file is part of Mandelbulber.    §R.r= jw",M  Km .mM  FW ",§=ß., ,TN
 *                                     ,4R =%["w[N=7]J '"5=],""]]M,w,-; T=]M
 * Mandelbulber is free software:     §R.ß~-Q/M=,=5"v"]=Qf,'§"M= =,M.§ Rz]M"Kw
 * you can redistribute it and/or     §w "xDY.J ' -"m=====WeC=\ ""%""y=%"]"" §
 * modify it under the terms of the    "§M=M =D=4"N #"%==A%p M§ M6  R' #"=~.4M
 * GNU General Public License as        §W =, ][T"]C  §  § '§ e===~ U  !§[Z ]N
 * published by the                    4M",,Jm=,"=e~  §  §  j]]""N  BmM"py=ßM
 * Free Software Foundation,          ]§ T,M=& 'YmMMpM9MMM%=w=,,=MT]M m§;'§,
 * either version 3 of the License,    TWw [.j"5=~N[=§%=%W,T ]R,"=="Y[LFT ]N
 * or (at your option)                   TW=,-#"%=;[  =Q:["V""  ],,M.m == ]N
 * any later version.                      J§"mr"] ,=,," =="""J]= M"M"]==ß"
 *                                          §= "=C=4 §"eM "=B:m|4"]#F,§~
 * Mandelbulber is distributed in            "9w=,,]w em%wJ '"~" ,=,,ß"
 * the hope that it will be useful,                 . "K=  ,=RMMMßM"""
 * but WITHOUT ANY WARRANTY;                            .'''
 * without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with Mandelbulber. If not, see <http://www.gnu.org/licenses/>.
 *
 * ###########################################################################
 *
 * Authors: Krzysztof Marczak (buddhi1980@gmail.com), Graeme McLaren,
 *  Sebastian Jennen (jenzebas@gmail.com)
 *
 * functions for calculation single fractal iterations
 */

#include "fractal_formulas.hpp"
#include "common_math.h"
#include "common_params.hpp"
#include "fractal.h"

using namespace fractal;
using std::swap;

/**
 * Classic Mandelbulb fractal.
 * @reference http://www.fractalforums.com/3d-fractal-generation/true-3d-mandlebrot-type-fractal/
 */
void MandelbulbIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	// if (aux.r < 1e-21) aux.r = 1e-21;
	double th0 = asin(z.z / aux.r) + fractal->bulb.betaAngleOffset;
	double ph0 = atan2(z.y, z.x) + fractal->bulb.alphaAngleOffset;
	double rp = pow(aux.r, fractal->bulb.power - 1.0);
	double th = th0 * fractal->bulb.power;
	double ph = ph0 * fractal->bulb.power;
	double cth = cos(th);
	aux.r_dz = (rp * aux.r_dz) * fractal->bulb.power + 1.0;
	rp *= aux.r;
	z.x = cth * cos(ph) * rp;
	z.y = cth * sin(ph) * rp;
	z.z = sin(th) * rp;
}

/**
 * Mandelbox fractal known as AmazingBox or ABox, invented by Tom Lowe in 2010
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/amazing-fractal/msg12467/#msg12467
 */
void MandelboxIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->mandelbox.rotationsEnabled)
	{
		CVector4 zRot;
		// cast vector to array pointer for address taking of components in opencl
		double *zRotP = (double *)&zRot;
		const double *colP = (const double *)&fractal->mandelbox.color.factor;
		for (int dim = 0; dim < 3; dim++)
		{
			// handle each dimension x, y and z sequentially in pointer var dim
			double *rotDim = (dim == 0) ? &zRotP[0] : ((dim == 1) ? &zRotP[1] : &zRotP[2]);
			const double *colorFactor = (dim == 0) ? &colP[0] : ((dim == 1) ? &colP[1] : &colP[2]);

			zRot = fractal->mandelbox.rot[0][dim].RotateVector(z);
			if (*rotDim > fractal->mandelbox.foldingLimit)
			{
				*rotDim = fractal->mandelbox.foldingValue - *rotDim;
				z = fractal->mandelbox.rotinv[0][dim].RotateVector(zRot);
				aux.color += *colorFactor;
			}
			else
			{
				zRot = fractal->mandelbox.rot[1][dim].RotateVector(z);
				if (*rotDim < -fractal->mandelbox.foldingLimit)
				{
					*rotDim = -fractal->mandelbox.foldingValue - *rotDim;
					z = fractal->mandelbox.rotinv[1][dim].RotateVector(zRot);
					aux.color += *colorFactor;
				}
			}
		}
	}
	else
	{
		if (fabs(z.x) > fractal->mandelbox.foldingLimit)
		{
			z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
			aux.color += fractal->mandelbox.color.factor.x;
		}
		if (fabs(z.y) > fractal->mandelbox.foldingLimit)
		{
			z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
			aux.color += fractal->mandelbox.color.factor.y;
		}
		if (fabs(z.z) > fractal->mandelbox.foldingLimit)
		{
			z.z = sign(z.z) * fractal->mandelbox.foldingValue - z.z;
			aux.color += fractal->mandelbox.color.factor.z;
		}
	}

	double r2 = z.Dot(z);

	z += fractal->mandelbox.offset;

	if (r2 < fractal->mandelbox.mR2)
	{
		z *= fractal->mandelbox.mboxFactor1;
		aux.DE *= fractal->mandelbox.mboxFactor1;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < fractal->mandelbox.fR2)
	{
		double tglad_factor2 = fractal->mandelbox.fR2 / r2;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}

	z -= fractal->mandelbox.offset;

	if (fractal->mandelbox.mainRotationEnabled) z = fractal->mandelbox.mainRot.RotateVector(z);

	z = z * fractal->mandelbox.scale;
	aux.DE = aux.DE * fabs(fractal->mandelbox.scale) + 1.0;
}

/**
 * Fractal formula created by Buddhi
 */
void Mandelbulb2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	aux.r_dz = aux.r_dz * 2.0 * aux.r;

	double temp, tempR;
	tempR = sqrt(z.x * z.x + z.y * z.y); //+ 1e-061
	z *= 1.0 / tempR;
	temp = z.x * z.x - z.y * z.y;
	z.y = 2.0 * z.x * z.y;
	z.x = temp;
	z *= tempR;

	tempR = sqrt(z.y * z.y + z.z * z.z); //+ 1e-061
	z *= 1.0 / tempR;
	temp = z.y * z.y - z.z * z.z;
	z.z = 2.0 * z.y * z.z;
	z.y = temp;
	z *= tempR;

	tempR = sqrt(z.x * z.x + z.z * z.z); //+ 1e-061
	z *= 1.0 / tempR;
	temp = z.x * z.x - z.z * z.z;
	z.z = 2.0 * z.x * z.z;
	z.x = temp;
	z *= tempR;

	z *= aux.r;
}

/**
 * Fractal formula created by Buddhi
 */
void Mandelbulb3Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	aux.r_dz = aux.r_dz * 2.0 * aux.r;

	double temp, tempR;

	double sign = 1.0;
	double sign2 = 1.0;

	if (z.x < 0.0) sign2 = -1.0;
	tempR = sqrt(z.x * z.x + z.y * z.y); //+ 1e-061
	z *= 1.0 / tempR;
	temp = z.x * z.x - z.y * z.y;
	z.y = 2.0 * z.x * z.y;
	z.x = temp;
	z *= tempR;

	if (z.x < 0.0) sign = -1.0;
	tempR = sqrt(z.x * z.x + z.z * z.z); //+ 1e-061
	z *= 1.0 / tempR;
	temp = z.x * z.x - z.z * z.z;
	z.z = 2.0 * z.x * z.z * sign2;
	z.x = temp * sign;
	z *= tempR;

	z *= aux.r;
}

/**
 * Fractal formula created by Buddhi
 */
void Mandelbulb4Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double rp = pow(aux.r, fractal->bulb.power - 1.0);
	aux.r_dz = rp * aux.r_dz * fractal->bulb.power + 1.0;

	double angZ = atan2(z.y, z.x) + fractal->bulb.alphaAngleOffset;
	double angY = atan2(z.z, z.x) + fractal->bulb.betaAngleOffset;
	double angX = atan2(z.z, z.y) + fractal->bulb.gammaAngleOffset;

	CRotationMatrix rotM;
	rotM.RotateX(angX * (fractal->bulb.power - 1.0));
	rotM.RotateY(angY * (fractal->bulb.power - 1.0));
	rotM.RotateZ(angZ * (fractal->bulb.power - 1.0));

	z = rotM.RotateVector(z) * rp;
}

/**
 * Classic Mandelbulb Power 2 fractal
 * @reference http://www.fractalforums.com/3d-fractal-generation/true-3d-mandlebrot-type-fractal/
 */
void MandelbulbPower2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	double x2 = z.x * z.x;
	double y2 = z.y * z.y;
	double z2 = z.z * z.z;
	double temp = 1.0 - z2 / (x2 + y2);
	double newx = (x2 - y2) * temp;
	double newy = 2.0 * z.x * z.y * temp;
	double newz = -2.0 * z.z * sqrt(x2 + y2);
	z.x = newx;
	z.y = newy;
	z.z = newz;
}

/**
 * Xenodreambuie
 * @reference
 * http://www.fractalforums.com/3d-fractal-generation/true-3d-mandlebrot-type-fractal/273/
 */
void XenodreambuieIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double rp = pow(aux.r, fractal->bulb.power - 1.0);
	aux.r_dz = rp * aux.r_dz * fractal->bulb.power + 1.0;
	rp *= aux.r;

	double th = atan2(z.y, z.x) + fractal->bulb.betaAngleOffset;
	double ph = acos(z.z / aux.r) + fractal->bulb.alphaAngleOffset;

	if (fabs(ph) > 0.5 * M_PI) ph = sign(ph) * M_PI - ph;

	z.x = rp * cos(th * fractal->bulb.power) * sin(ph * fractal->bulb.power);
	z.y = rp * sin(th * fractal->bulb.power) * sin(ph * fractal->bulb.power);
	z.z = rp * cos(ph * fractal->bulb.power);
}

/**
 * Menger Sponge formula created by Knighty
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/kaleidoscopic-(escape-time-ifs)/
 */
void MengerSpongeIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	z.x = fabs(z.x);
	z.y = fabs(z.y);
	z.z = fabs(z.z);

	if (z.x - z.y < 0.0) swap(z.x, z.y);
	if (z.x - z.z < 0.0) swap(z.x, z.z);
	if (z.y - z.z < 0.0) swap(z.y, z.z);

	z *= 3.0;

	z.x -= 2.0;
	z.y -= 2.0;
	if (z.z > 1.0) z.z -= 2.0;

	aux.DE *= 3.0;
}

/**
 * Smooth Mandelbox created by Buddhi
 * @reference http://www.fractalforums.com/3d-fractal-generation/mandelbox-with-'smooth'-conditions/
 */
void MandelboxSmoothIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double sm = fractal->mandelbox.sharpness;

	double zk1 = SmoothConditionAGreaterB(z.x, fractal->mandelbox.foldingLimit, sm);
	double zk2 = SmoothConditionALessB(z.x, -fractal->mandelbox.foldingLimit, sm);
	z.x = z.x * (1.0 - zk1) + (fractal->mandelbox.foldingValue - z.x) * zk1;
	z.x = z.x * (1.0 - zk2) + (-fractal->mandelbox.foldingValue - z.x) * zk2;
	aux.color += (zk1 + zk2) * fractal->mandelbox.color.factor.x;

	double zk3 = SmoothConditionAGreaterB(z.y, fractal->mandelbox.foldingLimit, sm);
	double zk4 = SmoothConditionALessB(z.y, -fractal->mandelbox.foldingLimit, sm);
	z.y = z.y * (1.0 - zk3) + (fractal->mandelbox.foldingValue - z.y) * zk3;
	z.y = z.y * (1.0 - zk4) + (-fractal->mandelbox.foldingValue - z.y) * zk4;
	aux.color += (zk3 + zk4) * fractal->mandelbox.color.factor.y;

	double zk5 = SmoothConditionAGreaterB(z.z, fractal->mandelbox.foldingLimit, sm);
	double zk6 = SmoothConditionALessB(z.z, -fractal->mandelbox.foldingLimit, sm);
	z.z = z.z * (1.0 - zk5) + (fractal->mandelbox.foldingValue - z.z) * zk5;
	z.z = z.z * (1.0 - zk6) + (-fractal->mandelbox.foldingValue - z.z) * zk6;
	aux.color += (zk5 + zk6) * fractal->mandelbox.color.factor.z;

	double r2 = z.Dot(z);
	double tglad_factor2 = fractal->mandelbox.fR2 / r2;
	double rk1 = SmoothConditionALessB(r2, fractal->mandelbox.mR2, sm);
	double rk2 = SmoothConditionALessB(r2, fractal->mandelbox.fR2, sm);
	double rk21 = (1.0 - rk1) * rk2;

	z = z * (1.0 - rk1) + z * (fractal->mandelbox.mboxFactor1 * rk1);
	z = z * (1.0 - rk21) + z * (tglad_factor2 * rk21);
	aux.DE = aux.DE * (1.0 - rk1) + aux.DE * (fractal->mandelbox.mboxFactor1 * rk1);
	aux.DE = aux.DE * (1.0 - rk21) + aux.DE * (tglad_factor2 * rk21);
	aux.color += rk1 * fractal->mandelbox.color.factorSp1;
	aux.color += rk21 * fractal->mandelbox.color.factorSp2;

	z = fractal->mandelbox.mainRot.RotateVector(z);
	z = z * fractal->mandelbox.scale;

	aux.DE = aux.DE * fabs(fractal->mandelbox.scale) + 1.0;
}

/**
 * Hybrid of Mandelbox and Mandelbulb power 2 with scaling of z axis
 */
void BoxFoldBulbPow2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	(void)aux;
	if (fabs(z.x) > fractal->foldingIntPow.foldFactor)
		z.x = sign(z.x) * fractal->foldingIntPow.foldFactor * 2.0 - z.x;
	if (fabs(z.y) > fractal->foldingIntPow.foldFactor)
		z.y = sign(z.y) * fractal->foldingIntPow.foldFactor * 2.0 - z.y;
	if (fabs(z.z) > fractal->foldingIntPow.foldFactor)
		z.z = sign(z.z) * fractal->foldingIntPow.foldFactor * 2.0 - z.z;

	double fR2_2 = 1.0;
	double mR2_2 = 0.25;
	double r2_2 = z.Dot(z);
	double tglad_factor1_2 = fR2_2 / mR2_2;

	if (r2_2 < mR2_2)
	{
		z = z * tglad_factor1_2;
	}
	else if (r2_2 < fR2_2)
	{
		double tglad_factor2_2 = fR2_2 / r2_2;
		z = z * tglad_factor2_2;
	}

	z = z * 2.0;
	double x2 = z.x * z.x;
	double y2 = z.y * z.y;
	double z2 = z.z * z.z;
	double temp = 1.0 - z2 / (x2 + y2);
	CVector4 zTemp;
	zTemp.x = (x2 - y2) * temp;
	zTemp.y = 2.0 * z.x * z.y * temp;
	zTemp.z = -2.0 * z.z * sqrt(x2 + y2);
	zTemp.w = z.w;
	z = zTemp;
	z.z *= fractal->foldingIntPow.zFactor;

	// INFO remark: changed sequence of operation.
	// adding of C constant was before multiplying by z-factor
}

/**
 * Kaleidoscopic Iterated Function Systems (IFS)
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/kaleidoscopic-(escape-time-ifs)/
 */
void KaleidoscopicIfsIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->IFS.absX) z.x = fabs(z.x);
	if (fractal->IFS.absY) z.y = fabs(z.y);
	if (fractal->IFS.absZ) z.z = fabs(z.z);

	for (int i = 0; i < IFS_VECTOR_COUNT; i++)
	{
		if (fractal->IFS.enabled[i])
		{
			z = fractal->IFS.rot[i].RotateVector(z);
			double length = z.Dot(fractal->IFS.direction[i]);

			if (length < fractal->IFS.distance[i])
			{
				z -= fractal->IFS.direction[i]
						 * (2.0 * (length - fractal->IFS.distance[i]) * fractal->IFS.intensity[i]);
			}
		}
	}

	if (fractal->IFS.rotationEnabled)
		z = fractal->IFS.mainRot.RotateVector(z - fractal->IFS.offset) + fractal->IFS.offset;

	if (fractal->IFS.edge.x > 0.0) z.x = fractal->IFS.edge.x - fabs(fractal->IFS.edge.x - z.x);
	if (fractal->IFS.edge.y > 0.0) z.y = fractal->IFS.edge.y - fabs(fractal->IFS.edge.y - z.y);
	if (fractal->IFS.edge.z > 0.0) z.z = fractal->IFS.edge.z - fabs(fractal->IFS.edge.z - z.z);

	z *= fractal->IFS.scale;
	if (fractal->IFS.mengerSpongeMode)
	{
		z.x -= fractal->IFS.offset.x * (fractal->IFS.scale - 1.0);
		z.y -= fractal->IFS.offset.y * (fractal->IFS.scale - 1.0);
		if (z.z > 0.5 * fractal->IFS.offset.z * (fractal->IFS.scale - 1.0))
			z.z -= fractal->IFS.offset.z * (fractal->IFS.scale - 1.0);
	}
	else
	{
		z -= fractal->IFS.offset * (fractal->IFS.scale - 1.0);
	}
	aux.DE *= fractal->IFS.scale;
}

/**
 * Aexion's Quadray Sets from FractalForums
 * @reference http://www.fractalforums.com/the-3d-mandelbulb/quadray-sets/msg31458/#msg31458
 */
void AexionIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (aux.i == 0)
	{
		double cx = fabs(aux.c.x + aux.c.y + aux.c.z) + fractal->aexion.cadd;
		double cy = fabs(-aux.c.x - aux.c.y + aux.c.z) + fractal->aexion.cadd;
		double cz = fabs(-aux.c.x + aux.c.y - aux.c.z) + fractal->aexion.cadd;
		double cw = fabs(aux.c.x - aux.c.y - aux.c.z) + fractal->aexion.cadd;
		aux.c.x = cx;
		aux.c.y = cy;
		aux.c.z = cz;
		aux.cw = cw;
		double tempX = fabs(z.x + z.y + z.z) + fractal->aexion.cadd;
		double tempY = fabs(-z.x - z.y + z.z) + fractal->aexion.cadd;
		double tempZ = fabs(-z.x + z.y - z.z) + fractal->aexion.cadd;
		double tempW = fabs(z.x - z.y - z.z) + fractal->aexion.cadd;
		z.x = tempX;
		z.y = tempY;
		z.z = tempZ;
		z.w = tempW;
	}
	double tempX = z.x * z.x - z.y * z.y + 2.0 * z.w * z.z + aux.c.x;
	double tempY = z.y * z.y - z.x * z.x + 2.0 * z.w * z.z + aux.c.y;
	double tempZ = z.z * z.z - z.w * z.w + 2.0 * z.x * z.y + aux.c.z;
	double tempW = z.w * z.w - z.z * z.z + 2.0 * z.x * z.y + aux.cw;
	z.x = tempX;
	z.y = tempY;
	z.z = tempZ;
	z.w = tempW;
}

/**
 * 3D Mandelbrot formula invented by David Makin
 * @reference
 * http://www.fractalforums.com/3d-fractal-generation/true-3d-mandlebrot-type-fractal/msg7235/#msg7235
 */
void HypercomplexIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	double newx = z.x * z.x - z.y * z.y - z.z * z.z - z.w * z.w;
	double newy = 2.0 * z.x * z.y - 2.0 * z.w * z.z;
	double newz = 2.0 * z.x * z.z - 2.0 * z.y * z.w;
	double neww = 2.0 * z.x * z.w - 2.0 * z.y * z.z;
	z.x = newx;
	z.y = newy;
	z.z = newz;
	z.w = neww;
}

/**
 * Quaternion fractal
 * @reference http://www.fractalforums.com/3d-fractal-generation/true-3d-mandlebrot-type-fractal/
 */
void QuaternionIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	double newx = z.x * z.x - z.y * z.y - z.z * z.z - z.w * z.w;
	double newy = 2.0 * z.x * z.y;
	double newz = 2.0 * z.x * z.z;
	double neww = 2.0 * z.x * z.w;
	z.x = newx;
	z.y = newy;
	z.z = newz;
	z.w = neww;
}

/**
 * Formula invented by Benesi
 * @reference http://www.fractalforums.com/index.php?action=profile;u=1170
 */
void BenesiIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	CVector4 c = aux.const_c;
	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	double r1 = z.y * z.y + z.z * z.z;
	double newx;
	if (c.x < 0.0 || z.x < sqrt(r1))
	{
		newx = z.x * z.x - r1;
	}
	else
	{
		newx = -z.x * z.x + r1;
	}
	r1 = -1.0 / sqrt(r1) * 2.0 * fabs(z.x);
	double newy = r1 * (z.y * z.y - z.z * z.z);
	double newz = r1 * 2.0 * z.y * z.z;

	z.x = newx;
	z.y = newy;
	z.z = newz;
}

/**
 * Bristorbrot formula
 * @reference http://www.fractalforums.com/theory/bristorbrot-3d/
 * by Doug Bristor
 */
void BristorbrotIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	double newx = z.x * z.x - z.y * z.y - z.z * z.z;
	double newy = z.y * (2.0 * z.x - z.z);
	double newz = z.z * (2.0 * z.x + z.y);
	z.x = newx;
	z.y = newy;
	z.z = newz;
}

/**
 * From M3D. A formula made by Trafassel, the original Ide's Formula thread
 * @reference http://www.fractalforums.com/videos/formula-21-%28julia-set-interpretation%29/
 */
void IdesIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	if (fabs(z.x) < 2.5) z.x = z.x * .9;
	if (fabs(z.z) < 2.5) z.z = z.z * .9;

	CVector4 z2 = z * z;
	CVector4 newZ;
	newZ.x = fractal->transformCommon.constantMultiplier121.x * z2.x
					 - fractal->transformCommon.additionConstant0555.x * (z2.y + z2.z);
	newZ.y = fractal->transformCommon.constantMultiplier121.y * z.x * z.y * z.z;
	newZ.z = fractal->transformCommon.constantMultiplier121.z * z2.z
					 - fractal->transformCommon.additionConstant0555.z * (z2.x + z2.y);
	newZ.w = z.w;
	z = newZ;
}

/**
 * From M3D. A formula made by Trafassel, the original Ide's Formula thread
 * @reference http://www.fractalforums.com/videos/formula-21-%28julia-set-interpretation%29/
 */
void Ides2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	CVector4 z2 = z * z;
	CVector4 newZ;
	newZ.x = fractal->transformCommon.constantMultiplier121.x * z2.x
					 - fractal->transformCommon.additionConstant0555.x * (z2.y + z2.z);
	newZ.y = fractal->transformCommon.constantMultiplier121.y * z.x * z.y * z.z;
	newZ.z = fractal->transformCommon.constantMultiplier121.z * z2.z
					 - fractal->transformCommon.additionConstant0555.z * (z2.x + z2.y);
	newZ.w = z.w;
	z = newZ + z;
}

/**
 * Buffalo fractal
 */
void BuffaloIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	aux.r_dz = aux.r_dz * 2.0 * aux.r;

	if (fractal->buffalo.preabsx) z.x = fabs(z.x);
	if (fractal->buffalo.preabsy) z.y = fabs(z.y);
	if (fractal->buffalo.preabsz) z.z = fabs(z.z);

	double x2 = z.x * z.x;
	double y2 = z.y * z.y;
	double z2 = z.z * z.z;
	double temp = 1.0 - z2 / (x2 + y2);
	double newx = (x2 - y2) * temp;
	double newy = 2.0 * z.x * z.y * temp;
	double newz = (fractal->buffalo.posz ? 2.0 : -2.0) * z.z * sqrt(x2 + y2);

	z.x = fractal->buffalo.absx ? fabs(newx) : newx;
	z.y = fractal->buffalo.absy ? fabs(newy) : newy;
	z.z = fractal->buffalo.absz ? fabs(newz) : newz;
}

/**
 * From M3D
 * @reference http://www.fractalforums.com/3d-fractal-generation/another-shot-at-the-holy-grail/
 */
void QuickDudleyIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	double x2 = z.x * z.x;
	double y2 = z.y * z.y;
	double z2 = z.z * z.z;
	double newx = x2 - 2.0 * z.y * z.z;
	double newy = z2 + 2.0 * z.x * z.y;
	double newz = y2 + 2.0 * z.x * z.z;
	z.x = newx;
	z.y = newy;
	z.z = newz;
}

/**
 * From M3D
 * @reference http://www.fractalforums.com/3d-fractal-generation/another-shot-at-the-holy-grail/
 */
void QuickDudleyModIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	double x2 = z.x * z.x;
	double y2 = z.y * z.y;
	double z2 = z.z * z.z;
	double newx = x2 * fractal->transformCommon.constantMultiplier111.x
								- fractal->transformCommon.constantMultiplier222.x * z.y * z.z;
	double newy = z2 * fractal->transformCommon.constantMultiplier111.y
								+ fractal->transformCommon.constantMultiplier222.y * z.x * z.y;
	double newz = y2 * fractal->transformCommon.constantMultiplier111.z
								+ fractal->transformCommon.constantMultiplier222.z * z.z * z.x;
	z.x = newx;
	z.y = newy;
	z.z = newz;
}

/**
 * LkmitchIteration
 * @reference http://www.fractalforums.com/3d-fractal-generation/another-shot-at-the-holy-grail/
 */
void LkmitchIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	double x2 = z.x * z.x;
	double y2 = z.y * z.y;
	double z2 = z.z * z.z;
	double newx = x2 - 2.0 * z.y * z.z;
	double newy = z2 + 2.0 * z.x * z.y;
	double newz = y2 - 2.0 * z.x * z.z;
	z.x = newx;
	z.y = newy;
	z.z = newz;
}

/**
 * Makin3D-2 found through the another shot at the holy grail topic at ff
 * @reference http://www.fractalforums.com/3d-fractal-generation/another-shot-at-the-holy-grail/
 */
void Makin3d2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	double x2 = z.x * z.x;
	double y2 = z.y * z.y;
	double z2 = z.z * z.z;
	double newx = x2 + 2.0 * z.y * z.z;
	double newy = -y2 - 2.0 * z.x * z.z;
	double newz = -z2 + 2.0 * z.x * z.y;
	z.x = newx;
	z.y = newy;
	z.z = newz;
}

// NEW FORMULAS-----------------------------------------------------------------

/**
 * aBoxMod1, a variation of Mandelbox fractal known as AmazingBox or ABox,
 * invented by Tom Lowe in 2010, the variation by DarkBeam
 *
 * This formula has a different box fold to the standard Tglad fold
 * This formula has a c.x c.y SWAP (in compute_fractals.cpp)
 * In V2.11 minimum radius is MinimumR2, for settings made in
 * older versions, you need to use the square root of the old parameter.
 *
 * based on DarkBeam's Mandelbulb3D code
 *
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/amazing-fractal/msg12467/#msg12467
 */
void AboxMod1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	aux.actualScale =
		fractal->mandelbox.scale + fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);

	z.x = fractal->mandelbox.foldingValue
				- fabs(fabs(z.x + fractal->transformCommon.additionConstant000.x)
							 - fractal->mandelbox.foldingValue)
				- fabs(fractal->transformCommon.additionConstant000.x);
	z.y = fractal->mandelbox.foldingValue
				- fabs(fabs(z.y + fractal->transformCommon.additionConstant000.y)
							 - fractal->mandelbox.foldingValue)
				- fabs(fractal->transformCommon.additionConstant000.y);

	if (fractal->transformCommon.functionEnabledz)
	{
		z.z = fractal->mandelbox.foldingValue * fractal->transformCommon.scale1
					- fabs(fabs(z.z + fractal->transformCommon.additionConstant000.z)
								 - fractal->mandelbox.foldingValue * fractal->transformCommon.scale1)
					- fabs(fractal->transformCommon.additionConstant000.z);
	}

	double rr = (z.x * z.x + z.y * z.y + z.z * z.z);
	if (fractal->transformCommon.functionEnabledFalse)
	{
		rr = pow(rr, fractal->mandelboxVary4D.rPower);
	}

	if (rr < fractal->transformCommon.minR0)
	{
		double tglad_factor1 = fractal->transformCommon.maxR2d1 / fractal->transformCommon.minR0;
		z *= tglad_factor1;
		aux.DE *= tglad_factor1;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (rr < fractal->transformCommon.maxR2d1)
	{
		double tglad_factor2 = fractal->transformCommon.maxR2d1 / rr;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}

	z *= aux.actualScale;
	aux.DE = aux.DE * fabs(aux.actualScale) + 1.0;

	if (fractal->transformCommon.addCpixelEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsE
			&& aux.i < fractal->transformCommon.stopIterationsE)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplier111;
	}

	if (fractal->transformCommon.rotationEnabled && aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}
}

/**
 * aBoxMod2, a variation of the Mandelbox fractal known as AmazingBox or ABox,
 * invented by Tom Lowe in 2010, this variation by DarkBeam
 *
 * The formula Cylinder Half Size transform changes the spherical fold
 * In V2.11 minimum radius is MinimumR2, for settings made in
 * older versions, you need to use the square root of the old parameter.
 *
 * based on DarkBeam's Mandelbulb3D code
 *
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/amazing-fractal/msg12467/#msg12467
 */
void AboxMod2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	// Tglad Fold
	z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
				- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
	z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
				- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
	z.z = fabs(z.z + fractal->transformCommon.additionConstant111.z)
				- fabs(z.z - fractal->transformCommon.additionConstant111.z) - z.z; // default was 1.5

	/*	double rr;
		if (temp > 0.0)
			rr = z.x * z.x + z.y * z.y + z.z * z.z; // on top & bottom of cyl. z.z should be tempZ
		else
			rr = z.x * z.x + z.y * z.y; // on cyl body*/
	// cylinder half size
	double tempZ = fabs(z.z) - fractal->transformCommon.offset05;
	double rr = z.x * z.x + z.y * z.y;
	if (tempZ > 0.0) rr = rr + (tempZ * tempZ * fractal->transformCommon.scale1);
	// rPower
	if (fractal->transformCommon.functionEnabledFalse)
	{
		rr = pow(rr, fractal->mandelboxVary4D.rPower);
	}
	// Spherical Fold
	if (rr < fractal->transformCommon.minR2p25)
	{
		z *= fractal->transformCommon.maxMinR2factor;
		aux.DE *= fractal->transformCommon.maxMinR2factor;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (rr < fractal->transformCommon.maxR2d1)
	{
		double tglad_factor2 = fractal->transformCommon.maxR2d1 / rr;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	// Scale
	aux.actualScale =
		fractal->mandelbox.scale + fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);
	z *= aux.actualScale;
	aux.DE = aux.DE * fabs(aux.actualScale) + 1.0;
	// addCpixel
	if (fractal->transformCommon.addCpixelEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsE
			&& aux.i < fractal->transformCommon.stopIterationsE)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplier111;
	}
	// rotation
	if (fractal->transformCommon.rotationEnabled && aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}
}

/**
 * ABoxMod11,
 * The Mandelbox fractal known as AmazingBox or ABox, invented by Tom Lowe in 2010
 * Variations from DarkBeam, Buddhi, Eiffie and mclarekin
 * based on DarkBeam's Mandelbulb3D code
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/amazing-fractal/msg12467/#msg12467
 */
void AboxMod11Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	// tglad fold
	if (aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB)
	{
		z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
					- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
		z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
					- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
		z.z = fabs(z.z + fractal->transformCommon.additionConstant111.z)
					- fabs(z.z - fractal->transformCommon.additionConstant111.z) - z.z;

		aux.color += fractal->mandelbox.color.factor.x;
		aux.color += fractal->mandelbox.color.factor.y;
		aux.color += fractal->mandelbox.color.factor.z;
	}
	if (fractal->transformCommon.functionEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsD1)
	{
		CVector4 limit = fractal->transformCommon.additionConstant111;
		CVector4 length = 2.0 * limit;
		CVector4 tgladS = 1.0 / length;
		CVector4 Add;
		if (fabs(z.x) < limit.x) Add.x = z.x * z.x * tgladS.x;
		if (fabs(z.y) < limit.y) Add.y = z.y * z.y * tgladS.y;
		if (fabs(z.z) < limit.z) Add.z = z.z * z.z * tgladS.z;
		if (fabs(z.x) > limit.x && fabs(z.x) < length.x)
			Add.x = (length.x - fabs(z.x)) * (length.x - fabs(z.x)) * tgladS.x;
		if (fabs(z.y) > limit.y && fabs(z.y) < length.y)
			Add.y = (length.y - fabs(z.y)) * (length.y - fabs(z.y)) * tgladS.y;
		if (fabs(z.z) > limit.z && fabs(z.z) < length.z)
			Add.z = (length.z - fabs(z.z)) * (length.z - fabs(z.z)) * tgladS.z;
		Add *= fractal->transformCommon.scale3D000;
		z.x = (z.x - (sign(z.x) * (Add.x)));
		z.y = (z.y - (sign(z.y) * (Add.y)));
		z.z = (z.z - (sign(z.z) * (Add.z)));
	}
	// swap
	if (fractal->transformCommon.functionEnabledSwFalse)
	{
		z = CVector4(z.z, z.y, z.x, z.w);
	}

	// spherical fold
	if (aux.i >= fractal->transformCommon.startIterationsS
			&& aux.i < fractal->transformCommon.stopIterationsS)
	{
		double para = 0.0;
		double paraAddP0 = 0.0;
		if (fractal->transformCommon.functionEnabledyFalse)
		{
			// para += paraAddP0 + fractal->transformCommon.minR2p25;
			if (fractal->Cpara.enabledLinear)
			{
				para = fractal->Cpara.para00; // parameter value at iter 0
				double temp0 = para;
				double tempA = fractal->Cpara.paraA;
				double tempB = fractal->Cpara.paraB;
				double tempC = fractal->Cpara.paraC;
				double lengthAB = fractal->Cpara.iterB - fractal->Cpara.iterA;
				double lengthBC = fractal->Cpara.iterC - fractal->Cpara.iterB;
				double grade1 = (tempA - temp0) / fractal->Cpara.iterA;
				double grade2 = (tempB - tempA) / lengthAB;
				double grade3 = (tempC - tempB) / lengthBC;

				// slopes
				if (aux.i < fractal->Cpara.iterA)
				{
					para = temp0 + (aux.i * grade1);
				}
				if (aux.i < fractal->Cpara.iterB && aux.i >= fractal->Cpara.iterA)
				{
					para = tempA + (aux.i - fractal->Cpara.iterA) * grade2;
				}
				if (aux.i >= fractal->Cpara.iterB)
				{
					para = tempB + (aux.i - fractal->Cpara.iterB) * grade3;
				}

				// Curvi part on "true"
				if (fractal->Cpara.enabledCurves)
				{
					double paraAdd = 0.0;
					double paraIt;
					if (lengthAB > 2.0 * fractal->Cpara.iterA) // stop  error, todo fix.
					{
						double curve1 = (grade2 - grade1) / (4.0 * fractal->Cpara.iterA);
						double tempL = lengthAB - fractal->Cpara.iterA;
						double curve2 = (grade3 - grade2) / (4.0 * tempL);
						if (aux.i < 2 * fractal->Cpara.iterA)
						{
							paraIt = tempA - fabs(tempA - aux.i);
							paraAdd = paraIt * paraIt * curve1;
						}
						if (aux.i >= 2 * fractal->Cpara.iterA && aux.i < fractal->Cpara.iterB + tempL)
						{
							paraIt = tempB - fabs(tempB * aux.i);
							paraAdd = paraIt * paraIt * curve2;
						}
						para += paraAdd;
					}
				}
			}
			paraAddP0 = 0.0;
			if (fractal->Cpara.enabledParabFalse)
			{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
				paraAddP0 = fractal->Cpara.parabOffset0 + (aux.i * fractal->Cpara.parabSlope)
										+ (aux.i * aux.i * 0.001 * fractal->Cpara.parabScale);
			}
		}
		para += paraAddP0 + fractal->transformCommon.minR2p25;
		// spherical fold
		double rr = z.Dot(z);

		z += fractal->mandelbox.offset;

		// if (r2 < 1e-21) r2 = 1e-21;
		if (rr < para)
		{
			double tglad_factor1 = fractal->transformCommon.maxR2d1 / para;
			z *= tglad_factor1;
			aux.DE *= tglad_factor1;
			aux.color += fractal->mandelbox.color.factorSp1;
		}
		else if (rr < fractal->transformCommon.maxR2d1)
		{
			double tglad_factor2 = fractal->transformCommon.maxR2d1 / rr;
			z *= tglad_factor2;
			aux.DE *= tglad_factor2;
			aux.color += fractal->mandelbox.color.factorSp2;
		}
		z -= fractal->mandelbox.offset;
	}
	// scale, incl DarkBeams Scale vary
	if (aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		aux.actualScale =
			fractal->mandelbox.scale + fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);
		z *= aux.actualScale;
		aux.DE = aux.DE * fabs(aux.actualScale) + 1.0;
	}
	// offset
	z += fractal->transformCommon.additionConstant000;
	// addCpixel
	if (fractal->transformCommon.addCpixelEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsE
			&& aux.i < fractal->transformCommon.stopIterationsE)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplier111;
	}
	// rotation
	if (fractal->transformCommon.rotationEnabled && aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}
	// color
	aux.DE = aux.DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
	aux.foldFactor = fractal->foldColor.compFold; // fold group weight
	aux.minRFactor = fractal->foldColor.compMinR; // orbit trap weight

	double scaleColor = fractal->foldColor.colorMin + fabs(aux.actualScale);
	// scaleColor += fabs(fractal->mandelbox.scale);
	aux.scaleFactor = scaleColor * fractal->foldColor.compScale;
}

/**
 * ABoxModKali, a formula from Mandelbulb3D
 * @reference http://www.fractalforums.com/new-theories-and-research/aboxmodkali-the-2d-version/
 */
void AboxModKaliIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	z = fractal->transformCommon.additionConstant0555 - fabs(z);
	double rr = z.Dot(z);
	double MinR = fractal->transformCommon.minR06;
	double dividend = rr < MinR ? MinR : min(rr, 1.0);
	double m = fractal->transformCommon.scale015 / dividend;
	z = z * m;
	aux.DE = aux.DE * fabs(m) + 1.0;
}

/**
 * ABoxModKaliEiffie, modified formula from Mandelbulb3D
 * @reference http://www.fractalforums.com/new-theories-and-research/aboxmodkali-the-2d-version/
 */
void AboxModKaliEiffieIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
				- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
	z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
				- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
	z.z = fabs(z.z + fractal->transformCommon.additionConstant111.z)
				- fabs(z.z - fractal->transformCommon.additionConstant111.z) - z.z;

	aux.color += fractal->mandelbox.color.factor.x;
	aux.color += fractal->mandelbox.color.factor.y;
	aux.color += fractal->mandelbox.color.factor.z;

	if (fractal->transformCommon.functionEnabledxFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA) // box fold z.z
	{
		double zLimit =
			fractal->transformCommon.additionConstant111.z * fractal->transformCommon.scale1;
		double zValue = fractal->mandelbox.foldingValue * fractal->transformCommon.scale1;
		if (fabs(z.z) > zLimit)
		{
			z.z = sign(z.z) * zValue - z.z;
			aux.color += fractal->mandelbox.color.factor.z;
		}
	}

	if (fractal->transformCommon.functionEnabled)
	{
		z = CVector4(z.z, z.y, z.x, z.w); // swap
	}
	if (z.y > z.x) z = CVector4(z.y, z.x, z.z, z.w); // conditional

	double rr = z.Dot(z);
	double MinR = fractal->transformCommon.minR05;
	double m = fractal->transformCommon.scale015;
	if (rr < MinR)
	{
		m = m / MinR;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else
	{
		if (rr < 1.0)
		{
			m = m / rr;
			aux.color += fractal->mandelbox.color.factorSp2;
		}
	}
	z = z * m;

	z += fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplier111;
	}
	if (fractal->transformCommon.rotationEnabled && aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}
	aux.DE = aux.DE * fabs(m) + 1.0;

	aux.foldFactor = fractal->foldColor.compFold0; // fold group weight
	aux.minRFactor = fractal->foldColor.compMinR;	// orbit trap weight

	double scaleColor = fractal->foldColor.colorMin + fabs(m);
	// scaleColor += fabs(fractal->mandelbox.scale);
	aux.scaleFactor = scaleColor * fractal->foldColor.compScale;
}

/**
 * ABoxVS_icen1, a formula from Mandelbulb3D.
 * Inspired from a 2D formula proposed by Kali at Fractal Forums
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/kaliset-plus-boxfold-nice-new-2d-fractal/msg33670/#new
 */
void AboxVSIcen1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	aux.actualScale += fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);

	z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
				- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
	z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
				- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
	z.z = fabs(z.z + fractal->transformCommon.additionConstant111.z)
				- fabs(z.z - fractal->transformCommon.additionConstant111.z) - z.z;

	if (fractal->transformCommon.juliaMode)
	{
		z += c * fractal->transformCommon.constantMultiplier111;
	}
	else
	{
		z += fractal->transformCommon.juliaC;
	}

	double r2 = z.Dot(z);
	// if (r2 < 1e-21 && r2 > -1e-21)
	//	r2 = (r2 > 0) ? 1e-21 : -1e-21;
	if (r2 < fractal->mandelbox.mR2)
	{
		z *= fractal->mandelbox.mboxFactor1;
		aux.DE *= fractal->mandelbox.mboxFactor1;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < fractal->mandelbox.fR2)
	{
		double tglad_factor2 = fractal->mandelbox.fR2 / r2;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z *= aux.actualScale;

	aux.DE = aux.DE * fabs(aux.actualScale) + 1.0;

	if (fractal->transformCommon.juliaMode)
	{
		z += fractal->transformCommon.juliaC
				 - c * fractal->transformCommon.constantMultiplier111;
	}
	else
	{
		z += c * fractal->transformCommon.constantMultiplier111 + fractal->transformCommon.juliaC;
	}
}

/**
 * aexion octopus. Based on Aex_Octo from Mandelbulber3D
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void AexionOctopusModIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	CVector4 tempN;
	tempN.x = z.x * z.z * fractal->transformCommon.scale3D111.x;
	tempN.y = (z.x * z.x - z.z * z.z) * fractal->transformCommon.scale3D111.y;
	tempN.z = z.y;

	if (fractal->transformCommon.functionEnabledAx) tempN.x = fabs(tempN.x);
	if (fractal->transformCommon.functionEnabledAy) tempN.y = fabs(tempN.y);
	if (fractal->transformCommon.functionEnabledAz) tempN.z = fabs(tempN.z);

	z = tempN;

	if (fractal->transformCommon.rotationEnabled)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	z += fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.constantMultiplier000;

		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}
}

/**
 * amazing surf from Mandelbulber3D. Formula proposed by Kali, with features added by Darkbeam
 *
 * This formula has a c.x c.y SWAP
 *
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void AmazingSurfIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	aux.actualScale =
		fractal->mandelbox.scale + fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);

	z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
				- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
	z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
				- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
	// no z fold

	double rr = z.Dot(z);
	if (fractal->transformCommon.functionEnabledFalse) // force cylinder fold
		rr -= z.z * z.z;

	double sqrtMinR = sqrt(fractal->transformCommon.minR05);
	double dividend = rr < sqrtMinR ? sqrtMinR : min(rr, 1.0);
	double m = aux.actualScale / dividend;
	z *= m * fractal->transformCommon.scale1 + 1.0 * (1.0 - fractal->transformCommon.scale1);
	aux.DE = aux.DE * fabs(m) + 1.0;

	if (fractal->transformCommon.addCpixelEnabledFalse)
		z += CVector4(c.y, c.x, c.z, c.w) * fractal->transformCommon.constantMultiplier111;

	z = fractal->transformCommon.rotationMatrix.RotateVector(z);
}

/**
 * Based on Amazing Surf Mod 1 from Mandelbulber3D, a formula proposed by Kali,
 * with features added by Darkbeam
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void AmazingSurfMod1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	aux.actualScale =
		fractal->mandelbox.scale + fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);

	if (fractal->transformCommon.functionEnabledAx)
	{
		z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
					- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
		z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
					- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
		aux.color += fractal->mandelbox.color.factor.x;
		aux.color += fractal->mandelbox.color.factor.y;
	}

	// z = fold - fabs( fabs(z) - fold)
	if (fractal->transformCommon.functionEnabledAyFalse)
	{
		z.x = fractal->transformCommon.additionConstant111.x
					- fabs(fabs(z.x) - fractal->transformCommon.additionConstant111.x);
		z.y = fractal->transformCommon.additionConstant111.y
					- fabs(fabs(z.y) - fractal->transformCommon.additionConstant111.y);
		aux.color += fractal->mandelbox.color.factor.x;
		aux.color += fractal->mandelbox.color.factor.y;
	}

	if (fractal->transformCommon.functionEnabledAzFalse)
	{
		z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x);
		z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y);
		aux.color += fractal->mandelbox.color.factor.x;
		aux.color += fractal->mandelbox.color.factor.y;
	}

	// if z > limit) z =  Value -z,   else if z < limit) z = - Value - z,
	if (fractal->transformCommon.functionEnabledxFalse)
	{
		if (fabs(z.x) > fractal->transformCommon.additionConstant111.x)
		{
			z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
			// aux.color += fractal->mandelbox.color.factor.x;
		}
		if (fabs(z.y) > fractal->transformCommon.additionConstant111.y)
		{
			z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
			// aux.color += fractal->mandelbox.color.factor.y;
		}
		aux.color += fractal->mandelbox.color.factor.x;
		aux.color += fractal->mandelbox.color.factor.y;
	}

	// z = fold2 - fabs( fabs(z + fold) - fold2) - fabs(fold)
	if (fractal->transformCommon.functionEnabledyFalse)
	{
		z.x = fractal->transformCommon.offset2
					- fabs(fabs(z.x + fractal->transformCommon.additionConstant111.x)
								 - fractal->transformCommon.offset2)
					- fractal->transformCommon.additionConstant111.x;
		z.y = fractal->transformCommon.offset2
					- fabs(fabs(z.y + fractal->transformCommon.additionConstant111.y)
								 - fractal->transformCommon.offset2)
					- fractal->transformCommon.additionConstant111.y;
		aux.color += fractal->mandelbox.color.factor.x;
		aux.color += fractal->mandelbox.color.factor.y;
	}

	z += fractal->transformCommon.additionConstant000;

	double r2;
	r2 = z.Dot(z);
	if (fractal->transformCommon.functionEnabledFalse) // force cylinder fold
		r2 -= z.z * z.z;

	// if (r2 < 1e-21)
	//	r2 = 1e-21;

	double sqrtMinR = fractal->transformCommon.sqtR;
	if (r2 < sqrtMinR)
	{
		z *= fractal->transformCommon.mboxFactor1;
		aux.DE *= fractal->transformCommon.mboxFactor1;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < 1.0)
	{
		z *= 1.0 / r2;
		aux.DE *= 1.0 / r2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}

	z *= aux.actualScale * fractal->transformCommon.scale1
			 + 1.0 * (1.0 - fractal->transformCommon.scale1);

	aux.DE = aux.DE * fabs(aux.actualScale) + 1.0;
	z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	aux.foldFactor = fractal->foldColor.compFold0; // fold group weight
	aux.minRFactor = fractal->foldColor.compMinR;	// orbit trap weight

	double scaleColor = fractal->foldColor.colorMin + fabs(fractal->mandelbox.scale);
	// scaleColor += fabs(fractal->mandelbox.scale);
	aux.scaleFactor = scaleColor * fractal->foldColor.compScale;
}

/**
 * Amazing Surface Multi
 * Based on Amazing Surf Mod 1 from Mandelbulber3D, a formula proposed by Kali,
 * with features added by Darkbeam
 */
void AmazingSurfMultiIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	bool functionEnabledN[5] = {fractal->transformCommon.functionEnabledAx,
		fractal->transformCommon.functionEnabledAyFalse,
		fractal->transformCommon.functionEnabledAzFalse,
		fractal->transformCommon.functionEnabledBxFalse,
		fractal->transformCommon.functionEnabledByFalse};
	int startIterationN[5] = {fractal->transformCommon.startIterationsA,
		fractal->transformCommon.startIterationsB, fractal->transformCommon.startIterationsC,
		fractal->transformCommon.startIterationsD, fractal->transformCommon.startIterationsE};
	int stopIterationN[5] = {fractal->transformCommon.stopIterationsA,
		fractal->transformCommon.stopIterationsB, fractal->transformCommon.stopIterationsC,
		fractal->transformCommon.stopIterationsD, fractal->transformCommon.stopIterationsE};
	enumMulti_orderOfFolds foldN[5] = {fractal->surfFolds.orderOfFolds1,
		fractal->surfFolds.orderOfFolds2, fractal->surfFolds.orderOfFolds3,
		fractal->surfFolds.orderOfFolds4, fractal->surfFolds.orderOfFolds5};

	for (int f = 0; f < 5; f++)
	{
		if (functionEnabledN[f] && aux.i >= startIterationN[f] && aux.i < stopIterationN[f])
		{
			switch (foldN[f])
			{
				case multi_orderOfFolds_type1:
				default:
					z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
								- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
					z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
								- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
					aux.color += fractal->mandelbox.color.factor.x;
					aux.color += fractal->mandelbox.color.factor.y;
					break;
				case multi_orderOfFolds_type2: // z = fold - fabs( fabs(z) - fold)
					z.x = fractal->transformCommon.additionConstant111.x
								- fabs(fabs(z.x) - fractal->transformCommon.additionConstant111.x);
					z.y = fractal->transformCommon.additionConstant111.y
								- fabs(fabs(z.y) - fractal->transformCommon.additionConstant111.y);
					aux.color += fractal->mandelbox.color.factor.x;
					aux.color += fractal->mandelbox.color.factor.y;
					break;
				case multi_orderOfFolds_type3:
					z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x);
					z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y);
					aux.color += fractal->mandelbox.color.factor.x;
					aux.color += fractal->mandelbox.color.factor.y;
					break;
				case multi_orderOfFolds_type4:
					// if z > limit) z =  Value -z,   else if z < limit) z = - Value - z,
					if (fabs(z.x) > fractal->transformCommon.additionConstant111.x)
					{
						z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
						// aux.color += fractal->mandelbox.color.factor.x;
					}
					if (fabs(z.y) > fractal->transformCommon.additionConstant111.y)
					{
						z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
						// aux.color += fractal->mandelbox.color.factor.y;
					}
					aux.color += fractal->mandelbox.color.factor.x;
					aux.color += fractal->mandelbox.color.factor.y;
					break;
				case multi_orderOfFolds_type5:
					// z = fold2 - fabs( fabs(z + fold) - fold2) - fabs(fold)
					z.x = fractal->transformCommon.offset2
								- fabs(fabs(z.x + fractal->transformCommon.additionConstant111.x)
											 - fractal->transformCommon.offset2)
								- fractal->transformCommon.additionConstant111.x;
					z.y = fractal->transformCommon.offset2
								- fabs(fabs(z.y + fractal->transformCommon.additionConstant111.y)
											 - fractal->transformCommon.offset2)
								- fractal->transformCommon.additionConstant111.y;
					break;
			}
		}
	}

	if (fractal->transformCommon.functionEnabledAxFalse)
	{ // enable z scale
		double zLimit = fractal->transformCommon.foldingLimit * fractal->transformCommon.scale0;
		double zValue = fractal->transformCommon.foldingValue * fractal->transformCommon.scale0;
		if (fabs(z.z) > zLimit)
		{
			z.z = sign(z.z) * zValue - z.z;
			aux.color += fractal->mandelbox.color.factor.z;
		}
	}
	z += fractal->transformCommon.additionConstant000;

	// standard functions
	if (fractal->transformCommon.functionEnabledAy)
	{
		double r2;
		r2 = z.Dot(z);
		if (fractal->transformCommon.functionEnabledFalse)		// force cylinder fold
			r2 -= z.z * z.z * fractal->transformCommon.scaleB1; // fold weight  ;

		if (fractal->transformCommon.functionEnabledAz
				&& aux.i >= fractal->transformCommon.startIterationsT
				&& aux.i < fractal->transformCommon.stopIterationsT)
		{
			// Abox Spherical fold
			z += fractal->mandelbox.offset;
			double sqrtMinR = fractal->transformCommon.sqtR;

			if (r2 < sqrtMinR)
			{
				z *= fractal->transformCommon.mboxFactor1;
				aux.DE *= fractal->transformCommon.mboxFactor1;
				aux.color += fractal->mandelbox.color.factorSp1;
			}
			else if (r2 < 1.0)
			{
				z *= 1.0 / r2;
				aux.DE *= 1.0 / r2;
				aux.color += fractal->mandelbox.color.factorSp2;
			}
			z -= fractal->mandelbox.offset;
		}

		// Mandelbox Spherical fold
		if (fractal->transformCommon.functionEnabledzFalse
				&& aux.i >= fractal->transformCommon.startIterationsM
				&& aux.i < fractal->transformCommon.stopIterationsM)
		{
			// r2 = z.Dot(z);
			z += fractal->mandelbox.offset;
			if (r2 < fractal->mandelbox.mR2) // mR2 = minR^2
			{
				z *= fractal->mandelbox.mboxFactor1; // fR2/mR2
				aux.DE *= fractal->mandelbox.mboxFactor1;
				aux.color += fractal->mandelbox.color.factorSp1;
			}
			else if (r2 < fractal->mandelbox.fR2)
			{
				double tglad_factor2 = fractal->mandelbox.fR2 / r2;
				z *= tglad_factor2;
				aux.DE *= tglad_factor2;
				aux.color += fractal->mandelbox.color.factorSp2;
			}
			z -= fractal->mandelbox.offset;
		}

		if (aux.i >= fractal->transformCommon.startIterationsS
				&& aux.i < fractal->transformCommon.stopIterationsS)
		{ // scale
			z *= fractal->mandelbox.scale * fractal->transformCommon.scale1
					 + 1.0 * (1.0 - fractal->transformCommon.scale1);
			aux.DE = aux.DE * fabs(fractal->mandelbox.scale) + 1.0;
		}
	}
	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplier111;
	}
	if (fractal->mandelbox.mainRotationEnabled && aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
		z = fractal->mandelbox.mainRot.RotateVector(z);

	aux.foldFactor = fractal->foldColor.compFold0; // fold group weight
	aux.minRFactor = fractal->foldColor.compMinR;	// orbit trap weight

	double scaleColor = fractal->foldColor.colorMin + fabs(fractal->mandelbox.scale);
	// scaleColor += fabs(fractal->mandelbox.scale);
	aux.scaleFactor = scaleColor * fractal->foldColor.compScale;
}

/**
 * benesiFastPwr2PineTree 3D
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void BenesiPineTreeIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	CVector4 temp = z;
	aux.r = z.Length();
	z *= z;
	double t = 2.0 * temp.x;
	if (z.y + z.z > 0.0)
		t = t / sqrt(z.y + z.z);
	else
		t = 1.0;

	z.x = (z.x - z.y - z.z) + c.x * fractal->transformCommon.constantMultiplier100.x;
	z.z = (t * (z.y - z.z)) + c.y * fractal->transformCommon.constantMultiplier100.y;
	z.y = (2.0 * t * temp.y * temp.z) + c.z * fractal->transformCommon.constantMultiplier100.z;
	aux.r_dz = aux.r * aux.r_dz * 2.0 + 1.0;
}

/**
 * benesiT1PineTree 3D
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void BenesiT1PineTreeIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	if (fractal->transformCommon.benesiT1Enabled && aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D222;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		if (fractal->transformCommon.rotationEnabled)
		{
			z = fractal->transformCommon.rotationMatrix.RotateVector(z);
		}

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
		z = z - fractal->transformCommon.offset200;
	}

	if (fractal->transformCommon.addCpixelEnabled
			&& aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		CVector4 temp = z;
		aux.r = z.Length();
		z *= z;
		double t = 2.0 * temp.x;
		if (z.y + z.z > 0.0)
			t = t / sqrt(z.y + z.z);
		else
			t = 1.0;
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = CVector4(aux.c.x, aux.c.z, aux.c.y, aux.c.w);
			aux.c = tempC;
		}
		else
		{
			tempC = CVector4(c.x, c.z, c.y, c.w);
		}
		z.x = (z.x - z.y - z.z) + tempC.x * fractal->transformCommon.constantMultiplier100.x;
		z.z = (t * (z.y - z.z)) + tempC.z * fractal->transformCommon.constantMultiplier100.y;
		z.y = (2.0 * t * temp.y * temp.z) + tempC.y * fractal->transformCommon.constantMultiplier100.z;
		aux.r_dz = aux.r * aux.r_dz * 2.0 + 1.0;
	}

	if (fractal->transformCommon.functionEnabledBxFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsD)
	{
		z = CVector4(z.x * cos(z.y * fractal->transformCommon.scale1),
					z.x * sin(z.y * fractal->transformCommon.scale1), z.z * fractal->transformCommon.scaleC1,
					z.w)
				* fractal->transformCommon.scaleA1;
		aux.r_dz *= fabs(fractal->transformCommon.scaleA1);
	}

	if (fractal->transformCommon.juliaMode)
	{
		z.x += fractal->transformCommon.offset000.x;
		z.y += fractal->transformCommon.offset000.y;
		z.z += fractal->transformCommon.offset000.z;
	}
}

/**
 * benesiMagTransforms
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void BenesiMagTransformsIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	if (fractal->transformCommon.benesiT1Enabled && aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D222;
		// if (tempL < 1e-21) tempL =  1e-21;
		double avgScale = z.Length() / tempL;

		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
		z = z - fractal->transformCommon.offset200;
	}

	if (fractal->transformCommon.rotationEnabled)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	if (fractal->transformCommon.addCpixelEnabled
			&& aux.i >= fractal->transformCommon.startIterationsT
			&& aux.i < fractal->transformCommon.stopIterationsT)
	{
		CVector4 temp = z;
		aux.r = z.Length();
		z *= z;
		double t = 2.0 * temp.x;
		if (z.y + z.z > 0.0)
			t = t / sqrt(z.y + z.z);
		else
			t = 1.0;
		if (fractal->transformCommon.addCpixelEnabled)
		{
			CVector4 tempC = c;
			if (fractal->transformCommon.alternateEnabledFalse) // alternate
			{
				tempC = CVector4(aux.c.x, aux.c.z, aux.c.y, aux.c.w);
				aux.c = tempC;
			}
			else
			{
				tempC = CVector4(c.x, c.z, c.y, c.w);
			}
			z.x = (z.x - z.y - z.z) + tempC.x * fractal->transformCommon.constantMultiplier100.x;
			z.z = (t * (z.y - z.z)) + tempC.z * fractal->transformCommon.constantMultiplier100.y;
			z.y =
				(2.0 * t * temp.y * temp.z) + tempC.y * fractal->transformCommon.constantMultiplier100.z;
		}
		aux.r_dz = aux.r * aux.r_dz * 2.0 + 1.0;
	}
	if (fractal->transformCommon.juliaMode)
	{
		z.x += fractal->transformCommon.juliaC.x * fractal->transformCommon.constantMultiplier100.x;
		z.z += fractal->transformCommon.juliaC.y * fractal->transformCommon.constantMultiplier100.y;
		z.y += fractal->transformCommon.juliaC.z * fractal->transformCommon.constantMultiplier100.z;
	}
	// additional transform slot controls
	bool functionEnabledN[5] = {fractal->transformCommon.functionEnabledAxFalse,
		fractal->transformCommon.functionEnabledAyFalse,
		fractal->transformCommon.functionEnabledAzFalse,
		fractal->transformCommon.functionEnabledBxFalse,
		fractal->transformCommon.functionEnabledByFalse};
	int startIterationN[5] = {fractal->transformCommon.startIterationsA,
		fractal->transformCommon.startIterationsB, fractal->transformCommon.startIterationsC,
		fractal->transformCommon.startIterationsD, fractal->transformCommon.startIterationsE};
	int stopIterationN[5] = {fractal->transformCommon.stopIterationsA,
		fractal->transformCommon.stopIterationsB, fractal->transformCommon.stopIterationsC,
		fractal->transformCommon.stopIterationsD, fractal->transformCommon.stopIterationsE};

	enumMulti_orderOfTransf transfN[5] = {fractal->magTransf.orderOfTransf1,
		fractal->magTransf.orderOfTransf2, fractal->magTransf.orderOfTransf3,
		fractal->magTransf.orderOfTransf4, fractal->magTransf.orderOfTransf5};

	for (int f = 0; f < 5; f++)
	{
		if (functionEnabledN[f] && aux.i >= startIterationN[f] && aux.i < stopIterationN[f])
		{
			double tempXZ;
			double tempL;
			CVector4 temp;
			double avgScale;
			CVector4 tempV2;

			switch (transfN[f])
			{
				case multi_orderOfTransf_typeT1:
				default:
					tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
					z = CVector4((tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2,
						z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);
					z = fabs(z) * fractal->transformCommon.scale3Da222;
					tempL = temp.Length();
					// if (tempL < 1e-21) tempL = 1e-21;
					avgScale = z.Length() / tempL;
					aux.r_dz *= avgScale;
					aux.DE = aux.DE * avgScale + 1.0;
					tempXZ = (z.y + z.x) * SQRT_1_2;
					z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
						z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
					z = z - fractal->transformCommon.offsetA200;
					break;

				case multi_orderOfTransf_typeT1Mod:
					tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
					z = CVector4((tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2,
						z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);
					z = fabs(z) * fractal->transformCommon.scale3D333;
					tempL = temp.Length();
					// if (tempL < 1e-21) tempL = 1e-21;
					avgScale = z.Length() / tempL;
					aux.r_dz *= avgScale;
					aux.DE = aux.DE * avgScale + 1.0;
					z = (fabs(z + fractal->transformCommon.offset111)
							 - fabs(z - fractal->transformCommon.offset111) - z);
					tempXZ = (z.y + z.x) * SQRT_1_2;
					z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
						z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
					break;

				case multi_orderOfTransf_typeT2:
					tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
					z = CVector4((tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2,
						z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);
					tempV2 = z;
					tempV2.x = sqrt(z.y * z.y + z.z * z.z);
					tempV2.y = sqrt(z.x * z.x + z.z * z.z); // switching, squared, sqrt
					tempV2.z = sqrt(z.x * z.x + z.y * z.y);
					z = fabs(tempV2 - fractal->transformCommon.offsetA111);
					temp = z;
					tempL = temp.Length();
					z = fabs(z) * fractal->transformCommon.scale3D444;
					// if (tempL < 1e-21) tempL = 1e-21;
					avgScale = z.Length() / tempL;
					aux.r_dz *= avgScale;
					aux.DE = aux.DE * avgScale + 1.0;
					tempXZ = (z.y + z.x) * SQRT_1_2;
					z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
						z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
					break;

				case multi_orderOfTransf_typeT3:
					tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
					z = CVector4((tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2,
						z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

					tempV2 = z;
					tempV2.x = (z.y + z.z);
					tempV2.y = (z.x + z.z); // switching
					tempV2.z = (z.x + z.y);
					z = (fabs(tempV2 - fractal->transformCommon.offset222))
							* fractal->transformCommon.scale3Db222;

					tempXZ = (z.y + z.x) * SQRT_1_2;
					z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
						z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
					break;

				case multi_orderOfTransf_typeT4:
					tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
					z = CVector4((tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2,
						z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

					tempV2 = z;
					tempV2.x = (z.y * z.y + z.z * z.z);
					tempV2.y = (z.x * z.x + z.z * z.z); // switching, squared,
					tempV2.z = (z.x * z.x + z.y * z.y);
					z = (fabs(tempV2 - fractal->transformCommon.offsetB111))
							* fractal->transformCommon.scale3Dc222;

					tempXZ = (z.y + z.x) * SQRT_1_2;
					z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
						z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
					break;

				case multi_orderOfTransf_typeT5b:
					tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
					z = CVector4((tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2,
						z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);
					// if (z.x > -1e-21 && z.x < 1e-21)
					//	z.x = (z.x > 0) ? 1e-21 : -1e-21;
					// if (z.y > -1e-21 && z.y < 1e-21)
					//	z.y = (z.y > 0) ? 1e-21 : -1e-21;
					// if (z.z > -1e-21 && z.z < 1e-21)
					//	z.z = (z.z > 0) ? 1e-21 : -1e-21;
					tempV2 = z;
					tempV2.x = fabs(
						pow(pow(z.y, fractal->transformCommon.int8X) + pow(z.z, fractal->transformCommon.int8X),
							fractal->transformCommon.power025.x));
					tempV2.y = fabs(
						pow(pow(z.x, fractal->transformCommon.int8Y) + pow(z.z, fractal->transformCommon.int8Y),
							fractal->transformCommon.power025.y));
					tempV2.z = fabs(
						pow(pow(z.x, fractal->transformCommon.int8Z) + pow(z.y, fractal->transformCommon.int8Z),
							fractal->transformCommon.power025.z));
					z = (fabs(tempV2 - fractal->transformCommon.offsetC111))
							* fractal->transformCommon.scale3Dd222;

					tempXZ = (z.y + z.x) * SQRT_1_2;
					z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
						z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
					break;
			}
		}
	}
}

/**
 * benesiPwr2 mandelbulb
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void BenesiPwr2MandelbulbIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	{
		CVector4 gap = fractal->transformCommon.constantMultiplier000;
		double t;
		double dot1;

		if (fractal->transformCommon.functionEnabledPFalse
				&& aux.i >= fractal->transformCommon.startIterationsP
				&& aux.i < fractal->transformCommon.stopIterationsP1)
		{
			z.y = fabs(z.y);
			z.z = fabs(z.z);
			dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
			t = max(0.0, dot1);
			z.x -= t * -SQRT_3;
			z.y = fabs(z.y - t);

			if (z.y > z.z) swap(z.y, z.z);
			z -= gap * CVector4(SQRT_3_4, 1.5, 1.5, 0.0);

			if (z.z > z.x) swap(z.z, z.x);
			if (z.x > 0.0)
			{
				z.y = max(0.0, z.y);
				z.z = max(0.0, z.z);
			}
		}
	}

	if (fractal->transformCommon.benesiT1Enabled && aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D222;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		if (fractal->transformCommon.rotationEnabled)
		{ // rotation inside T1
			z = fractal->transformCommon.rotationMatrix.RotateVector(z);
		}

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
		z = z - fractal->transformCommon.offset200;
	}

	if (fractal->transformCommon.addCpixelEnabled
			&& aux.i >= fractal->transformCommon.startIterationsF
			&& aux.i < fractal->transformCommon.stopIterationsF)
	{											// Benesi original pwr2
		aux.r = z.Length(); // needed when alternating pwr2s
		aux.r_dz = aux.r_dz * 2.0 * aux.r;
		double r1 = z.y * z.y + z.z * z.z;
		CVector4 newZ = CVector4(0.0, 0.0, 0.0, z.w);
		if (c.x < 0.0 || z.x < sqrt(r1))
		{
			newZ.x = z.x * z.x - r1;
		}
		else
		{
			newZ.x = -z.x * z.x + r1;
		}
		r1 = -1.0 / sqrt(r1) * 2.0 * fabs(z.x);
		newZ.y = r1 * (z.y * z.y - z.z * z.z);
		newZ.z = r1 * 2.0 * z.y * z.z;
		z = newZ + (c * fractal->transformCommon.constantMultiplierA100);
	}

	if (fractal->transformCommon.addCpixelEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{ // pine tree
		CVector4 temp = z;
		aux.r = z.Length(); // needed when alternating pwr2s
		z *= z;
		double t = 2.0 * temp.x;
		if (z.y + z.z > 0.0)
			t = t / sqrt(z.y + z.z);
		else
			t = 1.0;
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w);
			aux.c = tempC;
		}
		else
		{
			tempC = CVector4(c.x, c.z, c.y, c.w);
		}
		z.x = (z.x - z.y - z.z) + tempC.x * fractal->transformCommon.constantMultiplier100.x;
		z.z = (t * (z.y - z.z)) + tempC.z * fractal->transformCommon.constantMultiplier100.y;
		z.y = (2.0 * t * temp.y * temp.z) + tempC.y * fractal->transformCommon.constantMultiplier100.z;
		aux.r_dz = aux.r * aux.r_dz * 2.0 + 1.0;
	}

	if (fractal->transformCommon.functionEnabledBxFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsD)
	{
		z = CVector4(z.x * cos(z.y * fractal->transformCommon.scale1),
					z.x * sin(z.y * fractal->transformCommon.scale1), z.z * fractal->transformCommon.scaleC1,
					z.w)
				* fractal->transformCommon.scaleA1;
		aux.r_dz *= fabs(fractal->transformCommon.scaleA1);
	}

	if (fractal->transformCommon.juliaMode)
	{
		z.x += fractal->transformCommon.offset000.x;
		z.y += fractal->transformCommon.offset000.y;
		z.z += fractal->transformCommon.offset000.z;
	}
}

/**
 * CollatzIteration formula
 * @reference https://mathr.co.uk/blog/2016-04-10_collatz_fractal.html
 *            https://en.wikipedia.org/wiki/Collatz_conjecture#Iterating_on_real_or_complex_numbers
 */
void CollatzIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);

	CVector4 xV = CVector4(1.0, 1.0, 1.0, 0.0);
	CVector4 temp = CVector4(xV + 2.0 * z);
	temp *= z.RotateAroundVectorByAngle(xV.GetXYZ(), M_PI);
	z = xV + 4.0 * z - temp;
	z /= 4.0;
	aux.DE = aux.DE * 4.0 + 1.0;
	aux.r_dz *= 4.0;
}

/**
 * CollatzModIteration formula
 * @reference https://mathr.co.uk/blog/2016-04-10_collatz_fractal.html
 *            https://en.wikipedia.org/wiki/Collatz_conjecture#Iterating_on_real_or_complex_numbers
 */
void CollatzModIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	z = fractal->transformCommon.constantMultiplierB111 + fractal->transformCommon.scale4 * z
			- (fractal->transformCommon.constantMultiplier111 + fractal->transformCommon.scale2 * z)
					* z.RotateAroundVectorByAngle(fractal->transformCommon.constantMultiplier111.GetXYZ(),
								M_PI * fractal->transformCommon.scale1); // * cPI ;

	z *= fractal->transformCommon.scale025;

	aux.DE = aux.DE * 4.0 * fractal->analyticDE.scaleLin + 1.0;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		c = CVector4(c.z, c.y, c.x, c.w);
		z += c * fractal->transformCommon.constantMultiplierA111;
	}
}

/**
 * Modified Mandelbox (ABox) formula
 */
void FoldBoxMod1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		CVector4 tempA, tempB;

		if (fractal->transformCommon.functionEnabledx)
			tempA.x = fabs(z.x + fractal->transformCommon.additionConstant111.x);
		if (fractal->transformCommon.functionEnabledAx)
			tempB.x = fabs(z.x - fractal->transformCommon.additionConstantA111.x);
		z.x = tempA.x - tempB.x - (z.x * fractal->transformCommon.scale3D111.x);

		if (fractal->transformCommon.functionEnabledy)
			tempA.y = fabs(z.y + fractal->transformCommon.additionConstant111.y);
		if (fractal->transformCommon.functionEnabledAy)
			tempB.y = fabs(z.y - fractal->transformCommon.additionConstantA111.y);
		z.y = tempA.y - tempB.y - (z.y * fractal->transformCommon.scale3D111.y);

		if (fractal->transformCommon.functionEnabledz)
			tempA.z = fabs(z.z + fractal->transformCommon.additionConstant111.z);
		if (fractal->transformCommon.functionEnabledAz)
			tempB.z = fabs(z.z - fractal->transformCommon.additionConstantA111.z);
		z.z = tempA.z - tempB.z - (z.z * fractal->transformCommon.scale3D111.z);
	}

	if (fractal->transformCommon.functionEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA) // box fold
	{
		if (fabs(z.x) > fractal->mandelbox.foldingLimit)
		{
			z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
			aux.color += fractal->mandelbox.color.factor.x;
		}
		if (fabs(z.y) > fractal->mandelbox.foldingLimit)
		{
			z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
			aux.color += fractal->mandelbox.color.factor.y;
		}
		double zLimit = fractal->mandelbox.foldingLimit * fractal->transformCommon.scale1;
		double zValue = fractal->mandelbox.foldingValue * fractal->transformCommon.scale1;
		if (fabs(z.z) > zLimit)
		{
			z.z = sign(z.z) * zValue - z.z;
			aux.color += fractal->mandelbox.color.factor.z;
		}
	}

	if (aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB) // spherical fold
	{
		double r2 = z.Dot(z);
		// if (r2 < 1e-21 && r2 > -1e-21)
		//	r2 = (r2 > 0) ? 1e-21 : -1e-21;
		if (r2 < fractal->mandelbox.mR2)
		{
			z *= fractal->mandelbox.mboxFactor1;
			aux.DE *= fractal->mandelbox.mboxFactor1;
			aux.color += fractal->mandelbox.color.factorSp1;
		}
		else if (r2 < fractal->mandelbox.fR2)
		{
			double tglad_factor2 = fractal->mandelbox.fR2 / r2;
			z *= tglad_factor2;
			aux.DE *= tglad_factor2;
			aux.color += fractal->mandelbox.color.factorSp2;
		}
	}

	z *= fractal->mandelbox.scale;
	aux.DE = aux.DE * fabs(fractal->mandelbox.scale) + 1.0;
	aux.r_dz *= fabs(fractal->mandelbox.scale);

	if (fractal->mandelbox.mainRotationEnabled && aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		z = fractal->mandelbox.mainRot.RotateVector(z);
	}
}

/**
 * IQ-Bulb from Mandelbulb 3D and Inigo Quilez
 * @reference http://iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
 */
void IqBulbIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	// extract polar coordinates
	double wr = aux.r;
	double wo = acos(z.y / wr);
	double wi = atan2(z.x, z.z);

	// scale and rotate the point
	wr = pow(wr, fractal->transformCommon.pwr8 - 1.0);
	aux.r_dz = wr * aux.r_dz * fractal->transformCommon.pwr8 + 1.0;
	wr *= aux.r;
	wo *= fractal->transformCommon.pwr8;
	wi *= fractal->transformCommon.pwr8a;

	// convert back to cartesian coordinates
	z.x = sin(wo) * sin(wi);
	z.y = cos(wo);
	z.z = sin(wo) * cos(wi);

	z *= wr; // then add Cpixel constant
}

/**
 * Based on Kalisets1 and KaliDucks, from Mandelbulb 3D, and refer Formula proposed by Kali, with
 * features added by Darkbeam.
 *
 * <b>M3D notes:</b>
 * Try out julia and low R_bailout values of 2 down to 1!
 * You might have to cutoff at z=0 or so, to see something.
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns
 */
void Kalisets1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	z = fabs(z);
	double sqs = (z.x * z.x + z.y * z.y + z.z * z.z + 1e-21); // sph inv
	double m;
	double minR = fractal->transformCommon.minR0; //  KaliDucks

	if (sqs < minR)
		m = 1.0 / sqrt(minR);
	else
		m = fractal->transformCommon.scale / sqs; // kalisets

	z = z * m;
	aux.DE = aux.DE * fabs(m) + 1.0;

	if (fractal->transformCommon.addCpixelEnabledFalse)
		z += c * fractal->transformCommon.constantMultiplier111;

	if (fractal->transformCommon.juliaMode) z += fractal->transformCommon.juliaC;

	if (fractal->transformCommon.rotationEnabled)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
}

/**
 * Mandelbox Menger Sponge Hybrid
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/amazing-fractal/msg12467/#msg12467
 */
void MandelboxMengerIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	if (fractal->mandelbox.rotationsEnabled)
	{
		CVector4 zRot;
		// cast vector to array pointer for address taking of components in opencl
		double *zRotP = (double *)&zRot;
		const double *colP = (const double *)&fractal->mandelbox.color.factor;
		for (int dim = 0; dim < 3; dim++)
		{
			// handle each dimension x, y and z sequentially in pointer var dim
			double *rotDim = (dim == 0) ? &zRotP[0] : ((dim == 1) ? &zRotP[1] : &zRotP[2]);
			const double *colorFactor = (dim == 0) ? &colP[0] : ((dim == 1) ? &colP[1] : &colP[2]);

			zRot = fractal->mandelbox.rot[0][dim].RotateVector(z);
			if (*rotDim > fractal->mandelbox.foldingLimit)
			{
				*rotDim = fractal->mandelbox.foldingValue - *rotDim;
				z = fractal->mandelbox.rotinv[0][dim].RotateVector(zRot);
				aux.color += *colorFactor;
			}
			else
			{
				zRot = fractal->mandelbox.rot[1][dim].RotateVector(z);
				if (*rotDim < -fractal->mandelbox.foldingLimit)
				{
					*rotDim = -fractal->mandelbox.foldingValue - *rotDim;
					z = fractal->mandelbox.rotinv[1][dim].RotateVector(zRot);
					aux.color += *colorFactor;
				}
			}
		}
	}
	else
	{
		if (aux.i >= fractal->transformCommon.startIterationsA
				&& aux.i < fractal->transformCommon.stopIterationsA)
		{
			if (fabs(z.x) > fractal->mandelbox.foldingLimit)
			{
				z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
				aux.color += fractal->mandelbox.color.factor.x;
			}
			if (fabs(z.y) > fractal->mandelbox.foldingLimit)
			{
				z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
				aux.color += fractal->mandelbox.color.factor.y;
			}
			double zLimit = fractal->mandelbox.foldingLimit * fractal->transformCommon.scale1;
			double zValue = fractal->mandelbox.foldingValue * fractal->transformCommon.scale1;
			if (fabs(z.z) > zLimit)
			{
				z.z = sign(z.z) * zValue - z.z;
				aux.color += fractal->mandelbox.color.factor.z;
			}
		}
	}

	if (aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB)
	{
		double r2 = z.Dot(z);
		z += fractal->mandelbox.offset;
		if (r2 < fractal->mandelbox.mR2)
		{
			z *= fractal->mandelbox.mboxFactor1;
			aux.DE *= fractal->mandelbox.mboxFactor1;
			aux.color += fractal->mandelbox.color.factorSp1;
		}
		else if (r2 < fractal->mandelbox.fR2)
		{
			double tglad_factor2 = fractal->mandelbox.fR2 / r2;
			z *= tglad_factor2;
			aux.DE *= tglad_factor2;
			aux.color += fractal->mandelbox.color.factorSp2;
		}
		z -= fractal->mandelbox.offset;
	}
	if (fractal->mandelbox.mainRotationEnabled && aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
		z = fractal->mandelbox.mainRot.RotateVector(z);

	if (aux.i >= fractal->transformCommon.startIterationsS
			&& aux.i < fractal->transformCommon.stopIterationsS)
	{
		z = z * fractal->mandelbox.scale;
		aux.DE = aux.DE * fabs(fractal->mandelbox.scale) + 1.0;
	}

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplier111;
	}
	if (fractal->transformCommon.functionEnabled && aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		int count = fractal->transformCommon.int1; // Menger Sponge
		int k;
		for (k = 0; k < count; k++)
		{
			z = fabs(z);
			if (z.x - z.y < 0.0) swap(z.y, z.x);
			if (z.x - z.z < 0.0) swap(z.z, z.x);
			if (z.y - z.z < 0.0) swap(z.z, z.y);
			z *= fractal->transformCommon.scale3;
			z.x -= 2.0 * fractal->transformCommon.constantMultiplierA111.x;
			z.y -= 2.0 * fractal->transformCommon.constantMultiplierA111.y;
			if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplierA111.z;
			aux.DE *= fractal->transformCommon.scale3 * fractal->transformCommon.scaleA1;

			z += fractal->transformCommon.additionConstantA000;

			if (fractal->transformCommon.functionEnabledxFalse) // addCpixel options
			{
				switch (fractal->mandelbulbMulti.orderOfXYZC)
				{
					case multi_OrderOfXYZ_xyz:
					default: c = CVector4(c.x, c.y, c.z, c.w); break;
					case multi_OrderOfXYZ_xzy: c = CVector4(c.x, c.z, c.y, c.w); break;
					case multi_OrderOfXYZ_yxz: c = CVector4(c.y, c.x, c.z, c.w); break;
					case multi_OrderOfXYZ_yzx: c = CVector4(c.y, c.z, c.x, c.w); break;
					case multi_OrderOfXYZ_zxy: c = CVector4(c.z, c.x, c.y, c.w); break;
					case multi_OrderOfXYZ_zyx: c = CVector4(c.z, c.y, c.x, c.w); break;
				}
				z += c * fractal->transformCommon.constantMultiplierB111;
			}
		}
	}
	aux.foldFactor = fractal->foldColor.compFold;
	aux.minRFactor = fractal->foldColor.compMinR;
	double scaleColor = fractal->foldColor.colorMin + fabs(fractal->mandelbox.scale);
	// scaleColor += fabs(fractal->mandelbox.scale);
	aux.scaleFactor = scaleColor * fractal->foldColor.compScale;
}

/**
 * mandelbulbBermarte
 *
 * abs() version of Mandelbulb Kali modified by visual.bermarte
 * @reference http://www.fractalforums.com/theory/mandelbulb-variant/
 */
void MandelbulbBermarteIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledAxFalse) z.x = fabs(z.x);
	if (fractal->transformCommon.functionEnabledAyFalse) z.y = fabs(z.y);
	if (fractal->transformCommon.functionEnabledAzFalse) z.z = fabs(z.z);

	double th0;
	double ph0;
	double costh;
	double sinth;
	double zp;

	if (fractal->transformCommon.functionEnabledFalse)
	{
		th0 = acos(z.z / aux.r) + fractal->bulb.betaAngleOffset + 1e-061; // MUST keep exception catch
		ph0 = atan(z.y / z.x) + fractal->bulb.alphaAngleOffset;
		th0 *= fractal->transformCommon.pwr8 * fractal->transformCommon.scaleA1;
		sinth = sin(th0);
		z = aux.r * CVector4(sinth * cos(ph0), sin(ph0) * sinth, cos(th0), 0.0);
	}
	else
	{
		th0 = asin(z.z / aux.r) + fractal->bulb.betaAngleOffset + 1e-061; // MUST keep exception catch
		ph0 = atan2(z.y, z.x) + fractal->bulb.alphaAngleOffset;
		th0 *= fractal->transformCommon.pwr8 * fractal->transformCommon.scaleA1;
		costh = cos(th0);
		z = aux.r * CVector4(costh * sin(ph0), cos(ph0) * costh, sin(th0), 0.0);
	}
	if (fractal->transformCommon.functionEnabledAx) z.x = fabs(z.x);
	if (fractal->transformCommon.functionEnabledAy) z.y = fabs(z.y);
	if (fractal->transformCommon.functionEnabledAz) z.z = fabs(z.z);

	if (fractal->transformCommon.functionEnabledxFalse)
	{
		th0 = acos(z.z / aux.r) + fractal->transformCommon.betaAngleOffset
					+ 1e-061; // MUST keep exception catch ??;
		ph0 = atan(z.y / z.x);
		ph0 *= fractal->transformCommon.pwr8 * fractal->transformCommon.scaleB1;
		zp = pow(aux.r, fractal->transformCommon.pwr8);
		sinth = sin(th0);
		z = zp * CVector4(sinth * cos(ph0), sin(ph0) * sinth, cos(th0), 0.0);
	}
	else
	{
		th0 = asin(z.z / aux.r) + fractal->transformCommon.betaAngleOffset
					+ 1e-061; // MUST keep exception catch ??;
		ph0 = atan2(z.y, z.x);
		ph0 *= fractal->transformCommon.pwr8 * fractal->transformCommon.scaleB1;
		zp = pow(aux.r, fractal->transformCommon.pwr8);
		costh = cos(th0);
		z = zp * CVector4(costh * sin(ph0), cos(ph0) * costh, sin(th0), 0.0);
	}

	if (fractal->transformCommon.functionEnabledBxFalse) z.x = fabs(z.x);
	if (fractal->transformCommon.functionEnabledByFalse) z.y = fabs(z.y);

	if (fractal->analyticDE.enabledFalse)
	{ // analytic log DE adjustment
		aux.r_dz = pow(aux.r, fractal->transformCommon.pwr8 - fractal->analyticDE.offset1) * aux.r_dz
								 * fractal->transformCommon.pwr8 * fractal->analyticDE.scale1
							 + fractal->analyticDE.offset2;
	}
	else // default, i.e. scale1 & offset1 & offset2 = 1.0
	{
		aux.r_dz =
			pow(aux.r, fractal->transformCommon.pwr8 - 1.0) * fractal->transformCommon.pwr8 * aux.r_dz
			+ 1.0;
	}
}
/**
 * mandelbulb juliabulb hybrid 3D
 */
void MandelbulbJuliabulbIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		aux.r = z.Length();
		if (fractal->transformCommon.functionEnabledFalse)
		{
			if (fractal->transformCommon.functionEnabledAxFalse) z.x = fabs(z.x);
			if (fractal->transformCommon.functionEnabledAyFalse) z.y = fabs(z.y);
			if (fractal->transformCommon.functionEnabledAzFalse) z.z = fabs(z.z);
		}

		double th0 = fractal->bulb.betaAngleOffset;
		double ph0 = fractal->bulb.alphaAngleOffset;
		CVector3 v;

		switch (fractal->mandelbulbMulti.orderOfXYZ)
		{
			case multi_OrderOfXYZ_xyz:
			default: v = CVector3(z.x, z.y, z.z); break;
			case multi_OrderOfXYZ_xzy: v = CVector3(z.x, z.z, z.y); break;
			case multi_OrderOfXYZ_yxz: v = CVector3(z.y, z.x, z.z); break;
			case multi_OrderOfXYZ_yzx: v = CVector3(z.y, z.z, z.x); break;
			case multi_OrderOfXYZ_zxy: v = CVector3(z.z, z.x, z.y); break;
			case multi_OrderOfXYZ_zyx: v = CVector3(z.z, z.y, z.x); break;
		}
		// if (aux.r < 1e-21)
		//	aux.r = 1e-21;
		// if (v3 < 1e-21 && v3 > -1e-21)
		//	v3 = (v3 > 0) ? 1e-21 : -1e-21;

		if (fractal->mandelbulbMulti.acosOrAsin == multi_acosOrAsin_acos)
			th0 += acos(v.x / aux.r);
		else
			th0 += asin(v.x / aux.r);

		if (fractal->mandelbulbMulti.atanOrAtan2 == multi_atanOrAtan2_atan)
			ph0 += atan(v.y / v.z);
		else
			ph0 += atan2(v.y, v.z);

		double rp = pow(aux.r, fractal->bulb.power - 1.0);
		double th = th0 * fractal->bulb.power * fractal->transformCommon.scaleA1;
		double ph = ph0 * fractal->bulb.power * fractal->transformCommon.scaleB1;

		aux.r_dz = rp * aux.r_dz * fractal->bulb.power + 1.0;
		rp *= aux.r;

		if (fractal->transformCommon.functionEnabledxFalse)
		{ // cosine mode
			double sinth = th;
			if (fractal->transformCommon.functionEnabledyFalse) sinth = th0;
			sinth = sin(sinth);
			z = rp * CVector4(sinth * sin(ph), cos(ph) * sinth, cos(th), 0.0);
		}
		else
		{ // sine mode ( default = V2.07))
			double costh = th;
			if (fractal->transformCommon.functionEnabledzFalse) costh = th0;
			costh = cos(costh);
			z = rp * CVector4(costh * cos(ph), sin(ph) * costh, sin(th), 0.0);
		}
	}
	// sym4
	if (aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsD)
	{
		aux.r = z.Length();
		aux.r_dz = aux.r_dz * 2.0 * aux.r;
		CVector4 temp = z;
		double tempL = temp.Length();
		// if (tempL < 1e-21)
		//	tempL = 1e-21;
		z *= fractal->transformCommon.scale3D111;

		aux.r_dz *= fabs(z.Length() / tempL);

		if (fabs(z.x) < fabs(z.z)) swap(z.x, z.z);
		if (fabs(z.x) < fabs(z.y)) swap(z.x, z.y);
		if (fabs(z.y) < fabs(z.z)) swap(z.y, z.z);

		if (z.x * z.z < 0.0) z.z = -z.z;
		if (z.x * z.y < 0.0) z.y = -z.y;

		temp.x = z.x * z.x - z.y * z.y - z.z * z.z;
		temp.y = 2.0 * z.x * z.y;
		temp.z = 2.0 * z.x * z.z;

		z = temp + fractal->transformCommon.offsetF000;

		/*double lengthTempZ = -z.Length();
		// if (lengthTempZ > -1e-21)
		//	lengthTempZ = -1e-21;   //  z is neg.)
		z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
		// scale
		z *= fractal->transformCommon.scale1;
		aux.r_dz *= fabs(fractal->transformCommon.scale1);*/
	}
	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 c = aux.const_c;
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplierC111;
	}
	// radial offset
	double lengthTempZ = -z.Length();
	// if (lengthTempZ > -1e-21)
	//	lengthTempZ = -1e-21;   //  z is neg.)
	z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
	// scale
	z *= fractal->transformCommon.scale1;
	aux.r_dz *= fabs(fractal->transformCommon.scale1);
}
/**
 * mandelbulb Kali modification
 * @reference http://www.fractalforums.com/theory/mandelbulb-variant/
 */
void MandelbulbKaliIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledFalse)
	{
		if (fractal->transformCommon.functionEnabledAxFalse) z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledAyFalse) z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAzFalse) z.z = fabs(z.z);
	}

	double th0 =
		acos(z.z / aux.r) + fractal->bulb.betaAngleOffset + 1e-061; // MUST keep exception catch
	double ph0 = atan(z.y / z.x) + fractal->bulb.alphaAngleOffset;
	th0 *= fractal->transformCommon.pwr8 * fractal->transformCommon.scaleA1;
	double sinth = sin(th0);
	z = aux.r * CVector4(sinth * cos(ph0), sin(ph0) * sinth, cos(th0), 0.0);

	th0 = acos(z.z / aux.r) + fractal->transformCommon.betaAngleOffset
				+ 1e-061; // MUST keep exception catch ??;
	ph0 = atan(z.y / z.x);
	ph0 *= fractal->transformCommon.pwr8 * fractal->transformCommon.scaleB1;
	double zp = pow(aux.r, fractal->transformCommon.pwr8);
	sinth = sin(th0);
	z = zp * CVector4(sinth * cos(ph0), sin(ph0) * sinth, cos(th0), 0.0);

	if (fractal->analyticDE.enabledFalse)
	{ // analytic log DE adjustment
		aux.r_dz = pow(aux.r, fractal->transformCommon.pwr8 - fractal->analyticDE.offset1) * aux.r_dz
								 * fractal->transformCommon.pwr8 * fractal->analyticDE.scale1
							 + fractal->analyticDE.offset2;
	}
	else // default, i.e. scale1 & offset1 & offset2 = 1.0
	{
		aux.r_dz =
			pow(aux.r, fractal->transformCommon.pwr8 - 1.0) * fractal->transformCommon.pwr8 * aux.r_dz
			+ 1.0;
	}
}

/**
 * based on mandelbulb Kali multi
 * @reference http://www.fractalforums.com/theory/mandelbulb-variant/
 */
void MandelbulbKaliMultiIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	if (fractal->transformCommon.functionEnabledFalse)
	{
		if (fractal->transformCommon.functionEnabledAxFalse) z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledAyFalse) z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAzFalse) z.z = fabs(z.z);
	}
	double costh;
	double sinth;
	double th0 = fractal->bulb.betaAngleOffset + 1e-061; // MUST keep exception catch
	double ph0 = fractal->bulb.alphaAngleOffset;
	CVector4 v;
	switch (fractal->mandelbulbMulti.orderOfXYZ)
	{
		case multi_OrderOfXYZ_xyz:
		default: v = CVector4(z.x, z.y, z.z, z.w); break;
		case multi_OrderOfXYZ_xzy: v = CVector4(z.x, z.z, z.y, z.w); break;
		case multi_OrderOfXYZ_yxz: v = CVector4(z.y, z.x, z.z, z.w); break;
		case multi_OrderOfXYZ_yzx: v = CVector4(z.y, z.z, z.x, z.w); break;
		case multi_OrderOfXYZ_zxy: v = CVector4(z.z, z.x, z.y, z.w); break;
		case multi_OrderOfXYZ_zyx: v = CVector4(z.z, z.y, z.x, z.w); break;
	}

	if (fractal->mandelbulbMulti.acosOrAsin == multi_acosOrAsin_acos)
		th0 += acos(v.x / aux.r);
	else
		th0 += asin(v.x / aux.r);
	if (fractal->mandelbulbMulti.atanOrAtan2 == multi_atanOrAtan2_atan)
		ph0 += atan(v.y / v.z);
	else
		ph0 += atan2(v.y, v.z);

	th0 *= fractal->transformCommon.pwr8 * fractal->transformCommon.scaleA1;

	if (fractal->transformCommon.functionEnabledBxFalse)
	{
		costh = cos(th0);
		z = aux.r * CVector4(costh * sin(ph0), cos(ph0) * costh, sin(th0), 0.0);
	}
	else
	{ // cosine mode default
		sinth = sin(th0);
		z = aux.r * CVector4(sinth * cos(ph0), sin(ph0) * sinth, cos(th0), 0.0);
	}

	if (fractal->transformCommon.functionEnabledxFalse)
	{
		switch (fractal->mandelbulbMulti.orderOfXYZ2)
		{
			case multi_OrderOfXYZ_xyz:
			default: v = CVector4(z.x, z.y, z.z, z.w); break;
			case multi_OrderOfXYZ_xzy: v = CVector4(z.x, z.z, z.y, z.w); break;
			case multi_OrderOfXYZ_yxz: v = CVector4(z.y, z.x, z.z, z.w); break;
			case multi_OrderOfXYZ_yzx: v = CVector4(z.y, z.z, z.x, z.w); break;
			case multi_OrderOfXYZ_zxy: v = CVector4(z.z, z.x, z.y, z.w); break;
			case multi_OrderOfXYZ_zyx: v = CVector4(z.z, z.y, z.x, z.w); break;
		}
		if (fractal->mandelbulbMulti.acosOrAsinA == multi_acosOrAsin_acos)
			th0 = acos(v.x / aux.r) + fractal->transformCommon.betaAngleOffset
						+ 1e-061; // MUST keep exception catch
		else
			th0 += asin(v.x / aux.r) + fractal->transformCommon.betaAngleOffset
						 + 1e-061; // MUST keep exception catch;

		if (fractal->mandelbulbMulti.atanOrAtan2A == multi_atanOrAtan2_atan)
			ph0 += atan(v.y / v.z);
		else
			ph0 += atan2(v.y, v.z);
	}
	else
	{
		switch (fractal->mandelbulbMulti.orderOfXYZ)
		{
			case multi_OrderOfXYZ_xyz:
			default: v = CVector4(z.x, z.y, z.z, z.w); break;
			case multi_OrderOfXYZ_xzy: v = CVector4(z.x, z.z, z.y, z.w); break;
			case multi_OrderOfXYZ_yxz: v = CVector4(z.y, z.x, z.z, z.w); break;
			case multi_OrderOfXYZ_yzx: v = CVector4(z.y, z.z, z.x, z.w); break;
			case multi_OrderOfXYZ_zxy: v = CVector4(z.z, z.x, z.y, z.w); break;
			case multi_OrderOfXYZ_zyx: v = CVector4(z.z, z.y, z.x, z.w); break;
		}

		if (fractal->mandelbulbMulti.acosOrAsin == multi_acosOrAsin_acos)
			th0 = acos(v.x / aux.r) + fractal->transformCommon.betaAngleOffset
						+ 1e-061; // MUST keep exception catch ??;
		else
			th0 += asin(v.x / aux.r) + fractal->transformCommon.betaAngleOffset
						 + 1e-061; // MUST keep exception catch ??;

		if (fractal->mandelbulbMulti.atanOrAtan2 == multi_atanOrAtan2_atan)
			ph0 += atan(v.y / v.z);
		else
			ph0 += atan2(v.y, v.z);
	}

	ph0 *= fractal->transformCommon.pwr8 * fractal->transformCommon.scaleB1 * 0.5; // 0.5 retain
	double zp = pow(aux.r, fractal->transformCommon.pwr8);

	if (fractal->transformCommon.functionEnabledzFalse)
	{ // sine mode
		costh = cos(th0);
		z = zp * CVector4(costh * sin(ph0), cos(ph0) * costh, sin(th0), 0.0);
	}
	else
	{ // cosine mode default
		sinth = sin(th0);
		z = zp * CVector4(sinth * cos(ph0), sin(ph0) * sinth, cos(th0), 0.0);
	}

	if (fractal->analyticDE.enabledFalse)
	{ // analytic log DE adjustment
		aux.r_dz = pow(aux.r, fractal->transformCommon.pwr8 - fractal->analyticDE.offset1) * aux.r_dz
								 * fractal->transformCommon.pwr8 * fractal->analyticDE.scale1
							 + fractal->analyticDE.offset2;
	}
	else // default, i.e. scale1 & offset1 & offset2 = 1.0
	{
		aux.r_dz =
			pow(aux.r, fractal->transformCommon.pwr8 - 1.0) * fractal->transformCommon.pwr8 * aux.r_dz
			+ 1.0;
	}
	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplierC111;
	}
}

/**
 * mandelbulbMulti 3D
 */
void MandelbulbMultiIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	aux.r = z.Length();
	if (fractal->transformCommon.functionEnabledFalse)
	{
		if (fractal->transformCommon.functionEnabledAxFalse) z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledAyFalse) z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAzFalse) z.z = fabs(z.z);
	}

	double th0 = fractal->bulb.betaAngleOffset;
	double ph0 = fractal->bulb.alphaAngleOffset;
	CVector4 v;

	switch (fractal->mandelbulbMulti.orderOfXYZ)
	{
		case multi_OrderOfXYZ_xyz:
		default: v = CVector4(z.x, z.y, z.z, z.w); break;
		case multi_OrderOfXYZ_xzy: v = CVector4(z.x, z.z, z.y, z.w); break;
		case multi_OrderOfXYZ_yxz: v = CVector4(z.y, z.x, z.z, z.w); break;
		case multi_OrderOfXYZ_yzx: v = CVector4(z.y, z.z, z.x, z.w); break;
		case multi_OrderOfXYZ_zxy: v = CVector4(z.z, z.x, z.y, z.w); break;
		case multi_OrderOfXYZ_zyx: v = CVector4(z.z, z.y, z.x, z.w); break;
	}
	// if (aux.r < 1e-21)
	//	aux.r = 1e-21;
	// if (v3 < 1e-21 && v3 > -1e-21)
	//	v3 = (v3 > 0) ? 1e-21 : -1e-21;

	if (fractal->mandelbulbMulti.acosOrAsin == multi_acosOrAsin_acos)
		th0 += acos(v.x / aux.r);
	else
		th0 += asin(v.x / aux.r);

	if (fractal->mandelbulbMulti.atanOrAtan2 == multi_atanOrAtan2_atan)
		ph0 += atan(v.y / v.z);
	else
		ph0 += atan2(v.y, v.z);

	double rp = pow(aux.r, fractal->bulb.power - 1.0);
	double th = th0 * fractal->bulb.power * fractal->transformCommon.scaleA1;
	double ph = ph0 * fractal->bulb.power * fractal->transformCommon.scaleB1;

	aux.r_dz = rp * aux.r_dz * fractal->bulb.power + 1.0;
	rp *= aux.r;

	if (fractal->transformCommon.functionEnabledxFalse)
	{ // cosine mode
		double sinth = th;
		if (fractal->transformCommon.functionEnabledyFalse) sinth = th0;
		sinth = sin(sinth);
		z = rp * CVector4(sinth * sin(ph), cos(ph) * sinth, cos(th), 0.0);
	}
	else
	{ // sine mode ( default = V2.07))
		double costh = th;
		if (fractal->transformCommon.functionEnabledzFalse) costh = th0;
		costh = cos(costh);
		z = rp * CVector4(costh * cos(ph), sin(ph) * costh, sin(th), 0.0);
	}

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplierC111;
	}
}
/**
 * mandelbulbMulti2 3D
 */
void MandelbulbMulti2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;
	aux.r = z.Length();
	if (fractal->transformCommon.functionEnabledFalse)
	{
		if (fractal->transformCommon.functionEnabledAxFalse
				&& aux.i >= fractal->transformCommon.startIterationsX
				&& aux.i < fractal->transformCommon.stopIterationsX)
			z.x = fabs(z.x);
		if (fractal->transformCommon.functionEnabledAyFalse
				&& aux.i >= fractal->transformCommon.startIterationsY
				&& aux.i < fractal->transformCommon.stopIterationsY)
			z.y = fabs(z.y);
		if (fractal->transformCommon.functionEnabledAzFalse
				&& aux.i >= fractal->transformCommon.startIterationsZ
				&& aux.i < fractal->transformCommon.stopIterationsZ)
			z.z = fabs(z.z);
	}

	double th0 = fractal->bulb.betaAngleOffset;
	double ph0 = fractal->bulb.alphaAngleOffset;

	CVector3 v;
	switch (fractal->sinTan2Trig.orderOfZYX)
	{
		case multi_OrderOfZYX_zyx:
		default: v = CVector3(z.z, z.y, z.x); break;
		case multi_OrderOfZYX_zxy: v = CVector3(z.z, z.x, z.y); break;
		case multi_OrderOfZYX_yzx: v = CVector3(z.y, z.z, z.x); break;
		case multi_OrderOfZYX_yxz: v = CVector3(z.y, z.x, z.z); break;
		case multi_OrderOfZYX_xzy: v = CVector3(z.x, z.z, z.y); break;
		case multi_OrderOfZYX_xyz: v = CVector3(z.x, z.y, z.z); break;
	}

	if (fractal->sinTan2Trig.asinOrAcos == multi_asinOrAcos_asin)
		th0 += asin(v.x / aux.r);
	else
		th0 += acos(v.x / aux.r);

	if (fractal->sinTan2Trig.atan2OrAtan == multi_atan2OrAtan_atan2)
		ph0 += atan2(v.y , v.z);
	else
		ph0 += atan(v.y / v.z);

	double rp = pow(aux.r, fractal->bulb.power - 1.0);
	double th = th0 * fractal->bulb.power * fractal->transformCommon.scaleA1;
	double ph = ph0 * fractal->bulb.power * fractal->transformCommon.scaleB1;

	aux.r_dz = rp * aux.r_dz * fractal->bulb.power + 1.0;
	rp *= aux.r;

	if (fractal->transformCommon.functionEnabledxFalse)
	{ // cosine mode
		double sinth = th;
		if (fractal->transformCommon.functionEnabledyFalse) sinth = th0;
		sinth = sin(sinth);
		z = rp * CVector4(sinth * sin(ph), cos(ph) * sinth, cos(th), 0.0);
	}
	else
	{ // sine mode ( default = V2.07))
		double costh = th;
		if (fractal->transformCommon.functionEnabledzFalse) costh = th0;
		costh = cos(costh);
		z = rp * CVector4(costh * cos(ph), sin(ph) * costh, sin(th), 0.0);
	}

	if (fractal->transformCommon.functionEnabledKFalse)
	{
		if (fractal->transformCommon.functionEnabledDFalse
				&& aux.i >= fractal->transformCommon.startIterationsD
				&& aux.i < fractal->transformCommon.stopIterationsD)
			swap(z.x, z.y);
		if (fractal->transformCommon.functionEnabledEFalse
				&& aux.i >= fractal->transformCommon.startIterationsE
				&& aux.i < fractal->transformCommon.stopIterationsE)
			swap(z.x, z.z);

		// swap
		if (fractal->transformCommon.functionEnabledBxFalse) z.x = -z.x;
		if (fractal->transformCommon.functionEnabledByFalse) z.y = -z.y;
		if (fractal->transformCommon.functionEnabledBzFalse) z.z = -z.z;
	}
	z += fractal->transformCommon.additionConstant000;
	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplierC111;
	}
	// rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}
}

/**
 * mandelbulb Quaternion
 *
 */
void MandelbulbQuatIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.addCpixelEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC1)
	{
		CVector4 c = aux.const_c;
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplierC111;
	}

	// Quaternion fold
	if (aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsD1)
	{
		aux.r = z.Length();
		z = CVector4(z.x * z.x - z.y * z.y - z.z * z.z, z.x * z.y, z.x * z.z, 0.0);
		aux.r_dz = aux.r_dz * 2.0 * aux.r;
		double tempL = z.Length();
		z *= fractal->transformCommon.constantMultiplier122;
		// if (tempL < 1e-21) tempL = 1e-21;
		CVector3 tempAvgScale = CVector3(z.x, z.y / 2.0, z.z / 2.0);
		double avgScale = tempAvgScale.Length() / tempL;
		double tempAux = aux.r_dz * avgScale;
		aux.r_dz = aux.r_dz + (tempAux - aux.r_dz) * fractal->transformCommon.scaleF1;
		z += fractal->transformCommon.offset000;
	}
	// sym4
	if (fractal->transformCommon.functionEnabledCxFalse
			&& aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB)
	{
		aux.r = z.Length();
		aux.r_dz = aux.r_dz * 2.0 * aux.r;
		CVector4 temp = z;
		double tempL = temp.Length();
		// if (tempL < 1e-21)
		//	tempL = 1e-21;
		z *= fractal->transformCommon.scale3D111;

		aux.r_dz *= fabs(z.Length() / tempL);

		if (fabs(z.x) < fabs(z.z)) swap(z.x, z.z);
		if (fabs(z.x) < fabs(z.y)) swap(z.x, z.y);
		if (fabs(z.y) < fabs(z.z)) swap(z.y, z.z);

		if (z.x * z.z < 0.0) z.z = -z.z;
		if (z.x * z.y < 0.0) z.y = -z.y;

		temp.x = z.x * z.x - z.y * z.y - z.z * z.z;
		temp.y = 2.0 * z.x * z.y;
		temp.z = 2.0 * z.x * z.z;

		z = temp + fractal->transformCommon.offsetF000;
		// radial offset
		double lengthTempZ = -z.Length();
		// if (lengthTempZ > -1e-21)
		//	lengthTempZ = -1e-21;   //  z is neg.)
		z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
		// scale
		z *= fractal->transformCommon.scale1;
		aux.r_dz *= fabs(fractal->transformCommon.scale1);
	}

	//mandelbulb multi
	if (aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{

		aux.r = z.Length();
		if (fractal->transformCommon.functionEnabledFalse)
		{
			if (fractal->transformCommon.functionEnabledAxFalse
					&& aux.i >= fractal->transformCommon.startIterationsX
					&& aux.i < fractal->transformCommon.stopIterationsX)
				z.x = fabs(z.x);
			if (fractal->transformCommon.functionEnabledAyFalse
					&& aux.i >= fractal->transformCommon.startIterationsY
					&& aux.i < fractal->transformCommon.stopIterationsY)
				z.y = fabs(z.y);
			if (fractal->transformCommon.functionEnabledAzFalse
					&& aux.i >= fractal->transformCommon.startIterationsZ
					&& aux.i < fractal->transformCommon.stopIterationsZ)
				z.z = fabs(z.z);
		}

		double th0 = fractal->bulb.betaAngleOffset;
		double ph0 = fractal->bulb.alphaAngleOffset;

		CVector3 v;
		switch (fractal->sinTan2Trig.orderOfZYX)
		{
			case multi_OrderOfZYX_zyx:
			default: v = CVector3(z.z, z.y, z.x); break;
			case multi_OrderOfZYX_zxy: v = CVector3(z.z, z.x, z.y); break;
			case multi_OrderOfZYX_yzx: v = CVector3(z.y, z.z, z.x); break;
			case multi_OrderOfZYX_yxz: v = CVector3(z.y, z.x, z.z); break;
			case multi_OrderOfZYX_xzy: v = CVector3(z.x, z.z, z.y); break;
			case multi_OrderOfZYX_xyz: v = CVector3(z.x, z.y, z.z); break;
		}

		if (fractal->sinTan2Trig.asinOrAcos == multi_asinOrAcos_asin)
			th0 += asin(v.x / aux.r);
		else
			th0 += acos(v.x / aux.r);

		if (fractal->sinTan2Trig.atan2OrAtan == multi_atan2OrAtan_atan2)
			ph0 += atan2(v.y , v.z);
		else
			ph0 += atan(v.y / v.z);

		double rp = pow(aux.r, fractal->bulb.power - 1.0);
		double th = th0 * fractal->bulb.power * fractal->transformCommon.scaleA1;
		double ph = ph0 * fractal->bulb.power * fractal->transformCommon.scaleB1;

		aux.r_dz = rp * aux.r_dz * fractal->bulb.power + 1.0;
		rp *= aux.r;

		if (fractal->transformCommon.functionEnabledxFalse)
		{ // cosine mode
			double sinth = th;
			if (fractal->transformCommon.functionEnabledyFalse) sinth = th0;
			sinth = sin(sinth);
			z = rp * CVector4(sinth * sin(ph), cos(ph) * sinth, cos(th), 0.0);
		}
		else
		{ // sine mode
			double costh = th;
			if (fractal->transformCommon.functionEnabledzFalse) costh = th0;
			costh = cos(costh);
			z = rp * CVector4(costh * cos(ph), sin(ph) * costh, sin(th), 0.0);
		}

		if (fractal->transformCommon.functionEnabledKFalse)
		{
			if (fractal->transformCommon.functionEnabledDFalse) swap(z.x, z.y);
			if (fractal->transformCommon.functionEnabledEFalse) swap(z.x, z.z);

			// swap
			if (fractal->transformCommon.functionEnabledBxFalse) z.x = -z.x;
			if (fractal->transformCommon.functionEnabledByFalse) z.y = -z.y;
			if (fractal->transformCommon.functionEnabledBzFalse) z.z = -z.z;
		}
	}
	z += fractal->transformCommon.additionConstant000;

	// rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}
}

/**
 * mandelbulb vary scaleV1
 */
void MandelbulbVaryPowerV1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double tempVC = fractal->bulb.power; // constant to be varied

	if (aux.i >= fractal->transformCommon.startIterations250
			&& aux.i < fractal->transformCommon.stopIterations
			&& (fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250
					 != 0))
	{
		int iterationRange = fractal->transformCommon.stopIterations
													- fractal->transformCommon.startIterations250;
		int currentIteration = (aux.i - fractal->transformCommon.startIterations250);
		tempVC += fractal->transformCommon.offset0 * (1.0 * currentIteration) / iterationRange;
	}
	if (aux.i >= fractal->transformCommon.stopIterations)
	{
		tempVC = (tempVC + fractal->transformCommon.offset0);
	}

	aux.r = z.Length();
	// if (aux.r < 1e-21)
	//	aux.r = 1e-21;
	double th0 = asin(z.z / aux.r) + fractal->bulb.betaAngleOffset;
	double ph0 = atan2(z.y, z.x) + fractal->bulb.alphaAngleOffset;
	double rp = pow(aux.r, tempVC - 1.0);
	double th = th0 * tempVC;
	double ph = ph0 * tempVC;
	double cth = cos(th);
	aux.r_dz = rp * aux.r_dz * tempVC + 1.0;
	rp *= aux.r;
	z = CVector4(cth * cos(ph), cth * sin(ph), sin(th), 0.0) * rp;
}
/**
 * Menger Cross KIFS
 * from code by Knighty
 * http://www.fractalforums.com/fragmentarium/
 * cross-menger!-can-anyone-do-this/msg93972/#new
	*/
void MengerCrossKIFSIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 gap = fractal->transformCommon.constantMultiplier000;
	double t;
	double dot1;
	if (fractal->transformCommon.functionEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations1)
	{
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		if (fractal->transformCommon.functionEnabledFFalse) z.x = fabs(z.x);

		dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
		t = max(0.0, dot1);
		z.x -= (t * -SQRT_3) - 0.5;
		z.y = fabs(z.y - t);

		if (z.y > z.z) swap(z.y, z.z);
		z.y -= 0.75;
		z -= gap * CVector4(SQRT_3_4, 1.5, 1.5, 0.0);

		if (z.z > z.x) swap(z.z, z.x);

		z *= fractal->transformCommon.constantMultiplier111; // post scale

		if (fractal->transformCommon.functionEnabledyFalse)
		{
			z.y = max(0.0, z.y) * fractal->transformCommon.scaleA1;
			z.z = max(0.0, z.z) * fractal->transformCommon.scaleB1;
		}

		z += fractal->transformCommon.additionConstant000; // post offset
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

		aux.DE *= fractal->analyticDE.scale1; // tweak
	}

	if (fractal->transformCommon.functionEnabled && aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{ // CrossMengerKIFS
		float lengthL;
		// z = ( z );//+ preAdd );
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		if (fractal->transformCommon.functionEnabledzFalse) z.x = fabs(z.x);

		lengthL = z.x * SQRT_3_4 - z.y * 0.5;
		if (lengthL < 0.0)
		{
			z.x -= SQRT_3_4 * (2.0 * lengthL);
			z.y -= -lengthL;
		}
		lengthL = z.y;
		if (lengthL < 0.0)
		{
			z.y -= 2.0 * lengthL;
		}
		lengthL = (-z.y + z.z) * SQRT_1_2;
		if (lengthL < 0.0)
		{
			z.y -= -SQRT_1_2 * (2.0 * lengthL);
			z.z -= SQRT_1_2 * (2.0 * lengthL);
		}
		lengthL = z.y;
		if (lengthL < 0.5)
		{
			z.y -= 2.0 * (lengthL - 0.5);
		}
		CVector4 edge = fractal->transformCommon.offset222;
		if (fractal->transformCommon.functionEnabledxFalse)
		{
			z.x = edge.x - z.x;
			z.y = edge.y - z.y;
			z.z = edge.z - z.z;
		}

		z *= fractal->transformCommon.scale3;
		z -= CVector4(SQRT_3_4, 1.5, 1.5, 0.0) * (fractal->transformCommon.scale3 - 1.0);
		aux.DE *= fractal->transformCommon.scale3;

		z += fractal->transformCommon.offset000; // post offset
		if (fractal->transformCommon.functionEnabledPFalse)
		{
			z.x = fabs(z.x + fractal->transformCommon.offset) + fractal->transformCommon.offset0;
		}
		if (fractal->transformCommon.functionEnabledRFalse
				&& aux.i >= fractal->transformCommon.startIterationsR
				&& aux.i < fractal->transformCommon.stopIterationsR)
		{
			z = fractal->transformCommon.rotationMatrix2.RotateVector(z);
		}
	}
}

/**
 * Menger Cross Mod1
 * from code by Knighty
 * http://www.fractalforums.com/fragmentarium/
 * cross-menger!-can-anyone-do-this/msg93972/#new
 */
void MengerCrossMod1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 gap = fractal->transformCommon.constantMultiplier000;

	if (fractal->transformCommon.functionEnabledx && aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations1)
	{
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		if (fractal->transformCommon.functionEnabledFFalse) z.x = fabs(z.x);
		double dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
		double t = max(0.0, dot1);
		z.x -= t * -SQRT_3 - (0.5 * SQRT_3_4);

		z.y = fabs(z.y - t);

		if (z.y > z.z) swap(z.y, z.z);
		z.y -= 1.5;
		z -= gap * CVector4(SQRT_3_4, -1.5, 1.5, 0.0);

		if (z.z > z.x) swap(z.z, z.x);
		if (fractal->transformCommon.functionEnabledyFalse)
		{
			if (z.x >= 0.0)
			{
				z.y = max(0.0, z.y) * fractal->transformCommon.scaleA1;
				z.z = max(0.0, z.z) * fractal->transformCommon.scaleB1;
			}
		}
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
		aux.DE *= fractal->analyticDE.scale1; // tweak
	}

	if (fractal->transformCommon.functionEnabledy
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{ // CrossMengerTrick
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		if (fractal->transformCommon.functionEnabledzFalse) z.x = fabs(z.x);

		double dot1 = (z.x * -SQRT_3_4 + z.y * 0.5);
		double t = 1.0 * max(0.0, dot1);
		z.x -= t * -SQRT_3;
		if (fractal->transformCommon.functionEnabledXFalse)
			z.y = fabs(z.y) - t;
		else
		{
			z.y = fabs(z.y - t);
		}
		z.x -= SQRT_3_4;

		// Choose nearest corner/edge to get translation symmetry (all y & z code)
		double dy = 0.0;
		double dz = 0.0;
		if (z.y > 0.5 && z.z > 0.5) // if both y & z > 0.5  then =1.5
		{
			dy = 1.5;
			dz = 1.5;
		}
		else if (z.z < z.y)
		{
			dy = 1.5; // and dz is unchanged
		}
		else
			dz = 1.5; // and dy is unchanged

		z.y -= dy;
		z.z -= dz;
		z *= fractal->transformCommon.scale3;
		aux.DE *= fractal->transformCommon.scale3;
		z.y += dy;
		z.z += dz;

		z.x += SQRT_3_4;

		if (fractal->transformCommon.functionEnabledPFalse)
		{
			z.x = fabs(z.x + fractal->transformCommon.offset) + fractal->transformCommon.offset0;
		}
		if (fractal->transformCommon.functionEnabledRFalse
				&& aux.i >= fractal->transformCommon.startIterationsR
				&& aux.i < fractal->transformCommon.stopIterationsR)
		{
			z = fractal->transformCommon.rotationMatrix2.RotateVector(z);
		}
	}
}

/**
 * Menger Sponge formula modified by Mclarekin
 * from code by Knighty
 */
void MengerMod1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	z = fabs(z);
	if (z.x - z.y < 0.0) swap(z.y, z.x);
	if (z.x - z.z < 0.0) swap(z.z, z.x);
	if (z.y - z.z < 0.0) swap(z.z, z.y);
	z *= fractal->transformCommon.scale3;
	z.x -= 2.0 * fractal->transformCommon.constantMultiplier111.x;
	z.y -= 2.0 * fractal->transformCommon.constantMultiplier111.y;
	if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplier111.z;

	aux.DE *= fractal->transformCommon.scale3;

	if (fractal->transformCommon.rotationEnabled && aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}
	z += fractal->transformCommon.additionConstant000;
	if (fractal->transformCommon.functionEnabledxFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA) // box offset
	{
		CVector4 temp = z;
		z.x = sign(z.x) * fractal->transformCommon.additionConstantA000.x + z.x;
		z.y = sign(z.y) * fractal->transformCommon.additionConstantA000.y + z.y;
		z.z = sign(z.z) * fractal->transformCommon.additionConstantA000.z + z.z;

		if (fractal->transformCommon.functionEnabledFalse)
		{
			double tempL = temp.Length();
			// if (tempL < 1e-21) tempL = 1e-21;
			double avgScale = z.Length() / tempL;
			aux.r_dz *= avgScale;
			aux.DE = aux.DE * avgScale + 1.0;
		}
	}

	if (fractal->transformCommon.functionEnabledFalse)
	{
		CVector4 zA = (aux.i == fractal->transformCommon.intA) ? z : CVector4();
		CVector4 zB = (aux.i == fractal->transformCommon.intB) ? z : CVector4();

		z = (z * fractal->transformCommon.scale1) + (zA * fractal->transformCommon.offset)
				+ (zB * fractal->transformCommon.offset0);
		aux.DE *= fractal->transformCommon.scale1;
	}
}

/**
 * Menger Middle Mod
 */
void MengerMiddleModIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{ // fabs() and menger fold
	z = fabs(z + fractal->transformCommon.additionConstantA000);
	if (z.x - z.y < 0.0) swap(z.y, z.x);
	if (z.x - z.z < 0.0) swap(z.z, z.x);
	if (z.y - z.z < 0.0) swap(z.z, z.y);

	if (fractal->mandelbox.mainRotationEnabled && aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC) // rotation
	{
		z = fractal->mandelbox.mainRot.RotateVector(z);
	}

	if (fractal->transformCommon.functionEnabledBxFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA) // box fold
	{
		if (fabs(z.x) > fractal->mandelbox.foldingLimit)
		{
			z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
			aux.color += fractal->mandelbox.color.factor.x;
		}
		if (fabs(z.y) > fractal->mandelbox.foldingLimit)
		{
			z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
			aux.color += fractal->mandelbox.color.factor.y;
		}
		double zLimit = fractal->mandelbox.foldingLimit * fractal->transformCommon.scale1;
		double zValue = fractal->mandelbox.foldingValue * fractal->transformCommon.scale1;
		if (fabs(z.z) > zLimit)
		{
			z.z = sign(z.z) * zValue - z.z;
			aux.color += fractal->mandelbox.color.factor.z;
		}
	}
	if (fractal->transformCommon.functionEnabledByFalse
			&& aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{ // fabsBoxFold
		CVector4 tempA, tempB;

		if (fractal->transformCommon.functionEnabledx)
			tempA.x = fabs(z.x + fractal->transformCommon.additionConstant111.x);
		if (fractal->transformCommon.functionEnabledAx)
			tempB.x = fabs(z.x - fractal->transformCommon.additionConstantA111.x);
		z.x = tempA.x - tempB.x - (z.x * fractal->transformCommon.scale3D111.x);

		if (fractal->transformCommon.functionEnabledy)
			tempA.y = fabs(z.y + fractal->transformCommon.additionConstant111.y);
		if (fractal->transformCommon.functionEnabledAy)
			tempB.y = fabs(z.y - fractal->transformCommon.additionConstantA111.y);
		z.y = tempA.y - tempB.y - (z.y * fractal->transformCommon.scale3D111.y);

		if (fractal->transformCommon.functionEnabledz)
			tempA.z = fabs(z.z + fractal->transformCommon.additionConstant111.z);
		if (fractal->transformCommon.functionEnabledAz)
			tempB.z = fabs(z.z - fractal->transformCommon.additionConstantA111.z);
		z.z = tempA.z - tempB.z - (z.z * fractal->transformCommon.scale3D111.z);

		if (aux.i >= fractal->transformCommon.startIterationsB
				&& aux.i < fractal->transformCommon.stopIterationsB)
		{
			z *= fractal->transformCommon.scaleA1;
			aux.DE *= fabs(fractal->transformCommon.scaleA1);
		}
	}
	// menger scales and offsets
	z *= fractal->transformCommon.scale3;
	z.x -= 2.0 * fractal->transformCommon.constantMultiplier111.x;
	z.y -= 2.0 * fractal->transformCommon.constantMultiplier111.y;
	if (fractal->transformCommon.functionEnabled)
	{
		if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplier111.z;
	}
	else
	{
		z.z -= 2.0 * fractal->transformCommon.constantMultiplier111.z;
	}
	aux.DE *= fractal->transformCommon.scale3;

	if (fractal->transformCommon.addCpixelEnabledFalse) // addCpixel options
	{
		switch (fractal->mandelbulbMulti.orderOfXYZC)
		{
			case multi_OrderOfXYZ_xyz:
			default: aux.c = CVector4(aux.c.x, aux.c.y, aux.c.z, aux.c.w); break;
			case multi_OrderOfXYZ_xzy: aux.c = CVector4(aux.c.x, aux.c.z, aux.c.y, aux.c.w); break;
			case multi_OrderOfXYZ_yxz: aux.c = CVector4(aux.c.y, aux.c.x, aux.c.z, aux.c.w); break;
			case multi_OrderOfXYZ_yzx: aux.c = CVector4(aux.c.y, aux.c.z, aux.c.x, aux.c.w); break;
			case multi_OrderOfXYZ_zxy: aux.c = CVector4(aux.c.z, aux.c.x, aux.c.y, aux.c.w); break;
			case multi_OrderOfXYZ_zyx: aux.c = CVector4(aux.c.z, aux.c.y, aux.c.x, aux.c.w); break;
		}
		z += aux.c * fractal->transformCommon.constantMultiplierC111;
	}
}

/**
 * Menger Sponge and octo
 * from code by Knighty
 */
void MengerOctoIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{ // octo
	if (aux.i >= fractal->transformCommon.startIterationsE
			&& aux.i < fractal->transformCommon.stopIterationsE)
	{
		if (z.x + z.y < 0.0) z = CVector4(-z.y, -z.x, z.z, z.w);

		if (z.x + z.z < 0.0) // z.xz = -z.zx;
			z = CVector4(-z.z, z.y, -z.x, z.w);

		if (z.x - z.y < 0.0) // z.xy = z.yx;
			z = CVector4(z.y, z.x, z.z, z.w);

		if (z.x - z.z < 0.0) // z.xz = z.zx;
			z = CVector4(z.z, z.y, z.x, z.w);

		z.x = fabs(z.x);
		z = z * fractal->transformCommon.scale2
				- fractal->transformCommon.offset100 * (fractal->transformCommon.scale2 - 1.0);

		aux.DE *= fractal->transformCommon.scale2;
	}
	// box offset
	if (fractal->transformCommon.functionEnabledxFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		CVector4 temp = z;
		z.x = sign(z.x) * fractal->transformCommon.additionConstantA000.x + z.x;
		z.y = sign(z.y) * fractal->transformCommon.additionConstantA000.y + z.y;
		z.z = sign(z.z) * fractal->transformCommon.additionConstantA000.z + z.z;

		if (fractal->transformCommon.functionEnabledzFalse)
		{
			double tempL = temp.Length();
			// if (tempL < 1e-21) tempL = 1e-21;
			double avgScale = z.Length() / tempL;
			aux.DE = aux.DE * avgScale + 1.0;
		}
	}
	// spherical fold
	if (fractal->transformCommon.functionEnabledSFalse
			&& aux.i >= fractal->transformCommon.startIterationsS
			&& aux.i < fractal->transformCommon.stopIterationsS)
	{
		double para = 0.0;
		double paraAddP0 = 0.0;
		if (fractal->transformCommon.functionEnabledyFalse)
		{
			// para += paraAddP0 + fractal->transformCommon.minR2p25;
			if (fractal->Cpara.enabledLinear)
			{
				para = fractal->Cpara.para00; // parameter value at iter 0
				double temp0 = para;
				double tempA = fractal->Cpara.paraA;
				double tempB = fractal->Cpara.paraB;
				double tempC = fractal->Cpara.paraC;
				double lengthAB = fractal->Cpara.iterB - fractal->Cpara.iterA;
				double lengthBC = fractal->Cpara.iterC - fractal->Cpara.iterB;
				double grade1 = (tempA - temp0) / fractal->Cpara.iterA;
				double grade2 = (tempB - tempA) / lengthAB;
				double grade3 = (tempC - tempB) / lengthBC;

				// slopes
				if (aux.i < fractal->Cpara.iterA)
				{
					para = temp0 + (aux.i * grade1);
				}
				if (aux.i < fractal->Cpara.iterB && aux.i >= fractal->Cpara.iterA)
				{
					para = tempA + (aux.i - fractal->Cpara.iterA) * grade2;
				}
				if (aux.i >= fractal->Cpara.iterB)
				{
					para = tempB + (aux.i - fractal->Cpara.iterB) * grade3;
				}

				// Curvi part on "true"
				if (fractal->Cpara.enabledCurves)
				{
					double paraAdd = 0.0;
					double paraIt;
					if (lengthAB > 2.0 * fractal->Cpara.iterA) // stop  error, todo fix.
					{
						double curve1 = (grade2 - grade1) / (4.0 * fractal->Cpara.iterA);
						double tempL = lengthAB - fractal->Cpara.iterA;
						double curve2 = (grade3 - grade2) / (4.0 * tempL);
						if (aux.i < 2 * fractal->Cpara.iterA)
						{
							paraIt = tempA - fabs(tempA - aux.i);
							paraAdd = paraIt * paraIt * curve1;
						}
						if (aux.i >= 2 * fractal->Cpara.iterA && aux.i < fractal->Cpara.iterB + tempL)
						{
							paraIt = tempB - fabs(tempB * aux.i);
							paraAdd = paraIt * paraIt * curve2;
						}
						para += paraAdd;
					}
				}
			}
			paraAddP0 = 0.0;
			if (fractal->Cpara.enabledParabFalse)
			{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
				paraAddP0 = fractal->Cpara.parabOffset0 + (aux.i * fractal->Cpara.parabSlope)
										+ (aux.i * aux.i * 0.001 * fractal->Cpara.parabScale);
			}
		}
		para += paraAddP0 + fractal->transformCommon.minR2p25;
		// spherical fold
		double rr = z.Dot(z);

		z += fractal->mandelbox.offset;

		// if (r2 < 1e-21) r2 = 1e-21;
		if (rr < para)
		{
			double tglad_factor1 = fractal->transformCommon.maxR2d1 / para;
			z *= tglad_factor1;
			aux.DE *= tglad_factor1;
			aux.color += fractal->mandelbox.color.factorSp1;
		}
		else if (rr < fractal->transformCommon.maxR2d1)
		{
			double tglad_factor2 = fractal->transformCommon.maxR2d1 / rr;
			z *= tglad_factor2;
			aux.DE *= tglad_factor2;
			aux.color += fractal->mandelbox.color.factorSp2;
		}
		z -= fractal->mandelbox.offset;
		z *= fractal->transformCommon.scale08;
		aux.DE = aux.DE * fabs(fractal->transformCommon.scale08);
	}
	// rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}
	// menger
	if (fractal->transformCommon.functionEnabledM
			&& aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		z = fabs(z);
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0 * fractal->transformCommon.constantMultiplier111.x;
		z.y -= 2.0 * fractal->transformCommon.constantMultiplier111.y;
		if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplier111.z;
		aux.DE *= fractal->transformCommon.scale3;
		z += fractal->transformCommon.additionConstant000;
	}
	// iter weight
	if (fractal->transformCommon.functionEnabledFalse)
	{
		CVector4 zA = (aux.i == fractal->transformCommon.intA) ? z : CVector4();
		CVector4 zB = (aux.i == fractal->transformCommon.intB) ? z : CVector4();

		z = (z * fractal->transformCommon.scale1) + (zA * fractal->transformCommon.offsetA0)
				+ (zB * fractal->transformCommon.offsetB0);
		aux.DE *= fractal->transformCommon.scale1;
		aux.r_dz *= fractal->transformCommon.scale1;
	}
}

/**
 * Menger Sponge Polynomial Hybrid modified by Mclarekin
 */
void MengerPwr2PolyIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	if (aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations1)
	{
		CVector4 partA = z;
		if (fractal->transformCommon.functionEnabledFalse) // fabs
			partA = fabs(z);
		if (fractal->transformCommon.functionEnabledxFalse) // pwr3 or z * fabs(z^2)
			partA *= z;
		partA =
			partA * fractal->transformCommon.scale2 + fractal->transformCommon.constantMultiplier111;

		CVector4 fnZ1 = z;

		if (fractal->transformCommon.functionEnabledBxFalse) // cos(z*Pi)
		{
			double scalePi = M_PI * fractal->transformCommon.scaleC1;
			fnZ1.x = cos(z.x * scalePi);
			fnZ1.y = cos(z.y * scalePi);
			fnZ1.z = cos(z.z * scalePi);
		}

		if (fractal->transformCommon.functionEnabledyFalse) // pi rotation
			fnZ1 = fnZ1.RotateAroundVectorByAngle(fractal->transformCommon.constantMultiplier111.GetXYZ(),
				M_PI * fractal->transformCommon.scaleB1);

		if (fractal->transformCommon.functionEnabledzFalse) // box offset
		{
			fnZ1.x = (sign(fnZ1.x) * fractal->transformCommon.additionConstant000.x) + fnZ1.x;
			fnZ1.y = (sign(fnZ1.y) * fractal->transformCommon.additionConstant000.y) + fnZ1.y;
			fnZ1.z = (sign(fnZ1.z) * fractal->transformCommon.additionConstant000.z) + fnZ1.z;
		}

		if (fractal->transformCommon.functionEnabledAxFalse) // fabs(fnZ1)
			fnZ1 = fabs(fnZ1);
		CVector4 partB = z;
		if (fractal->transformCommon.functionEnabledAzFalse) partB = fabs(z);
		partB *= fractal->transformCommon.scale4;
		z = fractal->transformCommon.constantMultiplierB111 + partB - partA * fnZ1;
		z *= fractal->transformCommon.scale025;
		aux.DE = aux.DE * 4.0 * fractal->analyticDE.scaleLin + fractal->analyticDE.offsetLin;
	}
	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplierC111;
	}

	int count = fractal->transformCommon.int1; // Menger Sponge
	int k;
	for (k = 0; k < count; k++)
	{
		z = fabs(z);
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0 * fractal->transformCommon.constantMultiplierA111.x;
		z.y -= 2.0 * fractal->transformCommon.constantMultiplierA111.y;
		if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplierA111.z;

		aux.DE *= fractal->transformCommon.scale3;

		if (fractal->transformCommon.rotationEnabled
				&& aux.i >= fractal->transformCommon.startIterationsA
				&& aux.i < fractal->transformCommon.stopIterationsA) // rotation
		{
			z = fractal->transformCommon.rotationMatrix.RotateVector(z);
		}
		z += fractal->transformCommon.additionConstantA000;
	}
}

/**
 * Menger Prism Shape
 * from code by Knighty
 * http://www.fractalforums.com/fragmentarium/cross-menger!-can-anyone-do-this/msg93972/#new
 */
void MengerPrismShapeIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 gap = fractal->transformCommon.constantMultiplier000;
	double t;
	double dot1;

	if (aux.i >= fractal->transformCommon.startIterationsP
			&& aux.i < fractal->transformCommon.stopIterationsP1)
	{
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
		t = max(0.0, dot1);
		z.x -= t * -SQRT_3;
		z.y = fabs(z.y - t);

		if (z.y > z.z) swap(z.y, z.z);
		z -= gap * CVector4(SQRT_3_4, 1.5, 1.5, 0.0);
		// z was pos, now some points neg (ie neg shift)
		if (z.z > z.x) swap(z.z, z.x);
		if (z.x > 0.0)
		{
			z.y = max(0.0, z.y);
			z.z = max(0.0, z.z);
		}
	}

	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	if (fractal->transformCommon.benesiT1EnabledFalse
			&& aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterationsT1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D222;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
		z = z - fractal->transformCommon.offset200;
	}

	if (fractal->transformCommon.functionEnabledxFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsTM1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D333;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		z = (fabs(z + fractal->transformCommon.additionConstant111)
				 - fabs(z - fractal->transformCommon.additionConstant111) - z);

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
	}
	if (fractal->transformCommon.functionEnabledFFalse
			&& aux.i >= fractal->transformCommon.startIterationsF
			&& aux.i < fractal->transformCommon.stopIterationsF)
	{
		CVector4 tempA, tempB;

		if (fractal->transformCommon.functionEnabledAx)
			tempA.x = fabs(z.x + fractal->transformCommon.offsetF000.x);

		if (fractal->transformCommon.functionEnabledx)
			tempB.x = fabs(z.x - fractal->transformCommon.offset000.x);

		z.x = tempA.x - tempB.x - (z.x * fractal->transformCommon.scale3D111.x);

		if (fractal->transformCommon.functionEnabledAy)
			tempA.y = fabs(z.y + fractal->transformCommon.offsetF000.y);

		if (fractal->transformCommon.functionEnabledy)
			tempB.y = fabs(z.y - fractal->transformCommon.offset000.y);

		z.y = tempA.y - tempB.y - (z.y * fractal->transformCommon.scale3D111.y);

		if (fractal->transformCommon.functionEnabledAz)
			tempA.z = fabs(z.z + fractal->transformCommon.offsetF000.z);

		if (fractal->transformCommon.functionEnabledz)
			tempB.z = fabs(z.z - fractal->transformCommon.offset000.z);

		z.z = tempA.z - tempB.z - (z.z * fractal->transformCommon.scale3D111.z);

		z += fractal->transformCommon.offsetA000;
	}
	if (fractal->transformCommon.functionEnabled && aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		z = fabs(z);
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0 * fractal->transformCommon.constantMultiplierA111.x;
		z.y -= 2.0 * fractal->transformCommon.constantMultiplierA111.y;
		if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplierA111.z;
		aux.DE *= fractal->transformCommon.scale3 * fractal->transformCommon.scaleA1;

		z += fractal->transformCommon.additionConstantA000;
	}

	aux.DE *= fractal->transformCommon.scaleB1;
}

/**
 * Menger Prism Shape2
 * from code by Knighty
 * http://www.fractalforums.com/fragmentarium/
 * cross-menger!-can-anyone-do-this/msg93972/#new
 */
void MengerPrismShape2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledSwFalse)
	{
		z = CVector4(-z.z, z.x, z.y, z.w);
	}
	CVector4 gap = fractal->transformCommon.constantMultiplier000;

	if (fractal->transformCommon.functionEnabledx
			&& aux.i >= fractal->transformCommon.startIterationsP
			&& aux.i < fractal->transformCommon.stopIterationsP1)
	{
		double t;
		double dot1;
		if (fractal->transformCommon.functionEnabledCxFalse)
		{
			z.x = fabs(z.x);
		}
		if (fractal->transformCommon.functionEnabledCy)
		{
			z.y = fabs(z.y);
		}
		if (fractal->transformCommon.functionEnabledCz)
		{
			z.z = fabs(z.z);
		}
		double tempOff = (fractal->transformCommon.offset0 + SQRT_3_4d2);
		switch (fractal->combo.modeA)
		{
			case combo_mode0:
			default:
				dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
				t = max(0.0, dot1);
				z.x -= t * -SQRT_3 - tempOff;
				z.y = fabs(z.y - t);
				break;
			case combo_mode1:
				dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
				t = max(0.0, dot1);
				z.y -= t * -SQRT_3 - tempOff;
				z.x = fabs(z.x - t); // x y swap
				break;
			case combo_mode2:
				dot1 = (z.z * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
				t = max(0.0, dot1);
				z.z -= t * -SQRT_3 - tempOff;
				z.y = fabs(z.y - t); // z y swap
				break;
			case combo_mode3:
				dot1 = (z.z * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
				t = max(0.0, dot1);
				z.y -= t * -SQRT_3 - tempOff;
				z.z = fabs(z.z - t); // z y swap
				break;
			case combo_mode4:
				dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
				t = max(0.0, dot1);
				z.y -= t * -SQRT_3 - tempOff;
				z.z -= t * -SQRT_3 - tempOff;
				z.x = fabs(z.x - t); // x y swap and other things
				z.y = fabs(z.y - t);
				break;
			case combo_mode5:
				dot1 = (z.z * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
				t = max(0.0, dot1);
				z.x -= t * -SQRT_3 - tempOff;
				z.y -= t * -SQRT_3 - tempOff;
				z.y = fabs(z.y - t); // x y swap and other things
				z.z = fabs(z.z - t);
				break;
			case combo_mode6:
				dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
				t = max(0.0, dot1);
				z.y -= t * -SQRT_3 - (fractal->transformCommon.offset0);
				z.z -= t * -SQRT_3 - (fractal->transformCommon.offset0);
				z.x = fabs(z.y - t); // x y swap and other things and swizzle
				z.y = fabs(z.x - t);
				break;
			case combo_mode7:
				dot1 = (z.z * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
				t = max(0.0, dot1);
				z.x -= t * -SQRT_3 - (fractal->transformCommon.offset0);
				z.y -= t * -SQRT_3 - (fractal->transformCommon.offset0);
				z.y = fabs(z.z - t); // x y swap and other things and swizzle
				z.z = fabs(z.y - t);
				break;
		}

		if (z.y > z.z) swap(z.y, z.z);
		z.y -= fractal->transformCommon.offsetB0;

		z -= gap * CVector4(SQRT_3_4, 1.5, 1.5, 0.0);

		if (z.z > z.x) swap(z.z, z.x);

		if (fractal->transformCommon.functionEnabledyFalse)
		{
			z.y = max(0.0, z.y) * fractal->transformCommon.scaleA1;
			z.z = max(0.0, z.z) * fractal->transformCommon.scaleB1;
		}
		if (fractal->transformCommon.functionEnabledzFalse)
		{
			if (z.x >= 0.0)
			{
				z.y = max(0.0, z.y) * fractal->transformCommon.scaleA1;
				z.z = max(0.0, z.z) * fractal->transformCommon.scaleB1;
			}
		}
		z *= fractal->transformCommon.scale1;
		aux.DE *= fractal->transformCommon.scale1;
		aux.DE *= fractal->transformCommon.scaleC1;
	}

	if (fractal->transformCommon.functionEnabledXFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)

	{ // CrossMengerTrick
		double dd = aux.DE;
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		if (fractal->transformCommon.functionEnabledCyFalse)
		{
			z.x = fabs(z.x);
		}
		double dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scaleD1;
		double t = max(0.0, dot1);
		z.x -= t * -SQRT_3;
		if (fractal->transformCommon.functionEnabledBzFalse)
		{
			z.y = fabs(z.y) - t;
		}
		else
		{
			z.y = fabs(z.y - t);
		}
		z.x -= SQRT_3_4;
		double dy = 0.0;
		double dz = 0.0;
		if (z.y > 0.5 && z.z > 0.5)
		{
			dy = 1.5;
			dz = 1.5;
		}
		else if (z.z < z.y)
		// else if ((z.y - 1.5) * (z.y - 1.5) + z.z * z.z < z.y * z.y + (z.z - 1.5) * (z.z - 1.5))
		{
			dy = 1.5;
		}
		else
			dz = 1.5;

		z.y -= dy;
		z.z -= dz;
		z *= fractal->transformCommon.scaleA3;
		aux.DE *= fractal->transformCommon.scaleA3;
		z.y += dy;
		z.z += dz;
		z.x += SQRT_3_4;

		if (fractal->transformCommon.functionEnabledFalse)
		{
			dd *= FRAC_1_3 * fractal->transformCommon.scaleE1;
			z *= dd;
			aux.DE *= dd;
		}
		aux.DE *= fractal->transformCommon.offset1; //.DE tweak cross menger trick part
	}
	if (fractal->transformCommon.functionEnabledPFalse)
	{
		z.x = fabs(z.x + fractal->transformCommon.offset) + fractal->transformCommon.offsetC0;
	}

	// void KIFS(vec3 z)
	//{//Pure KIFS... almost correct

	if (fractal->transformCommon.functionEnabledKFalse
			&& aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB)
	{
		if (fractal->transformCommon.functionEnabledCzFalse)
		{
			z.x = fabs(fractal->transformCommon.offset - z.x) + fractal->transformCommon.offsetC0;
		}

		z.y = fabs(z.y);
		z.z = fabs(z.z);

		double dot2 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scaleF1;
		double t = max(0.0, dot2);

		z.x -= t * -SQRT_3;
		z.y = fabs(z.y - t);

		if (z.y > z.z) swap(z.y, z.z);

		z.y = fabs(z.y - 0.5) + 0.5;

		z -= CVector4(0.5 * SQRT_3, 1.5, 1.5, 0.0);

		z *= fractal->transformCommon.scaleB3;
		aux.DE *= fractal->transformCommon.scaleB3;

		z += CVector4(0.5 * SQRT_3, 1.5, 1.5, 0.0);
	}

	if (fractal->transformCommon.benesiT1EnabledFalse
			&& aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterationsT1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 tempZ = z;
		double tempL = tempZ.Length();
		z = fabs(z) * fractal->transformCommon.scale3D222;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
		z = z - fractal->transformCommon.offset200;
	}

	if (fractal->transformCommon.benesiT1MEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsTM1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D333;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		z = (fabs(z + fractal->transformCommon.additionConstant111)
				 - fabs(z - fractal->transformCommon.additionConstant111) - z);

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
	}

	if (fractal->transformCommon.functionEnabledFFalse
			&& aux.i >= fractal->transformCommon.startIterationsF
			&& aux.i < fractal->transformCommon.stopIterationsF)
	{
		CVector4 tempA, tempB;

		if (fractal->transformCommon.functionEnabledAx)
			tempA.x = fabs(z.x + fractal->transformCommon.offsetF000.x);

		if (fractal->transformCommon.functionEnabledx)
			tempB.x = fabs(z.x - fractal->transformCommon.offset000.x);

		z.x = tempA.x - tempB.x - (z.x * fractal->transformCommon.scale3D111.x);

		if (fractal->transformCommon.functionEnabledAy)
			tempA.y = fabs(z.y + fractal->transformCommon.offsetF000.y);

		if (fractal->transformCommon.functionEnabledy)
			tempB.y = fabs(z.y - fractal->transformCommon.offset000.y);

		z.y = tempA.y - tempB.y - (z.y * fractal->transformCommon.scale3D111.y);

		if (fractal->transformCommon.functionEnabledAz)
			tempA.z = fabs(z.z + fractal->transformCommon.offsetF000.z);

		if (fractal->transformCommon.functionEnabledz)
			tempB.z = fabs(z.z - fractal->transformCommon.offset000.z);

		z.z = tempA.z - tempB.z - (z.z * fractal->transformCommon.scale3D111.z);

		z += fractal->transformCommon.offsetA000;
	}

	// rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	// menger
	if (fractal->transformCommon.functionEnabledM
			&& aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		z = fabs(z);
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0 * fractal->transformCommon.constantMultiplierA111.x;
		z.y -= 2.0 * fractal->transformCommon.constantMultiplierA111.y;
		if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplierA111.z;
		aux.DE *= fractal->transformCommon.scale3;

		z += fractal->transformCommon.additionConstantA000;
	}
	aux.DE *= fractal->analyticDE.scale1;
}

/**
 * Menger Smooth
 * http://www.fractalforums.com/fragmentarium/help-t22583/
 */
void MengerSmoothIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double sc1 = fractal->transformCommon.scale3 - 1.0;		// 3 - 1 = 2, 2/3 = 0.6667;
	double sc2 = sc1 / fractal->transformCommon.scale3;		//  8 - 1 = 7, 7/8 = 0.89ish;
	double OffsetS = fractal->transformCommon.offset0005; //

	if (fractal->transformCommon.functionEnabled)
	{
		// the closer to origin the greater the effect of OffsetSQ
		z = CVector4(
			sqrt(z.x * z.x + OffsetS), sqrt(z.y * z.y + OffsetS), sqrt(z.z * z.z + OffsetS), z.w);
	}

	double t;
	CVector4 OffsetC = fractal->transformCommon.offset1105;

	t = z.x - z.y;
	t = 0.5 * (t - sqrt(t * t + OffsetS));
	z.x = z.x - t;
	z.y = z.y + t;

	t = z.x - z.z;
	t = 0.5 * (t - sqrt(t * t + OffsetS));
	z.x = z.x - t;
	z.z = z.z + t;

	t = z.y - z.z;
	t = 0.5 * (t - sqrt(t * t + OffsetS));
	z.y = z.y - t;
	z.z = z.z + t;

	z.z = z.z - OffsetC.z * sc2; // sc2 reduces C.z
	z.z = -sqrt(z.z * z.z + OffsetS);
	z.z = z.z + OffsetC.z * sc2;

	z.x = fractal->transformCommon.scale3 * z.x - OffsetC.x * sc1; // sc1 scales up C.x
	z.y = fractal->transformCommon.scale3 * z.y - OffsetC.y * sc1;
	z.z = fractal->transformCommon.scale3 * z.z;

	aux.DE *= fractal->transformCommon.scale3;

	if (fractal->transformCommon.rotationEnabled && aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}

	if (fractal->transformCommon.functionEnabledzFalse)
	{
		CVector4 zA = (aux.i == fractal->transformCommon.intA) ? z : CVector4();
		CVector4 zB = (aux.i == fractal->transformCommon.intB) ? z : CVector4();
		z = (z * fractal->transformCommon.scale1) + (zA * fractal->transformCommon.offsetA0)
				+ (zB * fractal->transformCommon.offsetB0);
		aux.DE *= fractal->transformCommon.scale1;
	}
}

/**
 * Menger Smooth Mod1, based on :
 * http://www.fractalforums.com/fragmentarium/help-t22583/
 */
void MengerSmoothMod1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{

	if (fractal->transformCommon.functionEnabled)
	{
		z = CVector4(sqrt(z.x * z.x + fractal->transformCommon.offset0),
			sqrt(z.y * z.y + fractal->transformCommon.offset0),
			sqrt(z.z * z.z + fractal->transformCommon.offset0), z.w);
	}
	if (fractal->transformCommon.functionEnabledFFalse)
	{
		z = fabs(z);
		double s = fractal->transformCommon.offset;
		z += CVector4(s, s, s, 0.0);
	}

	double t;
	double ScaleP5 = fractal->transformCommon.scale05;
	CVector4 OffsetC = fractal->transformCommon.constantMultiplier221;
	double OffsetS = fractal->transformCommon.offset0005;

	t = z.x - z.y;
	t = ScaleP5 * (t - sqrt(t * t + OffsetS * fractal->transformCommon.constantMultiplier111.x));
	z.x = z.x - t;
	z.y = z.y + t;

	t = z.x - z.z;
	t = ScaleP5 * (t - sqrt(t * t + OffsetS * fractal->transformCommon.constantMultiplier111.y));
	z.x = z.x - t;
	z.z = z.z + t;

	t = z.y - z.z;
	t = ScaleP5 * (t - sqrt(t * t + OffsetS * fractal->transformCommon.constantMultiplier111.z));
	z.y = z.y - t;
	z.z = z.z + t;

	z.z = z.z - OffsetC.z / 3.0;
	z.z = -sqrt(z.z * z.z + OffsetS);
	z.z = z.z + OffsetC.z / 3.0;

	z.x = fractal->transformCommon.scale3 * z.x - OffsetC.x;
	z.y = fractal->transformCommon.scale3 * z.y - OffsetC.y;
	z.z = fractal->transformCommon.scale3 * z.z;

	aux.DE *= fractal->transformCommon.scale3;

	if (fractal->transformCommon.rotationEnabled && aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}

	if (fractal->transformCommon.functionEnabledxFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA) // box offset
	{
		z.x = sign(z.x) * fractal->transformCommon.additionConstantA000.x + z.x;
		z.y = sign(z.y) * fractal->transformCommon.additionConstantA000.y + z.y;
		z.z = sign(z.z) * fractal->transformCommon.additionConstantA000.z + z.z;
	}

	if (fractal->transformCommon.functionEnabledzFalse)
	{
		CVector4 zA = (aux.i == fractal->transformCommon.intA) ? z : CVector4();
		CVector4 zB = (aux.i == fractal->transformCommon.intB) ? z : CVector4();

		z = (z * fractal->transformCommon.scale1) + (zA * fractal->transformCommon.offsetA0)
				+ (zB * fractal->transformCommon.offsetB0);
		aux.DE *= fractal->transformCommon.scale1;
	}
}

/**
 * Msltoe Donut formula
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * low-hanging-dessert-an-escape-time-donut-fractal/msg90171/#msg90171
 */
void MsltoeDonutIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double rad2 = fractal->donut.ringThickness;
	double nSect = 2.0 * M_PI / fractal->donut.number;
	double fact = fractal->donut.factor;

	double R = sqrt(z.x * z.x + z.y * z.y);
	double R2 = fractal->donut.ringRadius - R;
	double t = R2 * R2 + z.z * z.z - rad2 * rad2;

	double theta = atan2(z.y, z.x);
	double theta2 = nSect * round(theta / nSect);

	if (t > 0.03)
	{
		double c1 = cos(theta2);
		double s1 = sin(theta2);

		double x1 = c1 * z.x + s1 * z.y;
		double y1 = -s1 * z.x + c1 * z.y;
		double z1 = z.z;

		x1 = x1 - fractal->donut.ringRadius;

		z.x = fact * x1;
		z.y = fact * z1;
		z.z = fact * y1;
	}
	else
	{
		z /= t;
	}
	aux.color += theta2;
}

/**
 * MsltoeSym2Mod based on the formula from Mandelbulb3D
 * @reference http://www.fractalforums.com/theory/choosing-the-squaring-formula-by-location/15/
 */
void MsltoeSym2ModIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	CVector4 temp = z;

	if (fabs(z.y) < fabs(z.z)) // then swap
	{
		z.y = temp.z; // making z.y furthest away from axis
		z.z = temp.y;
	}
	if (z.y > z.z) // then change sign of z.x and z.z
	{
		z.x = -z.x;
	}

	CVector4 z2 = z * z;							// squares
	double v3 = (z2.x + z2.y + z2.z); // sum of squares
	// if (v3 < 1e-21 && v3 > -1e-21)
	//	v3 = (v3 > 0) ? 1e-21 : -1e-21;
	double zr = 1.0 - z2.z / v3;
	temp.x = (z2.x - z2.y) * zr;
	temp.y = 2.0 * z.x * z.y * zr * fractal->transformCommon.scale; // scaling temp.y
	temp.z = 2.0 * z.z * sqrt(z2.x + z2.y);
	z = temp + fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.constantMultiplier000;

		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}

	double lengthTempZ = -z.Length();
	// if (lengthTempZ > -1e-21)
	//	lengthTempZ = -1e-21;   //  z is neg.)
	z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
	z *= fractal->transformCommon.scale1;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale1) + 1.0;
	aux.r_dz *= fabs(fractal->transformCommon.scale1);
}

/**
 * MsltoeSym3Mod based on the formula from Mandelbulb3D
 * @reference http://www.fractalforums.com/theory/choosing-the-squaring-formula-by-location/15/
 */
void MsltoeSym3ModIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;
	aux.r = z.Length();
	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	CVector4 temp = z;
	if (fabs(z.y) < fabs(z.z)) // then swap
	{
		z.y = temp.z; // making z.y furthest away from axis
		z.z = temp.y;
	}
	if (z.y > z.z) // then change sign of z.x and z.z
	{
		z.x = -z.x;
		z.z = -z.z;
	}
	CVector4 z2 = z * z; // squares
	double v3 = (z2.x + z2.y + z2.z)
							+ fractal->transformCommon.scale0 * fractal->transformCommon.scale0 * z2.y * z2.z;
	; // sum of squares
	// if (v3 < 1e-21 && v3 > -1e-21) v3 = (v3 > 0) ? 1e-21 : -1e-21;
	double zr = 1.0 - z2.z / v3;
	temp.x = (z2.x - z2.y) * zr;
	temp.y = 2.0 * z.x * z.y * zr * fractal->transformCommon.scale; // scaling temp.y
	temp.z = 2.0 * z.z * sqrt(z2.x + z2.y);

	z = temp + fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.constantMultiplier000;
		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}
	double lengthTempZ = -z.Length();
	// if (lengthTempZ > -1e-21) lengthTempZ = -1e-21;   //  z is neg.)
	z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
	z *= fractal->transformCommon.scale1;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale1) + 1.0;
	aux.r_dz *= fabs(fractal->transformCommon.scale1);

	if (fractal->transformCommon.functionEnabledFalse // quaternion fold
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		aux.r_dz = aux.r_dz * 2.0 * z.Length();
		z = CVector4(z.x * z.x - z.y * z.y - z.z * z.z, z.x * z.y, z.x * z.z, z.w);
		if (fractal->transformCommon.functionEnabledAxFalse)
		{
			aux.r = z.Length();
			CVector4 temp2 = z;
			double tempL = temp2.Length();
			z *= CVector4(1.0, 2.0, 2.0, 1.0);
			// if (tempL < 1e-21) tempL = 1e-21;
			double avgScale = z.Length() / tempL;
			// aux.r_dz *= avgScale * fractal->transformCommon.scaleA1;
			double tempAux = aux.r_dz * avgScale;
			aux.r_dz = aux.r_dz + (tempAux - aux.r_dz) * fractal->transformCommon.scaleA1;
		}
		else
		{
			z *= CVector4(1.0, 2.0, 2.0, 1.0);
		}
	}
}

/**
 * MsltoeJuliaBulb Eiffie. Refer post by Eiffie    Reply #69 on: January 27, 2015
 * @reference http://www.fractalforums.com/theory/choosing-the-squaring-formula-by-location/60/
 */
void EiffieMsltoeIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;
	aux.r = z.Length();
	double psi = fabs(fmod(atan2(z.z, z.y) + M_PI + M_PI_8, M_PI_4) - M_PI_8);
	double lengthYZ = sqrt(z.y * z.y + z.z * z.z);

	z.y = cos(psi) * lengthYZ;
	z.z = sin(psi) * lengthYZ;
	aux.r_dz = aux.r_dz * 2.0 * aux.r;

	CVector4 z2 = z * z;
	double rr = z2.x + z2.y + z2.z;
	double m = 1.0 - z2.z / rr;
	CVector4 temp;
	temp.x = (z2.x - z2.y) * m;
	temp.y = 2.0 * z.x * z.y * m * fractal->transformCommon.scale; // scaling y;;
	temp.z = 2.0 * z.z * sqrt(z2.x + z2.y);
	temp.w = z.w;
	z = temp + fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.constantMultiplier000;
		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}
	double lengthTempZ = -z.Length();
	// if (lengthTempZ > -1e-21) lengthTempZ = -1e-21;   //  z is neg.)
	z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
	z *= fractal->transformCommon.scale1;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale1) + 1.0;
	// aux.r_dz *= fabs(fractal->transformCommon.scale1);

	if (fractal->analyticDE.enabledFalse)
	{ // analytic log DE adjustment
		aux.r_dz *= fabs(fractal->transformCommon.scale1) * fractal->analyticDE.scale1;
	}
	else
	{
		aux.r_dz *= fabs(fractal->transformCommon.scale1);
	}
}

/**
 * Msltoe_Julia_Bulb_Mod2
 * @reference
 * http://www.fractalforums.com/theory/choosing-the-squaring-formula-by-location/msg14198/#msg14198
 */
void MsltoeSym3Mod2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	aux.r_dz = aux.r_dz * 2.0 * aux.r;

	double theta;
	double phi;
	CVector4 z2 = z * z;
	double r = z2.x + z2.y + z2.z;
	// if (r < 1e-21)
	//	r = 1e-21;
	double r1 = sqrt(r + fractal->transformCommon.scale0 * z2.y * z2.z);
	// if (r1 < 1e-21)
	//	r1 = 1e-21;
	if (z2.z < z2.y)
	{
		theta = 2.0 * atan2(z.y, z.x);
		phi = 2.0 * asin(z.z / r1);
		z.x = r * cos(theta) * cos(phi);
		z.y = r * sin(theta) * cos(phi);
		z.z = -r * sin(phi);
	}
	else
	{
		theta = 2.0 * atan2(z.z, z.x);
		phi = 2.0 * asin(z.y / r1);
		z.x = r * cos(theta) * cos(phi);
		z.y = -r * sin(phi);
		z.z = r * sin(theta) * cos(phi);
	}
	z += fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.constantMultiplier000;
		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}
	double lengthTempZ = -z.Length();
	// if (lengthTempZ > -1e-21)
	//	lengthTempZ = -1e-21;   //  z is neg.)
	z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
	z *= fractal->transformCommon.scale1;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale1) + 1.0;
	aux.r_dz *= fabs(fractal->transformCommon.scale1);
}

/**
 * Msltoe_Julia_Bulb_Mod3
 * @reference
 * http://www.fractalforums.com/theory/choosing-the-squaring-formula-by-location/msg14320/#msg14320
 */
void MsltoeSym3Mod3Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;
	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	CVector4 z1 = z;
	double psi = atan2(z.z, z.y) + M_PI * 2.0;
	double psi2 = 0;
	while (psi > M_PI_8)
	{
		psi -= M_PI_4;
		psi2 -= M_PI_4; // M_PI_4 = pi/4
	}
	double cs = cos(psi2);
	double sn = sin(psi2);
	z1.y = z.y * cs - z.z * sn;
	z1.z = z.y * sn + z.z * cs;
	z.y = z1.y;
	z.z = z1.z;
	CVector4 zs = z * z;
	double zs2 = zs.x + zs.y;
	// if (zs2 < 1e-21)
	//	zs2 = 1e-21;
	double zs3 =
		(zs2 + zs.z) + fractal->transformCommon.scale0 * fractal->transformCommon.scale0 * zs.y * zs.z;
	double zsd = (1.0 - zs.z / zs3);

	z1.x = (zs.x - zs.y) * zsd;
	z1.y = (2.0 * z.x * z.y) * zsd * fractal->transformCommon.scale; // scaling y;
	z1.z = 2.0 * z.z * sqrt(zs2);
	z.x = z1.x;
	z.y = z1.y * cs + z1.z * sn;
	z.z = -z1.y * sn + z1.z * cs;
	z += fractal->transformCommon.additionConstant000;
	if (fractal->transformCommon.addCpixelEnabledFalse) // symmetrical addCpixel
	{
		CVector4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.constantMultiplier000;
		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}
	double lengthTempZ = -z.Length(); // spherical offset
	// if (lengthTempZ > -1e-21)
	//	lengthTempZ = -1e-21;   //  z is neg.)
	z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
	z *= fractal->transformCommon.scale1;
	aux.r_dz *= fabs(fractal->transformCommon.scale1);

	if (fractal->transformCommon.functionEnabledFalse // quaternion fold
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		aux.r = z.Length();
		aux.r_dz = aux.r_dz * 2.0 * z.Length();
		z = CVector4(z.x * z.x - z.y * z.y - z.z * z.z, z.x * z.y, z.x * z.z, z.w);
		if (fractal->transformCommon.functionEnabledAxFalse)
		{
			CVector4 temp = z;
			double tempL = temp.Length();
			z *= CVector4(1.0, 2.0, 2.0, 1.0);
			// if (tempL < 1e-21)
			//	tempL = 1e-21;
			double avgScale = z.Length() / tempL;
			aux.r_dz *= avgScale;
		}
		else
		{
			z *= CVector4(1.0, 2.0, 2.0, 1.0);
		}
	}
}

/**
 * MsltoeSym4Mod  Based on the formula from Mandelbulb3D
 * @reference http://www.fractalforums.com/theory/choosing-the-squaring-formula-by-location/15/
 */
void MsltoeSym4ModIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	CVector4 temp = z;
	double tempL = temp.Length();
	// if (tempL < 1e-21)
	//	tempL = 1e-21;
	z *= fractal->transformCommon.scale3D111;

	aux.r_dz *= fabs(z.Length() / tempL);

	if (fabs(z.x) < fabs(z.z)) swap(z.x, z.z);
	if (fabs(z.x) < fabs(z.y)) swap(z.x, z.y);
	if (fabs(z.y) < fabs(z.z)) swap(z.y, z.z);

	if (z.x * z.z < 0.0) z.z = -z.z;
	if (z.x * z.y < 0.0) z.y = -z.y;

	temp.x = z.x * z.x - z.y * z.y - z.z * z.z;
	temp.y = 2.0 * z.x * z.y;
	temp.z = 2.0 * z.x * z.z;

	z = temp + fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.rotationEnabled)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.constantMultiplier000;
		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}
	double lengthTempZ = -z.Length();
	// if (lengthTempZ > -1e-21)
	//	lengthTempZ = -1e-21;   //  z is neg.)
	z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
	z *= fractal->transformCommon.scale1;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale1) + 1.0;
	aux.r_dz *= fabs(fractal->transformCommon.scale1);
}

/**
 * MsltoeToroidal
 * @reference http://www.fractalforums.com/theory/toroidal-coordinates/msg9428/
 */
void MsltoeToroidalIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledFalse) // pre-scale
	{
		z *= fractal->transformCommon.scale3D111;
		aux.r_dz *= z.Length() / aux.r;
		aux.DE = aux.DE * z.Length() / aux.r + 1.0;
	}
	// Toroidal bulb
	double r1 = fractal->transformCommon.minR05; // default 0.5
	double theta = atan2(z.y, z.x);
	double x1 = r1 * cos(theta);
	double y1 = r1 * sin(theta);
	aux.r = (z.x - x1) * (z.x - x1) + (z.y - y1) * (z.y - y1) + z.z * z.z; //+ 1e-061
	double phi = asin(z.z / sqrt(aux.r));
	double rp = pow(aux.r, fractal->transformCommon.pwr4); // default 4.0

	phi *= fractal->transformCommon.pwr8; // default 8
	theta *= fractal->bulb.power;					// default 9 gives 8 symmetry
	// convert back to cartesian coordinates
	z.x = (r1 + rp * cos(phi)) * cos(theta);
	z.y = (r1 + rp * cos(phi)) * sin(theta);
	z.z = -rp * sin(phi);

	if (fractal->analyticDE.enabledFalse)
	{ // analytic log DE adjustment
		if (fractal->analyticDE.enabledAuxR2False) aux.r_dz *= aux.r_dz;
		aux.r_dz = pow(aux.r, fractal->transformCommon.pwr4 - fractal->analyticDE.offset1)
								 * fractal->transformCommon.pwr4 * fractal->analyticDE.scale1 * aux.r_dz
							 + fractal->analyticDE.offset2;
	}
	else
	{ // default, i.e. scale1 & offset1 & offset2 = 1.0
		aux.r_dz = pow(aux.r, fractal->transformCommon.pwr4 - 1.0) * aux.r_dz * aux.r_dz
								 * fractal->transformCommon.pwr4
							 + 1.0;
	}

	if (fractal->transformCommon.functionEnabledAxFalse) // spherical offset
	{
		double lengthTempZ = -z.Length();
		// if (lengthTempZ > -1e-21) lengthTempZ = -1e-21;   //  z is neg.)
		z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
		z *= fractal->transformCommon.scale;
		aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;
		aux.r_dz *= fabs(fractal->transformCommon.scale);
	}
	// then add Cpixel constant vector
}

/**
 * MsltoeToroidalMulti
 * @reference http://www.fractalforums.com/theory/toroidal-coordinates/msg9428/
 */
void MsltoeToroidalMultiIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledFalse) // pre-scale
	{
		z *= fractal->transformCommon.scale3D111;
		aux.r_dz *= z.Length() / aux.r;
		aux.DE = aux.DE * z.Length() / aux.r + 1.0;
	}
	// Toroidal bulb multi
	double th0 = fractal->bulb.betaAngleOffset;
	double ph0 = fractal->bulb.alphaAngleOffset;
	double v1, v2, v3;
	switch (fractal->sinTan2Trig.orderOfZYX)
	{
		case multi_OrderOfZYX_zyx:
		default:
			v1 = z.z;
			v2 = z.y;
			v3 = z.x;
			break;
		case multi_OrderOfZYX_zxy:
			v1 = z.z;
			v2 = z.x;
			v3 = z.y;
			break;
		case multi_OrderOfZYX_yzx:
			v1 = z.y;
			v2 = z.z;
			v3 = z.x;
			break;
		case multi_OrderOfZYX_yxz:
			v1 = z.y;
			v2 = z.x;
			v3 = z.z;
			break;
		case multi_OrderOfZYX_xzy:
			v1 = z.x;
			v2 = z.z;
			v3 = z.y;
			break;
		case multi_OrderOfZYX_xyz:
			v1 = z.x;
			v2 = z.y;
			v3 = z.z;
			break;
	}

	switch (fractal->sinTan2Trig.atan2OrAtan)
	{
		case multi_atan2OrAtan_atan2: ph0 += atan2(v2, v3); break;
		case multi_atan2OrAtan_atan: ph0 += atan(v2 / v3); break;
	}

	double r1 = fractal->transformCommon.minR05;
	double x1 = r1 * cos(ph0);
	double y1 = r1 * sin(ph0);

	aux.r = (z.x - x1) * (z.x - x1) + (z.y - y1) * (z.y - y1) + z.z * z.z; //+ 1e-061

	double sqrT = aux.r;
	if (fractal->transformCommon.functionEnabledAy) // sqrt
	{
		sqrT = sqrt(aux.r);
	}

	switch (fractal->sinTan2Trig.asinOrAcos)
	{
		case multi_asinOrAcos_asin: th0 += asin(v1 / sqrT); break;
		case multi_asinOrAcos_acos: th0 += acos(v1 / sqrT); break;
	}

	th0 *= fractal->transformCommon.pwr8; // default 8
	ph0 *= fractal->bulb.power;						// default 9 gives 8 symmetry

	double rp = pow(aux.r, fractal->transformCommon.pwr4);

	double costh = cos(th0);
	double sinth = sin(th0);

	if (fractal->transformCommon.functionEnabledzFalse)
	{ // cosine mode
		z.x = (r1 + rp * sinth) * sin(ph0);
		z.y = (r1 + rp * sinth) * cos(ph0);
		z.z = -rp * costh;
	}
	else
	{ // sine mode default
		z.x = (r1 + rp * costh) * cos(ph0);
		z.y = (r1 + rp * costh) * sin(ph0);
		z.z = -rp * sinth;
	}

	if (fractal->analyticDE.enabledFalse)
	{ // analytic log DE adjustment
		if (fractal->analyticDE.enabledAuxR2False) aux.r_dz *= aux.r_dz;
		aux.r_dz = pow(aux.r, fractal->transformCommon.pwr4 - fractal->analyticDE.offset1)
								 * fractal->transformCommon.pwr4 * fractal->analyticDE.scale1 * aux.r_dz
							 + fractal->analyticDE.offset2;
	}
	else
	{ // default, i.e. scale1 & offset1 & offset2 = 1.0
		aux.r_dz = pow(aux.r, fractal->transformCommon.pwr4 - 1.0) * aux.r_dz * aux.r_dz
								 * fractal->transformCommon.pwr4
							 + 1.0;
	}

	if (fractal->transformCommon.functionEnabledAxFalse) // spherical offset
	{
		double lengthTempZ = -z.Length();
		// if (lengthTempZ > -1e-21) lengthTempZ = -1e-21;   //  z is neg.)
		z *= 1.0 + fractal->transformCommon.offset / lengthTempZ;
		z *= fractal->transformCommon.scale;
		aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;
		aux.r_dz *= fabs(fractal->transformCommon.scale);
	}
	// then add Cpixel constant vector
}
/**
 * Pseudo Kleinian Knighty - Theli-at's Pseudo Kleinian (Scale 1 JuliaBox + Something
 * @reference https://github.com/Syntopia/Fragmentarium/blob/master/
 * Fragmentarium-Source/Examples/Knighty%20Collection/PseudoKleinian.frag
 */
void PseudoKleinianIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 gap = fractal->transformCommon.constantMultiplier000;
	double t;
	double dot1;

	if (fractal->transformCommon.functionEnabledPFalse
			&& aux.i >= fractal->transformCommon.startIterationsP
			&& aux.i < fractal->transformCommon.stopIterationsP1)
	{
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
		t = max(0.0, dot1);
		z.x -= t * -SQRT_3;
		z.y = fabs(z.y - t);

		if (z.y > z.z) swap(z.y, z.z);
		z -= gap * CVector4(SQRT_3_4, 1.5, 1.5, 0.0);
		// z was pos, now some points neg (ie neg shift)
		if (z.z > z.x) swap(z.z, z.x);
		if (z.x > 0.0)
		{
			z.y = max(0.0, z.y);
			z.z = max(0.0, z.z);
		}
	}

	if (fractal->transformCommon.functionEnabledBxFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		if (fabs(z.x) > fractal->mandelbox.foldingLimit)
		{
			z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
			aux.color += fractal->mandelbox.color.factor.x;
		}
		if (fabs(z.y) > fractal->mandelbox.foldingLimit)
		{
			z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
			aux.color += fractal->mandelbox.color.factor.y;
		}
		double zLimit = fractal->mandelbox.foldingLimit * fractal->transformCommon.scale1;
		double zValue = fractal->mandelbox.foldingValue * fractal->transformCommon.scale1;
		if (fabs(z.z) > zLimit)
		{
			z.z = sign(z.z) * zValue - z.z;
			aux.color += fractal->mandelbox.color.factor.z;
		}
	}

	CVector4 cSize = fractal->transformCommon.additionConstant0777;
	CVector4 tempZ = z; //  correct c++ version.
	if (z.x > cSize.x) tempZ.x = cSize.x;
	if (z.x < -cSize.x) tempZ.x = -cSize.x;
	if (z.y > cSize.y) tempZ.y = cSize.y;
	if (z.y < -cSize.y) tempZ.y = -cSize.y;
	if (z.z > cSize.z) tempZ.z = cSize.z;
	if (z.z < -cSize.z) tempZ.z = -cSize.z;

	z = tempZ * 2.0 - z;
	double k = max(fractal->transformCommon.minR05 / z.Dot(z), 1.0);
	z *= k;
	aux.DE *= k + fractal->analyticDE.tweak005;

	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	z += fractal->transformCommon.additionConstant000;
	// no bailout

	aux.pseudoKleinianDE = fractal->analyticDE.scale1;
}

/**
 * Pseudo Kleinian Mod1, Knighty - Theli-at's Pseudo Kleinian (Scale 1 JuliaBox + Something
 * @reference https://github.com/Syntopia/Fragmentarium/blob/master/
 * Fragmentarium-Source/Examples/Knighty%20Collection/PseudoKleinian.frag
 */
void PseudoKleinianMod1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 gap = fractal->transformCommon.constantMultiplier000;

	if (fractal->transformCommon.functionEnabledPFalse
			&& aux.i >= fractal->transformCommon.startIterationsP
			&& aux.i < fractal->transformCommon.stopIterationsP1)
	{
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		double dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
		double t = max(0.0, dot1);
		z.x -= t * -SQRT_3;
		z.y = fabs(z.y - t);

		if (z.y > z.z) swap(z.y, z.z);
		z -= gap * CVector4(SQRT_3_4, 1.5, 1.5, 0.0);
		// z was pos, now some points neg (ie neg shift)
		if (z.z > z.x) swap(z.z, z.x);
		if (z.x > 0.0)
		{
			z.y = max(0.0, z.y);
			z.z = max(0.0, z.z);
		}
	}

	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	if (fractal->transformCommon.benesiT1EnabledFalse
			&& aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterationsT1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 tempZ = z;
		double tempL = tempZ.Length();
		z = fabs(z) * fractal->transformCommon.scale3D222;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
		z = z - fractal->transformCommon.offset200;
	}

	if (fractal->transformCommon.functionEnabledxFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsTM1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D333;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		z = (fabs(z + fractal->transformCommon.additionConstant111)
				 - fabs(z - fractal->transformCommon.additionConstant111) - z);

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
	}
	if (fractal->transformCommon.functionEnabledFFalse
			&& aux.i >= fractal->transformCommon.startIterationsF
			&& aux.i < fractal->transformCommon.stopIterationsF)
	{
		CVector4 tempA, tempB;

		if (fractal->transformCommon.functionEnabledAx)
			tempA.x = fabs(z.x + fractal->transformCommon.offsetF000.x);

		if (fractal->transformCommon.functionEnabledx)
			tempB.x = fabs(z.x - fractal->transformCommon.offset000.x);

		z.x = tempA.x - tempB.x - (z.x * fractal->transformCommon.scale3D111.x);

		if (fractal->transformCommon.functionEnabledAy)
			tempA.y = fabs(z.y + fractal->transformCommon.offsetF000.y);

		if (fractal->transformCommon.functionEnabledy)
			tempB.y = fabs(z.y - fractal->transformCommon.offset000.y);

		z.y = tempA.y - tempB.y - (z.y * fractal->transformCommon.scale3D111.y);

		if (fractal->transformCommon.functionEnabledAz)
			tempA.z = fabs(z.z + fractal->transformCommon.offsetF000.z);

		if (fractal->transformCommon.functionEnabledz)
			tempB.z = fabs(z.z - fractal->transformCommon.offset000.z);

		z.z = tempA.z - tempB.z - (z.z * fractal->transformCommon.scale3D111.z);

		z += fractal->transformCommon.offsetA000;
	}

	double k;
	// Pseudo kleinian
	CVector4 cSize = fractal->transformCommon.additionConstant0777;
	if (fractal->transformCommon.functionEnabledAy
			&& aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		CVector4 tempZ = z; //  correct c++ version. non conditional mult 2.0

		if (z.x > cSize.x) tempZ.x = cSize.x;
		if (z.x < -cSize.x) tempZ.x = -cSize.x;
		if (z.y > cSize.y) tempZ.y = cSize.y;
		if (z.y < -cSize.y) tempZ.y = -cSize.y;
		if (z.z > cSize.z) tempZ.z = cSize.z;
		if (z.z < -cSize.z) tempZ.z = -cSize.z;

		z.z *= fractal->transformCommon.scaleB1;
		aux.DE *= fractal->transformCommon.scaleB1;

		z = tempZ * 2.0 - z;
		k = max(fractal->transformCommon.minR05 / z.Dot(z), 1.0);
		z *= k;
		aux.DE *= k + fractal->analyticDE.tweak005;
	}

	if (fractal->transformCommon.functionEnabledAyFalse
			&& aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB)
	{
		//  variation from openCL conditional mult 2.0
		if (z.x > cSize.x) z.x = cSize.x * 2.0 - z.x;
		if (z.x < -cSize.x) z.x = -cSize.x * 2.0 - z.x;
		if (z.y > cSize.y) z.y = cSize.y * 2.0 - z.y;
		if (z.y < -cSize.y) z.y = -cSize.y * 2.0 - z.y;
		if (z.z > cSize.z) z.z = cSize.z * 2.0 - z.z;
		if (z.z < -cSize.z) z.z = -cSize.z * 2.0 - z.z;

		k = max(fractal->transformCommon.minR05 / z.Dot(z), 1.0);
		z *= k;
		aux.DE *= k + fractal->analyticDE.tweak005;
	}

	z += fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.functionEnabledMFalse
			&& aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		z = fabs(z);
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0 * fractal->transformCommon.constantMultiplierA111.x;
		z.y -= 2.0 * fractal->transformCommon.constantMultiplierA111.y;
		if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplierA111.z;
		aux.DE *= fractal->transformCommon.scale3 * fractal->transformCommon.scaleA1;

		z += fractal->transformCommon.additionConstantA000;
	}
	aux.pseudoKleinianDE = fractal->analyticDE.scale1; // pK DE
}
/**
 * Pseudo Kleinian Mod2, Knighty - Theli-at's Pseudo Kleinian (Scale 1 JuliaBox + Something
 * @reference https://github.com/Syntopia/Fragmentarium/blob/master/
 * Fragmentarium-Source/Examples/Knighty%20Collection/PseudoKleinian.frag
 */
void PseudoKleinianMod2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	// spherical fold
	if (fractal->transformCommon.functionEnabledSFalse
			&& aux.i >= fractal->transformCommon.startIterationsS
			&& aux.i < fractal->transformCommon.stopIterationsS)
	{
		double para = 0.0;
		double paraAddP0 = 0.0;
		if (fractal->transformCommon.functionEnabledyFalse)
		{
			if (fractal->Cpara.enabledLinear)
			{
				para = fractal->Cpara.para00; // parameter value at iter 0
				double temp0 = para;
				double tempA = fractal->Cpara.paraA;
				double tempB = fractal->Cpara.paraB;
				double tempC = fractal->Cpara.paraC;
				double lengthAB = fractal->Cpara.iterB - fractal->Cpara.iterA;
				double lengthBC = fractal->Cpara.iterC - fractal->Cpara.iterB;
				double grade1 = (tempA - temp0) / fractal->Cpara.iterA;
				double grade2 = (tempB - tempA) / lengthAB;
				double grade3 = (tempC - tempB) / lengthBC;

				// slopes
				if (aux.i < fractal->Cpara.iterA)
				{
					para = temp0 + (aux.i * grade1);
				}
				if (aux.i < fractal->Cpara.iterB && aux.i >= fractal->Cpara.iterA)
				{
					para = tempA + (aux.i - fractal->Cpara.iterA) * grade2;
				}
				if (aux.i >= fractal->Cpara.iterB)
				{
					para = tempB + (aux.i - fractal->Cpara.iterB) * grade3;
				}

				// Curvi part on "true"
				if (fractal->Cpara.enabledCurves)
				{
					double paraAdd = 0.0;
					double paraIt;
					if (lengthAB > 2.0 * fractal->Cpara.iterA) // stop  error, todo fix.
					{
						double curve1 = (grade2 - grade1) / (4.0 * fractal->Cpara.iterA);
						double tempL = lengthAB - fractal->Cpara.iterA;
						double curve2 = (grade3 - grade2) / (4.0 * tempL);
						if (aux.i < 2 * fractal->Cpara.iterA)
						{
							paraIt = tempA - fabs(tempA - aux.i);
							paraAdd = paraIt * paraIt * curve1;
						}
						if (aux.i >= 2 * fractal->Cpara.iterA && aux.i < fractal->Cpara.iterB + tempL)
						{
							paraIt = tempB - fabs(tempB * aux.i);
							paraAdd = paraIt * paraIt * curve2;
						}
						para += paraAdd;
					}
				}
			}
			paraAddP0 = 0.0;
			if (fractal->Cpara.enabledParabFalse)
			{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
				paraAddP0 = fractal->Cpara.parabOffset0 + (aux.i * fractal->Cpara.parabSlope)
										+ (aux.i * aux.i * 0.001 * fractal->Cpara.parabScale);
			}
		}
		para += paraAddP0 + fractal->transformCommon.minR2p25;

		// spherical fold
		double rr = z.Dot(z);

		z += fractal->mandelbox.offset;

		// if (rr < 1e-21) rr = 1e-21;
		if (rr < para)
		{
			double tglad_factor1 = fractal->transformCommon.maxR2d1 / para;
			z *= tglad_factor1;
			aux.DE *= tglad_factor1;
			// aux.color += fractal->mandelbox.color.factorSp1;
		}
		else if (rr < fractal->transformCommon.maxR2d1) // fractal->mandelbox.fR2
		{
			double tglad_factor2 = fractal->transformCommon.maxR2d1 / rr;
			z *= tglad_factor2;
			aux.DE *= tglad_factor2;
			// aux.color += fractal->mandelbox.color.factorSp2;
		}
		z -= fractal->mandelbox.offset;
		z *= fractal->transformCommon.scale1;
		aux.DE = aux.DE * fabs(fractal->transformCommon.scale1) + fractal->analyticDE.offset0;
	}

	if (fractal->transformCommon.functionEnabledPFalse
			&& aux.i >= fractal->transformCommon.startIterationsP
			&& aux.i < fractal->transformCommon.stopIterationsP1)
	{
		CVector4 gap = fractal->transformCommon.constantMultiplier000;
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		double dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
		double t = max(0.0, dot1);
		z.x -= t * -SQRT_3;
		z.y = fabs(z.y - t);

		if (z.y > z.z) swap(z.y, z.z);
		z -= gap * CVector4(SQRT_3_4, 1.5, 1.5, 0.0);
		// z was pos, now some points neg (ie neg shift)
		if (z.z > z.x) swap(z.z, z.x);
		if (z.x > 0.0)
		{
			z.y = max(0.0, z.y);
			z.z = max(0.0, z.z);
		}
	}

	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	if (fractal->transformCommon.benesiT1EnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsT
			&& aux.i < fractal->transformCommon.stopIterationsT1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, 0.0);

		CVector4 tempZ = z;
		double tempL = tempZ.Length();
		z = fabs(z) * fractal->transformCommon.scale3D222;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, 0.0);
		z = z - fractal->transformCommon.offset200;
	}

	if (fractal->transformCommon.functionEnabledxFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsTM1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, 0.0);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D333;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		z = (fabs(z + fractal->transformCommon.additionConstant111)
				 - fabs(z - fractal->transformCommon.additionConstant111) - z);

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, 0.0);
	}

	double k;
	// Pseudo kleinian
	CVector4 cSize = fractal->transformCommon.additionConstant0777;
	if (fractal->transformCommon.functionEnabledAy
			&& aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		CVector4 tempZ = z; //  correct c++ version. non conditional mult 2.0

		if (z.x > cSize.x) tempZ.x = cSize.x;
		if (z.x < -cSize.x) tempZ.x = -cSize.x;
		if (z.y > cSize.y) tempZ.y = cSize.y;
		if (z.y < -cSize.y) tempZ.y = -cSize.y;
		if (z.z > cSize.z) tempZ.z = cSize.z;
		if (z.z < -cSize.z) tempZ.z = -cSize.z;

		z = tempZ * 2.0 - z;
		k = max(fractal->transformCommon.minR05 / z.Dot(z), 1.0);
		z *= k;
		aux.DE *= k + fractal->analyticDE.tweak005;
	}

	if (fractal->transformCommon.functionEnabledAyFalse
			&& aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB)
	{
		//  variation from openCL  conditional mult 2.0
		if (z.x > cSize.x) z.x = cSize.x * 2.0 - z.x;
		if (z.x < -cSize.x) z.x = -cSize.x * 2.0 - z.x;
		if (z.y > cSize.y) z.y = cSize.y * 2.0 - z.y;
		if (z.y < -cSize.y) z.y = -cSize.y * 2.0 - z.y;
		if (z.z > cSize.z) z.z = cSize.z * 2.0 - z.z;
		if (z.z < -cSize.z) z.z = -cSize.z * 2.0 - z.z;

		k = max(fractal->transformCommon.minR05 / z.Dot(z), 1.0);
		z *= k;
		aux.DE *= k + fractal->analyticDE.tweak005;
	}

	z += fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.functionEnabledFFalse
			&& aux.i >= fractal->transformCommon.startIterationsF
			&& aux.i < fractal->transformCommon.stopIterationsF)
	{
		z = fabs(z + fractal->transformCommon.offsetA000)
				- fabs(z - fractal->transformCommon.offsetA000) - z;

		if (fractal->transformCommon.functionEnabledFalse
				&& aux.i >= fractal->transformCommon.startIterationsA
				&& aux.i < fractal->transformCommon.stopIterationsA)
		{
			CVector4 limit = fractal->transformCommon.offsetA000;
			CVector4 length = 2.0 * limit;
			CVector4 tgladS = 1.0 / length;
			CVector4 Add;
			if (fabs(z.x) < limit.x) Add.x = z.x * z.x * tgladS.x;
			if (fabs(z.y) < limit.y) Add.y = z.y * z.y * tgladS.y;
			if (fabs(z.z) < limit.z) Add.z = z.z * z.z * tgladS.z;
			if (fabs(z.x) > limit.x && fabs(z.x) < length.x)
				Add.x = (length.x - fabs(z.x)) * (length.x - fabs(z.x)) * tgladS.x;
			if (fabs(z.y) > limit.y && fabs(z.y) < length.y)
				Add.y = (length.y - fabs(z.y)) * (length.y - fabs(z.y)) * tgladS.y;
			if (fabs(z.z) > limit.z && fabs(z.z) < length.z)
				Add.z = (length.z - fabs(z.z)) * (length.z - fabs(z.z)) * tgladS.z;
			Add *= fractal->transformCommon.scale3D000;
			z.x = (z.x - (sign(z.x) * (Add.x)));
			z.y = (z.y - (sign(z.y) * (Add.y)));
			z.z = (z.z - (sign(z.z) * (Add.z)));
		}
	}
	if (fractal->transformCommon.addCpixelEnabledFalse) // symmetrical addCpixel
	{
		CVector4 tempFAB = c;
		if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
		if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
		if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

		tempFAB *= fractal->transformCommon.offsetF000;
		z.x += sign(z.x) * tempFAB.x;
		z.y += sign(z.y) * tempFAB.y;
		z.z += sign(z.z) * tempFAB.z;
	}
	aux.pseudoKleinianDE = fractal->analyticDE.scale1; // pK DE
	// aux.pseudoKleinianZZ = fractal->transformCommon.scale0; // pK z.z * z.z * scale0
}

/**
 * Pseudo Kleinian std DE Knighty - Theli-at's Pseudo Kleinian (Scale 1 JuliaBox + Something
 * @reference https://github.com/Syntopia/Fragmentarium/blob/master/
 * Fragmentarium-Source/Examples/Knighty%20Collection/PseudoKleinian.frag
 */
void PseudoKleinianStdDEIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 gap = fractal->transformCommon.constantMultiplier000;
	double t;
	double dot1;

	if (fractal->transformCommon.functionEnabledPFalse
			&& aux.i >= fractal->transformCommon.startIterationsP
			&& aux.i < fractal->transformCommon.stopIterationsP1)
	{
		z.y = fabs(z.y);
		z.z = fabs(z.z);
		dot1 = (z.x * -SQRT_3_4 + z.y * 0.5) * fractal->transformCommon.scale;
		t = max(0.0, dot1);
		z.x -= t * -SQRT_3;
		z.y = fabs(z.y - t);

		if (z.y > z.z) swap(z.y, z.z);
		z -= gap * CVector4(SQRT_3_4, 1.5, 1.5, 0.0);
		// z was pos, now some points neg (ie neg shift)
		if (z.z > z.x) swap(z.z, z.x);
		if (z.x > 0.0)
		{
			z.y = max(0.0, z.y);
			z.z = max(0.0, z.z);
		}
	}

	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	if (fractal->transformCommon.benesiT1EnabledFalse
			&& aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterationsT1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 tempZ = z;
		double tempL = tempZ.Length();
		z = fabs(z) * fractal->transformCommon.scale3D222;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
		z = z - fractal->transformCommon.offset200;
	}

	if (fractal->transformCommon.functionEnabledxFalse
			&& aux.i >= fractal->transformCommon.startIterationsD
			&& aux.i < fractal->transformCommon.stopIterationsTM1)
	{
		double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
		z = CVector4(
			(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

		CVector4 temp = z;
		double tempL = temp.Length();
		z = fabs(z) * fractal->transformCommon.scale3D333;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;

		z = (fabs(z + fractal->transformCommon.additionConstant111)
				 - fabs(z - fractal->transformCommon.additionConstant111) - z);

		tempXZ = (z.y + z.x) * SQRT_1_2;

		z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
			z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
	}
	if (fractal->transformCommon.functionEnabledFFalse
			&& aux.i >= fractal->transformCommon.startIterationsF
			&& aux.i < fractal->transformCommon.stopIterationsF)
	{
		CVector4 tempA, tempB;

		if (fractal->transformCommon.functionEnabledAx)
			tempA.x = fabs(z.x + fractal->transformCommon.offsetF000.x);

		if (fractal->transformCommon.functionEnabledx)
			tempB.x = fabs(z.x - fractal->transformCommon.offset000.x);

		z.x = tempA.x - tempB.x - (z.x * fractal->transformCommon.scale3D111.x);

		if (fractal->transformCommon.functionEnabledAy)
			tempA.y = fabs(z.y + fractal->transformCommon.offsetF000.y);

		if (fractal->transformCommon.functionEnabledy)
			tempB.y = fabs(z.y - fractal->transformCommon.offset000.y);

		z.y = tempA.y - tempB.y - (z.y * fractal->transformCommon.scale3D111.y);

		if (fractal->transformCommon.functionEnabledAz)
			tempA.z = fabs(z.z + fractal->transformCommon.offsetF000.z);

		if (fractal->transformCommon.functionEnabledz)
			tempB.z = fabs(z.z - fractal->transformCommon.offset000.z);

		z.z = tempA.z - tempB.z - (z.z * fractal->transformCommon.scale3D111.z);

		z += fractal->transformCommon.offsetA000;
	}
	if (fractal->transformCommon.functionEnabled && aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		z = fabs(z);
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0 * fractal->transformCommon.constantMultiplierA111.x;
		z.y -= 2.0 * fractal->transformCommon.constantMultiplierA111.y;
		if (z.z > 1.0) z.z -= 2.0 * fractal->transformCommon.constantMultiplierA111.z;
		aux.DE *= fractal->transformCommon.scale3 * fractal->transformCommon.scaleA1;

		z += fractal->transformCommon.additionConstantA000;
	}

	aux.DE *= fractal->transformCommon.scaleB1;
	double k;
	// Pseudo kleinian
	CVector4 cSize = fractal->transformCommon.additionConstant0777;
	if (fractal->transformCommon.functionEnabledAy
			&& aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		CVector4 tempZ = z; //  correct c++ version.

		if (z.x > cSize.x) tempZ.x = cSize.x;
		if (z.x < -cSize.x) tempZ.x = -cSize.x;
		if (z.y > cSize.y) tempZ.y = cSize.y;
		if (z.y < -cSize.y) tempZ.y = -cSize.y;
		if (z.z > cSize.z) tempZ.z = cSize.z;
		if (z.z < -cSize.z) tempZ.z = -cSize.z;

		z *= fractal->transformCommon.scaleB1;
		z = tempZ * 2.0 - z;

		k = max(fractal->transformCommon.minR05 / z.Dot(z), 1.0);
		z *= k;
		aux.DE *= k + fractal->analyticDE.tweak005;
	}

	if (fractal->transformCommon.functionEnabledAyFalse
			&& aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB)
	{
		//  variation from openCL
		if (z.x > cSize.x) z.x = cSize.x * 2.0 - z.x;
		if (z.x < -cSize.x) z.x = -cSize.x * 2.0 - z.x;
		if (z.y > cSize.y) z.y = cSize.y * 2.0 - z.y;
		if (z.y < -cSize.y) z.y = -cSize.y * 2.0 - z.y;
		if (z.z > cSize.z) z.z = cSize.z * 2.0 - z.z;
		if (z.z < -cSize.z) z.z = -cSize.z * 2.0 - z.z;
		k = max(fractal->transformCommon.minR05 / z.Dot(z), 1.0);
		z *= k;
		aux.DE *= k + fractal->analyticDE.tweak005;
	}

	z += fractal->transformCommon.additionConstant000;
	// aux.pseudoKleinianZZ = fractal->transformCommon.scale0; // pK z.z * z.z * scale0
}

/**
 * Quaternion3DE - Quaternion fractal with extended controls
 * @reference http://www.fractalforums.com/3d-fractal-generation
 * /true-3d-mandlebrot-type-fractal/
 */
void Quaternion3dIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	aux.r = z.Length();
	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	z = CVector4(z.x * z.x - z.y * z.y - z.z * z.z, z.x * z.y, z.x * z.z, z.w);

	double tempL = z.Length();
	z *= fractal->transformCommon.constantMultiplier122;
	// if (tempL < 1e-21) tempL = 1e-21;
	double avgScale = CVector4(z.x, z.y / 2.0, z.z / 2.0, z.w).Length() / tempL;
	double tempAux = aux.r_dz * avgScale;
	aux.r_dz = aux.r_dz + (tempAux - aux.r_dz) * fractal->transformCommon.scaleA1;

	if (fractal->transformCommon.rotationEnabled)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	z += fractal->transformCommon.additionConstant000;
}

/**
 * RiemannSphereMsltoe
 * @reference http://www.fractalforums.com/the-3d-mandelbulb/
 * riemann-fractals/msg33500/#msg33500
 */
void RiemannSphereMsltoeIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	if (fractal->transformCommon.rotationEnabled)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	double r = z.Length();
	// if (r < 1e-21) r = 1e-21;
	z *= fractal->transformCommon.scale / r;

	double w = 1.0 - z.z;
	// if (w > -1e-21 && w < 1e-21) w = (w > 0) ? 1e-21 : -1e-21;
	w = 1.0 / w;

	double s = z.x * w;
	double t = z.y * w;

	w = 1.0 + s * s + t * t;

	s = fabs(sin(M_PI * s));
	t = fabs(sin(M_PI * t));

	r *= r;
	// if (r < 1e-21)
	//	r = 1e-21;

	if (w > 36) w = 36; // problem with pow()
	r = -0.25 + pow(r, w);

	w = r / (1.0 + s * s + t * t);
	z.x = 2.0 * s;
	z.y = 2.0 * t;
	z.z = -1.0 + s * s + t * t;
	z *= w;

	z += fractal->transformCommon.additionConstant000;
}

/**
 * RiemannSphereMsltoe Variation1
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * revisiting-the-riemann-sphere-%28again%29/
 */
void RiemannSphereMsltoeV1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	double r = z.Length();
	// if (r < 1e-21) r = 1e-21;
	z *= fractal->transformCommon.scale / r;
	double w = 1.0 - z.z;
	// if (w > -1e-21 && w < 1e-21)
	//	w = (w > 0) ? 1e-21 : -1e-21;
	w = 1.0 / w;
	CVector4 t3;
	t3.x = z.x * w;
	t3.y = z.y * w;

	t3.z = (r - 1.5) * (1.0 + t3.x * t3.x + t3.y * t3.y);

	if (fractal->transformCommon.rotationEnabled)
		t3 = fractal->transformCommon.rotationMatrix.RotateVector(t3);

	t3.x = t3.x - floor(t3.x + 0.5);
	t3.y = t3.y - floor(t3.y + 0.5);
	t3.w = z.w;

	z = t3 * fractal->transformCommon.constantMultiplier441;

	z += fractal->transformCommon.additionConstant000;
}

/**
 * RiemannBulbMsltoe Mod2
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * another-way-to-make-my-riemann-sphere-'bulb'-using-a-conformal-transformation/
 */
void RiemannBulbMsltoeMod2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	double rad2 = fractal->transformCommon.minR05;
	double r2 = z.x * z.x + z.y * z.y + z.z * z.z; // r2 or point radius squared
	if (r2 < rad2 * rad2)
	{
		if (fractal->transformCommon.functionEnabled)
			// smooth inside
			z *= rad2 * ((r2 * 0.1) + 0.4) * 1.18 * fractal->transformCommon.scaleA1 / r2;
		else
		{
			z *= fractal->transformCommon.constantMultiplier111;
		}
	} // if internal smooth function disabled, then z = z * scale, default vect3(1,1,1)
	else
	{
		// 1st scale variable, default vect3 (1.7, 1.7, 1.7),
		z *= fractal->transformCommon.constantMultiplier222;
		double shift = fractal->transformCommon.offset1;
		// r1 = length^2,  //  + 1e-061
		double r1 = z.x * z.x + (z.y - shift) * (z.y - shift) + z.z * z.z;
		// inversions by length^2
		z.x = z.x / r1;
		z.y = (z.y - shift) / r1;
		z.z = z.z / r1;
		// 2nd scale variable , default = double (3.0)
		z *= fractal->transformCommon.scale3;
		// y offset variable, default = double (1.9);
		z.y = z.y + fractal->transformCommon.offset105;
		if (fractal->transformCommon.functionEnabledx)
		{
			z.x = z.x - round(z.x); // periodic cubic tiling,
			z.z = z.z - round(z.z);
		}
		if (fractal->transformCommon.functionEnabledxFalse)
		{
			z.x = fabs(sin(M_PI * z.x * fractal->transformCommon.scale1));
			z.z = fabs(sin(M_PI * z.z * fractal->transformCommon.scale1));
		}
	}
}

/**
 * Sierpinski3D. made from Darkbeam's Sierpinski code from M3D
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void Sierpinski3dIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{

	CVector4 temp = z;

	// Normal full tetra-fold;
	if (fractal->transformCommon.functionEnabled)
	{
		if (z.x - z.y < 0.0) swap(z.x, z.y);
		if (z.x - z.z < 0.0) swap(z.x, z.z);
		if (z.y - z.z < 0.0) swap(z.y, z.z);

		if (z.x + z.y < 0.0)
		{
			temp.x = -z.y;
			z.y = -z.x;
			z.x = temp.x;
		}
		if (z.x + z.z < 0.0)
		{
			temp.x = -z.z;
			z.z = -z.x;
			z.x = temp.x;
		}
		if (z.y + z.z < 0.0)
		{
			temp.y = -z.z;
			z.z = -z.y;
			z.y = temp.y;
		}
	}

	// Reversed full tetra-fold;
	if (fractal->transformCommon.functionEnabledFalse)
	{
		if (z.x + z.y < 0.0)
		{
			temp.x = -z.y;
			z.y = -z.x;
			z.x = temp.x;
		}
		if (z.x + z.z < 0.0)
		{
			temp.x = -z.z;
			z.z = -z.x;
			z.x = temp.x;
		}
		if (z.y + z.z < 0.0)
		{
			temp.y = -z.z;
			z.z = -z.y;
			z.y = temp.y;
		}
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
	}

	z = z * fractal->transformCommon.scaleA2;
	aux.DE *= fractal->transformCommon.scaleA2;

	if (aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		z -= fractal->transformCommon.offset111; // neg offset
	}
	// rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}

	aux.DE *= fractal->analyticDE.scale1;
}

/**
 * GeneralizedFoldBoxIteration - Quaternion fractal with extended controls
 * @reference http://www.fractalforums.com/new-theories-and-research/generalized-box-fold/
 */
void GeneralizedFoldBoxIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector3 zXYZ = z.GetXYZ();
	int i;
	const CVector3 *Nv;
	int sides;

	Nv = fractal->genFoldBox.Nv_tet;
	sides = fractal->genFoldBox.sides_tet;

	switch (fractal->genFoldBox.type)
	{
	case generalizedFoldBoxType_foldCube:
		Nv = fractal->genFoldBox.Nv_cube;
		sides = fractal->genFoldBox.sides_cube;
		break;
	case generalizedFoldBoxType_foldOct:
		Nv = fractal->genFoldBox.Nv_oct;
		sides = fractal->genFoldBox.sides_oct;
		break;
	case generalizedFoldBoxType_foldDodeca:
		Nv = fractal->genFoldBox.Nv_dodeca;
		sides = fractal->genFoldBox.sides_dodeca;
		break;
	case generalizedFoldBoxType_foldOctCube:
		Nv = fractal->genFoldBox.Nv_oct_cube;
		sides = fractal->genFoldBox.sides_oct_cube;
		break;
	case generalizedFoldBoxType_foldIcosa:
		Nv = fractal->genFoldBox.Nv_icosa;
		sides = fractal->genFoldBox.sides_icosa;
		break;
	case generalizedFoldBoxType_foldBox6:
		Nv = fractal->genFoldBox.Nv_box6;
		sides = fractal->genFoldBox.sides_box6;
		break;
	case generalizedFoldBoxType_foldBox5:
		Nv = fractal->genFoldBox.Nv_box5;
		sides = fractal->genFoldBox.sides_box5;
		break;
	}

	double melt = fractal->mandelbox.melt;
	double solid = fractal->mandelbox.solid;

	// Find the closest cutting plane if any that cuts the line between the origin and z.
	// Line is parameterized as X = Y + L*a;
	// Cutting plane is X.Dot(Nv) = Solid.
	// (Y + L*a).Dot(Nv) = solid.
	// a = (solid - Y.Dot(Nv))/L.Dot(Nv) = b/c
	CVector3 L = zXYZ;
	double a = 1;
	CVector3 Y; // Y is the origin in this case.
	int side = -1;
	double b, c;

	for (i = 0; i < sides; i++)
	{
		b = solid;
		c = L.Dot(Nv[i]);
		// A bit subtle here. a_r must be positive and I want to avoid divide by zero.
		if ((c > 0.0) && ((a * c) > b))
		{
			side = i;
			a = b / c;
		}
	}

	// If z is above the foldingValue we may have to fold. Else early out.
	if (side != -1)
	{ // mirror check
		int side_m = side;
		CVector3 Nv_m = Nv[side_m];
		CVector3 X_m = zXYZ - Nv_m * (zXYZ.Dot(Nv_m) - solid);

		// Find any plane (Nv_r) closest to X_m that cuts the line between Nv_m and X_m.
		// Nv_m cross Nv_r will define a possible rotation axis.
		// a = (solid - Y.Dot(Nv)/L.Dot(Nv) = b/c.
		L = X_m - Nv_m;
		Y = Nv_m;
		a = 1;
		side = -1;

		for (i = 0; i < sides; i++)
		{
			if (i != side_m)
			{
				b = solid - Y.Dot(Nv[i]);
				c = L.Dot(Nv[i]);
				// A bit subtle here. a_r must be positive and I want to avoid divide by zero.
				if ((c > 0.0) && ((a * c) > b))
				{
					side = i;
					a = b / c;
				}
			}
		}

		// Was a cutting plane found?
		if (side != -1)
		{ // rotation check
			CVector3 Xmr_intersect = Y + L * a;
			int side_r = side;
			CVector3 Nv_r = Nv[side_r];
			// The axis of rotation is define by the cross product of Nv_m and Nv_r and
			// the intersection of the line between Nv_m and Nv_r and  Xmr_intersect.
			CVector3 L_r = Nv_m.Cross(Nv_r);
			// The closest point between z and the line of rotation can be found by minimizing
			// the square of the distance (D) between z and the line
			// X = Xmr_intersect + L_r * a_rmin.
			// Setting dD/da_rmin equal to zero and solving for a_rmin.
			double a_rmin = (zXYZ.Dot(L_r) - Xmr_intersect.Dot(L_r)) / (L_r.Dot(L_r));

			// force a_rmin to be positive. I think I made an even number of sign errors here.
			if (a_rmin < 0.0)
			{
				a_rmin = -a_rmin;
				L_r = L_r * (-1);
			}
			CVector3 X_r = Xmr_intersect + L_r * a_rmin;

			// Find any plane (Nv_i) closest to Xmr_intersect that cuts the line between
			// Xmr_intersect and X_r. This will define a possible inversion point.
			// a = (solid - Y.Dot(Nv)/L.Dot(Nv) = b/c.
			L = X_r - Xmr_intersect;
			Y = Xmr_intersect;
			a = 1;
			side = -1;

			for (i = 0; i < sides; i++)
			{
				if ((i != side_m) && (i != side_r))
				{
					b = solid - Y.Dot(Nv[i]);
					c = L.Dot(Nv[i]);
					// A bit subtle here. a must be positive and I want to avoid divide by zero.
					if ((c > 0.0) && ((a * c) > b))
					{
						side = i;
						a = b / c;
					}
				}
			}

			if (side != -1)
			{ // inversion check
				// Only inversion point possible but still need to check for melt.

				CVector3 X_i = Y + L * a;
				CVector3 z2X = X_i - zXYZ;
				// Is z above the melt layer.
				if (z2X.Dot(z2X) > (melt * melt))
				{
					double z2X_mag = z2X.Length();
					zXYZ += z2X * (2.0 * (z2X_mag - melt) / (z2X_mag + .00000001));
					aux.color += fractal->mandelbox.color.factor.z;
				}
			}
			else
			{
				// Only rotation line possible but still need to check for melt.
				// Is z above the melt layer.
				CVector3 z2X = X_r - zXYZ;
				if (z2X.Dot(z2X) > (melt * melt))
				{
					double z2X_mag = z2X.Length();
					zXYZ += z2X * (2.0 * (z2X_mag - melt) / (z2X_mag + .00000001));
					aux.color += fractal->mandelbox.color.factor.y;
				}
			}
		}
		else
		{
			// Only mirror plane possible but still need to check for melt.
			CVector3 z2X = X_m - zXYZ;
			if (z2X.Dot(z2X) > (melt * melt))
			{
				double z2X_mag = z2X.Length();
				zXYZ += z2X * (2.0 * (z2X_mag - melt) / (z2X_mag + .00000001));
				aux.color += fractal->mandelbox.color.factor.x;
			}
		}
	} // outside solid

	double r2 = zXYZ.Dot(zXYZ);

	z = CVector4(zXYZ, z.w);

	z += fractal->mandelbox.offset;

	if (r2 < fractal->mandelbox.mR2)
	{
		z *= fractal->mandelbox.mboxFactor1;
		aux.DE *= fractal->mandelbox.mboxFactor1;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < fractal->mandelbox.fR2)
	{
		double tglad_factor2 = fractal->mandelbox.fR2 / r2;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}

	z -= fractal->mandelbox.offset;

	if (fractal->mandelbox.mainRotationEnabled)
	{
		z = fractal->mandelbox.mainRot.RotateVector(z);
	}

	z *= fractal->mandelbox.scale;
	aux.DE = aux.DE * fabs(fractal->mandelbox.scale) + 1.0;
}

void BoxFolding(CVector4 &z, const sFractalFoldings *foldings, sExtendedAux &aux)
{
	if (z.x > foldings->boxLimit)
	{
		z.x = foldings->boxValue - z.x;
		aux.color *= 0.9;
	}
	else if (z.x < -foldings->boxLimit)
	{
		z.x = -foldings->boxValue - z.x;
		aux.color *= 0.9;
	}
	if (z.y > foldings->boxLimit)
	{
		z.y = foldings->boxValue - z.y;
		aux.color *= 0.9;
	}
	else if (z.y < -foldings->boxLimit)
	{
		z.y = -foldings->boxValue - z.y;
		aux.color *= 0.9;
	}
	if (z.z > foldings->boxLimit)
	{
		z.z = foldings->boxValue - z.z;
		aux.color *= 0.9;
	}
	else if (z.z < -foldings->boxLimit)
	{
		z.z = -foldings->boxValue - z.z;
		aux.color *= 0.9;
	}
}

void SphericalFolding(CVector4 &z, const sFractalFoldings *foldings, sExtendedAux &aux)
{
	double fR2_2 = foldings->sphericalOuter * foldings->sphericalOuter;
	double mR2_2 = foldings->sphericalInner * foldings->sphericalInner;
	double r2_2 = aux.r * aux.r;
	double fold_factor1_2 = fR2_2 / mR2_2;

	if (r2_2 < mR2_2)
	{
		z = z * fold_factor1_2;
		aux.DE *= fold_factor1_2;
		aux.r_dz *= sqrt(fold_factor1_2);
		aux.color *= 0.9;
	}
	else if (r2_2 < fR2_2)
	{
		double fold_factor2_2 = fR2_2 / r2_2;
		z = z * fold_factor2_2;
		aux.DE *= fold_factor2_2;
		aux.r_dz *= sqrt(fold_factor1_2);
		aux.color *= 0.9;
	}
}

/**
 * quadratic iteration in imaginary scator algebra
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * ix-possibly-the-holy-grail-fractal-%28in-fff-lore%29
 *            http://luz.izt.uam.mx/index.html/?q=node/95&language=en
 * @author Manuel Fernandez-Guasti
 */
void ImaginaryScatorPower2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	double x2 = z.x * z.x; //+ 1e-061
	double y2 = z.y * z.y;
	double z2 = z.z * z.z;

	double newx = x2 - y2 - z2 + (y2 * z2) / x2;
	double newy = 2.0 * z.x * z.y * (1.0 - z2 / x2);
	double newz = 2.0 * z.x * z.z * (1.0 - y2 / x2);

	z.x = newx;
	z.y = newy;
	z.z = newz;
}

// NEW TRANSFORM FORMULAS-----------------------------------------------------------------

/**
 * Adds c constant to z vector
 */
void TransfAddConstantIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	z += fractal->transformCommon.additionConstant000;
	z += fractal->transformCommon.additionConstantA000;
}

/**
 * Adds c constant to z vector. C addition constant varies based on iteration parameters.
 */
void TransfAddConstantVaryV1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 tempVC = fractal->transformCommon.additionConstant000; // constant to be varied
	if (aux.i < fractal->transformCommon.startIterations250)
	{
		;
	}
	if (aux.i >= fractal->transformCommon.startIterations250
			&& aux.i < fractal->transformCommon.stopIterations
			&& (fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250
					 != 0))
	{
		tempVC =
			(tempVC
				+ fractal->transformCommon.offset000 * (aux.i - fractal->transformCommon.startIterations250)
						/ (fractal->transformCommon.stopIterations
								- fractal->transformCommon.startIterations250));
	}
	if (aux.i >= fractal->transformCommon.stopIterations)
	{
		tempVC = (tempVC + fractal->transformCommon.offset000);
	}
	z += tempVC;
}

/**
 * Adds Cpixel constant to z vector
 */
void TransfAddCpixelIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	z += aux.const_c * fractal->transformCommon.constantMultiplier111;
}

/**
 * Adds Cpixel constant to z vector, swapping the Cpixel vector x and y axes
 * disable swap for normal mode
 */
void TransfAddCpixelCxCyAxisSwapIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	CVector4 tempC = c;
	if (fractal->transformCommon.functionEnabled)
	{
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w);
			aux.c = tempC;
		}
		else
		{
			tempC = CVector4(c.y, c.x, c.z, c.w);
		}
	}
	z += tempC * fractal->transformCommon.constantMultiplier111;
}

/**
 * Adds Cpixel constant to z vector. Possible to swap Cpixel vector axes.
 */
void TransfAddCpixelAxisSwapIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	CVector4 tempC = c;
	if (fractal->transformCommon.alternateEnabledFalse) // alternate
	{
		tempC = aux.c;
		switch (fractal->mandelbulbMulti.orderOfXYZ)
		{
			case multi_OrderOfXYZ_xyz:
			default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
			case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
			case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
			case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
			case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
			case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
		}
		aux.c = tempC;
	}
	else
	{
		switch (fractal->mandelbulbMulti.orderOfXYZ)
		{
			case multi_OrderOfXYZ_xyz:
			default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
			case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
			case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
			case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
			case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
			case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
		}
	}
	z += tempC * fractal->transformCommon.constantMultiplier111;
}

/**
 * Adds Cpixel constant to z vector, with symmetry
 */
void TransfAddCpixelSymmetricalIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 tempFAB = aux.const_c;
	if (fractal->transformCommon.functionEnabledx) tempFAB.x = fabs(tempFAB.x);
	if (fractal->transformCommon.functionEnabledy) tempFAB.y = fabs(tempFAB.y);
	if (fractal->transformCommon.functionEnabledz) tempFAB.z = fabs(tempFAB.z);

	tempFAB *= fractal->transformCommon.constantMultiplier111;

	z.x += sign(z.x) * tempFAB.x;
	z.y += sign(z.y) * tempFAB.y;
	z.z += sign(z.z) * tempFAB.z;
}

/**
 * Adds Cpixel constant to z, Cpixel scaled  based on variable iteration parameters.
 */
void TransfAddCpixelVaryV1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 tempVC = fractal->transformCommon.constantMultiplier111; // constant to be varied
	if (aux.i < fractal->transformCommon.startIterations250)
	{
		;
	}
	if (aux.i >= fractal->transformCommon.startIterations250
			&& aux.i < fractal->transformCommon.stopIterations
			&& (fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250
					 != 0))
	{
		tempVC =
			(tempVC
				+ fractal->transformCommon.offset000 * (aux.i - fractal->transformCommon.startIterations250)
						/ (fractal->transformCommon.stopIterations
								- fractal->transformCommon.startIterations250));
	}
	if (aux.i >= fractal->transformCommon.stopIterations)
	{
		tempVC = (tempVC + fractal->transformCommon.offset000);
	}
	z += aux.const_c * tempVC;
}

/**
 * Add exp2(z)
 */
void TransfAddExp2ZIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 tempZ = z;

	if (fractal->transformCommon.functionEnabledx)
	{
		if (fractal->transformCommon.functionEnabledAx)
		{
			tempZ.x = fabs(z.x);
			if (fractal->transformCommon.functionEnabledAxFalse)
			{
				tempZ.x = -tempZ.x;
			}
			tempZ.x = exp2(tempZ.x * fractal->transformCommon.constantMultiplier000.x) - 1.0;
			z.x += sign(z.x) * tempZ.x;
		}
		else
			z.x += exp2(tempZ.x * fractal->transformCommon.constantMultiplier000.x) - 1.0;
	}

	if (fractal->transformCommon.functionEnabledy)
	{
		if (fractal->transformCommon.functionEnabledAy)
		{
			tempZ.y = fabs(z.y);
			if (fractal->transformCommon.functionEnabledAyFalse)
			{
				tempZ.y = -tempZ.y;
			}
			tempZ.y = exp2(tempZ.y * fractal->transformCommon.constantMultiplier000.y) - 1.0;
			z.y += sign(z.y) * tempZ.y;
		}
		else
			z.y += exp2(tempZ.y * fractal->transformCommon.constantMultiplier000.y) - 1.0;
	}

	if (fractal->transformCommon.functionEnabledz)
	{
		if (fractal->transformCommon.functionEnabledAz)
		{
			tempZ.z = fabs(z.z);
			if (fractal->transformCommon.functionEnabledAzFalse)
			{
				tempZ.z = -tempZ.z;
			}
			tempZ.z = exp2(tempZ.z * fractal->transformCommon.constantMultiplier000.z) - 1.0;
			z.z += sign(z.z) * tempZ.z;
		}
		else
			z.z += exp2(tempZ.z * fractal->transformCommon.constantMultiplier000.z) - 1.0;
	}
	aux.DE *= fractal->analyticDE.scale1; // DE tweak

	// DE calc version
	if (fractal->transformCommon.functionEnabledBzFalse)
	{
		CVector4 tempS;
		if (fractal->transformCommon.functionEnabled)
		{
			tempS = fabs(z);
			if (fractal->transformCommon.functionEnabledFalse)
			{
				tempS *= -1.0;
			}
			CVector4 tempT = tempS * fractal->transformCommon.scale0;
			tempS.x = exp2(tempT.x) - 1.0;
			tempS.y = exp2(tempT.y) - 1.0;
			tempS.z = exp2(tempT.z) - 1.0;

			z.x += sign(z.x) * tempS.x;
			z.y += sign(z.y) * tempS.y;
			z.z += sign(z.z) * tempS.z;
		}
		else
		{
			CVector4 tempR = z * fractal->transformCommon.scale0;
			z.x += exp2(tempR.x) - 1.0;
			z.y += exp2(tempR.y) - 1.0;
			z.z += exp2(tempR.z) - 1.0;
		}
		aux.DE = aux.DE + exp2(aux.DE * fractal->transformCommon.scale0) - 1.0;
	}
}

/**
 * benesiT1  3D
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void TransfBenesiT1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
	z = CVector4(
		(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

	CVector4 temp = z;
	double tempL = temp.Length();
	z = fabs(z) * fractal->transformCommon.scale3D222;
	// if (tempL < 1e-21) tempL = 1e-21;
	double avgScale = z.Length() / tempL;
	aux.r_dz *= avgScale;
	aux.DE = aux.DE * avgScale + 1.0;

	if (fractal->transformCommon.rotationEnabled)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}

	tempXZ = (z.y + z.x) * SQRT_1_2;

	z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
		z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
	z = z - fractal->transformCommon.offset200;
}

/**
 * benesiT1Mod  3D based on benesiT1
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void TransfBenesiT1ModIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
	z = CVector4(
		(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

	CVector4 temp = z;
	double tempL = temp.Length();
	z = fabs(z) * fractal->transformCommon.scale3D333;
	// if (tempL < 1e-21) tempL = 1e-21;
	double avgScale = z.Length() / tempL;
	aux.r_dz *= avgScale;
	aux.DE = aux.DE * avgScale + 1.0;

	z = (fabs(z + fractal->transformCommon.additionConstant111)
			 - fabs(z - fractal->transformCommon.additionConstant111) - z);

	if (fractal->transformCommon.rotationEnabled)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}

	tempXZ = (z.y + z.x) * SQRT_1_2;

	z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
		z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
}

/**
 * benesi T2
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void TransfBenesiT2Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
	z = CVector4(
		(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

	CVector4 tempV2 = z;
	tempV2.x = sqrt(z.y * z.y + z.z * z.z);
	tempV2.y = sqrt(z.x * z.x + z.z * z.z); // switching, squared, sqrt
	tempV2.z = sqrt(z.x * z.x + z.y * z.y);

	z = fabs(tempV2 - fractal->transformCommon.additionConstant111);

	CVector4 temp = z;
	double tempL = temp.Length();
	z = fabs(z) * fractal->transformCommon.scale3D444;
	// if (tempL < 1e-21) tempL = 1e-21;
	double avgScale = z.Length() / tempL;
	aux.r_dz *= avgScale;
	aux.DE = aux.DE * avgScale + 1.0;

	if (fractal->transformCommon.rotationEnabled)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}

	tempXZ = (z.y + z.x) * SQRT_1_2;
	z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
		z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
}

/**
 * benesi T3
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void TransfBenesiT3Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
	z = CVector4(
		(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

	CVector4 tempV2 = z;
	tempV2.x = (z.y + z.z);
	tempV2.y = (z.x + z.z); // switching
	tempV2.z = (z.x + z.y);
	z = (fabs(tempV2 - fractal->transformCommon.additionConstant222))
			* fractal->transformCommon.scale3D222;

	if (fractal->transformCommon.rotationEnabled)
	{
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);
	}

	tempXZ = (z.y + z.x) * SQRT_1_2;
	z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
		z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
}

/**
 * benesi T4
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void TransfBenesiT4Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
	z = CVector4(
		(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

	CVector4 tempV2 = z;
	tempV2.x = (z.y * z.y + z.z * z.z);
	tempV2.y = (z.x * z.x + z.z * z.z); // switching, squared,
	tempV2.z = (z.x * z.x + z.y * z.y);
	z = (fabs(tempV2 - fractal->transformCommon.additionConstant111))
			* fractal->transformCommon.scale3D222;

	if (fractal->transformCommon.rotationEnabled)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	tempXZ = (z.y + z.x) * SQRT_1_2;
	z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
		z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
}

/**
 * benesi T5
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/
 * do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void TransfBenesiT5bIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
	z = CVector4(
		(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);

	// if (z.x > -1e-21 && z.x < 1e-21)
	// z.x = (z.x > 0) ? 1e-21 : -1e-21;
	// if (z.y > -1e-21 && z.y < 1e-21)
	// z.y = (z.y > 0) ? 1e-21 : -1e-21;
	// if (z.z > -1e-21 && z.z < 1e-21)
	// z.z = (z.z > 0) ? 1e-21 : -1e-21;
	CVector4 tempV2 = z;
	tempV2.x =
		fabs(pow(pow(z.y, fractal->transformCommon.int8X) + pow(z.z, fractal->transformCommon.int8X),
			fractal->transformCommon.power025.x));
	tempV2.y =
		fabs(pow(pow(z.x, fractal->transformCommon.int8Y) + pow(z.z, fractal->transformCommon.int8Y),
			fractal->transformCommon.power025.y));
	tempV2.z =
		fabs(pow(pow(z.x, fractal->transformCommon.int8Z) + pow(z.y, fractal->transformCommon.int8Z),
			fractal->transformCommon.power025.z));
	z = (fabs(tempV2 - fractal->transformCommon.offsetC111)) * fractal->transformCommon.scale3Dd222;

	if (fractal->transformCommon.rotationEnabled)
		z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	tempXZ = (z.y + z.x) * SQRT_1_2;
	z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
		z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
}

/**
 * benesi mag forward
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void TransfBenesiMagForwardIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	double tempXZ = z.x * SQRT_2_3 - z.z * SQRT_1_3;
	z = CVector4(
		(tempXZ - z.y) * SQRT_1_2, (tempXZ + z.y) * SQRT_1_2, z.x * SQRT_1_3 + z.z * SQRT_2_3, z.w);
}

/**
 * benesi mag backward
 * @reference
 * http://www.fractalforums.com/new-theories-and-research/do-m3d-formula-have-to-be-distance-estimation-formulas/
 */
void TransfBenesiMagBackwardIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	double tempXZ = (z.y + z.x) * SQRT_1_2;
	z = CVector4(z.z * SQRT_1_3 + tempXZ * SQRT_2_3, (z.y - z.x) * SQRT_1_2,
		z.z * SQRT_2_3 - tempXZ * SQRT_1_3, z.w);
}

/**
 * Benesi Cube to sphere transform
 * Warps a cube to a sphere; transform made by M.Benesi, optimized by Luca.
 * @reference http://www.fractalforums.com/mathematics/circle2square/
 */
void TransfBenesiCubeSphereIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	z *= z; // so all now positive

	// if (z.x == 0.0)
	//	z.x = 1e-21;
	// if (z.z == 0.0)
	//	z.z = 1e-21;

	double rCyz = z.y / z.z;

	double rCxyz = (z.y + z.z) / z.x;

	if (rCxyz == -1.0) z.z = 1.0; //+ 1e-21
	if (rCyz < 1.0)
		rCyz = sqrt(rCyz + 1.0);
	else
		rCyz = sqrt(1.0 / rCyz + 1.0);

	if (rCxyz < 1.0)
		rCxyz = sqrt(rCxyz + 1.0);
	else
		rCxyz = sqrt(1.0 / rCxyz + 1.0);

	z.y *= rCyz;
	z.z *= rCyz;

	z *= rCxyz / SQRT_3_2;
}

/**
 * Benesi Sphere to Cube transform
 * Warps a sphere to a cube; transform made by M.Benesi, optimized by
 * Luca.  Scavenged and edited from code optimized by Luca.
 * @reference http://www.fractalforums.com/mathematics/circle2square/
 */
void TransfBenesiSphereCubeIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(fractal);
	Q_UNUSED(aux);

	z *= z;
	// if (z.z == 0.0) z.z = 1e-21;
	double rCyz = z.y / z.z;
	if (rCyz < 1.0)
		rCyz = 1.0 / sqrt(rCyz + 1.0);
	else
		rCyz = 1.0 / sqrt(1.0 / rCyz + 1.0);

	z.y *= rCyz;
	z.z *= rCyz;

	// if (z.x == 0.0) z.x = 1e-21;
	double rCxyz = (z.y * z.y + z.z * z.z) / z.x;

	if (rCxyz < 1.0)
		rCxyz = 1.0 / sqrt(rCxyz + 1.0);
	else
		rCxyz = 1.0 / sqrt(1.0 / rCxyz + 1.0);

	z *= rCxyz * SQRT_3_2;
}

/**
 * Box Fold
 */
void TransfBoxFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fabs(z.x) > fractal->mandelbox.foldingLimit)
	{
		z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
		aux.color += fractal->mandelbox.color.factor.x;
	}
	if (fabs(z.y) > fractal->mandelbox.foldingLimit)
	{
		z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
		aux.color += fractal->mandelbox.color.factor.y;
	}
	double zLimit = fractal->mandelbox.foldingLimit * fractal->transformCommon.scale1;
	double zValue = fractal->mandelbox.foldingValue * fractal->transformCommon.scale1;
	if (fabs(z.z) > zLimit)
	{
		z.z = sign(z.z) * zValue - z.z;
		aux.color += fractal->mandelbox.color.factor.z;
	}
}

/**
 * Box Fold VaryV1. Varies folding limit based on iteration conditions
 */
void TransfBoxFoldVaryV1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double limit = fractal->mandelbox.foldingLimit;
	// double value = 2.0 *fractal->mandelbox.foldingLimit;
	double tempVC = limit; // constant to be varied

	if (aux.i >= fractal->transformCommon.startIterations250
			&& aux.i < fractal->transformCommon.stopIterations
			&& (fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250
					 != 0))
	{
		tempVC =
			(tempVC
				+ fractal->transformCommon.offset * (aux.i - fractal->transformCommon.startIterations250)
						/ (fractal->transformCommon.stopIterations
								- fractal->transformCommon.startIterations250));
	}
	if (aux.i >= fractal->transformCommon.stopIterations)
	{
		tempVC = (tempVC + fractal->transformCommon.offset);
	}

	limit = tempVC;
	double value = fractal->transformCommon.scale2 * limit;

	if (z.x > limit)
	{
		z.x = value - z.x;
		aux.color += fractal->mandelbox.color.factor.x;
	}
	else if (z.x < -limit)
	{
		z.x = -value - z.x;
		aux.color += fractal->mandelbox.color.factor.x;
	}
	if (z.y > limit)
	{
		z.y = value - z.y;
		aux.color += fractal->mandelbox.color.factor.y;
	}
	else if (z.y < -limit)
	{
		z.y = -value - z.y;
		aux.color += fractal->mandelbox.color.factor.y;
	}
	double zLimit = limit * fractal->transformCommon.scale1;
	double zValue = value * fractal->transformCommon.scale1;
	if (z.z > zLimit)
	{
		z.z = zValue - z.z;
		aux.color += fractal->mandelbox.color.factor.z;
	}
	else if (z.z < -zLimit)
	{
		z.z = -zValue - z.z;
		aux.color += fractal->mandelbox.color.factor.z;
	}
	aux.DE *= fractal->analyticDE.scale1;
}

/**
 * Box Fold XYZ, set different folding parameters for each axis
 */
void TransfBoxFoldXYZIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (z.x > fractal->transformCommon.additionConstant111.x)
	{
		z.x = fractal->transformCommon.additionConstant222.x - z.x;
		aux.color += fractal->mandelbox.color.factor.x;
	}
	else if (z.x < -fractal->transformCommon.additionConstant111.x)
	{
		z.x = -fractal->transformCommon.additionConstant222.x - z.x;
		aux.color += fractal->mandelbox.color.factor.x;
	}
	if (z.y > fractal->transformCommon.additionConstant111.y)
	{
		z.y = fractal->transformCommon.additionConstant222.y - z.y;
		aux.color += fractal->mandelbox.color.factor.y;
	}
	else if (z.y < -fractal->transformCommon.additionConstant111.y)
	{
		z.y = -fractal->transformCommon.additionConstant222.y - z.y;
		aux.color += fractal->mandelbox.color.factor.y;
	}
	if (z.z > fractal->transformCommon.additionConstant111.z)
	{
		z.z = fractal->transformCommon.additionConstant222.z - z.z;
		aux.color += fractal->mandelbox.color.factor.z;
	}
	else if (z.z < -fractal->transformCommon.additionConstant111.z)
	{
		z.z = -fractal->transformCommon.additionConstant222.z - z.z;
		aux.color += fractal->mandelbox.color.factor.z;
	}
}

/**
 * Box Offset, set different offset for each axis, added symmetrically about the origin
 * will cause cuts along axis
 */
void TransfBoxOffsetIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 temp = z;
	z.x = sign(z.x) * fractal->transformCommon.additionConstant000.x + z.x;
	z.y = sign(z.y) * fractal->transformCommon.additionConstant000.y + z.y;
	z.z = sign(z.z) * fractal->transformCommon.additionConstant000.z + z.z;

	if (fractal->transformCommon.functionEnabledFalse)
	{
		double tempL = temp.Length();
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale + 1.0;
	}
}

/**
 * fabs add  constant,  z = fabs( z + constant)
 */
void TransfFabsAddConstantIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	z += fractal->transformCommon.additionConstant000;

	if (fractal->transformCommon.functionEnabledx) z.x = fabs(z.x);

	if (fractal->transformCommon.functionEnabledy) z.y = fabs(z.y);

	if (fractal->transformCommon.functionEnabledz) z.z = fabs(z.z);

	z += fractal->transformCommon.offsetA000;
}

/**
 * fabs. Add fabs constantV2,  z = fabs( z + constant) - fabs( z - constant) - z:
 * tglad's fold, with a fold tweak option
 */
void TransfFabsAddTgladFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	z = fabs(z + fractal->transformCommon.additionConstant000)
			- fabs(z - fractal->transformCommon.additionConstant000) - z;

	if (fractal->transformCommon.functionEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		CVector4 limit = fractal->transformCommon.additionConstant000;
		CVector4 length = 2.0 * limit;
		CVector4 tgladS = 1.0 / length;
		CVector4 Add;
		if (fabs(z.x) < limit.x) Add.x = z.x * z.x * tgladS.x;
		if (fabs(z.y) < limit.y) Add.y = z.y * z.y * tgladS.y;
		if (fabs(z.z) < limit.z) Add.z = z.z * z.z * tgladS.z;

		if (fabs(z.x) > limit.x && fabs(z.x) < length.x)
			Add.x = (length.x - fabs(z.x)) * (length.x - fabs(z.x)) * tgladS.x;
		if (fabs(z.y) > limit.y && fabs(z.y) < length.y)
			Add.y = (length.y - fabs(z.y)) * (length.y - fabs(z.y)) * tgladS.y;
		if (fabs(z.z) > limit.z && fabs(z.z) < length.z)
			Add.z = (length.z - fabs(z.z)) * (length.z - fabs(z.z)) * tgladS.z;
		Add *= fractal->transformCommon.offset000;
		z.x = (z.x - (sign(z.x) * (Add.x)));
		z.y = (z.y - (sign(z.y) * (Add.y)));
		z.z = (z.z - (sign(z.z) * (Add.z)));
	}
}

/**
* fabs add conditional
*/
void TransfFabsAddConditionalIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledx)
	{
		z.x = sign(z.x) * (fractal->transformCommon.offset111.x - fabs(z.x)
												+ fabs(z.x) * fractal->transformCommon.offset000.x);
	}

	if (fractal->transformCommon.functionEnabledy)
	{
		z.y = sign(z.y) * (fractal->transformCommon.offset111.y - fabs(z.y)
												+ fabs(z.y) * fractal->transformCommon.offset000.y);
	}

	if (fractal->transformCommon.functionEnabledz)
	{
		z.z = sign(z.z) * (fractal->transformCommon.offset111.z - fabs(z.z)
												+ fabs(z.z) * fractal->transformCommon.offset000.z);

	}																			// aux.DE = aux.DE * l/L;
	aux.DE *= fractal->analyticDE.scale1; // DE tweak
}

/**
 * fabs. Negative fabs constant,  z = - fabs( z + constant)
 */
void TransfNegFabsAddConstantIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	z += fractal->transformCommon.additionConstant000;
	if (fractal->transformCommon.functionEnabledx) z.x = -fabs(z.x);

	if (fractal->transformCommon.functionEnabledy) z.y = -fabs(z.y);

	if (fractal->transformCommon.functionEnabledz) z.z = -fabs(z.z);
}

/**
 * fabs Multi - Multiple parameters for fabs functions
 */
void TransfFabsAddMultiIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	CVector4 tempA, tempB;

	if (fractal->transformCommon.functionEnabledAx)
		tempA.x = fabs(z.x + fractal->transformCommon.additionConstant000.x);

	if (fractal->transformCommon.functionEnabledx)
		tempB.x = fabs(z.x - fractal->transformCommon.offset000.x);

	z.x = tempA.x - tempB.x - (z.x * fractal->transformCommon.scale3D111.x);

	if (fractal->transformCommon.functionEnabledAy)
		tempA.y = fabs(z.y + fractal->transformCommon.additionConstant000.y);

	if (fractal->transformCommon.functionEnabledy)
		tempB.y = fabs(z.y - fractal->transformCommon.offset000.y);

	z.y = tempA.y - tempB.y - (z.y * fractal->transformCommon.scale3D111.y);

	if (fractal->transformCommon.functionEnabledAz)
		tempA.z = fabs(z.z + fractal->transformCommon.additionConstant000.z);

	if (fractal->transformCommon.functionEnabledz)
		tempB.z = fabs(z.z - fractal->transformCommon.offset000.z);

	z.z = tempA.z - tempB.z - (z.z * fractal->transformCommon.scale3D111.z);

	z += fractal->transformCommon.offsetA000;
}

/**
 * folding tetra3D from M3D (Luca GN 2011):
 * Code taken from the forums, KIFS original thread
 * side note - if you disable the 1st half, 2nd half will be
 * done even if you disable it... (to avoid a NOP transform)
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfFoldingTetra3dIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	double x1;
	double y1;
	if (fractal->transformCommon.functionEnabledx)
	{
		if (z.x + z.y < 0.0)
		{
			x1 = -z.y;
			z.y = -z.x;
			z.x = x1;
		}
		if (z.x + z.z < 0.0)
		{
			x1 = -z.z;
			z.z = -z.x;
			z.x = x1;
		}
		if (z.y + z.z < 0.0)
		{
			y1 = -z.z;
			z.z = -z.y;
			z.y = y1;
		}
	}
	if (fractal->transformCommon.functionEnabledy)
	{
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
	}
}

/**
 * iteration weight. Influence fractal based on the weight of
 * Z values after different iterations
 */
void TransfIterationWeightIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 zA = (aux.i == fractal->transformCommon.intA) ? z : CVector4();
	CVector4 zB = (aux.i == fractal->transformCommon.intB) ? z : CVector4();

	z = (z * fractal->transformCommon.scale) + (zA * fractal->transformCommon.offset)
			+ (zB * fractal->transformCommon.offset0);
	aux.DE *= fractal->transformCommon.scale;
	aux.r_dz *= fractal->transformCommon.scale;
}

/**
 * Inverse cylindrical coordinates, very easy transform
 * Formula by Luca GN 2011
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfInvCylindricalIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double newZx = z.x;
	double newZy = z.y;

	if (fractal->transformCommon.functionEnabledFalse) newZx = z.x * cos(z.y);
	if (fractal->transformCommon.functionEnabledxFalse) newZy = z.x * sin(z.y);

	z = CVector4(z.x * cos(newZy * fractal->transformCommon.scale1),
				newZx * sin(z.y * fractal->transformCommon.scale1), z.z * fractal->transformCommon.scaleC1,
				z.w)
			* fractal->transformCommon.scaleA1;

	aux.DE = aux.DE * fabs(fractal->transformCommon.scaleA1) * fractal->transformCommon.scaleB1
					 + fractal->transformCommon.offset1;
	aux.r_dz *= fabs(fractal->transformCommon.scaleA1) * fractal->transformCommon.scaleB1;
}

/**
 * Linear Combine transform from Mandelbulb3D.
 * Can create multiple combination for the addition of Cpixel
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfLinCombineCXYZIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	CVector4 temp = c;
	CVector4 mulX = fractal->transformCommon.constantMultiplier100;
	CVector4 mulY = fractal->transformCommon.constantMultiplier010;
	CVector4 mulZ = fractal->transformCommon.constantMultiplier001;
	c.x = mulX.x * temp.x + mulX.y * temp.y + mulX.z * temp.z;
	c.y = mulY.x * temp.x + mulY.y * temp.y + mulY.z * temp.z;
	c.z = mulZ.x * temp.x + mulZ.y * temp.y + mulZ.z * temp.z;

	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplier111;
	}
}
/**
 * Transform Menger Fold
 * Menger Sponge formula created by Knighty
 * @reference
 * http://www.fractalforums.com/ifs-iterated-function-systems/kaleidoscopic-(escape-time-ifs)/
 */
void TransfMengerFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledx)
	{ // fabs() and menger fold
		z = fabs(z + fractal->transformCommon.additionConstantA000);
		if (z.x - z.y < 0.0) swap(z.y, z.x);
		if (z.x - z.z < 0.0) swap(z.z, z.x);
		if (z.y - z.z < 0.0) swap(z.z, z.y);
	}
	if (fractal->transformCommon.functionEnabledy)
	{ // menger scales and offsets
		z *= fractal->transformCommon.scale3;
		z.x -= 2.0 * fractal->transformCommon.constantMultiplier111.x;
		z.y -= 2.0 * fractal->transformCommon.constantMultiplier111.y;
		if (fractal->transformCommon.functionEnabled)
		{
			if (z.z > 1.0) // z.z < 1.0  z.z = z.z
				z.z -= 2.0 * fractal->transformCommon.constantMultiplier111.z;
		}
		else
		{
			z.z -= 2.0 * fractal->transformCommon.constantMultiplier111.z;
		}
		aux.DE *= fractal->transformCommon.scale3;
	}
}

/**
 * multiple angle
 */
void TransfMultipleAngleIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double th0 = asin(z.z / aux.r);
	double ph0 = atan2(z.y, z.x);
	double th = th0 * fractal->transformCommon.multiplication;
	double ph = ph0 * fractal->transformCommon.multiplication;
	double cth = cos(th);
	z = CVector4(cth * cos(ph), cth * sin(ph), sin(th), 0.0) * aux.r;
}

/**
 * Octo
 */
void TransfOctoFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (z.x + z.y < 0.0) z = CVector4(-z.y, -z.x, z.z, z.w);

	if (z.x + z.z < 0.0) // z.xz = -z.zx;
		z = CVector4(-z.z, z.y, -z.x, z.w);

	if (z.x - z.y < 0.0) // z.xy = z.yx;
		z = CVector4(z.y, z.x, z.z, z.w);

	if (z.x - z.z < 0.0) // z.xz = z.zx;
		z = CVector4(z.z, z.y, z.x, z.w);

	z.x = fabs(z.x);
	z = z * fractal->transformCommon.scale2
			- fractal->transformCommon.offset100 * (fractal->transformCommon.scale2 - 1.0);

	aux.DE *= fractal->transformCommon.scale2;
}

/**
 * Parabolic Fold by mclarekin
 */
void TransfParabFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 temp = z;

	CVector4 slope2 = fractal->transformCommon.constantMultiplier111;
	CVector4 length = fractal->transformCommon.additionConstant000;
	CVector4 factorP;

	factorP.x = -slope2.x / (length.x * 2.0);
	factorP.y = -slope2.y / (length.y * 2.0);
	factorP.z = -slope2.z / (length.z * 2.0);

	if (temp.x > 0)
	{
		if (temp.x < length.x)
		{
			z.x = z.x * z.x * factorP.x;
		}
		else
		{
			z.x = (z.x - length.x / 2.0) * slope2.x;
		}
	}
	if (temp.y > 0)
	{
		if (temp.y < length.y)
		{
			z.y = z.y * z.y * factorP.y;
		}
		else
		{
			z.y = (z.y - length.y / 2.0) * slope2.y;
		}
	}
	if (temp.z > 0)
	{
		if (temp.z < length.z)
		{
			z.z = z.z * z.z * factorP.z;
		}
		else
		{
			z.z = (z.z - length.z / 2.0) * slope2.z;
		}
	}

	if (temp.x < 0)
	{
		if (temp.x > -length.x)
		{
			z.x = z.x * z.x * -factorP.x;
		}
		else
		{
			z.x = (z.x + length.x / 2.0) * slope2.x;
		}
	}
	if (temp.y < 0)
	{
		if (temp.y > -length.y)
		{
			z.y = z.y * z.y * -factorP.y;
		}
		else
		{
			z.y = (z.y + length.y / 2.0) * slope2.y;
		}
	}

	if (temp.z < 0)
	{

		if (temp.z > -length.z)
		{
			z.z = z.z * z.z * -factorP.z;
		}
		else
		{
			z.z = (z.z + length.z / 2.0) * slope2.z;
		}
	}

	// z.x = sign(temp.x) * z.x;
	// z.y = sign(temp.y) * z.y;
	// z.z = sign(temp.z) * z.z;

	if (fractal->transformCommon.functionEnabledxFalse)
	{
		z = (z - temp) * fractal->transformCommon.scale3D111;
	}
	if (fractal->transformCommon.functionEnabledyFalse)
	{
		z = (z - temp) * temp * fractal->transformCommon.scale3D111;

		z.x = sign(temp.x) * z.x;
		z.y = sign(temp.y) * z.y;
		z.z = sign(temp.z) * z.z;
	}
	if (fractal->transformCommon.functionEnabledzFalse)
	{
		z = (z - temp) * temp * fractal->transformCommon.scale3D111;
	}

	if (fractal->transformCommon.functionEnabledFalse)
	{
		double tempL = temp.Length();
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = z.Length() / tempL;
		aux.r_dz *= avgScale;
		aux.DE = aux.DE * avgScale;
	}
	aux.DE = aux.DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0;
}

/**
 * platonic solid
 * @reference
 * http://www.fractalforums.com/3d-fractal-generation/platonic-dimensions/msg36528/#msg36528
 */
void TransfPlatonicSolidIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	double rho = sqrt(z.Length()); // the radius
	double theta =
		cos(fractal->platonicSolid.frequency * z.x) * sin(fractal->platonicSolid.frequency * z.y)
		+ cos(fractal->platonicSolid.frequency * z.y) * sin(fractal->platonicSolid.frequency * z.z)
		+ cos(fractal->platonicSolid.frequency * z.z) * sin(fractal->platonicSolid.frequency * z.x);
	double r = theta * fractal->platonicSolid.amplitude + rho * fractal->platonicSolid.rhoMul;
	z *= r;
}

/**
 * z(n+1) = z(n) * abs(z(n)) ^ p-1
 */
void TransfRPowerIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double rp = pow(aux.r, fractal->bulb.power - 1.0);
	aux.r_dz = rp * aux.r_dz * fractal->bulb.power + 1.0;
	z *= rp;
	aux.DE *= rp;
}

/**
 * z = (Az^2 + Bz + C ) / D; ((VectA +ScaleA * z) * fn(z) + scaleB * z + VectC ) * ScaleD
 */
void TransfPwr2PolynomialIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 partA = z;
	if (fractal->transformCommon.functionEnabledFalse) // fabs
		partA = fabs(z);
	if (fractal->transformCommon.functionEnabledxFalse) // pwr3 or z * fabs(z^2)
		partA *= z;
	partA = partA * fractal->transformCommon.scale2 + fractal->transformCommon.constantMultiplier111;

	CVector4 fnZ1 = z;
	if (fractal->transformCommon.functionEnabledBxFalse) // cos(z*Pi)
	{
		double scalePi = M_PI * fractal->transformCommon.scaleC1;
		fnZ1.x = cos(z.x * scalePi);
		fnZ1.y = cos(z.y * scalePi);
		fnZ1.z = cos(z.z * scalePi);
	}
	if (fractal->transformCommon.functionEnabledyFalse) // pi rotation
		fnZ1 = fnZ1.RotateAroundVectorByAngle(fractal->transformCommon.constantMultiplier111.GetXYZ(),
			M_PI * fractal->transformCommon.scale0);				// * cPI ;
	if (fractal->transformCommon.functionEnabledzFalse) // box offset
	{
		fnZ1.x = fnZ1.x + sign(fnZ1.x) * fractal->transformCommon.additionConstant000.x;
		fnZ1.y = fnZ1.y + sign(fnZ1.y) * fractal->transformCommon.additionConstant000.y;
		fnZ1.z = fnZ1.z + sign(fnZ1.z) * fractal->transformCommon.additionConstant000.z;
	}

	if (fractal->transformCommon.functionEnabledAxFalse) // fabs fnZ1
		fnZ1 = fabs(fnZ1);

	CVector4 partB = z;
	if (fractal->transformCommon.functionEnabledAzFalse) partB = fabs(z);
	partB *= fractal->transformCommon.scale4;
	z = fractal->transformCommon.constantMultiplierB111 + partB - partA * fnZ1;
	z *= fractal->transformCommon.scale025;

	aux.DE = aux.DE * 4.0 * fractal->analyticDE.scaleLin + fractal->analyticDE.offsetLin;
}

/**
 * Quaternion Fold Transform
 * @reference http://www.fractalforums.com/3d-fractal-generation/true-3d-mandlebrot-type-fractal/
 */
void TransfQuaternionFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;
	aux.r = z.Length();
	z = CVector4(z.x * z.x - z.y * z.y - z.z * z.z, z.x * z.y, z.x * z.z, z.w);
	if (fractal->transformCommon.functionEnabledFalse)
	{

		aux.r_dz = aux.r_dz * 2.0 * aux.r;
		double tempL = z.Length();
		z *= fractal->transformCommon.constantMultiplier122;
		// if (tempL < 1e-21) tempL = 1e-21;
		double avgScale = CVector4(z.x, z.y / 2.0, z.z / 2.0, z.w).Length() / tempL;
		double tempAux = aux.r_dz * avgScale;
		aux.r_dz = aux.r_dz + (tempAux - aux.r_dz) * fractal->transformCommon.scaleA1;
		z += fractal->transformCommon.additionConstant000;
	}
	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		CVector4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux.c;
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(tempC.x, tempC.y, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(tempC.x, tempC.z, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(tempC.y, tempC.x, tempC.z, tempC.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(tempC.y, tempC.z, tempC.x, tempC.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(tempC.z, tempC.x, tempC.y, tempC.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(tempC.z, tempC.y, tempC.x, tempC.w); break;
			}
			aux.c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZC)
			{
				case multi_OrderOfXYZ_xyz:
				default: tempC = CVector4(c.x, c.y, c.z, c.w); break;
				case multi_OrderOfXYZ_xzy: tempC = CVector4(c.x, c.z, c.y, c.w); break;
				case multi_OrderOfXYZ_yxz: tempC = CVector4(c.y, c.x, c.z, c.w); break;
				case multi_OrderOfXYZ_yzx: tempC = CVector4(c.y, c.z, c.x, c.w); break;
				case multi_OrderOfXYZ_zxy: tempC = CVector4(c.z, c.x, c.y, c.w); break;
				case multi_OrderOfXYZ_zyx: tempC = CVector4(c.z, c.y, c.x, c.w); break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplierC111;
	}
}

/**
 * Reciprocal3  based on Darkbeam's code from M3D,
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfReciprocal3Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 tempZ = z;

	if (fractal->transformCommon.functionEnabledx)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.x = (1.0 / fractal->transformCommon.offset111.x)
								- 1.0 / (fabs(z.x) + fractal->transformCommon.offset111.x);

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.x = (fractal->transformCommon.offsetA111.x)
								- 1.0 / (fabs(z.x) + fractal->transformCommon.offset111.x);

		if (fractal->transformCommon.functionEnabledBxFalse)
		{
			double M1 = fractal->transformCommon.scale1;
			double M2 = fractal->transformCommon.scaleA1;
			tempZ.x = (1.0 / fractal->transformCommon.offset111.x)
								+ (1.0 / fractal->transformCommon.offsetA111.x)
								- 1.0 / (fabs(z.x * M1) + fractal->transformCommon.offset111.x)
								- 1.0 / ((z.x * z.x * M2) + fractal->transformCommon.offsetA111.x);
		}
		if (fractal->transformCommon.functionEnabledCxFalse)
		{
			double M1 = fractal->transformCommon.scale1;
			double M2 = fractal->transformCommon.scaleA1;
			tempZ.x = fractal->transformCommon.offsetB111.x
								- 1.0 / (fabs(z.x * M1) + fractal->transformCommon.offset111.x)
								- 1.0 / ((z.x * z.x * M2) + fractal->transformCommon.offsetA111.x);
		}

		tempZ.x += fabs(z.x) * fractal->transformCommon.offset000.x; // function slope
		z.x = sign(z.x) * tempZ.x;
	}

	if (fractal->transformCommon.functionEnabledy)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.y = (1.0 / fractal->transformCommon.offset111.y)
								- 1.0 / (fabs(z.y) + fractal->transformCommon.offset111.y);

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.y = (fractal->transformCommon.offsetA111.y)
								- 1.0 / (fabs(z.y) + fractal->transformCommon.offset111.y);

		if (fractal->transformCommon.functionEnabledBxFalse)
		{
			double M1 = fractal->transformCommon.scale1;
			double M2 = fractal->transformCommon.scaleA1;
			tempZ.y = (1.0 / fractal->transformCommon.offset111.y)
								+ (1.0 / fractal->transformCommon.offsetA111.y)
								- 1.0 / (fabs(z.y * M1) + fractal->transformCommon.offset111.y)
								- 1.0 / ((z.y * z.y * M2) + fractal->transformCommon.offsetA111.y);
		}

		if (fractal->transformCommon.functionEnabledCxFalse)
		{
			double M1 = fractal->transformCommon.scale1;
			double M2 = fractal->transformCommon.scaleA1;
			tempZ.y = fractal->transformCommon.offsetB111.y
								- 1.0 / (fabs(z.y * M1) + fractal->transformCommon.offset111.y)
								- 1.0 / ((z.y * z.y * M2) + fractal->transformCommon.offsetA111.y);
		}
		tempZ.y += fabs(z.y) * fractal->transformCommon.offset000.y;
		z.y = sign(z.y) * tempZ.y;
	}

	if (fractal->transformCommon.functionEnabledz)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.z = (1.0 / fractal->transformCommon.offset111.z)
								- 1.0 / (fabs(z.z) + fractal->transformCommon.offset111.z);

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.z = (fractal->transformCommon.offsetA111.z)
								- 1.0 / (fabs(z.z) + fractal->transformCommon.offset111.z);

		if (fractal->transformCommon.functionEnabledBxFalse)
		{
			double M1 = fractal->transformCommon.scale1;
			double M2 = fractal->transformCommon.scaleA1;
			tempZ.z = (1.0 / fractal->transformCommon.offset111.z)
								+ (1.0 / fractal->transformCommon.offsetA111.z)
								- 1.0 / (fabs(z.z * M1) + fractal->transformCommon.offset111.z)
								- 1.0 / ((z.z * z.z * M2) + fractal->transformCommon.offsetA111.z);
		}
		if (fractal->transformCommon.functionEnabledCxFalse)
		{
			double M1 = fractal->transformCommon.scale1;
			double M2 = fractal->transformCommon.scaleA1;
			tempZ.z = fractal->transformCommon.offsetB111.z
								- 1.0 / (fabs(z.z * M1) + fractal->transformCommon.offset111.z)
								- 1.0 / ((z.z * z.z * M2) + fractal->transformCommon.offsetA111.z);
		}

		tempZ.z += fabs(z.z) * fractal->transformCommon.offset000.z;
		z.z = sign(z.z) * tempZ.z;
	}
	// aux.DE = aux.DE * l/L;
	aux.DE *= fractal->analyticDE.scale1; // DE tweak
}

/**
 * rotation
 */
void TransfRotationIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	z = fractal->transformCommon.rotationMatrix.RotateVector(z);
}

/**
 * rotation folding plane
 */
void TransfRotationFoldingPlaneIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 zRot;
	// cast vector to array pointer for address taking of components in opencl
	double *zRotP = (double *)&zRot;
	const double *colP = (const double *)&fractal->mandelbox.color.factor;
	for (int dim = 0; dim < 3; dim++)
	{
		// handle each dimension x, y and z sequentially in pointer var dim
		double *rotDim = (dim == 0) ? &zRotP[0] : ((dim == 1) ? &zRotP[1] : &zRotP[2]);
		const double *colorFactor = (dim == 0) ? &colP[0] : ((dim == 1) ? &colP[1] : &colP[2]);

		zRot = fractal->mandelbox.rot[0][dim].RotateVector(z);
		if (*rotDim > fractal->mandelbox.foldingLimit)
		{
			*rotDim = fractal->mandelbox.foldingValue - *rotDim;
			z = fractal->mandelbox.rotinv[0][dim].RotateVector(zRot);
			aux.color += *colorFactor;
		}
		else
		{
			zRot = fractal->mandelbox.rot[1][dim].RotateVector(z);
			if (*rotDim < -fractal->mandelbox.foldingLimit)
			{
				*rotDim = -fractal->mandelbox.foldingValue - *rotDim;
				z = fractal->mandelbox.rotinv[1][dim].RotateVector(zRot);
				aux.color += *colorFactor;
			}
		}
	}
}

/**
 * Rpow3 from M3D.
 * Does a power of 3 on the current length of the  vector.
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfRpow3Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double sqrRout = z.Dot(z) * fractal->transformCommon.scale;

	z *= sqrRout;
	aux.DE = aux.DE * fabs(sqrRout) + fractal->analyticDE.offset1;
}

/**
 * rotation variation v1. Rotation angles vary based on iteration parameters.
 */
void TransfRotationVaryV1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 tempVC = CVector4(fractal->transformCommon.rotation, 0.0); // constant to be varied

	if (aux.i >= fractal->transformCommon.startIterations250
			&& aux.i < fractal->transformCommon.stopIterations
			&& (fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250
					 != 0))
	{
		tempVC =
			(tempVC
				+ fractal->transformCommon.offset000 * (aux.i - fractal->transformCommon.startIterations250)
						/ (fractal->transformCommon.stopIterations
								- fractal->transformCommon.startIterations250));
	}
	if (aux.i >= fractal->transformCommon.stopIterations)
	{
		tempVC = (tempVC + fractal->transformCommon.offset000);
	}

	tempVC *= M_PI / 180.0;

	z = z.RotateAroundVectorByAngle(CVector3(1.0, 0.0, 0.0), tempVC.x);
	z = z.RotateAroundVectorByAngle(CVector3(0.0, 1.0, 0.0), tempVC.y);
	z = z.RotateAroundVectorByAngle(CVector3(0.0, 0.0, 1.0), tempVC.z);
}

/**
 * rotatedAbs & Rotated Folding transform from M3D
 * - Rotate by the given angles
 *- fold
 *- RotateBack by the given angles
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */

void TransfRotationFoldingIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	z = fractal->transformCommon.rotationMatrix.RotateVector(z);

	if (fractal->transformCommon.functionEnabled)
	{
		if (fractal->transformCommon.functionEnabledx)
			z.x = fabs(z.x + fractal->transformCommon.offset000.x) - fractal->transformCommon.offset000.x;
		if (fractal->transformCommon.functionEnabledy)
			z.y = fabs(z.y + fractal->transformCommon.offset000.y) - fractal->transformCommon.offset000.y;
		if (fractal->transformCommon.functionEnabledz)
			z.z = fabs(z.z + fractal->transformCommon.offset000.z) - fractal->transformCommon.offset000.z;
	}

	if (fractal->transformCommon.functionEnabledAyFalse)
	{
		if (fractal->transformCommon.functionEnabledAx)
			z.x = fabs(z.x + fractal->transformCommon.offset111.x)
						- fabs(z.x - fractal->transformCommon.offset111.x) - z.x;
		if (fractal->transformCommon.functionEnabledAy)
			z.y = fabs(z.y + fractal->transformCommon.offset111.y)
						- fabs(z.y - fractal->transformCommon.offset111.y) - z.y;
		if (fractal->transformCommon.functionEnabledAz)
			z.z = fabs(z.z + fractal->transformCommon.offset111.z)
						- fabs(z.z - fractal->transformCommon.offset111.z) - z.z;
	}

	if (fractal->transformCommon.functionEnabledAzFalse)
	{
		if (fractal->transformCommon.functionEnabledBx)
		{
			if (fabs(z.x) > fractal->mandelbox.foldingLimit)
			{
				z.x = sign(z.x) * fractal->mandelbox.foldingValue - z.x;
				// aux.color += fractal->mandelbox.color.factor.x;
			}
		}
		if (fractal->transformCommon.functionEnabledBy)
		{
			if (fabs(z.y) > fractal->mandelbox.foldingLimit)
			{
				z.y = sign(z.y) * fractal->mandelbox.foldingValue - z.y;
				// aux.color += fractal->mandelbox.color.factor.y;
			}
		}
		if (fractal->transformCommon.functionEnabledBz)
		{
			double zLimit = fractal->mandelbox.foldingLimit * fractal->transformCommon.scale1;
			double zValue = fractal->mandelbox.foldingValue * fractal->transformCommon.scale1;
			if (fabs(z.z) > zLimit)
			{
				z.z = sign(z.z) * zValue - z.z;
				// aux.color += fractal->mandelbox.color.factor.z;
			}
		}
	}
	z = fractal->transformCommon.rotationMatrix2.RotateVector(z);
}

/**
 * scale - simple scale
 */
void TransfScaleIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	z *= fractal->transformCommon.scale;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;
	aux.r_dz *= fabs(fractal->transformCommon.scale);
}

/**
 * scale vary Abox- DarkBeams
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfScaleVaryAboxIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		aux.actualScale =
			fractal->mandelbox.scale + fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);
		z *= aux.actualScale;
		aux.DE = aux.DE * fabs(aux.actualScale) + 1.0;
		aux.r_dz *= fabs(aux.actualScale);
	}
	else
	{
		z *= fractal->mandelbox.scale;
		aux.DE = aux.DE * fabs(fractal->mandelbox.scale) + 1.0;
		aux.r_dz *= fabs(fractal->mandelbox.scale);
	}
}

/**
 * scale variation v1. Scale varies based on iteration parameters.
 */
void TransfScaleVaryV1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double tempVC = fractal->transformCommon.scale; // constant to be varied

	if (aux.i >= fractal->transformCommon.startIterations250
			&& aux.i < fractal->transformCommon.stopIterations
			&& (fractal->transformCommon.stopIterations - fractal->transformCommon.startIterations250
					 != 0))
	{
		tempVC =
			(tempVC
				+ fractal->transformCommon.offset0 * (aux.i - fractal->transformCommon.startIterations250)
						/ (fractal->transformCommon.stopIterations
								- fractal->transformCommon.startIterations250));
	}
	if (aux.i >= fractal->transformCommon.stopIterations)
	{
		tempVC = (tempVC + fractal->transformCommon.offset0);
	}
	z *= tempVC;
	aux.DE = aux.DE * fabs(tempVC) + 1.0; // prepared for future analytic DE for hybrids
	aux.r_dz *= fabs(tempVC);
}
/**
 *  Scale varies smoothly based on iteration parameters.
 */
void TransfScaleVaryVCLIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double para = 0.0;

	if (fractal->Cpara.enabledLinear)
	{
		para = fractal->Cpara.para0; // parameter value at iter 0
		double temp0 = para;
		double tempA = fractal->Cpara.paraA;
		double tempB = fractal->Cpara.paraB;
		double tempC = fractal->Cpara.paraC;
		double lengthAB = fractal->Cpara.iterB - fractal->Cpara.iterA;
		double lengthBC = fractal->Cpara.iterC - fractal->Cpara.iterB;
		double grade1 = (tempA - temp0) / fractal->Cpara.iterA;
		double grade2 = (tempB - tempA) / lengthAB;
		double grade3 = (tempC - tempB) / lengthBC;

		// slopes
		if (aux.i < fractal->Cpara.iterA)
		{
			para = temp0 + (aux.i * grade1);
		}
		if (aux.i < fractal->Cpara.iterB && aux.i >= fractal->Cpara.iterA)
		{
			para = tempA + (aux.i - fractal->Cpara.iterA) * grade2;
		}
		if (aux.i >= fractal->Cpara.iterB)
		{
			para = tempB + (aux.i - fractal->Cpara.iterB) * grade3;
		}

		// Curvi part on "true"
		if (fractal->Cpara.enabledCurves)
		{
			double paraAdd = 0.0;
			double paraIt;
			if (lengthAB > 2.0 * fractal->Cpara.iterA) // stop  error, todo fix.
			{
				double curve1 = (grade2 - grade1) / (4.0 * fractal->Cpara.iterA);
				double tempL = lengthAB - fractal->Cpara.iterA;
				double curve2 = (grade3 - grade2) / (4.0 * tempL);
				if (aux.i < 2 * fractal->Cpara.iterA)
				{
					paraIt = tempA - fabs(tempA - aux.i);
					paraAdd = paraIt * paraIt * curve1;
				}
				if (aux.i >= 2 * fractal->Cpara.iterA && aux.i < fractal->Cpara.iterB + tempL)
				{
					paraIt = tempB - fabs(tempB * aux.i);
					paraAdd = paraIt * paraIt * curve2;
				}
				para += paraAdd;
			}
		}
	}
	// Parabolic
	double paraAddP0 = 0.0;
	if (fractal->Cpara.enabledParabFalse)
	{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
		paraAddP0 = fractal->Cpara.parabOffset + (aux.i * fractal->Cpara.parabSlope)
								+ (aux.i * aux.i * 0.001 * fractal->Cpara.parabScale);
	}
	para += paraAddP0;

	z *= para; // using the parameter
	aux.DE = aux.DE * fabs(para) + 1.0;
	aux.r_dz *= fabs(para);
}

/**
 * scale 3d. Scale each axis individually
 */
void TransfScale3dIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	z *= fractal->transformCommon.scale3D111;
	aux.r_dz *= z.Length() / aux.r;
	aux.DE = aux.DE * z.Length() / aux.r + 1.0;
}

/**
 * spherical invert
 * from M3D. Formula by Luca GN 2011, updated May 2012.
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfSphericalInvIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double r2Temp = z.Dot(z);
	z += fractal->mandelbox.offset;
	z *= fractal->transformCommon.scale;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;

	double r2 = z.Dot(z);
	if (fractal->transformCommon.functionEnabledyFalse) r2 = r2Temp;
	double mode = r2;
	if (fractal->transformCommon.functionEnabledFalse) // Mode 1
	{
		if (r2 > fractal->transformCommon.minRNeg1) mode = 1.0f;
		if (r2 < fractal->mandelbox.fR2 && r2 > fractal->transformCommon.minRNeg1)
			mode = fractal->transformCommon.minRNeg1;
		if (r2 < fractal->mandelbox.fR2 && r2 < fractal->transformCommon.minRNeg1)
			mode = fractal->transformCommon.minRNeg1;
	}
	if (fractal->transformCommon.functionEnabledxFalse) // Mode 2
	{
		if (r2 > fractal->transformCommon.minRNeg1) mode = 1.0f;
		if (r2 < fractal->mandelbox.fR2 && r2 > fractal->transformCommon.minRNeg1)
			mode = fractal->transformCommon.minRNeg1;
		if (r2 < fractal->mandelbox.fR2 && r2 < fractal->transformCommon.minRNeg1)
			mode = 2.0 * fractal->transformCommon.minRNeg1 - r2;
	}
	mode = 1.0 / mode;
	z *= mode;
	aux.DE *= mode;

	z -= fractal->mandelbox.offset + fractal->transformCommon.additionConstant000;
}

/**
 * inverted sphere z & c- A transform from M3D
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfSphericalInvCIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 c = aux.const_c;

	c *= fractal->transformCommon.constantMultiplier111;
	double rSqrL = c.x * c.x + c.y * c.y + c.z * c.z;
	// if (rSqrL < 1e-21) rSqrL = 1e-21;
	rSqrL = 1.0 / rSqrL;
	c *= rSqrL;

	rSqrL = z.x * z.x + z.y * z.y + z.z * z.z;
	// if (rSqrL < 1e-21) rSqrL = 1e-21;
	rSqrL = 1.0 / rSqrL;
	z *= rSqrL;
}

/**
 * spherical fold MBox
 */
void TransfSphericalFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double r2 = z.Dot(z);

	z += fractal->mandelbox.offset;
	z *= fractal->transformCommon.scale;													// beta
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0; // beta
	// if (r2 < 1e-21) r2 = 1e-21;
	if (r2 < fractal->mandelbox.mR2)
	{
		z *= fractal->mandelbox.mboxFactor1;
		aux.DE *= fractal->mandelbox.mboxFactor1;
		aux.r_dz *= fractal->mandelbox.mboxFactor1;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < fractal->mandelbox.fR2)
	{
		double tglad_factor2 = fractal->mandelbox.fR2 / r2;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.r_dz *= fractal->mandelbox.mboxFactor1;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z -= fractal->mandelbox.offset;
}

/**
 * spherical fold ABox
 * from Fractal Forums and M3D
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfSphericalFoldAboxIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double r2 = z.Dot(z);
	z += fractal->mandelbox.offset;
	double sqrtMinR = fractal->transformCommon.sqtR;
	if (r2 < sqrtMinR)
	{
		z *= fractal->transformCommon.mboxFactor1;
		aux.DE *= fractal->transformCommon.mboxFactor1;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < 1.0)
	{
		z *= 1.0 / r2;
		aux.DE *= 1.0 / r2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z -= fractal->mandelbox.offset;
}

/**
 * spherical fold varyV1 MBox type
 */
void TransfSphericalFoldVaryV1Iteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double r2 = z.Dot(z);
	double tempVCf = fractal->mandelbox.foldingSphericalFixed; // constant to be varied
	double tempVCm = fractal->mandelbox.foldingSphericalMin;

	if (aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		tempVCf =
			(tempVCf
				+ fractal->transformCommon.offset * (aux.i - fractal->transformCommon.startIterationsA)
						/ (fractal->transformCommon.stopIterationsA
								- fractal->transformCommon.startIterationsA));
	}
	if (aux.i >= fractal->transformCommon.stopIterationsA)
	{
		tempVCf = (tempVCf + fractal->transformCommon.offset);
	}
	if (aux.i >= fractal->transformCommon.startIterationsB
			&& aux.i < fractal->transformCommon.stopIterationsB)
	{
		tempVCm =
			(tempVCm
				+ fractal->transformCommon.offset0 * (aux.i - fractal->transformCommon.startIterationsB)
						/ (fractal->transformCommon.stopIterationsB
								- fractal->transformCommon.startIterationsB));
	}
	if (aux.i >= fractal->transformCommon.stopIterationsB)
	{
		tempVCm = tempVCm + fractal->transformCommon.offset0;
	}

	z += fractal->mandelbox.offset;

	tempVCm *= tempVCm;
	tempVCf *= tempVCf;

	if (r2 < tempVCm)
	{
		z *= tempVCf / tempVCm;
		aux.DE *= tempVCf / tempVCm;
		;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < tempVCf)
	{
		double tglad_factor2 = tempVCf / r2;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z -= fractal->mandelbox.offset;
}

/**
 * spherical fold Parab, coded by mclarekin
 * @reference
 * http://www.fractalforums.com/amazing-box-amazing-surf-and-variations/smooth-spherical-fold/msg101051/#new
 */
void TransfSphericalFoldParabIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double m = 1.0;
	double rr;
	// spherical fold
	if (fractal->transformCommon.functionEnabledSFalse
			&& aux.i >= fractal->transformCommon.startIterationsS
			&& aux.i < fractal->transformCommon.stopIterationsS)
	{
		rr = z.Dot(z);
		double tempM = rr + fractal->transformCommon.offsetB0;
		m = fractal->transformCommon.maxMinR2factor;
		// if (r2 < 1e-21) r2 = 1e-21;
		if (rr < fractal->transformCommon.minR2p25)
		{
			if (fractal->transformCommon.functionEnabledAyFalse && m > tempM) m = tempM + (tempM - m);
			z *= m;
			aux.DE *= m;
			aux.r_dz *= m;
			aux.color += fractal->mandelbox.color.factorSp1;
		}
		else if (rr < fractal->transformCommon.maxR2d1)
		{

			double m = fractal->transformCommon.maxR2d1 / rr;
			if (fractal->transformCommon.functionEnabledAyFalse && m > tempM) m = tempM + (tempM - m);
			z *= m;
			aux.DE *= m;
			aux.r_dz *= m;
			aux.color += fractal->mandelbox.color.factorSp2;
		}
	}
	if (aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		rr = z.Dot(z);
		z += fractal->mandelbox.offset;
		z *= fractal->transformCommon.scale;
		aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;
		double maxScale = fractal->transformCommon.scale4;
		double midPoint = (maxScale - 1.0) * 0.5;
		rr += fractal->transformCommon.offset0;
		double maxR2 = fractal->transformCommon.scale1;
		double halfMax = maxR2 * 0.5;
		double factor = midPoint / (halfMax * halfMax);
		// double m = 1.0;

		double tempM = rr + fractal->transformCommon.offsetA0;
		if (rr < halfMax)
		{
			m = maxScale - (rr * rr) * factor;
			m = 1.0 + (maxR2 - rr) * (maxR2 - rr) * factor;
			if (fractal->transformCommon.functionEnabledAxFalse && m > tempM) m = tempM + (tempM - m);
			z *= m;
			aux.DE *= m;
			aux.r_dz *= m;
		}
		else if (rr < maxR2)
		{
			m = 1.0 + (maxR2 - rr) * (maxR2 - rr) * factor;
			if (fractal->transformCommon.functionEnabledAxFalse && m > tempM) m = tempM + (tempM - m);
			z *= m;
			aux.DE *= m;
			aux.r_dz *= m;
		}
	}
	z -= fractal->mandelbox.offset;
	if (aux.i >= fractal->transformCommon.startIterationsA
			&& aux.i < fractal->transformCommon.stopIterationsA)
	{
		aux.actualScale = fractal->transformCommon.scaleA1
											+ fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);
		z *= aux.actualScale;
		aux.DE = aux.DE * fabs(aux.actualScale) + 1.0;
		aux.r_dz *= fabs(aux.actualScale);
	}
}

/**
 * spherical fold CHS Cylinder Half Size. Darkbeam s code from M3D
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfSphericalFoldCHSIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double tempZ = fabs(z.z) - fractal->transformCommon.offset05;
	double rr;
	if (aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		rr = z.x * z.x + z.y * z.y;
		if (tempZ > 0.0) rr = rr + (tempZ * tempZ * fractal->transformCommon.scale1);
	}
	else
	{
		rr = z.Dot(z);
	}

	z += fractal->mandelbox.offset;
	z *= fractal->transformCommon.scale;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;

	// Spherical Fold
	if (rr < fractal->transformCommon.minR2p25)
	{
		z *= fractal->transformCommon.maxMinR2factor;
		aux.DE *= fractal->transformCommon.maxMinR2factor;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (rr < fractal->transformCommon.maxR2d1)
	{
		double tglad_factor2 = fractal->transformCommon.maxR2d1 / rr;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z -= fractal->mandelbox.offset;
}

/**
 * spherical fold varyVCL MBox type
 */
void TransfSphericalFoldVaryVCLIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double para = 0.0;
	if (fractal->Cpara.enabledLinear)
	{
		para = fractal->Cpara.para00; // parameter value at iter 0
		double temp0 = para;
		double tempA = fractal->Cpara.paraA;
		double tempB = fractal->Cpara.paraB;
		double tempC = fractal->Cpara.paraC;
		double lengthAB = fractal->Cpara.iterB - fractal->Cpara.iterA;
		double lengthBC = fractal->Cpara.iterC - fractal->Cpara.iterB;
		double grade1 = (tempA - temp0) / fractal->Cpara.iterA;
		double grade2 = (tempB - tempA) / lengthAB;
		double grade3 = (tempC - tempB) / lengthBC;

		// slopes
		if (aux.i < fractal->Cpara.iterA)
		{
			para = temp0 + (aux.i * grade1);
		}
		if (aux.i < fractal->Cpara.iterB && aux.i >= fractal->Cpara.iterA)
		{
			para = tempA + (aux.i - fractal->Cpara.iterA) * grade2;
		}
		if (aux.i >= fractal->Cpara.iterB)
		{
			para = tempB + (aux.i - fractal->Cpara.iterB) * grade3;
		}

		// Curvi part on "true"
		if (fractal->Cpara.enabledCurves)
		{
			double paraAdd = 0.0;
			double paraIt;
			if (lengthAB > 2.0 * fractal->Cpara.iterA) // stop  error, todo fix.
			{
				double curve1 = (grade2 - grade1) / (4.0 * fractal->Cpara.iterA);
				double tempL = lengthAB - fractal->Cpara.iterA;
				double curve2 = (grade3 - grade2) / (4.0 * tempL);
				if (aux.i < 2 * fractal->Cpara.iterA)
				{
					paraIt = tempA - fabs(tempA - aux.i);
					paraAdd = paraIt * paraIt * curve1;
				}
				if (aux.i >= 2 * fractal->Cpara.iterA && aux.i < fractal->Cpara.iterB + tempL)
				{
					paraIt = tempB - fabs(tempB * aux.i);
					paraAdd = paraIt * paraIt * curve2;
				}
				para += paraAdd;
			}
		}
	}
	double paraAddP0 = 0.0;
	if (fractal->Cpara.enabledParabFalse)
	{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
		paraAddP0 = fractal->Cpara.parabOffset0 + (aux.i * fractal->Cpara.parabSlope)
								+ (aux.i * aux.i * 0.001 * fractal->Cpara.parabScale);
	}
	para += paraAddP0 + fractal->transformCommon.minR2p25;

	// spherical fold
	double r2 = z.Dot(z);

	z += fractal->mandelbox.offset;

	// if (r2 < 1e-21) r2 = 1e-21;
	if (r2 < para)
	{
		z *= fractal->transformCommon.maxR2d1 / para;
		aux.DE *= fractal->transformCommon.maxR2d1 / para;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < fractal->mandelbox.fR2)
	{
		double tglad_factor2 = fractal->transformCommon.maxR2d1 / r2;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z -= fractal->mandelbox.offset;
	z *= fractal->transformCommon.scale;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale);
}

/**
 * spherical radial offset.
 */
void TransfSphericalOffsetIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double para = fractal->transformCommon.offset;
	if (fractal->transformCommon.functionEnabled)
	{
		double lengthTempZ = -z.Length();
		// if (lengthTempZ > -1e-21) lengthTempZ = -1e-21;   //  z is neg.)
		z *= 1.0 + para / lengthTempZ;
		z *= fractal->transformCommon.scale;
		aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;
		aux.r_dz *= fabs(fractal->transformCommon.scale);
	}
}

/**
 * spherical radial offset Curvilinear.
 */
void TransfSphericalOffsetVCLIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	double para = fractal->Cpara.para00;
	double paraAdd = 0.0;
	double paraDot = 0.0;
	double paraAddP0 = 0.0;
	// curvilinear mode
	if (fractal->transformCommon.functionEnabled)
	{
		if (fractal->Cpara.enabledLinear)
		{
			para = fractal->Cpara.para00; // parameter value at iter 0
			double temp0 = para;
			double tempA = fractal->Cpara.paraA0;
			double tempB = fractal->Cpara.paraB0;
			double tempC = fractal->Cpara.paraC0;
			double lengthAB = fractal->Cpara.iterB - fractal->Cpara.iterA;
			double lengthBC = fractal->Cpara.iterC - fractal->Cpara.iterB;
			double grade1 = (tempA - temp0) / fractal->Cpara.iterA;
			double grade2 = (tempB - tempA) / lengthAB;
			double grade3 = (tempC - tempB) / lengthBC;

			// slopes
			if (aux.i < fractal->Cpara.iterA)
			{
				para = temp0 + (aux.i * grade1);
			}
			if (aux.i < fractal->Cpara.iterB && aux.i >= fractal->Cpara.iterA)
			{
				para = tempA + (aux.i - fractal->Cpara.iterA) * grade2;
			}
			if (aux.i >= fractal->Cpara.iterB)
			{
				para = tempB + (aux.i - fractal->Cpara.iterB) * grade3;
			}

			// Curvi part on "true"
			if (fractal->Cpara.enabledCurves)
			{
				// double paraAdd = 0.0;
				double paraIt;
				if (lengthAB > 2.0 * fractal->Cpara.iterA) // stop  error, todo fix.
				{
					double curve1 = (grade2 - grade1) / (4.0 * fractal->Cpara.iterA);
					double tempL = lengthAB - fractal->Cpara.iterA;
					double curve2 = (grade3 - grade2) / (4.0 * tempL);
					if (aux.i < 2 * fractal->Cpara.iterA)
					{
						paraIt = tempA - fabs(tempA - aux.i);
						paraAdd = paraIt * paraIt * curve1;
					}
					if (aux.i >= 2 * fractal->Cpara.iterA && aux.i < fractal->Cpara.iterB + tempL)
					{
						paraIt = tempB - fabs(tempB * aux.i);
						paraAdd = paraIt * paraIt * curve2;
					}
				}
				para += paraAdd;
			}
		}
	}
	// Parabolic
	// double paraAddP0 = 0.0;
	if (fractal->Cpara.enabledParabFalse)
	{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
		paraAddP0 = fractal->Cpara.parabOffset0 + (aux.i * fractal->Cpara.parabSlope)
								+ (aux.i * aux.i * 0.001 * fractal->Cpara.parabScale);
		para += paraAddP0;
	}

	// using the parameter
	// z *= 1.0 + para / -z.Dot(z);

	if (fractal->transformCommon.functionEnabledFalse)
	{
		paraDot = fractal->transformCommon.offset0;
		para += paraDot;
	}

	z *= 1.0 + para / -z.Dot(z);
	// post scale
	z *= fractal->transformCommon.scale;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;
	aux.r_dz *= fabs(fractal->transformCommon.scale);

	aux.DE =
		aux.DE * fractal->analyticDE.scale1 + fractal->analyticDE.offset0; // DE tweak  or aux.DE +=
}

/**
 * spherical pwr fold
 */
void TransfSphericalPwrFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	// if (z.x > -1e-21 && z.x < 1e-21) z.x = (z.x > 0) ? 1e-21 : -1e-21;
	// if (z.y > -1e-21 && z.y < 1e-21) z.y = (z.y > 0) ? 1e-21 : -1e-21;
	// if (z.z > -1e-21 && z.z < 1e-21) z.z = (z.z > 0) ? 1e-21 : -1e-21;
	double r2 = pow(pow(z.x, fractal->transformCommon.pwr4) + pow(z.y, fractal->transformCommon.pwr4)
										+ pow(z.z, fractal->transformCommon.pwr4),
		fractal->transformCommon.pwr05);

	// if (r2 < 1e-21 && r2 > -1e-21)
	//	r2 = (r2 > 0) ? 1e-21 : -1e-21;
	if (r2 < fractal->mandelbox.mR2)
	{
		z *= fractal->mandelbox.mboxFactor1;
		aux.DE *= fractal->mandelbox.mboxFactor1;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (r2 < fractal->mandelbox.fR2)
	{
		double tglad_factor2 = fractal->mandelbox.fR2 / r2;
		z *= tglad_factor2;
		aux.DE *= tglad_factor2;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
}
/**
 * Surf Box Fold
 * allows different fold types for each axis.
 * amazing surface if one axis has no folds enabled.
 * made from various folds found in the posts at fractalforums.com
 */
void TransfSurfBoxFoldIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->surfBox.enabledX1)
	{
		z.x =
			fabs(z.x + fractal->surfBox.offset1A111.x) - fabs(z.x - fractal->surfBox.offset1A111.x) - z.x;
	}
	if (fractal->surfBox.enabledX2False)
	{ // z = fold - fabs( fabs(z) - fold), Tglad fold
		z.x = fractal->surfBox.offset1B111.x - fabs(fabs(z.x) - fractal->surfBox.offset1B111.x);
	}
	if (fractal->surfBox.enabledX3False)
	{
		z.x = fabs(z.x + fractal->surfBox.offset2A111.x);
	}
	if (fractal->surfBox.enabledX4False)
	{ // if z > limit) z =  Value -z,   else if z < limit) z = - Value - z,
		if (z.x > fractal->surfBox.offset2B111.x)
		{
			z.x = fractal->surfBox.offset1A222.x - z.x;
			aux.color += fractal->mandelbox.color.factor.x;
		}
		else if (z.x < -fractal->surfBox.offset2B111.x)
		{
			z.x = -fractal->surfBox.offset1A222.x - z.x;
			aux.color += fractal->mandelbox.color.factor.x;
		}
	}
	if (fractal->surfBox.enabledX5False)
	{ // z = fold2 - fabs( fabs(z + fold) - fold2) - fabs(fold)
		z.x = fractal->surfBox.offset1B222.x
					- fabs(fabs(z.x + fractal->surfBox.offset3A111.x) - fractal->surfBox.offset1B222.x)
					- fractal->surfBox.offset3A111.x;
	}

	if (fractal->surfBox.enabledY1)
	{
		z.y =
			fabs(z.y + fractal->surfBox.offset1A111.y) - fabs(z.y - fractal->surfBox.offset1A111.y) - z.y;
	}
	if (fractal->surfBox.enabledY2False)
	{ // z = fold - fabs( fabs(z) - fold)
		z.y = fractal->surfBox.offset1B111.y - fabs(fabs(z.y) - fractal->surfBox.offset1B111.y);
	}
	if (fractal->surfBox.enabledY3False)
	{
		z.y = fabs(z.y + fractal->surfBox.offset2A111.y);
	}
	if (fractal->surfBox.enabledY4False)
	{ // if z > limit) z =  Value -z,   else if z < limit) z = - Value - z,
		if (z.y > fractal->surfBox.offset2B111.y)
		{
			z.y = fractal->surfBox.offset1A222.y - z.y;
			aux.color += fractal->mandelbox.color.factor.y;
		}
		else if (z.y < -fractal->surfBox.offset2B111.y)
		{
			z.y = -fractal->surfBox.offset1A222.y - z.y;
			aux.color += fractal->mandelbox.color.factor.y;
		}
	}
	if (fractal->surfBox.enabledY5False)
	{ // z = fold2 - fabs( fabs(z + fold) - fold2) - fabs(fold)
		z.y = fractal->surfBox.offset1B222.y
					- fabs(fabs(z.y + fractal->surfBox.offset3A111.y) - fractal->surfBox.offset1B222.y)
					- fractal->surfBox.offset3A111.y;
	}

	if (fractal->surfBox.enabledZ1)
	{
		z.z =
			fabs(z.z + fractal->surfBox.offset1A111.z) - fabs(z.z - fractal->surfBox.offset1A111.z) - z.z;
	}
	if (fractal->surfBox.enabledZ2False)
	{ // z = fold - fabs( fabs(z) - fold)
		z.z = fractal->surfBox.offset1B111.z - fabs(fabs(z.z) - fractal->surfBox.offset1B111.z);
	}
	if (fractal->surfBox.enabledZ3False)
	{
		z.z = fabs(z.z + fractal->surfBox.offset2A111.z);
	}

	if (fractal->surfBox.enabledZ4False)
	{ // if z > limit) z =  Value -z,   else if z < limit) z = - Value - z,
		double limit = fractal->surfBox.offset2B111.z;
		double value = fractal->surfBox.offset1A222.z;
		double zLimit = limit * fractal->surfBox.scale1Z1;
		double zValue = value * fractal->surfBox.scale1Z1;
		if (z.z > zLimit)
		{
			z.z = zValue - z.z;
			aux.color += fractal->mandelbox.color.factor.z;
		}
		else if (z.z < -zLimit)
		{
			z.z = -zValue - z.z;
			aux.color += fractal->mandelbox.color.factor.z;
		}
	}

	if (fractal->surfBox.enabledZ5False)
	{ // z = fold2 - fabs( fabs(z + fold) - fold2) - fabs(fold)// DarkBeams Abox mod1 fold
		z.z = fractal->surfBox.offset1B222.z
					- fabs(fabs(z.z + fractal->surfBox.offset3A111.z) - fractal->surfBox.offset1B222.z)
					- fractal->surfBox.offset3A111.z;
	}
	aux.DE *= fractal->analyticDE.scale1; // tweak
}

/**
 * TransfSurfFoldMultiIteration
 * Based on amazing_surf_mod1 from Mandelbulb3D
 */
void TransfSurfFoldMultiIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledAx)
	{
		z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
					- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
		z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
					- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
	}

	if (fractal->transformCommon.functionEnabledAyFalse)
	{ // z = fold - fabs( fabs(z) - fold)
		z.x = fractal->transformCommon.additionConstant111.x
					- fabs(fabs(z.x) - fractal->transformCommon.additionConstant111.x);
		z.y = fractal->transformCommon.additionConstant111.y
					- fabs(fabs(z.y) - fractal->transformCommon.additionConstant111.y);
	}

	if (fractal->transformCommon.functionEnabledAzFalse)
	{
		z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x);
		z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y);
	}

	if (fractal->transformCommon.functionEnabledxFalse)
	{ // if z > limit) z =  Value -z,   else if z < limit) z = - Value - z,
		if (z.x > fractal->transformCommon.additionConstant111.x)
		{
			z.x = fractal->mandelbox.foldingValue - z.x;
			aux.color += fractal->mandelbox.color.factor.x;
		}
		else if (z.x < -fractal->transformCommon.additionConstant111.x)
		{
			z.x = -fractal->mandelbox.foldingValue - z.x;
			aux.color += fractal->mandelbox.color.factor.x;
		}
		if (z.y > fractal->transformCommon.additionConstant111.y)
		{
			z.y = fractal->mandelbox.foldingValue - z.y;
			aux.color += fractal->mandelbox.color.factor.y;
		}
		else if (z.y < -fractal->transformCommon.additionConstant111.y)
		{
			z.y = -fractal->mandelbox.foldingValue - z.y;
			aux.color += fractal->mandelbox.color.factor.y;
		}
	}

	if (fractal->transformCommon.functionEnabledyFalse)
	{ // z = fold2 - fabs( fabs(z + fold) - fold2) - fabs(fold)
		z.x = fractal->transformCommon.offset2
					- fabs(fabs(z.x + fractal->transformCommon.additionConstant111.x)
								 - fractal->transformCommon.offset2)
					- fractal->transformCommon.additionConstant111.x;
		z.y = fractal->transformCommon.offset2
					- fabs(fabs(z.y + fractal->transformCommon.additionConstant111.y)
								 - fractal->transformCommon.offset2)
					- fractal->transformCommon.additionConstant111.y;
	}
}

/**
 * z vector - axis swap
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfZvectorAxisSwapIteration(CVector4 &z, const sFractal *fractal, sExtendedAux &aux)
{
	switch (fractal->mandelbulbMulti.orderOfXYZ)
	{
		case multi_OrderOfXYZ_xyz:
		default: z = CVector4(z.x, z.y, z.z, z.w); break;
		case multi_OrderOfXYZ_xzy: z = CVector4(z.x, z.z, z.y, z.w); break;
		case multi_OrderOfXYZ_yxz: z = CVector4(z.y, z.x, z.z, z.w); break;
		case multi_OrderOfXYZ_yzx: z = CVector4(z.y, z.z, z.x, z.w); break;
		case multi_OrderOfXYZ_zxy: z = CVector4(z.z, z.x, z.y, z.w); break;
		case multi_OrderOfXYZ_zyx: z = CVector4(z.z, z.y, z.x, z.w); break;
	}
	if (fractal->transformCommon.functionEnabledxFalse) z.x = -z.x;
	if (fractal->transformCommon.functionEnabledyFalse) z.y = -z.y;
	if (fractal->transformCommon.functionEnabledzFalse) z.z = -z.z;

	if (fractal->transformCommon.functionEnabledFalse
			&& aux.i >= fractal->transformCommon.startIterations
			&& aux.i < fractal->transformCommon.stopIterations)
	{
		double xTemp = SQRT_1_2 * (z.x - z.y);
		z.y = SQRT_1_2 * (z.y + z.x);
		z.x = xTemp;
	}
}

//-------------------------------- 4D ----------------------------------------------

/**
 * Formula based on Mandelbox (ABox). Extended to 4 dimensions
 */
void Abox4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
	double paraAddP0 = 0.0;
	if (fractal->Cpara.enabledParabFalse)
	{
		double parabScale = 0.0;
		if (fractal->Cpara.parabScale != 0.0)
			parabScale = aux.i * aux.i * 0.001 * fractal->Cpara.parabScale;
		paraAddP0 = fractal->Cpara.parabOffset0 + (aux.i * fractal->Cpara.parabSlope) + (parabScale);
		z4D.w += paraAddP0;
	}

	CVector4 oldZ = z4D;
	z4D.x = fabs(z4D.x + fractal->transformCommon.offset1111.x)
					- fabs(z4D.x - fractal->transformCommon.offset1111.x) - z4D.x;
	z4D.y = fabs(z4D.y + fractal->transformCommon.offset1111.y)
					- fabs(z4D.y - fractal->transformCommon.offset1111.y) - z4D.y;
	z4D.z = fabs(z4D.z + fractal->transformCommon.offset1111.z)
					- fabs(z4D.z - fractal->transformCommon.offset1111.z) - z4D.z;
	z4D.w = fabs(z4D.w + fractal->transformCommon.offset1111.w)
					- fabs(z4D.w - fractal->transformCommon.offset1111.w) - z4D.w;

	if (z4D.x != oldZ.x) aux.color += fractal->mandelbox.color.factor4D.x;
	if (z4D.y != oldZ.y) aux.color += fractal->mandelbox.color.factor4D.y;
	if (z4D.z != oldZ.z) aux.color += fractal->mandelbox.color.factor4D.z;
	if (z4D.w != oldZ.w) aux.color += fractal->mandelbox.color.factor4D.w;

	double rr = z4D.Dot(z4D);
	if (fractal->mandelboxVary4D.rPower != 1.0) rr = pow(rr, fractal->mandelboxVary4D.rPower);

	z4D += fractal->transformCommon.offset0000;
	if (rr < fractal->transformCommon.minR2p25)
	{
		z4D *= fractal->transformCommon.maxMinR2factor;
		aux.DE *= fractal->transformCommon.maxMinR2factor;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (rr < fractal->transformCommon.maxR2d1)
	{
		z4D *= fractal->transformCommon.maxR2d1 / rr;
		aux.DE *= fractal->transformCommon.maxR2d1 / rr;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z4D -= fractal->transformCommon.offset0000;
	aux.actualScale =
		fractal->mandelbox.scale + fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);
	z4D *= aux.actualScale;
	aux.DE = aux.DE * fabs(aux.actualScale) + 1.0;
	// 6 plane rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		CVector4 tp;
		if (fractal->transformCommon.rotation44a.x != 0)
		{
			tp = z4D;
			double alpha = fractal->transformCommon.rotation44a.x * M_PI_180;
			z4D.x = tp.x * cos(alpha) + tp.y * sin(alpha);
			z4D.y = tp.x * -sin(alpha) + tp.y * cos(alpha);
		}
		if (fractal->transformCommon.rotation44a.y != 0)
		{
			tp = z4D;
			double beta = fractal->transformCommon.rotation44a.y * M_PI_180;
			z4D.y = tp.y * cos(beta) + tp.z * sin(beta);
			z4D.z = tp.y * -sin(beta) + tp.z * cos(beta);
		}
		if (fractal->transformCommon.rotation44a.z != 0)
		{
			tp = z4D;
			double gamma = fractal->transformCommon.rotation44a.z * M_PI_180;
			z4D.x = tp.x * cos(gamma) + tp.z * sin(gamma);
			z4D.z = tp.x * -sin(gamma) + tp.z * cos(gamma);
		}
		if (fractal->transformCommon.rotation44b.x != 0)
		{
			tp = z4D;
			double delta = fractal->transformCommon.rotation44b.x * M_PI_180;
			z4D.x = tp.x * cos(delta) + tp.w * sin(delta);
			z4D.w = tp.x * -sin(delta) + tp.w * cos(delta);
		}
		if (fractal->transformCommon.rotation44b.y != 0)
		{
			tp = z4D;
			double epsilon = fractal->transformCommon.rotation44b.y * M_PI_180;
			z4D.y = tp.y * cos(epsilon) + tp.w * sin(epsilon);
			z4D.w = tp.y * -sin(epsilon) + tp.w * cos(epsilon);
		}
		if (fractal->transformCommon.rotation44b.z != 0)
		{
			tp = z4D;
			double zeta = fractal->transformCommon.rotation44b.z * M_PI_180;
			z4D.z = tp.z * cos(zeta) + tp.w * sin(zeta);
			z4D.w = tp.z * -sin(zeta) + tp.w * cos(zeta);
		}
	}
	z4D += fractal->transformCommon.additionConstant0000;

	aux.foldFactor = fractal->foldColor.compFold;
	aux.minRFactor = fractal->foldColor.compMinR;
	double scaleColor = fractal->foldColor.colorMin + fabs(aux.actualScale);
	// scaleColor += fabs(fractal->mandelbox.scale);
	aux.scaleFactor = scaleColor * fractal->foldColor.compScale;
}

/**
 * Bristorbrot formula 4D
 * @reference https://code.google.com/archive/p/fractaldimension/
 * by Doug Bristor
 */
void Bristorbrot4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	aux.r_dz = aux.r_dz * 2.0 * aux.r;
	double newx = z4D.x * z4D.x - z4D.y * z4D.y - z4D.z * z4D.z - z4D.w * z4D.w;
	double newy = z4D.y * (2.0 * z4D.x - z4D.z - z4D.w);
	double newz = z4D.z * (2.0 * z4D.x + z4D.y - z4D.w);
	double neww = z4D.w * (2.0 * z4D.x + z4D.y + z4D.z);
	z4D.x = newx;
	z4D.y = newy;
	z4D.z = newz;
	z4D.w = neww;

	z4D += fractal->transformCommon.additionConstant0000;
}

/**
 * from Syntopia & Darkbeam's Menger4 code from M3D
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void Menger4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	if (aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		z4D += fractal->transformCommon.additionConstant0000; // offset
	}

	z4D = fabs(z4D);
	if (z4D.x - z4D.y < 0.0) swap(z4D.y, z4D.x);
	if (z4D.x - z4D.z < 0.0) swap(z4D.z, z4D.x);
	if (z4D.y - z4D.z < 0.0) swap(z4D.z, z4D.y);
	if (z4D.x - z4D.w < 0.0) swap(z4D.w, z4D.x);
	if (z4D.y - z4D.w < 0.0) swap(z4D.w, z4D.y);
	if (z4D.z - z4D.w < 0.0) swap(z4D.w, z4D.z);

	// 6 plane rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		CVector4 tp;
		if (fractal->transformCommon.rotation44a.x != 0)
		{
			tp = z4D;
			double alpha = fractal->transformCommon.rotation44a.x * M_PI_180;
			z4D.x = tp.x * cos(alpha) + tp.y * sin(alpha);
			z4D.y = tp.x * -sin(alpha) + tp.y * cos(alpha);
		}
		if (fractal->transformCommon.rotation44a.y != 0)
		{
			tp = z4D;
			double beta = fractal->transformCommon.rotation44a.y * M_PI_180;
			z4D.y = tp.y * cos(beta) + tp.z * sin(beta);
			z4D.z = tp.y * -sin(beta) + tp.z * cos(beta);
		}
		if (fractal->transformCommon.rotation44a.z != 0)
		{
			tp = z4D;
			double gamma = fractal->transformCommon.rotation44a.z * M_PI_180;
			z4D.x = tp.x * cos(gamma) + tp.z * sin(gamma);
			z4D.z = tp.x * -sin(gamma) + tp.z * cos(gamma);
		}
		if (fractal->transformCommon.rotation44b.x != 0)
		{
			tp = z4D;
			double delta = fractal->transformCommon.rotation44b.x * M_PI_180;
			z4D.x = tp.x * cos(delta) + tp.w * sin(delta);
			z4D.w = tp.x * -sin(delta) + tp.w * cos(delta);
		}
		if (fractal->transformCommon.rotation44b.y != 0)
		{
			tp = z4D;
			double epsilon = fractal->transformCommon.rotation44b.y * M_PI_180;
			z4D.y = tp.y * cos(epsilon) + tp.w * sin(epsilon);
			z4D.w = tp.y * -sin(epsilon) + tp.w * cos(epsilon);
		}
		if (fractal->transformCommon.rotation44b.z != 0)
		{
			tp = z4D;
			double zeta = fractal->transformCommon.rotation44b.z * M_PI_180;
			z4D.z = tp.z * cos(zeta) + tp.w * sin(zeta);
			z4D.w = tp.z * -sin(zeta) + tp.w * cos(zeta);
		}
	}

	double scaleM = fractal->transformCommon.scale3;
	CVector4 offsetM = fractal->transformCommon.additionConstant111d5;
	z4D.x = scaleM * z4D.x - offsetM.x;
	z4D.y = scaleM * z4D.y - offsetM.y;
	z4D.w = scaleM * z4D.w - offsetM.w;
	z4D.z -= 0.5 * offsetM.z / scaleM;
	z4D.z = -fabs(-z4D.z);
	z4D.z += 0.5 * offsetM.z / scaleM;
	z4D.z *= scaleM;
	aux.DE *= scaleM;

	if (fractal->transformCommon.functionEnabledSFalse
			&& aux.i >= fractal->transformCommon.startIterationsS
			&& aux.i < fractal->transformCommon.stopIterationsS)
	{
		double r2 = z4D.Dot(z4D);
		// if (r2 < 1e-21 && r2 > -1e-21) r2 = (r2 > 0) ? 1e-21 : -1e-21;

		if (r2 < fractal->transformCommon.minR2p25)
		{
			z4D *= fractal->transformCommon.maxMinR2factor;
			aux.DE *= fractal->transformCommon.maxMinR2factor;
			aux.color += fractal->mandelbox.color.factorSp1;
		}
		else if (r2 < fractal->transformCommon.maxR2d1)
		{
			double tglad_factor2 = fractal->transformCommon.maxR2d1 / r2;
			z4D *= tglad_factor2;
			aux.DE *= tglad_factor2;
			aux.color += fractal->mandelbox.color.factorSp2;
		}
	}

	if (fractal->transformCommon.functionEnabledFalse)
	{
		CVector4 zA4 = (aux.i == fractal->transformCommon.intA) ? z4D : CVector4();
		CVector4 zB4 = (aux.i == fractal->transformCommon.intB) ? z4D : CVector4();

		z4D = (z4D * fractal->transformCommon.scale) + (zA4 * fractal->transformCommon.offset)
					+ (zB4 * fractal->transformCommon.offset0);
		aux.DE *= fractal->transformCommon.scale;
		aux.r_dz *= fractal->transformCommon.scale;
	}

	aux.DE *= fractal->analyticDE.scale1;
}

/**
 * Menger4D MOD1   from Syntopia & Darkbeam's Menger4 code from M3D
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void Menger4dMod1Iteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	double paraAddP0 = 0.0;
	if (fractal->Cpara.enabledParabFalse)
	{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
		paraAddP0 = fractal->Cpara.parabOffset0 + (aux.i * fractal->Cpara.parabSlope)
								+ (aux.i * aux.i * 0.001 * fractal->Cpara.parabScale);
		z4D.w += paraAddP0;
	}

	if (aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		z4D += fractal->transformCommon.additionConstant0000; // offset
	}

	z4D = fabs(z4D);
	if (z4D.x - z4D.y < 0.0) swap(z4D.y, z4D.x);
	if (z4D.x - z4D.z < 0.0) swap(z4D.z, z4D.x);
	if (z4D.y - z4D.z < 0.0) swap(z4D.z, z4D.y);
	if (z4D.x - z4D.w < 0.0) swap(z4D.w, z4D.x);
	if (z4D.y - z4D.w < 0.0) swap(z4D.w, z4D.y);
	if (z4D.z - z4D.w < 0.0) swap(z4D.w, z4D.z);

	// 6 plane rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		CVector4 tp;
		if (fractal->transformCommon.rotation44a.x != 0)
		{
			tp = z4D;
			double alpha = fractal->transformCommon.rotation44a.x * M_PI_180;
			z4D.x = tp.x * cos(alpha) + tp.y * sin(alpha);
			z4D.y = tp.x * -sin(alpha) + tp.y * cos(alpha);
		}
		if (fractal->transformCommon.rotation44a.y != 0)
		{
			tp = z4D;
			double beta = fractal->transformCommon.rotation44a.y * M_PI_180;
			z4D.y = tp.y * cos(beta) + tp.z * sin(beta);
			z4D.z = tp.y * -sin(beta) + tp.z * cos(beta);
		}
		if (fractal->transformCommon.rotation44a.z != 0)
		{
			tp = z4D;
			double gamma = fractal->transformCommon.rotation44a.z * M_PI_180;
			z4D.x = tp.x * cos(gamma) + tp.z * sin(gamma);
			z4D.z = tp.x * -sin(gamma) + tp.z * cos(gamma);
		}
		if (fractal->transformCommon.rotation44b.x != 0)
		{
			tp = z4D;
			double delta = fractal->transformCommon.rotation44b.x * M_PI_180;
			z4D.x = tp.x * cos(delta) + tp.w * sin(delta);
			z4D.w = tp.x * -sin(delta) + tp.w * cos(delta);
		}
		if (fractal->transformCommon.rotation44b.y != 0)
		{
			tp = z4D;
			double epsilon = fractal->transformCommon.rotation44b.y * M_PI_180;
			z4D.y = tp.y * cos(epsilon) + tp.w * sin(epsilon);
			z4D.w = tp.y * -sin(epsilon) + tp.w * cos(epsilon);
		}
		if (fractal->transformCommon.rotation44b.z != 0)
		{
			tp = z4D;
			double zeta = fractal->transformCommon.rotation44b.z * M_PI_180;
			z4D.z = tp.z * cos(zeta) + tp.w * sin(zeta);
			z4D.w = tp.z * -sin(zeta) + tp.w * cos(zeta);
		}
	}
	double scaleM = fractal->transformCommon.scale3;
	CVector4 offsetM = fractal->transformCommon.additionConstant111d5;
	z4D.x = scaleM * z4D.x - offsetM.x;
	z4D.y = scaleM * z4D.y - offsetM.y;
	z4D.w = scaleM * z4D.w - offsetM.w;
	if (fractal->transformCommon.functionEnabledz
			&& aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		z4D.z -= 0.5 * offsetM.z / scaleM;
		z4D.z = -fabs(-z4D.z);
		z4D.z += 0.5 * offsetM.z / scaleM;
	}
	else
	{
		z4D.w = scaleM * z4D.w - offsetM.w;
	}
	z4D.z *= scaleM;
	aux.DE *= scaleM;

	if (fractal->transformCommon.functionEnabledSFalse
			&& aux.i >= fractal->transformCommon.startIterationsS
			&& aux.i < fractal->transformCommon.stopIterationsS)
	{
		double r2 = 0.;
		if (fractal->transformCommon.functionEnabledBxFalse)
		{
			r2 = z4D.x * z4D.x + z4D.y * z4D.y;
		}
		if (fractal->transformCommon.functionEnabledByFalse)
		{
			r2 = z4D.x * z4D.x + z4D.y * z4D.y + z4D.z * z4D.z;
		}
		if (fractal->transformCommon.functionEnabledBz)
		//{	r2 = z4D.Dot(z4D) ;}
		{
			r2 = z4D.x * z4D.x + z4D.y * z4D.y + z4D.z * z4D.z + z4D.w * z4D.w;
		}
		// if (r2 < 1e-21 && r2 > -1e-21) r2 = (r2 > 0) ? 1e-21 : -1e-21;

		if (r2 < fractal->transformCommon.minR2p25)
		{
			z4D *= fractal->transformCommon.maxMinR2factor;
			aux.DE *= fractal->transformCommon.maxMinR2factor;
			aux.color += fractal->mandelbox.color.factorSp1;
		}
		else if (r2 < fractal->transformCommon.maxR2d1)
		{
			double tglad_factor2 = fractal->transformCommon.maxR2d1 / r2;
			z4D *= tglad_factor2;
			aux.DE *= tglad_factor2;
			aux.color += fractal->mandelbox.color.factorSp2;
		}
	}

	if (fractal->transformCommon.functionEnabledFalse)
	{
		CVector4 zA4 = (aux.i == fractal->transformCommon.intA) ? z4D : CVector4();
		CVector4 zB4 = (aux.i == fractal->transformCommon.intB) ? z4D : CVector4();

		z4D = (z4D * fractal->transformCommon.scale) + (zA4 * fractal->transformCommon.offset)
					+ (zB4 * fractal->transformCommon.offset0);
		aux.DE *= fractal->transformCommon.scale;
	}

	aux.DE *= fractal->analyticDE.scale1;
}

/**
 * Darkbeam's MixPinski4 from M3D
 * A strange but intriguing fractal, that mixes Sierpinski and Menger folds.
 * The amazing thing is that in 3D it does not work so well! LUCA GN 2011
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void MixPinski4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	if (aux.i >= fractal->transformCommon.startIterationsS
			&& aux.i < fractal->transformCommon.stopIterationsS)
	{
		double temp;
		if (z4D.x + z4D.y < 0.0)
		{
			temp = z4D.x;
			z4D.x = -z4D.y;
			z4D.y = -temp;
		}

		if (z4D.x + z4D.z < 0.0)
		{
			temp = z4D.x;
			z4D.x = -z4D.z;
			z4D.z = -temp;
		}

		if (z4D.y + z4D.z < 0.0)
		{
			temp = z4D.z;
			z4D.z = -z4D.y;
			z4D.y = -temp;
		}

		if (z4D.x + z4D.w < 0.0)
		{
			temp = z4D.x;
			z4D.x = -z4D.w;
			z4D.w = -temp;
		}

		if (z4D.y + z4D.w < 0.0)
		{
			temp = z4D.y;
			z4D.y = -z4D.w;
			z4D.w = -temp;
		}

		if (z4D.z + z4D.w < 0.0)
		{
			temp = z4D.z;
			z4D.z = -z4D.w;
			z4D.w = -temp;
		}
		z4D *= fractal->transformCommon.scale1;
		aux.DE *= fractal->transformCommon.scale1;
	}

	if (aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		z4D += fractal->transformCommon.additionConstant0000; // offset
	}
	// 6 plane rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		CVector4 tp;
		if (fractal->transformCommon.rotation44a.x != 0)
		{
			tp = z4D;
			double alpha = fractal->transformCommon.rotation44a.x * M_PI_180;
			z4D.x = tp.x * cos(alpha) + tp.y * sin(alpha);
			z4D.y = tp.x * -sin(alpha) + tp.y * cos(alpha);
		}
		if (fractal->transformCommon.rotation44a.y != 0)
		{
			tp = z4D;
			double beta = fractal->transformCommon.rotation44a.y * M_PI_180;
			z4D.y = tp.y * cos(beta) + tp.z * sin(beta);
			z4D.z = tp.y * -sin(beta) + tp.z * cos(beta);
		}
		if (fractal->transformCommon.rotation44a.z != 0)
		{
			tp = z4D;
			double gamma = fractal->transformCommon.rotation44a.z * M_PI_180;
			z4D.x = tp.x * cos(gamma) + tp.z * sin(gamma);
			z4D.z = tp.x * -sin(gamma) + tp.z * cos(gamma);
		}
		if (fractal->transformCommon.rotation44b.x != 0)
		{
			tp = z4D;
			double delta = fractal->transformCommon.rotation44b.x * M_PI_180;
			z4D.x = tp.x * cos(delta) + tp.w * sin(delta);
			z4D.w = tp.x * -sin(delta) + tp.w * cos(delta);
		}
		if (fractal->transformCommon.rotation44b.y != 0)
		{
			tp = z4D;
			double epsilon = fractal->transformCommon.rotation44b.y * M_PI_180;
			z4D.y = tp.y * cos(epsilon) + tp.w * sin(epsilon);
			z4D.w = tp.y * -sin(epsilon) + tp.w * cos(epsilon);
		}
		if (fractal->transformCommon.rotation44b.z != 0)
		{
			tp = z4D;
			double zeta = fractal->transformCommon.rotation44b.z * M_PI_180;
			z4D.z = tp.z * cos(zeta) + tp.w * sin(zeta);
			z4D.w = tp.z * -sin(zeta) + tp.w * cos(zeta);
		}
	}
	if (aux.i >= fractal->transformCommon.startIterationsM
			&& aux.i < fractal->transformCommon.stopIterationsM)
	{
		double scaleM = fractal->transformCommon.scale2;
		CVector4 offsetM = fractal->transformCommon.additionConstant111d5;
		z4D.x = scaleM * z4D.x - offsetM.x * (scaleM - 1.0);
		z4D.y = scaleM * z4D.y - offsetM.y * (scaleM - 1.0);
		z4D.w = scaleM * z4D.w - offsetM.w * (scaleM - 1.0);
		z4D.z -= 0.5 * offsetM.z * (scaleM - 1.0) / scaleM;
		z4D.z = -fabs(-z4D.z);
		z4D.z += 0.5 * offsetM.z * (scaleM - 1.0) / scaleM;
		z4D.z = scaleM * z4D.z;
		aux.DE *= scaleM * fractal->analyticDE.scale1;
	}
}

/**
 * Sierpinski4D.from Syntopia & Darkbeam's code
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void Sierpinski4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{

	double temp;
	if (z4D.x + z4D.y < 0.0)
	{
		temp = z4D.x;
		z4D.x = -z4D.y;
		z4D.y = -temp;
	}

	if (z4D.x + z4D.z < 0.0)
	{
		temp = z4D.x;
		z4D.x = -z4D.z;
		z4D.z = -temp;
	}

	if (z4D.y + z4D.z < 0.0)
	{
		temp = z4D.z;
		z4D.z = -z4D.y;
		z4D.y = -temp;
	}

	if (z4D.x + z4D.w < 0.0)
	{
		temp = z4D.x;
		z4D.x = -z4D.w;
		z4D.w = -temp;
	}

	if (z4D.y + z4D.w < 0.0)
	{
		temp = z4D.y;
		z4D.y = -z4D.w;
		z4D.w = -temp;
	}

	if (z4D.z + z4D.w < 0.0)
	{
		temp = z4D.z;
		z4D.z = -z4D.w;
		z4D.w = -temp;
	}

	z4D = z4D * fractal->transformCommon.scaleA2;
	aux.DE *= fractal->transformCommon.scaleA2;

	if (aux.i >= fractal->transformCommon.startIterationsC
			&& aux.i < fractal->transformCommon.stopIterationsC)
	{
		z4D -= fractal->transformCommon.offset1111; // neg offset
	}

	// 6 plane rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		CVector4 tp;
		if (fractal->transformCommon.rotation44a.x != 0)
		{
			tp = z4D;
			double alpha = fractal->transformCommon.rotation44a.x * M_PI_180;
			z4D.x = tp.x * cos(alpha) + tp.y * sin(alpha);
			z4D.y = tp.x * -sin(alpha) + tp.y * cos(alpha);
		}
		if (fractal->transformCommon.rotation44a.y != 0)
		{
			tp = z4D;
			double beta = fractal->transformCommon.rotation44a.y * M_PI_180;
			z4D.y = tp.y * cos(beta) + tp.z * sin(beta);
			z4D.z = tp.y * -sin(beta) + tp.z * cos(beta);
		}
		if (fractal->transformCommon.rotation44a.z != 0)
		{
			tp = z4D;
			double gamma = fractal->transformCommon.rotation44a.z * M_PI_180;
			z4D.x = tp.x * cos(gamma) + tp.z * sin(gamma);
			z4D.z = tp.x * -sin(gamma) + tp.z * cos(gamma);
		}
		if (fractal->transformCommon.rotation44b.x != 0)
		{
			tp = z4D;
			double delta = fractal->transformCommon.rotation44b.x * M_PI_180;
			z4D.x = tp.x * cos(delta) + tp.w * sin(delta);
			z4D.w = tp.x * -sin(delta) + tp.w * cos(delta);
		}
		if (fractal->transformCommon.rotation44b.y != 0)
		{
			tp = z4D;
			double epsilon = fractal->transformCommon.rotation44b.y * M_PI_180;
			z4D.y = tp.y * cos(epsilon) + tp.w * sin(epsilon);
			z4D.w = tp.y * -sin(epsilon) + tp.w * cos(epsilon);
		}
		if (fractal->transformCommon.rotation44b.z != 0)
		{
			tp = z4D;
			double zeta = fractal->transformCommon.rotation44b.z * M_PI_180;
			z4D.z = tp.z * cos(zeta) + tp.w * sin(zeta);
			z4D.w = tp.z * -sin(zeta) + tp.w * cos(zeta);
		}
	}
	aux.DE *= fractal->analyticDE.scale1;
}

/**
 * Quaternion4D
 */
void Quaternion4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	z4D = CVector4(z4D.x * z4D.x - z4D.y * z4D.y - z4D.z * z4D.z - z4D.w * z4D.w, z4D.x * z4D.y,
		z4D.x * z4D.z, z4D.w);
	z4D *= fractal->transformCommon.constantMultiplier1220;
	z4D += fractal->transformCommon.additionConstant0000;

	// 6 plane rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		CVector4 tp;
		if (fractal->transformCommon.rotation44a.x != 0)
		{
			tp = z4D;
			double alpha = fractal->transformCommon.rotation44a.x * M_PI_180;
			z4D.x = tp.x * cos(alpha) + tp.y * sin(alpha);
			z4D.y = tp.x * -sin(alpha) + tp.y * cos(alpha);
		}
		if (fractal->transformCommon.rotation44a.y != 0)
		{
			tp = z4D;
			double beta = fractal->transformCommon.rotation44a.y * M_PI_180;
			z4D.y = tp.y * cos(beta) + tp.z * sin(beta);
			z4D.z = tp.y * -sin(beta) + tp.z * cos(beta);
		}
		if (fractal->transformCommon.rotation44a.z != 0)
		{
			tp = z4D;
			double gamma = fractal->transformCommon.rotation44a.z * M_PI_180;
			z4D.x = tp.x * cos(gamma) + tp.z * sin(gamma);
			z4D.z = tp.x * -sin(gamma) + tp.z * cos(gamma);
		}
		if (fractal->transformCommon.rotation44b.x != 0)
		{
			tp = z4D;
			double delta = fractal->transformCommon.rotation44b.x * M_PI_180;
			z4D.x = tp.x * cos(delta) + tp.w * sin(delta);
			z4D.w = tp.x * -sin(delta) + tp.w * cos(delta);
		}
		if (fractal->transformCommon.rotation44b.y != 0)
		{
			tp = z4D;
			double epsilon = fractal->transformCommon.rotation44b.y * M_PI_180;
			z4D.y = tp.y * cos(epsilon) + tp.w * sin(epsilon);
			z4D.w = tp.y * -sin(epsilon) + tp.w * cos(epsilon);
		}
		if (fractal->transformCommon.rotation44b.z != 0)
		{
			tp = z4D;
			double zeta = fractal->transformCommon.rotation44b.z * M_PI_180;
			z4D.z = tp.z * cos(zeta) + tp.w * sin(zeta);
			z4D.w = tp.z * -sin(zeta) + tp.w * cos(zeta);
		}
	}
}

/**
 * Formula based on Mandelbox (ABox). Extended to 4 dimensions and with variable scale parameter.
 */
void MandelboxVaryScale4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	double paraAddP0 = 0.0;
	if (fractal->Cpara.enabledParabFalse)
	{ // parabolic = paraOffset + iter *slope + (iter *iter *scale)
		paraAddP0 = fractal->Cpara.parabOffset0 + (aux.i * fractal->Cpara.parabSlope)
								+ (aux.i * aux.i * 0.001 * fractal->Cpara.parabScale);
		z4D.w += paraAddP0;
	}

	aux.actualScale =
		fractal->mandelbox.scale + fractal->mandelboxVary4D.scaleVary * (fabs(aux.actualScale) - 1.0);
	CVector4 oldZ = z4D;
	z4D.x = fabs(z4D.x + fractal->mandelboxVary4D.fold) - fabs(z4D.x - fractal->mandelboxVary4D.fold)
					- z4D.x;
	z4D.y = fabs(z4D.y + fractal->mandelboxVary4D.fold) - fabs(z4D.y - fractal->mandelboxVary4D.fold)
					- z4D.y;
	z4D.z = fabs(z4D.z + fractal->mandelboxVary4D.fold) - fabs(z4D.z - fractal->mandelboxVary4D.fold)
					- z4D.z;
	z4D.w = fabs(z4D.w + fractal->mandelboxVary4D.fold) - fabs(z4D.w - fractal->mandelboxVary4D.fold)
					- z4D.w;

	if (z4D.x != oldZ.x) aux.color += fractal->mandelbox.color.factor4D.x;
	if (z4D.y != oldZ.y) aux.color += fractal->mandelbox.color.factor4D.y;
	if (z4D.z != oldZ.z) aux.color += fractal->mandelbox.color.factor4D.z;
	if (z4D.w != oldZ.w) aux.color += fractal->mandelbox.color.factor4D.w;

	double rr = pow(
		z4D.x * z4D.x + z4D.y * z4D.y + z4D.z * z4D.z + z4D.w * z4D.w, fractal->mandelboxVary4D.rPower);
	double m = aux.actualScale;
	if (rr < fractal->mandelboxVary4D.minR * fractal->mandelboxVary4D.minR)
	{
		m = aux.actualScale / (fractal->mandelboxVary4D.minR * fractal->mandelboxVary4D.minR);
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (rr < 1.0)
	{
		m = aux.actualScale / rr;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z4D *= m;
	aux.DE = aux.DE * fabs(m) + 1.0;
	// 6 plane rotation
	if (fractal->transformCommon.functionEnabledRFalse
			&& aux.i >= fractal->transformCommon.startIterationsR
			&& aux.i < fractal->transformCommon.stopIterationsR)
	{
		CVector4 tp;
		if (fractal->transformCommon.rotation44a.x != 0)
		{
			tp = z4D;
			double alpha = fractal->transformCommon.rotation44a.x * M_PI_180;
			z4D.x = tp.x * cos(alpha) + tp.y * sin(alpha);
			z4D.y = tp.x * -sin(alpha) + tp.y * cos(alpha);
		}
		if (fractal->transformCommon.rotation44a.y != 0)
		{
			tp = z4D;
			double beta = fractal->transformCommon.rotation44a.y * M_PI_180;
			z4D.y = tp.y * cos(beta) + tp.z * sin(beta);
			z4D.z = tp.y * -sin(beta) + tp.z * cos(beta);
		}
		if (fractal->transformCommon.rotation44a.z != 0)
		{
			tp = z4D;
			double gamma = fractal->transformCommon.rotation44a.z * M_PI_180;
			z4D.x = tp.x * cos(gamma) + tp.z * sin(gamma);
			z4D.z = tp.x * -sin(gamma) + tp.z * cos(gamma);
		}
		if (fractal->transformCommon.rotation44b.x != 0)
		{
			tp = z4D;
			double delta = fractal->transformCommon.rotation44b.x * M_PI_180;
			z4D.x = tp.x * cos(delta) + tp.w * sin(delta);
			z4D.w = tp.x * -sin(delta) + tp.w * cos(delta);
		}
		if (fractal->transformCommon.rotation44b.y != 0)
		{
			tp = z4D;
			double epsilon = fractal->transformCommon.rotation44b.y * M_PI_180;
			z4D.y = tp.y * cos(epsilon) + tp.w * sin(epsilon);
			z4D.w = tp.y * -sin(epsilon) + tp.w * cos(epsilon);
		}
		if (fractal->transformCommon.rotation44b.z != 0)
		{
			tp = z4D;
			double zeta = fractal->transformCommon.rotation44b.z * M_PI_180;
			z4D.z = tp.z * cos(zeta) + tp.w * sin(zeta);
			z4D.w = tp.z * -sin(zeta) + tp.w * cos(zeta);
		}
	}
}

/**
 * Adds c constant to z vector 4D
 */
void TransfAddConstant4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	z4D += fractal->transformCommon.additionConstant0000;
}

/**
 * box fold 4D
 */
void TransfBoxFold4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	if (z4D.x > fractal->mandelbox.foldingLimit)
	{
		z4D.x = fractal->mandelbox.foldingValue - z4D.x;
		aux.color += fractal->mandelbox.color.factor4D.x;
	}
	else if (z4D.x < -fractal->mandelbox.foldingLimit)
	{
		z4D.x = -fractal->mandelbox.foldingValue - z4D.x;
		aux.color += fractal->mandelbox.color.factor4D.x;
	}
	if (z4D.y > fractal->mandelbox.foldingLimit)
	{
		z4D.y = fractal->mandelbox.foldingValue - z4D.y;
		aux.color += fractal->mandelbox.color.factor4D.y;
	}
	else if (z4D.y < -fractal->mandelbox.foldingLimit)
	{
		z4D.y = -fractal->mandelbox.foldingValue - z4D.y;
		aux.color += fractal->mandelbox.color.factor4D.y;
	}
	if (z4D.z > fractal->mandelbox.foldingLimit)
	{
		z4D.z = fractal->mandelbox.foldingValue - z4D.z;
		aux.color += fractal->mandelbox.color.factor4D.z;
	}
	else if (z4D.z < -fractal->mandelbox.foldingLimit)
	{
		z4D.z = -fractal->mandelbox.foldingValue - z4D.z;
		aux.color += fractal->mandelbox.color.factor4D.z;
	}
	if (z4D.z > fractal->mandelbox.foldingLimit)
	{
		z4D.z = fractal->mandelbox.foldingValue - z4D.w;
		aux.color += fractal->mandelbox.color.factor4D.w;
	}
	else if (z4D.z < -fractal->mandelbox.foldingLimit)
	{
		z4D.z = -fractal->mandelbox.foldingValue - z4D.w;
		aux.color += fractal->mandelbox.color.factor4D.w;
	}
}

/**
 * fabs add  constant,  z = fabs( z + constant)
 */
void TransfFabsAddConstant4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	z4D += fractal->transformCommon.additionConstant0000;

	if (fractal->transformCommon.functionEnabledx) z4D.x = fabs(z4D.x);
	if (fractal->transformCommon.functionEnabledy) z4D.y = fabs(z4D.y);
	if (fractal->transformCommon.functionEnabledz) z4D.z = fabs(z4D.z);
	if (fractal->transformCommon.functionEnabled) z4D.w = fabs(z4D.w);
}

/**
 * fabs.  Add fabs constantV2,  z = fabs( z + constant) - fabs( z - constant) - z:
 */
void TransfFabsAddTgladFold4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	Q_UNUSED(aux);

	z4D = fabs(z4D + fractal->transformCommon.additionConstant0000)
				- fabs(z4D - fractal->transformCommon.additionConstant0000) - z4D;
}

/**
* fabs add conditional4D
*/
void TransfFabsAddConditional4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	if (fractal->transformCommon.functionEnabledx)
	{
		z4D.x = sign(z4D.x) * (fractal->transformCommon.offset1111.x - fabs(z4D.x)
														+ fabs(z4D.x) * fractal->transformCommon.additionConstant0000.x);
	}

	if (fractal->transformCommon.functionEnabledy)
	{
		z4D.y = sign(z4D.y) * (fractal->transformCommon.offset1111.y - fabs(z4D.y)
														+ fabs(z4D.y) * fractal->transformCommon.additionConstant0000.y);
	}

	if (fractal->transformCommon.functionEnabledz)
	{
		z4D.z = sign(z4D.z) * (fractal->transformCommon.offset1111.z - fabs(z4D.z)
														+ fabs(z4D.z) * fractal->transformCommon.additionConstant0000.z);
	}

	if (fractal->transformCommon.functionEnabledw)
	{
		z4D.w = sign(z4D.w) * (fractal->transformCommon.offset1111.w - fabs(z4D.w)
														+ fabs(z4D.w) * fractal->transformCommon.additionConstant0000.w);
	}
	aux.DE *= fractal->analyticDE.scale1; // DE tweak
}

/**
 * iteration weight 4D
 */
void TransfIterationWeight4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 zA4 = (aux.i == fractal->transformCommon.intA) ? z4D : CVector4();
	CVector4 zB4 = (aux.i == fractal->transformCommon.intB) ? z4D : CVector4();

	z4D = (z4D * fractal->transformCommon.scale) + (zA4 * fractal->transformCommon.offset)
				+ (zB4 * fractal->transformCommon.offset0);
	aux.DE *= fractal->transformCommon.scale;
	aux.r_dz *= fractal->transformCommon.scale;
}

/**
 * Reciprocal4D from M3D, Darkbeam's code
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */
void TransfReciprocal4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	CVector4 tempZ = z4D;

	if (fractal->transformCommon.functionEnabledx)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.x = (1.0 / fractal->transformCommon.offset1111.x)
								- 1.0 / (fabs(z4D.x) + fractal->transformCommon.offset1111.x);

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.x = (fractal->transformCommon.offsetA1111.x)
								- 1.0 / (fabs(z4D.x) + fractal->transformCommon.offset1111.x);

		tempZ.x += fabs(z4D.x) * fractal->transformCommon.additionConstant0000.x; // function slope
		z4D.x = sign(z4D.x) * tempZ.x;
	}

	if (fractal->transformCommon.functionEnabledy)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.y = (1.0 / fractal->transformCommon.offset1111.y)
								- 1.0 / (fabs(z4D.y) + fractal->transformCommon.offset1111.y);

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.y = (fractal->transformCommon.offsetA1111.y)
								- 1.0 / (fabs(z4D.y) + fractal->transformCommon.offset1111.y);

		tempZ.y += fabs(z4D.y) * fractal->transformCommon.additionConstant0000.y;
		z4D.y = sign(z4D.y) * tempZ.y;
	}

	if (fractal->transformCommon.functionEnabledz)
	{
		if (fractal->transformCommon.functionEnabledAx)
			tempZ.z = (1.0 / fractal->transformCommon.offset1111.z)
								- 1.0 / (fabs(z4D.z) + fractal->transformCommon.offset1111.z);

		if (fractal->transformCommon.functionEnabledAxFalse)
			tempZ.z = (fractal->transformCommon.offsetA1111.z)
								- 1.0 / (fabs(z4D.z) + fractal->transformCommon.offset1111.z);

		tempZ.z += fabs(z4D.z) * fractal->transformCommon.additionConstant0000.z;
		z4D.z = sign(z4D.z) * tempZ.z;
	}

	if (fractal->transformCommon.functionEnabledw)
	{
		if (fractal->transformCommon.functionEnabledAw)
			tempZ.w = (1.0 / fractal->transformCommon.offset1111.w)
								- 1.0 / (fabs(z4D.w) + fractal->transformCommon.offset1111.w);

		if (fractal->transformCommon.functionEnabledAwFalse)
			tempZ.w = (fractal->transformCommon.offsetA1111.w)
								- 1.0 / (fabs(z4D.w) + fractal->transformCommon.offset1111.w);

		tempZ.z += fabs(z4D.w) * fractal->transformCommon.additionConstant0000.w;
		z4D.z = sign(z4D.w) * tempZ.w;
	}
	// aux.DE = aux.DE * l/L;
	aux.DE *= fractal->analyticDE.scale1; // DE tweak
}

/**
 * rotation 4D
 */
void TransfRotation4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
// temp 4D rotation
{
	Q_UNUSED(aux);

	CVector4 tp;
	if (fractal->transformCommon.rotation44a.x != 0)
	{
		tp = z4D;
		double alpha = fractal->transformCommon.rotation44a.x * M_PI_180;
		z4D.x = tp.x * cos(alpha) + tp.y * sin(alpha);
		z4D.y = tp.x * -sin(alpha) + tp.y * cos(alpha);
	}
	if (fractal->transformCommon.rotation44a.y != 0)
	{
		tp = z4D;
		double beta = fractal->transformCommon.rotation44a.y * M_PI_180;
		z4D.y = tp.y * cos(beta) + tp.z * sin(beta);
		z4D.z = tp.y * -sin(beta) + tp.z * cos(beta);
	}
	if (fractal->transformCommon.rotation44a.z != 0)
	{
		tp = z4D;
		double gamma = fractal->transformCommon.rotation44a.z * M_PI_180;
		z4D.x = tp.x * cos(gamma) + tp.z * sin(gamma);
		z4D.z = tp.x * -sin(gamma) + tp.z * cos(gamma);
	}
	if (fractal->transformCommon.rotation44b.x != 0)
	{
		tp = z4D;
		double delta = fractal->transformCommon.rotation44b.x * M_PI_180;
		z4D.x = tp.x * cos(delta) + tp.w * sin(delta);
		z4D.w = tp.x * -sin(delta) + tp.w * cos(delta);
	}
	if (fractal->transformCommon.rotation44b.y != 0)
	{
		tp = z4D;
		double epsilon = fractal->transformCommon.rotation44b.y * M_PI_180;
		z4D.y = tp.y * cos(epsilon) + tp.w * sin(epsilon);
		z4D.w = tp.y * -sin(epsilon) + tp.w * cos(epsilon);
	}
	if (fractal->transformCommon.rotation44b.z != 0)
	{
		tp = z4D;
		double zeta = fractal->transformCommon.rotation44b.z * M_PI_180;
		z4D.z = tp.z * cos(zeta) + tp.w * sin(zeta);
		z4D.w = tp.z * -sin(zeta) + tp.w * cos(zeta);
	}
}

/**
 * scale 4D
 */
void TransfScale4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	z4D *= fractal->transformCommon.scale;
	aux.DE = aux.DE * fabs(fractal->transformCommon.scale) + 1.0;
	aux.r_dz *= fabs(fractal->transformCommon.scale);
}

/**
 * spherical fold 4D
 */
void TransfSphericalFold4dIteration(CVector4 &z4D, const sFractal *fractal, sExtendedAux &aux)
{
	// double r2 = z4D.Dot(z4D);
	// double r2 = z4D.x * z4D.x + z4D.y * z4D.y;
	// if (r2 < 1e-21 && r2 > -1e-21) r2 = (r2 > 0) ? 1e-21 : -1e-21;
	// r2 += z4D.z * z4D.z;
	double rr = z4D.Dot(z4D);
	z4D += fractal->transformCommon.offset0000;
	if (rr < fractal->transformCommon.minR2p25)
	{
		z4D *= fractal->transformCommon.maxMinR2factor;
		aux.DE *= fractal->transformCommon.maxMinR2factor;
		aux.color += fractal->mandelbox.color.factorSp1;
	}
	else if (rr < fractal->transformCommon.maxR2d1)
	{
		z4D *= fractal->transformCommon.maxR2d1 / rr;
		aux.DE *= fractal->transformCommon.maxR2d1 / rr;
		aux.color += fractal->mandelbox.color.factorSp2;
	}
	z4D -= fractal->transformCommon.offset0000;
}
