/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.


    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


	source:
		quantizer module ; neuquant neural-net quantization

	revision history:
		???/??/1997 - Thomas Mannfred Carlsson - initial revision
		???/??/1997 - Thomas Mannfred Carlsson - bugfixes to original NeuQuant
		???/??/1997 - Jukka Liimatta - optimizations
		Jan/10/1999 - Jukka Liimatta - prophecy interface
		Jan/24/2001 - Jukka Liimatta - renaissance build
		
	notes:
	- reference implementation (c) 1994 Anthony Becker
	- original algorithm by Kohonen
*/
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// neuquant
// =================================================

// number of colours used
#define netsize		256

#define prime1		499
#define prime2		491
#define prime3		487
#define prime4		503

#define minpicturebytes	( 3 * prime4 )


#define maxnetpos		(netsize-1)
#define netbiasshift	4
#define ncycles			100

#define intbiasshift    16
#define intbias			(((int) 1)<<intbiasshift)
#define gammashift  	10
#define gamma   		(((int) 1)<<gammashift)
#define betashift  		10
#define beta			(intbias>>betashift)
#define betagamma		(intbias<<(gammashift-betashift))

#define initrad			(netsize>>3)
#define radiusbiasshift	6
#define radiusbias		(((int) 1)<<radiusbiasshift)
#define initradius		(initrad*radiusbias)
#define radiusdec		30

#define alphabiasshift	10
#define initalpha		(((int) 1)<<alphabiasshift)

#define radbiasshift	8
#define radbias			(((int) 1)<<radbiasshift)
#define alpharadbshift  (alphabiasshift+radbiasshift)
#define alpharadbias    (((int) 1)<<alpharadbshift)



static uint8* thepicture;		//  the input image itself
static int lengthcount;			//  lengthcount = H*W*4
static int samplefac;			//  sampling factor 1..30


typedef int pixel[4];
static pixel network[netsize];

static int netindex[256];

static int bias [netsize];
static int freq [netsize];
static int radpower[initrad];


static void initnet(uint8* thepic, int len, int sample)
{
	thepicture  = thepic;
	lengthcount = len;
	samplefac   = sample;

	for ( int i=0; i<netsize; i++ )
	{
		int* p = network[i];
		p[0] = p[1] = p[2] = (i << (netbiasshift+8))/netsize;
		freq[i] = intbias/netsize;
		bias[i] = 0;
	}
}


static void unbiasnet()
{
	for ( int i=0; i<netsize; i++ )
	{
		for (int j=0; j<3; j++)
			network[i][j] >>= netbiasshift;
		network[i][3] = i;
	}
}


static void writecolourmap(Color32* palette)
{
	for ( int i=0; i<netsize; i++ )
	{
		palette[i].r = network[i][2];
		palette[i].g = network[i][1];
		palette[i].b = network[i][0];
		palette[i].a = 0xff;
	}
}


template <typename T>
void swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}


static void inxbuild()
{
	int smallpos,smallval;
	int *p, *q;
	int previouscol,startpos;

	previouscol = 0;
	startpos = 0;
	for (int i=0; i<netsize; i++)
	{
		p = network[i];
		smallpos = i;
		smallval = p[1];

		for (int j=i+1; j<netsize; j++)
		{
			q = network[j];
			if (q[1] < smallval)
			{
				smallpos = j;
				smallval = q[1];
			}
		}
		q = network[smallpos];

		if (i != smallpos)
		{
			swap( q[0], p[0] );
			swap( q[1], p[1] );
			swap( q[2], p[2] );
			swap( q[3], p[3] );
		}

		if (smallval != previouscol)
		{
			netindex[previouscol] = (startpos+i)>>1;
			for (int j=previouscol+1; j<smallval; j++) netindex[j] = i;
			previouscol = smallval;
			startpos = i;
		}
	}
	netindex[previouscol] = (startpos+maxnetpos)>>1;
	for (int j=previouscol+1; j<256; j++) netindex[j] = maxnetpos;

}


static int inxsearch(int b, int g, int r)
{
	int	dist, a;
	int* p;

	int	bestd = 1000;
	int	best = -1;
	int	i = netindex[g];
	int	j = i-1;

	while ((i<netsize) || (j>=0))
	{
		if (i<netsize)
		{
			p = network[i];
			dist = p[1] - g;
			if (dist >= bestd)
				i = netsize;
			else
			{
				i++;
				if (dist<0) dist = -dist;
				a = p[0] - b;   if (a<0) a = -a;
				dist += a;
				if (dist<bestd)
				{
					a = p[2] - r;   if (a<0) a = -a;
					dist += a;
					if (dist<bestd) {bestd=dist; best=p[3];}
				}
			}
		}
		if (j>=0)
		{
			p = network[j];
			dist = g - p[1];
			if (dist >= bestd)
				j = -1;
			else
			{
				j--;
				if (dist<0) dist = -dist;
				a = p[0] - b;   if (a<0) a = -a;
				dist += a;
				if (dist<bestd)
				{
					a = p[2] - r;   if (a<0) a = -a;
					dist += a;
					if (dist<bestd) {bestd=dist; best=p[3];}
				}
			}
		}
	}

	return best;
}


static inline int contest(int b, int g, int r)
{
	int	bestd = ~(((int)1) << 31);
	int	bestbiasd = bestd;
	int	bestpos = -1;
	int	bestbiaspos = bestpos;
	int	*p = bias;
	int	*f = freq;


	pixel	*pnet = network;
	for (int i=0; i<netsize; i++)
	{
		int dist, a, biasdist, betafreq;
		int	*n = (int*)pnet++;

		dist = n[0] - b;	if (dist<0) dist = -dist;

		a = n[1] - g;
		if ( a<0 ) 	dist -= a;
		else		dist += a;

		a = n[2] - r;
		if ( a<0 ) 	dist -= a;
		else		dist += a;

		if (dist<bestd) {bestd=dist; bestpos=i;}
		biasdist = dist - ((*p) >> (intbiasshift-netbiasshift));
		if (biasdist<bestbiasd) {bestbiasd=biasdist; bestbiaspos=i;}

		betafreq = (*f >> betashift);
		*f++ -= betafreq;
		*p++ +=(betafreq << gammashift);
	}
	freq[bestpos] += beta;
	bias[bestpos] -= betagamma;

	return bestbiaspos;
}


static inline void altersingle( int alpha, int i, int b, int g, int r )
{
	int *n = network[i];

	n[0] -= (alpha * (n[0] - b)) >> alphabiasshift;
	n[1] -= (alpha * (n[1] - g)) >> alphabiasshift;
	n[2] -= (alpha * (n[2] - r)) >> alphabiasshift;
}


static inline void alterneigh( int rad, int i, int b, int g, int r )
{
	int	lo = i-rad;   if (lo<-1) lo=-1;
	int	hi = i+rad;   if (hi>netsize) hi=netsize;
	int	j  = i+1;
	int	k  = i-1;

	int	*q = radpower;
	while ((j<hi) || (k>lo))
	{
		int	*p;
		int	a = *(++q);


		if (j<hi)
		{
			p = network[j++];
			p[0] -= (a * (p[0] - b)) >> alpharadbshift;
			p[1] -= (a * (p[1] - g)) >> alpharadbshift;
			p[2] -= (a * (p[2] - r)) >> alpharadbshift;
		}
		if (k>lo)
		{
			p = network[k--];
			p[0] -= (a * (p[0] - b)) >> alpharadbshift;
			p[1] -= (a * (p[1] - g)) >> alpharadbshift;
			p[2] -= (a * (p[2] - r)) >> alpharadbshift;
		}
	}

}


static void learn()
{
	int		alphadec = 30 + ((samplefac-1)/3);
	uint8*	p = thepicture;
	uint8*	lim = thepicture + lengthcount;
	int		samplepixels = lengthcount/(4*samplefac);
	int		delta = samplepixels/ncycles;
	int		alpha = initalpha;
	int		radius = initradius;

	int		rad = radius >> radiusbiasshift;
	if (rad > 1)
	{
		int	radrad = rad*rad;
		int	adder = 1;
		int	bigalpha = (alpha << radbiasshift) / radrad;

		for (int i=0; i<rad; i++)
		{
			radpower[i] = bigalpha * radrad;
			radrad -= adder;
			adder += 2;
		}
	}
	else
		rad = 0;


	int	step;
	if ((lengthcount%prime1) != 0)
		step = 4*prime1;
	else
	{
		if ((lengthcount%prime2) !=0)
			step = 4*prime2;
		else
		{
			if ((lengthcount%prime3) !=0)
				step = 4*prime3;
			else
				step = 4*prime4;
		}
	}


	int 	j, b, g, r;
	int i = 0;
	int	phase = 0;
	while (i++ < samplepixels)
	{
		b = p[0] << netbiasshift;
		g = p[1] << netbiasshift;
		r = p[2] << netbiasshift;
		j = contest(b,g,r);

		altersingle(alpha,j,b,g,r);
		if (rad) alterneigh(rad,j,b,g,r);

		p += step;
		while ( p >= lim ) p -= lengthcount;

		if ( ++phase == delta )
		{
			phase = 0;


			alpha -= alpha / alphadec;
			radius -= radius / radiusdec;
			rad = radius >> radiusbiasshift;

			if (rad > 1)
			{
				int	radrad = rad*rad;
				int	adder = 1;
				int	bigalpha = (alpha << radbiasshift) / radrad;

				for (int jj=0; jj<rad; jj++)
				{
					radpower[jj] = bigalpha * radrad;
					radrad -= adder;
					adder += 2;
				}
			}
			else
				rad = 0;
		}
	}
}


// =================================================
// Bitmap::QuantizeImage()
// =================================================

void Bitmap::QuantizeImage(float quality)
{
	// assert
	assert( image );
	if ( format.IsIndexed() ) 
		return;
	
	// prepare for quantization
	ReformatImage(PIXELFORMAT_ARGB8888);
	
	// allocate arrays
	Bitmap temp(width,height,PIXELFORMAT_PALETTE8(NULL));

	// quantize
	initnet(reinterpret_cast<uint8*>(image),width*height*4,
		30-static_cast<int>(quality*29.0f+1.0f));

	learn();
	unbiasnet();
	writecolourmap(temp.format.GetPalette());
	inxbuild();

	// remap
	uint8* src = reinterpret_cast<uint8*>(image);
	uint8* dest = reinterpret_cast<uint8*>(temp.image);
	int count = width * height;
	if ( count < 1 )
		return;

	while ( count-- )
	{
		*dest++ = static_cast<uint8>(inxsearch(src[0],src[1],src[2]));
		src += 4;
	}

	// swap pointers
	delete[] image;
	image = temp.image;
	temp.image = NULL;

	format = temp.format;
	pitch  = temp.pitch;
}
