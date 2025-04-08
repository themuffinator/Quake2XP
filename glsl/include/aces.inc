//!#include "global.inc"

#define float3x3	mat3
#define float4		vec4 
#define float3		vec3
#define float2		vec2
#define half		float
#define half2		vec2
#define half3		vec3
#define mul(x,y)	y*x 
#define lerp(x,y,z) mix(x,y,z)
#define rcp(x)		1.0/x
#define atan2(x,y)	atan(y,x)
#define	Pi			3.14159265358979323846 

float log10( float x) {
return (1.0 / log(10.0)) * log(x);
}

float3x3 AP0_2_XYZ_MAT =
{
	{0.9525523959, 0.0000000000, 0.0000936786},
	{0.3439664498, 0.7281660966,-0.0721325464},
	{0.0000000000, 0.0000000000, 1.0088251844},
};

float3x3 XYZ_2_AP0_MAT =
{
	 {1.0498110175, 0.0000000000,-0.0000974845},
	{-0.4959030231, 1.3733130458, 0.0982400361},
	 {0.0000000000, 0.0000000000, 0.9912520182},
};

float3x3 AP1_2_XYZ_MAT = 
{
	 {0.6624541811, 0.1340042065, 0.1561876870},
	 {0.2722287168, 0.6740817658, 0.0536895174},
	{-0.0055746495, 0.0040607335, 1.0103391003},
};

float3x3 XYZ_2_AP1_MAT =
{
	 {1.6410233797, -0.3248032942, -0.2364246952},
	{-0.6636628587,  1.6153315917,  0.0167563477},
	 {0.0117218943, -0.0082844420,  0.9883948585},
};

float3x3 AP0_2_AP1_MAT = //mul( AP0_2_XYZ_MAT, XYZ_2_AP1_MAT );
{
	 {1.4514393161, -0.2365107469, -0.2149285693},
	{-0.0765537734,  1.1762296998, -0.0996759264},
	 {0.0083161484, -0.0060324498,  0.9977163014},
};

float3x3 AP1_2_AP0_MAT = //mul( AP1_2_XYZ_MAT, XYZ_2_AP0_MAT );
{
	 {0.6954522414,  0.1406786965,  0.1638690622},
	 {0.0447945634,  0.8596711185,  0.0955343182},
	{-0.0055258826,  0.0040252103,  1.0015006723},
};

float3 AP1_RGB2Y =
{
	0.2722287168, //AP1_2_XYZ_MAT[0][1],
	0.6740817658, //AP1_2_XYZ_MAT[1][1],
	0.0536895174, //AP1_2_XYZ_MAT[2][1]
};

// REC 709 primaries
float3x3 XYZ_2_sRGB_MAT =
{
	 {3.2409699419, -1.5373831776, -0.4986107603},
	{-0.9692436363,  1.8759675015,  0.0415550574},
	 {0.0556300797, -0.2039769589,  1.0569715142},
};

float3x3 sRGB_2_XYZ_MAT =
{
	{0.4124564, 0.3575761, 0.1804375},
	{0.2126729, 0.7151522, 0.0721750},
	{0.0193339, 0.1191920, 0.9503041},
};

// REC 2020 primaries
float3x3 XYZ_2_Rec2020_MAT =
{
	 {1.7166084, -0.3556621, -0.2533601},
	{-0.6666829,  1.6164776,  0.0157685},
	 {0.0176422, -0.0427763,  0.94222867}	
};

float3x3 Rec2020_2_XYZ_MAT =
{
	{0.6369736, 0.1446172, 0.1688585},
	{0.2627066, 0.6779996, 0.0592938},
	{0.0000000, 0.0280728, 1.0608437}
};

// P3, D65 primaries
float3x3 XYZ_2_P3D65_MAT =
{
	 {2.4933963, -0.9313459, -0.4026945},
	{-0.8294868,  1.7626597,  0.0236246},
	 {0.0358507, -0.0761827,  0.9570140}
};

float3x3 P3D65_2_XYZ_MAT =
{
	{0.4865906, 0.2656683, 0.1981905},
	{0.2289838, 0.6917402, 0.0792762},
	{0.0000000, 0.0451135, 1.0438031}
};

// Bradford chromatic adaptation transforms between ACES white point (D60) and sRGB white point (D65)
float3x3 D65_2_D60_CAT =
{
	 {1.01303,    0.00610531, -0.014971},
	 {0.00769823, 0.998165,   -0.00503203},
	{-0.00284131, 0.00468516,  0.924507},
};

float3x3 D60_2_D65_CAT =
{
	 {0.987224,   -0.00611327, 0.0159533},
	{-0.00759836,  1.00186,    0.00533002},
	 {0.00307257, -0.00509595, 1.08168},
};

float3 xyY_2_XYZ( float3 xyY )
{
	float3 XYZ;
	XYZ[0] = xyY[0] * xyY[2] / max( xyY[1], 1e-10);
	XYZ[1] = xyY[2];  
	XYZ[2] = (1.0 - xyY[0] - xyY[1]) * xyY[2] / max( xyY[1], 1e-10);

	return XYZ;
}

float3x3 ChromaticAdaptation( float2 src_xy, float2 dst_xy )
{
	// Von Kries chromatic adaptation 

	// Bradford
	const float3x3 ConeResponse =
	{
		{ 0.8951,  0.2664, -0.1614},
		{-0.7502,  1.7135,  0.0367},
		{0.0389, -0.0685,  1.0296},
	};
	const float3x3 InvConeResponse =
	{
		 {0.9869929, -0.1470543,  0.1599627},
		 {0.4323053,  0.5183603,  0.0492912},
		{-0.0085287,  0.0400428,  0.9684867},
	};

	float3 src_XYZ = xyY_2_XYZ( float3( src_xy, 1 ) );
	float3 dst_XYZ = xyY_2_XYZ( float3( dst_xy, 1 ) );

	float3 src_coneResp = mul( ConeResponse, src_XYZ );
	float3 dst_coneResp = mul( ConeResponse, dst_XYZ );

	float3x3 VonKriesMat =
	{
		{ dst_coneResp[0] / src_coneResp[0], 0.0, 0.0 },
		{ 0.0, dst_coneResp[1] / src_coneResp[1], 0.0 },
		{ 0.0, 0.0, dst_coneResp[2] / src_coneResp[2] }
	};

	return mul( InvConeResponse, mul( VonKriesMat, ConeResponse ) );
}


float HALF_MAX = 65504.0;

float rgb_2_saturation( float3 rgb )
{
	float minrgb = min( min(rgb.r, rgb.g ), rgb.b );
	float maxrgb = max( max(rgb.r, rgb.g ), rgb.b );
	return ( max( maxrgb, 1e-10 ) - max( minrgb, 1e-10 ) ) / max( maxrgb, 1e-2 );
}

float glow_fwd( float ycIn, float glowGainIn, float glowMid)
{
   float glowGainOut;

   if (ycIn <= 2.0/3.0 * glowMid) {
	 glowGainOut = glowGainIn;
   } else if ( ycIn >= 2.0 * glowMid) {
	 glowGainOut = 0.0;
   } else {
	 glowGainOut = glowGainIn * (glowMid / ycIn - 0.5);
   }

   return glowGainOut;
}

float glow_inv( float ycOut, float glowGainIn, float glowMid)
{
	float glowGainOut;

	if (ycOut <= ((1.0 + glowGainIn) * 2.0/3.0 * glowMid)) {
		glowGainOut = -glowGainIn / (1.0 + glowGainIn);
	} else if ( ycOut >= (2.0 * glowMid)) {
		glowGainOut = 0.0;
	} else {
		glowGainOut = glowGainIn * (glowMid / ycOut - 1.0/2.0) / (glowGainIn / 2.0 - 1.0);
	}

	return glowGainOut;
}


float sigmoid_shaper( float x)
{
	// Sigmoid function in the range 0 to 1 spanning -2 to +2.

	float t = max( 1.0 - abs( 0.5 * x ), 0.0 );
	float y = 1.0 + sign(x) * (1.0 - t*t);
	return 0.5 * y;
}


// ------- Red modifier functions
float cubic_basis_shaper
( 
  float x, 
  float w   // full base width of the shaper function (in degrees)
)
{
	//return Square( smoothstep( 0, 1, 1 - abs( 2 * x/w ) ) );

	float M[4][4] =
	{
		{ -1.0/6.0,  3.0/6.0, -3.0/6.0,  1.0/6.0 },
		{  3.0/6.0, -6.0/6.0,  3.0/6.0,  0.0/6.0 },
		{ -3.0/6.0,  0.0/6.0,  3.0/6.0,  0.0/6.0 },
		{  1.0/6.0,  4.0/6.0,  1.0/6.0,  0.0/6.0 }
	};
  
	float knots[5] = { -0.5 * w, -0.25 * w, 0.0, 0.25 * w, 0.5 * w };
  
	float y = 0.0;
	if ((x > knots[0]) && (x < knots[4]))
	{  
		float knot_coord = (x - knots[0]) * 4.0 / w;
		int j = int(knot_coord);
		float t = knot_coord - float(j);
	  
		float monomials[4] = { t*t*t, t*t, t, 1.0 };

		// (if/else structure required for compatibility with CTL < v1.5.)
		if ( j == 3) {
			y = monomials[0] * M[0][0] + monomials[1] * M[1][0] + 
				monomials[2] * M[2][0] + monomials[3] * M[3][0];
		} else if ( j == 2) {
			y = monomials[0] * M[0][1] + monomials[1] * M[1][1] + 
				monomials[2] * M[2][1] + monomials[3] * M[3][1];
		} else if ( j == 1) {
			y = monomials[0] * M[0][2] + monomials[1] * M[1][2] + 
				monomials[2] * M[2][2] + monomials[3] * M[3][2];
		} else if ( j == 0) {
			y = monomials[0] * M[0][3] + monomials[1] * M[1][3] + 
				monomials[2] * M[2][3] + monomials[3] * M[3][3];
		} else {
			y = 0.0;
		}
	}
  
	return y * 1.5;
}

float center_hue( float hue, float centerH)
{
	float hueCentered = hue - centerH;
	if (hueCentered < -180.)
		hueCentered += 360;
	else if (hueCentered > 180.)
		hueCentered -= 360;
	return hueCentered;
}


// Textbook monomial to basis-function conversion matrix.
float3x3 M =
{
	{  0.5, -1.0, 0.5 },
	{ -1.0,  1.0, 0.5 },
	{  0.5,  0.0, 0.0 }
};

struct SegmentedSplineParams_c5
{
	float coefsLow[6];    // coefs for B-spline between minPoint and midPoint (units of log luminance)
	float coefsHigh[6];   // coefs for B-spline between midPoint and maxPoint (units of log luminance)
	float2 minPoint; // {luminance, luminance} linear extension below this
	float2 midPoint; // {luminance, luminance} 
	float2 maxPoint; // {luminance, luminance} linear extension above this
	float slopeLow;       // log-log slope of low linear extension
	float slopeHigh;      // log-log slope of high linear extension
};

struct SegmentedSplineParams_c9
{
	float coefsLow[10];    // coefs for B-spline between minPoint and midPoint (units of log luminance)
	float coefsHigh[10];   // coefs for B-spline between midPoint and maxPoint (units of log luminance)
	float2 minPoint; // {luminance, luminance} linear extension below this
	float2 midPoint; // {luminance, luminance} 
	float2 maxPoint; // {luminance, luminance} linear extension above this
	float slopeLow;       // log-log slope of low linear extension
	float slopeHigh;      // log-log slope of high linear extension
};

float segmented_spline_c5_fwd( float x )
{
	// RRT_PARAMS
	SegmentedSplineParams_c5 C =
	{
		// coefsLow[6]
		{ -4.0000000000, -4.0000000000, -3.1573765773, -0.4852499958, 1.8477324706, 1.8477324706 },
		// coefsHigh[6]
		{ -0.7185482425, 2.0810307172, 3.6681241237, 4.0000000000, 4.0000000000, 4.0000000000 },
		{ 0.18*exp2(-15.0), 0.0001},    // minPoint
		{ 0.18,              4.8},    // midPoint
		{ 0.18*exp2(18.0),  10000.0},    // maxPoint
		0.0,  // slopeLow
		0.0   // slopeHigh
	};

	int N_KNOTS_LOW = 4;
	int N_KNOTS_HIGH = 4;

	// Check for negatives or zero before taking the log. If negative or zero,
	// set to ACESMIN.1
	float xCheck = x <= 0.0 ? exp2(-14.0) : x;

	float logx = log10( xCheck);
	float logy;

	if ( logx <= log10(C.minPoint.x) )
	{ 
		logy = logx * C.slopeLow + ( log10(C.minPoint.y) - C.slopeLow * log10(C.minPoint.x) );
	}
	else if (( logx > log10(C.minPoint.x) ) && ( logx < log10(C.midPoint.x) ))
	{
		float knot_coord = (N_KNOTS_LOW-1) * (logx-log10(C.minPoint.x))/(log10(C.midPoint.x)-log10(C.minPoint.x));
		int j = int(knot_coord);
		float t = knot_coord - j;

		float3 cf = { C.coefsLow[ j], C.coefsLow[ j + 1], C.coefsLow[ j + 2]};
	
		float3 monomials = { t * t, t, 1.0 };
		logy = dot( monomials, mul( cf, M));
	}
	else if (( logx >= log10(C.midPoint.x) ) && ( logx < log10(C.maxPoint.x) ))
	{
		float knot_coord = (N_KNOTS_HIGH-1) * (logx-log10(C.midPoint.x))/(log10(C.maxPoint.x)-log10(C.midPoint.x));
		int j = int(knot_coord);
		float t = knot_coord - j;

		float3 cf = { C.coefsHigh[ j], C.coefsHigh[ j + 1], C.coefsHigh[ j + 2]};

		float3 monomials = { t * t, t, 1.0 };
		logy = dot( monomials, mul( cf, M));
	}
	else
	{ //if ( logIn >= log10(C.maxPoint.x) ) { 
		logy = logx * C.slopeHigh + ( log10(C.maxPoint.y) - C.slopeHigh * log10(C.maxPoint.x) );
	}

	return pow( 10, logy );
}

float segmented_spline_c5_rev( float y )
{  
	// RRT_PARAMS
	SegmentedSplineParams_c5 C =
	{
		// coefsLow[6]
		{ -4.0000000000, -4.0000000000, -3.1573765773, -0.4852499958, 1.8477324706, 1.8477324706 },
		// coefsHigh[6]
		{ -0.7185482425, 2.0810307172, 3.6681241237, 4.0000000000, 4.0000000000, 4.0000000000 },
		{ 0.18*exp2(-15.0), 0.0001},    // minPoint
		{ 0.18,              4.8},    // midPoint
		{ 0.18*exp2(18.0),  10000.0},    // maxPoint
		0.0,  // slopeLow
		0.0   // slopeHigh
	};

	const int N_KNOTS_LOW = 4;
	const int N_KNOTS_HIGH = 4;

	float KNOT_INC_LOW = (log10(C.midPoint.x) - log10(C.minPoint.x)) / (N_KNOTS_LOW - 1.0);
	float KNOT_INC_HIGH = (log10(C.maxPoint.x) - log10(C.midPoint.x)) / (N_KNOTS_HIGH - 1.0);

	int i;
  
	// KNOT_Y is luminance of the spline at each knot
	float KNOT_Y_LOW[N_KNOTS_LOW];
	for (i = 0; i < N_KNOTS_LOW; i = i+1)
	{
		KNOT_Y_LOW[i] = (C.coefsLow[i] + C.coefsLow[i+1]) / 2.0;
	};

	float KNOT_Y_HIGH[N_KNOTS_HIGH];
	for (i = 0; i < N_KNOTS_HIGH; i = i+1) 
	{
		KNOT_Y_HIGH[i] = ( C.coefsHigh[i] + C.coefsHigh[i+1]) / 2.0;
	};

	float logy = log10(max(y,1e-10));

	float logx;
	if (logy <= log10(C.minPoint.y)) 
	{
		logx = log10(C.minPoint.x);
	} 
	else if ( (logy > log10(C.minPoint.y)) && (logy <= log10(C.midPoint.y)) ) 
	{
		uint j;
		float3 cf;
		if (logy > KNOT_Y_LOW[ 0] && logy <= KNOT_Y_LOW[ 1]) {
			cf[ 0] = C.coefsLow[0];  cf[ 1] = C.coefsLow[1];  cf[ 2] = C.coefsLow[2];  j = 0;
		} else if ( logy > KNOT_Y_LOW[ 1] && logy <= KNOT_Y_LOW[ 2]) {
			cf[ 0] = C.coefsLow[1];  cf[ 1] = C.coefsLow[2];  cf[ 2] = C.coefsLow[3];  j = 1;
		} else if ( logy > KNOT_Y_LOW[ 2] && logy <= KNOT_Y_LOW[ 3]) {
			cf[ 0] = C.coefsLow[2];  cf[ 1] = C.coefsLow[3];  cf[ 2] = C.coefsLow[4];  j = 2;
		} 
	
		float3 tmp = mul( cf, M);

		float a = tmp[ 0];
		float b = tmp[ 1];
		float c = tmp[ 2];
		c = c - logy;

		float d = sqrt( b * b - 4.0 * a * c);

		float t = ( 2.0 * c) / ( -d - b);

		logx = log10(C.minPoint.x) + ( t + j) * KNOT_INC_LOW;
	} 
	else if ( (logy > log10(C.midPoint.y)) && (logy < log10(C.maxPoint.y)) ) 
	{
		uint j;
		float3 cf;
		if ( logy > KNOT_Y_HIGH[ 0] && logy <= KNOT_Y_HIGH[ 1]) {
			cf[ 0] = C.coefsHigh[0];  cf[ 1] = C.coefsHigh[1];  cf[ 2] = C.coefsHigh[2];  j = 0;
		} else if ( logy > KNOT_Y_HIGH[ 1] && logy <= KNOT_Y_HIGH[ 2]) {
			cf[ 0] = C.coefsHigh[1];  cf[ 1] = C.coefsHigh[2];  cf[ 2] = C.coefsHigh[3];  j = 1;
		} else if ( logy > KNOT_Y_HIGH[ 2] && logy <= KNOT_Y_HIGH[ 3]) {
			cf[ 0] = C.coefsHigh[2];  cf[ 1] = C.coefsHigh[3];  cf[ 2] = C.coefsHigh[4];  j = 2;
		} 
	
		float3 tmp = mul( cf, M);

		float a = tmp[ 0];
		float b = tmp[ 1];
		float c = tmp[ 2];
		c = c - logy;

		float d = sqrt( b * b - 4. * a * c);

		float t = ( 2. * c) / ( -d - b);

		logx = log10(C.midPoint.x) + ( t + j) * KNOT_INC_HIGH;
	} 
	else 
	{ //if ( logy >= log10(C.maxPoint.y) ) {
		logx = log10(C.maxPoint.x);
	}
  
	return pow(10, logx);
}

float segmented_spline_c9_fwd( float x, SegmentedSplineParams_c9 C )
{
	int N_KNOTS_LOW = 8;
	int N_KNOTS_HIGH = 8;

	// Check for negatives or zero before taking the log. If negative or zero,
	// set to OCESMIN.
	float xCheck = x <= 0 ? 1e-4 : x;

	float logx = log10(xCheck);
	float logy;

	if (logx <= log10(C.minPoint.x))
	{
		logy = logx * C.slopeLow + (log10(C.minPoint.y) - C.slopeLow * log10(C.minPoint.x));
	}
	else if ((logx > log10(C.minPoint.x)) && (logx < log10(C.midPoint.x)))
	{
		float knot_coord = (N_KNOTS_LOW - 1) * (logx - log10(C.minPoint.x)) / (log10(C.midPoint.x) - log10(C.minPoint.x));
		int j = int(knot_coord);
		float t = knot_coord - j;

		float3 cf = { C.coefsLow[j], C.coefsLow[j + 1], C.coefsLow[j + 2] };

		float3 monomials = { t * t, t, 1.0 };
		logy = dot(monomials, mul(cf, M));
	}
	else if ((logx >= log10(C.midPoint.x)) && (logx < log10(C.maxPoint.x)))
	{
		float knot_coord = (N_KNOTS_HIGH - 1.0) * (logx - log10(C.midPoint.x)) / (log10(C.maxPoint.x) - log10(C.midPoint.x));
		int j = int(knot_coord);
		float t = knot_coord - j;

		float3 cf = { C.coefsHigh[j], C.coefsHigh[j + 1], C.coefsHigh[j + 2] };

		float3 monomials = { t * t, t, 1.0 };
		logy = dot(monomials, mul(cf, M));
	}
	else//if ( logIn >= log10(C.maxPoint.x) )
	{
		logy = logx * C.slopeHigh + (log10(C.maxPoint.y) - C.slopeHigh * log10(C.maxPoint.x));
	}

	return pow( 10, logy );
}

float segmented_spline_c9_rev( float y, SegmentedSplineParams_c9 C )
{
	const int N_KNOTS_LOW = 8;
	const int N_KNOTS_HIGH = 8;

	float KNOT_INC_LOW = (log10(C.midPoint.x) - log10(C.minPoint.x)) / (N_KNOTS_LOW - 1.0);
	float KNOT_INC_HIGH = (log10(C.maxPoint.x) - log10(C.midPoint.x)) / (N_KNOTS_HIGH - 1.0);
  
	int i;

	// KNOT_Y is luminance of the spline at each knot
	float KNOT_Y_LOW[ N_KNOTS_LOW];
	for (i = 0; i < N_KNOTS_LOW; i = i+1) {
		KNOT_Y_LOW[ i] = ( C.coefsLow[i] + C.coefsLow[i+1]) / 2.0;
	};

	float KNOT_Y_HIGH[ N_KNOTS_HIGH];
	for (i = 0; i < N_KNOTS_HIGH; i = i+1) {
		KNOT_Y_HIGH[ i] = ( C.coefsHigh[i] + C.coefsHigh[i+1]) / 2.0;
	};

	float logy = log10( max( y, 1e-10));

	float logx;
	if (logy <= log10(C.minPoint.y)) {
		logx = log10(C.minPoint.x);
	} else if ( (logy > log10(C.minPoint.y)) && (logy <= log10(C.midPoint.y)) ) {
		uint j;
		float3 cf;
		if ( logy > KNOT_Y_LOW[ 0] && logy <= KNOT_Y_LOW[ 1]) {
			cf[ 0] = C.coefsLow[0];  cf[ 1] = C.coefsLow[1];  cf[ 2] = C.coefsLow[2];  j = 0;
		} else if ( logy > KNOT_Y_LOW[ 1] && logy <= KNOT_Y_LOW[ 2]) {
			cf[ 0] = C.coefsLow[1];  cf[ 1] = C.coefsLow[2];  cf[ 2] = C.coefsLow[3];  j = 1;
		} else if ( logy > KNOT_Y_LOW[ 2] && logy <= KNOT_Y_LOW[ 3]) {
			cf[ 0] = C.coefsLow[2];  cf[ 1] = C.coefsLow[3];  cf[ 2] = C.coefsLow[4];  j = 2;
		} else if ( logy > KNOT_Y_LOW[ 3] && logy <= KNOT_Y_LOW[ 4]) {
			cf[ 0] = C.coefsLow[3];  cf[ 1] = C.coefsLow[4];  cf[ 2] = C.coefsLow[5];  j = 3;
		} else if ( logy > KNOT_Y_LOW[ 4] && logy <= KNOT_Y_LOW[ 5]) {
			cf[ 0] = C.coefsLow[4];  cf[ 1] = C.coefsLow[5];  cf[ 2] = C.coefsLow[6];  j = 4;
		} else if ( logy > KNOT_Y_LOW[ 5] && logy <= KNOT_Y_LOW[ 6]) {
			cf[ 0] = C.coefsLow[5];  cf[ 1] = C.coefsLow[6];  cf[ 2] = C.coefsLow[7];  j = 5;
		} else if ( logy > KNOT_Y_LOW[ 6] && logy <= KNOT_Y_LOW[ 7]) {
			cf[ 0] = C.coefsLow[6];  cf[ 1] = C.coefsLow[7];  cf[ 2] = C.coefsLow[8];  j = 6;
		}
	
		float3 tmp = mul( cf, M);

		float a = tmp[ 0];
		float b = tmp[ 1];
		float c = tmp[ 2];
		c = c - logy;

		float d = sqrt( b * b - 4.0 * a * c);

		float t = ( 2.0 * c) / ( -d - b);

		logx = log10(C.minPoint.x) + ( t + j) * KNOT_INC_LOW;
	} else if ( (logy > log10(C.midPoint.y)) && (logy < log10(C.maxPoint.y)) ) {
		uint j;
		float3 cf;
		if ( logy > KNOT_Y_HIGH[ 0] && logy <= KNOT_Y_HIGH[ 1]) {
			cf[ 0] = C.coefsHigh[0];  cf[ 1] = C.coefsHigh[1];  cf[ 2] = C.coefsHigh[2];  j = 0;
		} else if ( logy > KNOT_Y_HIGH[ 1] && logy <= KNOT_Y_HIGH[ 2]) {
			cf[ 0] = C.coefsHigh[1];  cf[ 1] = C.coefsHigh[2];  cf[ 2] = C.coefsHigh[3];  j = 1;
		} else if ( logy > KNOT_Y_HIGH[ 2] && logy <= KNOT_Y_HIGH[ 3]) {
			cf[ 0] = C.coefsHigh[2];  cf[ 1] = C.coefsHigh[3];  cf[ 2] = C.coefsHigh[4];  j = 2;
		} else if ( logy > KNOT_Y_HIGH[ 3] && logy <= KNOT_Y_HIGH[ 4]) {
			cf[ 0] = C.coefsHigh[3];  cf[ 1] = C.coefsHigh[4];  cf[ 2] = C.coefsHigh[5];  j = 3;
		} else if ( logy > KNOT_Y_HIGH[ 4] && logy <= KNOT_Y_HIGH[ 5]) {
			cf[ 0] = C.coefsHigh[4];  cf[ 1] = C.coefsHigh[5];  cf[ 2] = C.coefsHigh[6];  j = 4;
		} else if ( logy > KNOT_Y_HIGH[ 5] && logy <= KNOT_Y_HIGH[ 6]) {
			cf[ 0] = C.coefsHigh[5];  cf[ 1] = C.coefsHigh[6];  cf[ 2] = C.coefsHigh[7];  j = 5;
		} else if ( logy > KNOT_Y_HIGH[ 6] && logy <= KNOT_Y_HIGH[ 7]) {
			cf[ 0] = C.coefsHigh[6];  cf[ 1] = C.coefsHigh[7];  cf[ 2] = C.coefsHigh[8];  j = 6;
		}
	
		float3 tmp = mul( cf, M);

		float a = tmp[ 0];
		float b = tmp[ 1];
		float c = tmp[ 2];
		c = c - logy;

		float d = sqrt( b * b - 4.0 * a * c);

		float t = ( 2.0 * c) / ( -d - b);

		logx = log10(C.midPoint.x) + ( t + j) * KNOT_INC_HIGH;
	} 
	else 
	{ //if ( logy >= log10(C.maxPoint.y) ) {
		logx = log10(C.maxPoint.x);
	}
  
	return pow(10, logx);
}

// Transformations from RGB to other color representations
float rgb_2_hue( float3 rgb ) 
{
	// Returns a geometric hue angle in degrees (0-360) based on RGB values.
	// For neutral colors, hue is undefined and the function will return a quiet NaN value.
	float hue;
	if ((rgb[0] == rgb[1]) && (rgb[1] == rgb[2]))
	{
		//hue = FLT_NAN; // RGB triplets where RGB are equal have an undefined hue
		hue = 0.0;
	}
	else
	{
		hue = (180.0 / Pi) * atan2(sqrt(3.0)*(rgb[1] - rgb[2]), 2.0 * rgb[0] - rgb[1] - rgb[2]);
	}

	if (hue < 0.0)
		hue = hue + 360.0;

	return clamp( hue, 0.0, 360.0 );
}

float rgb_2_yc( float3 rgb)
{
	// Converts RGB to a luminance proxy, here called YC
	// YC is ~ Y + K * Chroma
	// Constant YC is a cone-shaped surface in RGB space, with the tip on the 
	// neutral axis, towards white.
	// YC is normalized: RGB 1 1 1 maps to YC = 1
	//
	// ycRadiusWeight defaults to 1.75, although can be overridden in function 
	// call to rgb_2_yc
	// ycRadiusWeight = 1 -> YC for pure cyan, magenta, yellow == YC for neutral 
	// of same value
	// ycRadiusWeight = 2 -> YC for pure red, green, blue  == YC for  neutral of 
	// same value.

	float r = rgb[0]; 
	float g = rgb[1]; 
	float b = rgb[2];
  
	float chroma = sqrt(b*(b-g)+g*(g-r)+r*(r-b));
	float ycRadiusWeight = 1.75;
	return ( b + g + r + ycRadiusWeight * chroma) / 3.0;
}

// 
// Reference Rendering Transform (RRT)
//
//   Input is ACES
//   Output is OCES
//
float3 RRT( float3 aces )
{
	// "Glow" module constants
	float RRT_GLOW_GAIN = 0.05;
	float RRT_GLOW_MID = 0.08;

	float saturation = rgb_2_saturation( aces );
	float ycIn = rgb_2_yc( aces );
	float s = sigmoid_shaper( (saturation - 0.4) / 0.2);
	float addedGlow = 1.0 + glow_fwd( ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);
	aces *= addedGlow;
  
	// --- Red modifier --- //
	float RRT_RED_SCALE = 0.82;
	float RRT_RED_PIVOT = 0.03;
	float RRT_RED_HUE = 0.0;
	float RRT_RED_WIDTH = 135.0;
	float hue = rgb_2_hue( aces );
	float centeredHue = center_hue( hue, RRT_RED_HUE );
	float hueWeight = cubic_basis_shaper( centeredHue, RRT_RED_WIDTH );
		
	aces.r += hueWeight * saturation * (RRT_RED_PIVOT - aces.r) * (1.0 - RRT_RED_SCALE);

	// --- ACES to RGB rendering space --- //
	aces = clamp( aces, vec3(0.0), vec3(65535.0));  // avoids saturated negative colors from becoming positive in the matrix

	float3 rgbPre = mul( AP0_2_AP1_MAT, aces );

	rgbPre = clamp( rgbPre, vec3(0.0), vec3(65535.0));

	// --- Global desaturation --- //
	float RRT_SAT_FACTOR = 0.96;
	rgbPre = lerp( vec3(dot( rgbPre, AP1_RGB2Y )), rgbPre, vec3(RRT_SAT_FACTOR));

	// --- Apply the tonescale independently in rendering-space RGB --- //
	float3 rgbPost;
	rgbPost[0] = segmented_spline_c5_fwd( rgbPre[0] );
	rgbPost[1] = segmented_spline_c5_fwd( rgbPre[1] );
	rgbPost[2] = segmented_spline_c5_fwd( rgbPre[2] );

	// --- RGB rendering space to OCES --- //
	float3 rgbOces = mul( AP1_2_AP0_MAT, rgbPost );
	return rgbOces;
}

// 
// Inverse Reference Rendering Transform (RRT)
//
//   Input is OCES
//   Output is ACES
//

float3 Inverse_RRT( float3 oces )
{
	// "Glow" module constants
	float RRT_GLOW_GAIN = 0.05;
	float RRT_GLOW_MID = 0.08;

	// --- OCES to RGB rendering space --- //
	float3 rgbPre = mul(AP0_2_AP1_MAT, oces);

	// --- Apply the tonescale independently in rendering-space RGB --- //
	float3 rgbPost;
	rgbPost[0] = segmented_spline_c5_rev( rgbPre[0]);
	rgbPost[1] = segmented_spline_c5_rev( rgbPre[1]);
	rgbPost[2] = segmented_spline_c5_rev( rgbPre[2]);

	// --- Global desaturation --- //
	float RRT_SAT_FACTOR = 0.96;
	rgbPost = lerp( vec3(dot( rgbPost, AP1_RGB2Y )), rgbPost, vec3(rcp(RRT_SAT_FACTOR)) );

	rgbPost = clamp( rgbPost, 0.0, HALF_MAX);

	// --- RGB rendering space to ACES --- //
	float3 aces = mul( AP1_2_AP0_MAT, rgbPost);

	aces = clamp( aces, 0.0, HALF_MAX);

	// --- Red modifier --- //
	float RRT_RED_SCALE = 0.82;
	float RRT_RED_PIVOT = 0.03;
	float RRT_RED_HUE = 0.0;
	float RRT_RED_WIDTH = 135.0;
	float hue = rgb_2_hue( aces);
	float centeredHue = center_hue( hue, RRT_RED_HUE);
	float hueWeight = cubic_basis_shaper( centeredHue, RRT_RED_WIDTH);

	float minChan;
	if (centeredHue < 0) { 
		// min_f3(aces) = aces[1] (i.e. magenta-red)
		minChan = aces[1];
	} else { // min_f3(aces) = aces[2] (i.e. yellow-red)
		minChan = aces[2];
	}

	float a = hueWeight * (1.0 - RRT_RED_SCALE) - 1.0;
	float b = aces[0] - hueWeight * (RRT_RED_PIVOT + minChan) * (1.0 - RRT_RED_SCALE);
	float c = hueWeight * RRT_RED_PIVOT * minChan * (1.0 - RRT_RED_SCALE);

	aces[0] = ( -b - sqrt( b * b - 4.0 * a * c)) / ( 2.0 * a);

	// --- Glow module --- //
	float saturation = rgb_2_saturation( aces);
	float ycOut = rgb_2_yc( aces);
	float s = sigmoid_shaper( (saturation - 0.4) / 0.2);
	float reducedGlow = 1.0 + glow_inv( ycOut, RRT_GLOW_GAIN * s, RRT_GLOW_MID);

	aces = reducedGlow * aces;

	// Assign ACES RGB to output variables (ACES)
	return aces;
}

// 
// Output Device Transform - Rec.2020 (1000 cd/m^2)
//

//
// Summary :
//  This transform is intended for mapping OCES onto a Rec.2020 HDR display calibrated 
//  to a D65 white point at 1000 cd/m^2. The assumed observer adapted white is 
//  D65, and the viewing environment is that of a dim surround. 
//
// Device Primaries : 
//  Primaries are those specified in Rec. ITU-R BT.2020
//  CIE 1931 chromaticities:  x         y         Y
//              Red:          0.708     0.292
//              Green:        0.17      0.797
//              Blue:         0.131     0.046
//              White:        0.3127    0.329     1000 cd/m^2
//
// Display EOTF :
//  The reference electro-optical transfer function specified in SMPTE ST 
//  2084-2014. This transform makes no attempt to address the Annex functions 
//  which address integer quantization.
//
// Assumed observer adapted white point:
//         CIE 1931 chromaticities:    x            y
//                                     0.3127       0.329
//
// Viewing Environment:
//  This ODT is designed for a viewing environment more typically associated 
//  with video mastering.
//

float3 ODT_1000nits( float3 oces )
{
	// OCES to RGB rendering space
	float3 rgbPre = mul( AP0_2_AP1_MAT, oces );

	SegmentedSplineParams_c9 ODT_1000nits =
	{
		// coefsLow[10]
		{ -4.9706219331, -3.0293780669, -2.1262, -1.5105, -1.0578, -0.4668, 0.11938, 0.7088134201, 1.2911865799, 1.2911865799 },
		// coefsHigh[10]
		{ 0.8089132070, 1.1910867930, 1.5683, 1.9483, 2.3083, 2.6384, 2.8595, 2.9872608805, 3.0127391195, 3.0127391195 },
		{segmented_spline_c5_fwd( 0.18*pow(2.,-12.) ), 0.0001},    // minPoint
		{segmented_spline_c5_fwd( 0.18 ),                10.0},    // midPoint  
		{segmented_spline_c5_fwd( 0.18*pow(2.,10.) ),  1000.0},    // maxPoint
		3.0,  // slopeLow
		0.06  // slopeHigh
	};

	// Apply the tonescale independently in rendering-space RGB
	float3 rgbPost;
	rgbPost[0] = segmented_spline_c9_fwd( rgbPre[0], ODT_1000nits );
	rgbPost[1] = segmented_spline_c9_fwd( rgbPre[1], ODT_1000nits );
	rgbPost[2] = segmented_spline_c9_fwd( rgbPre[2], ODT_1000nits );

	// Subtract small offset to allow for a code value of 0
	rgbPost -= 0.00003507384284432574;

	return rgbPost;
}


// 
// Output Device Transform - P3D60 (2000 cd/m^2)
//

//
// Summary :
//  This transform is intended for mapping OCES onto an HDR display calibrated 
//  to a D60 white point at 2000 cd/m^2. The assumed observer adapted white is 
//  D60, and the viewing environment is that of a dim surround. 
//
// Device Primaries : 
//  CIE 1931 chromaticities:  x         y         Y
//              Red:          0.68      0.32
//              Green:        0.265     0.69
//              Blue:         0.15      0.06
//              White:        0.32168   0.33767   2000 cd/m^2
//
//  Also assumes a black level of 0.005 cd/m^2
//
// Display EOTF :
//  The reference electro-optical transfer function specified in SMPTE ST 
//  2084-2014. This transform makes no attempt to address the Annex functions 
//  which address integer quantization.
//
// Assumed observer adapted white point:
//         CIE 1931 chromaticities:    x            y
//                                     0.32168      0.33767
//
// Viewing Environment:
//  This ODT is designed for a viewing environment more typically associated 
//  with video mastering.
//

float3 ODT_2000nits( float3 oces )
{
	// OCES to RGB rendering space
	float3 rgbPre = mul( AP0_2_AP1_MAT, oces );

	SegmentedSplineParams_c9 ODT_2000nits =
	{
	  // coefsLow[10]
	  { -2.3010299957, -2.3010299957, -1.9312000000, -1.5205000000, -1.0578000000, -0.4668000000, 0.1193800000, 0.7088134201, 1.2911865799, 1.2911865799 },
	  // coefsHigh[10]
	  { 0.8019952042, 1.1980047958, 1.5943000000, 1.9973000000, 2.3783000000, 2.7684000000, 3.0515000000, 3.2746293562, 3.3274306351, 3.3274306351 },
	  {segmented_spline_c5_fwd( 0.18*pow(2.,-12.) ),  0.005},    // minPoint
	  {segmented_spline_c5_fwd( 0.18 ),                10.0},    // midPoint  
	  {segmented_spline_c5_fwd( 0.18*pow(2.,11.) ),  2000.0},    // maxPoint
	  0.0,  // slopeLow
	  0.12  // slopeHigh
	};

	// Apply the tonescale independently in rendering-space RGB
	float3 rgbPost;
	rgbPost[0] = segmented_spline_c9_fwd( rgbPre[0], ODT_2000nits );
	rgbPost[1] = segmented_spline_c9_fwd( rgbPre[1], ODT_2000nits );
	rgbPost[2] = segmented_spline_c9_fwd( rgbPre[2], ODT_2000nits );

	return rgbPost;
}

//
// ACES D65 1000 nit Output Transform - Forward
//  Input is scene-referred linear values in the sRGB gamut
//  Output is output-referred linear values in the AP1 gamut
//
float3 ACESOutputTransforms1000( float3 SceneReferredLinearsRGBColor )
{
	float3x3 sRGB_2_AP0 = mul( XYZ_2_AP0_MAT, mul( D65_2_D60_CAT, sRGB_2_XYZ_MAT ) );

	float3 aces = mul( sRGB_2_AP0, SceneReferredLinearsRGBColor * 1.5 );
	float3 oces = RRT( aces );
	float3 OutputReferredLinearAP1Color = ODT_1000nits( oces );
	return OutputReferredLinearAP1Color;
}

//
// ACES D65 2000 nit Output Transform - Forward
//  Input is scene-referred linear values in the sRGB gamut
//  Output is output-referred linear values in the AP1 gamut
//
float3 ACESOutputTransforms2000( float3 SceneReferredLinearsRGBColor )
{
	float3x3 sRGB_2_AP0 = mul( XYZ_2_AP0_MAT, mul( D65_2_D60_CAT, sRGB_2_XYZ_MAT ) );

	float3 aces = mul( sRGB_2_AP0, SceneReferredLinearsRGBColor * 1.5 );
	float3 oces = RRT( aces );
	float3 OutputReferredLinearAP1Color = ODT_2000nits( oces );
	return OutputReferredLinearAP1Color;
}

float3x3 GamutMappingIdentityMatrix = 
{
	{1.0, 0.0, 0.0}, 
	{0.0, 1.0, 0.0}, 
	{0.0, 0.0, 1.0 }
};

float3x3 OuputGamutMappingMatrix( int OutputGamut )
{
	// Gamut mapping matrices used later
	float3x3 AP1_2_sRGB    = mul( XYZ_2_sRGB_MAT,		mul(D60_2_D65_CAT,	AP1_2_XYZ_MAT) );
	float3x3 AP1_2_DCI_D65 = mul( XYZ_2_P3D65_MAT,		mul(D60_2_D65_CAT,	AP1_2_XYZ_MAT) );
	float3x3 AP1_2_Rec2020 = mul( XYZ_2_Rec2020_MAT,	mul(D60_2_D65_CAT,	AP1_2_XYZ_MAT) );

	// Set gamut mapping matrix 
	// 0 = sRGB - D65
	// 1 = P3 - D65
	// 2 = Rec.2020 - D65
	// 3 = ACES AP0 - D60
	// 4 = ACES AP1 - D60

	if( OutputGamut == 1 )
		return AP1_2_DCI_D65;
	else if( OutputGamut == 2 )
		return AP1_2_Rec2020;
	else if( OutputGamut == 3 )
		return AP1_2_AP0_MAT;
	else if( OutputGamut == 4 )
		return GamutMappingIdentityMatrix;
	else
		return AP1_2_sRGB;
}

vec3 LinearToST2084(vec3 lin)
{
	const float m1 = 0.1593017578125; // = 2610. / 4096. * .25;
	const float m2 = 78.84375; // = 2523. / 4096. *  128;
	const float c1 = 0.8359375; // = 2392. / 4096. * 32 - 2413./4096.*32 + 1;
	const float c2 = 18.8515625; // = 2413. / 4096. * 32;
	const float c3 = 18.6875; // = 2392. / 4096. * 32;
	const float C = 10000.;

	vec3 L = lin/C;
	vec3 Lm = pow(L, vec3(m1));
	vec3 N1 = ( c1 + c2 * Lm );
	vec3 N2 = ( 1.0 + c3 * Lm );
	vec3 N = N1 * rcp(N2);
	vec3 P = pow( N, vec3(m2) );
	
	return P;
}

float Square( float x )
{
	return x*x;
}

vec2 Square( vec2 x )
{
	return x*x;
}

vec3 Square( vec3 x )
{
	return x*x;
}

vec4 Square( vec4 x )
{
	return x*x;
}

// Accurate for 1000K < Temp < 15000K
// [Krystek 1985, "An algorithm to calculate correlated colour temperature"]
vec2 PlanckianLocusChromaticity( float Temp )
{
	float u = ( 0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp*Temp ) / ( 1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp*Temp );
	float v = ( 0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp*Temp ) / ( 1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp*Temp );

	float x = 3*u / ( 2*u - 8*v + 4 );
	float y = 2*v / ( 2*u - 8*v + 4 );

	return vec2(x,y);
}

// Accurate for 4000K < Temp < 25000K
// in: correlated color temperature
// out: CIE 1931 chromaticity
vec2 D_IlluminantChromaticity( float Temp )
{
	// Correct for revision of Plank's law
	// This makes 6500 == D65
	Temp *= 1.4388 / 1.438;
	float OneOverTemp = 1.0/Temp;
	float x =	Temp <= 7000 ?
				0.244063 + ( 0.09911e3 + ( 2.9678e6 - 4.6070e9 * OneOverTemp ) * OneOverTemp) * OneOverTemp:
				0.237040 + ( 0.24748e3 + ( 1.9018e6 - 2.0064e9 * OneOverTemp ) * OneOverTemp ) * OneOverTemp;
	
	float y = -3 * x*x + 2.87 * x - 0.275;

	return vec2(x,y);
}

vec2 PlanckianIsothermal( float Temp, float Tint )
{
	float u = ( 0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp*Temp ) / ( 1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp*Temp );
	float v = ( 0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp*Temp ) / ( 1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp*Temp );

	float ud = ( -1.13758118e9f - 1.91615621e6f * Temp - 1.53177f * Temp*Temp ) / Square( 1.41213984e6f + 1189.62f * Temp + Temp*Temp );
	float vd = (  1.97471536e9f - 705674.0f * Temp - 308.607f * Temp*Temp ) / Square( 6.19363586e6f - 179.456f * Temp + Temp*Temp );

	vec2 uvd = normalize( vec2( u, v ) );

	// Correlated color temperature is meaningful within +/- 0.05
	u += -uvd.y * Tint * 0.05;
	v +=  uvd.x * Tint * 0.05;
	
	float x = 3*u / ( 2*u - 8*v + 4 );
	float y = 2*v / ( 2*u - 8*v + 4 );

	return vec2(x,y);
}

// Remap a value in the range [A,B] to [C,D].
float RemapVal( float val, float A, float B, float C, float D){

  if ( A == B )
    return val >= B ? D : C;

  return C + (D - C) * (val - A) / (B - A);
}

vec3 WhiteBalance( vec3 LinearColor ){
	float WhiteTemp = RemapVal(u_WhiteTemp, 100.0, 25000.0, 25000.0, 100.0);
	
	vec2 SrcWhiteDaylight = D_IlluminantChromaticity( WhiteTemp );
	vec2 SrcWhitePlankian = PlanckianLocusChromaticity( WhiteTemp );

	vec2 SrcWhite = WhiteTemp < 4000 ? SrcWhitePlankian : SrcWhiteDaylight;
	vec2 D65White = vec2( 0.31270,  0.32900 );

	// Offset along isotherm
	vec2 Isothermal = PlanckianIsothermal( WhiteTemp, u_WhiteTint ) - SrcWhitePlankian;
	SrcWhite += Isothermal;

	mat3 WhiteBalanceMat = ChromaticAdaptation( SrcWhite, D65White );

	WhiteBalanceMat = sRGB_2_XYZ_MAT * WhiteBalanceMat;
	WhiteBalanceMat = WhiteBalanceMat * XYZ_2_sRGB_MAT;

	return LinearColor * WhiteBalanceMat;
}
