// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
///
/// \file		cyIrradianceMap.h 
/// \author		Cem Yuksel
/// \version	0.3
/// \date		December 13, 2013
///
/// \brief irradiance map class.
///
///
/// @copydoc cyIrradianceMap
///
/// A simple class to store irradiance values for rendering using Monte Carlo
/// sampling for indirect illumination.
///
//-------------------------------------------------------------------------------

#ifndef _CY_IRRADIANCE_MAP_H_INCLUDED_
#define _CY_IRRADIANCE_MAP_H_INCLUDED_

//-------------------------------------------------------------------------------

#define CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_COLOR	0.04f
#define CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_Z		0.05f
#define CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_NORMAL	0.7f

//-------------------------------------------------------------------------------

/// Irradiance map class
template <class T> class cyIrradianceMap
{
public:
	cyIrradianceMap() : data(NULL), width(0), height(0), subdiv(0) {}
	virtual ~cyIrradianceMap() { if ( data ) delete [] data; }

	/// Initialize once by providing the image with and height.
	/// The subdiv parameter determines how many initial computation points will be generated.
	/// When subdiv is negative, a computation point is generated with 2^(-subdiv) pixels apart.
	/// When subdiv is positive, 2^(subdiv) computation points are generated per pixel.
	/// Typical usage: initialize with a small subdiv value and call Subdivide to generate
	/// more computation points.
	/// All computation points are marked as "invalid" and they are marked "valid"
	/// when their values are set using the Set method.
	void Initialize(int _width, int _height, int _subdiv=-5)
	{
		if ( data ) delete [] data;
		width  = _width;
		height = _height;
		subdiv = _subdiv;
		int n = GetDataCount(subdiv);
		data = new T[n];
		valid.SetSize(n);
		computed.SetSize(n);
	}

	/// Increments the subdivision, thereby generating more computation points.
	/// The values of the new points are interpolated from the previous points.
	/// If the interpolation is not good enough, the new point is marked as "invalid."
	/// If the conservative flag is set, "invalid" new points make other new
	/// points around them "invalid" as well.
	void Subdivide(bool conservative=true)
	{
		int s = subdiv+1;
		int ws1 = (width>>-subdiv);
		int hs1 = (height>>-subdiv);
		int w1, h1, w2, h2;
		GetDataCount(subdiv, w1, h1);
		int n = GetDataCount(s, w2, h2);
		T *d = new T[n];
		valid.SetSize(n);
		Validity computed2;
		computed2.SetSize(n);

		int endX=0, endY=0;
		float fx=0, fy=0;
		if ( subdiv < 0 ) {
			int rx = width % int(1<<-subdiv);
			if ( rx > 0 ) endX++;
			if ( rx > int(1<<-s) ) {
				endX++;
				fx = float(1<<-s) / float(rx);
			}
			int ry = height % int(1<<-subdiv);
			if ( ry > 0 ) endY++;
			if ( ry > int(1<<-s) ) {
				endY++;
				fy = float(1<<-s) / float(ry);
			}
		}

		d[0] = data[0];
		valid.Set(0);
		computed2.Set(0,computed.Get(0));
		int i=1, ii=0;
		{
			int x=0;
			for ( ; x<ws1; x++ ) {
				bool v = Interpolate( d[i], data[x], data[x+1] );
				valid.Set(i++,v);
				d[i] = data[x+1];
				valid.Set(i);
				computed2.Set(i++,computed.Get(x+1));
			}
			if ( endX > 1 ) {
				bool v = Interpolate( d[i], data[x], data[x+1], fx);
				valid.Set(i++,v);
			}
			if ( endX > 0 ) {
				d[i] = data[x+1];
				valid.Set(i);
				computed2.Set(i++,computed.Get(x+1));
			}
		}

		for ( int y=0; y<hs1; y++ ) {
			int x = ii;
			int xe = ii + ws1;

			int x2 = ii+w1+1;
			bool v1 = Interpolate( d[i], data[x], data[x2] );
			valid.Set(i++,v1);
			for ( ; x<xe; x++, x2++, i+=2 ) {
				bool v2 = Interpolate( d[i+1], data[x+1], data[x2+1] );
				valid.Set(i+1,v2);
				if ( v1 && v2 ) {
					bool v = Interpolate( d[i], d[i-1], d[i+1] );
					valid.Set(i,v);
				}
				v1 = v2;
			}
			if ( endX > 1 ) {
				bool v2 = Interpolate( d[i+1], data[x+1], data[x2+1] );
				valid.Set(i+1,v2);
				if ( v1 && v2 ) {
					bool v = Interpolate( d[i], d[i-1], d[i+1], fx );
					valid.Set(i,v);
				}
				i += 2;
				x++;
			} else if ( endX > 0 ) {
				bool v = Interpolate( d[i], data[x+1], data[x2+1]);
				valid.Set(i++,v);
				x++;
			}

			x++;
			ii = x;
			xe = x + ws1;
			d[i] = data[x];
			valid.Set(i);
			computed2.Set(i++,computed.Get(x));
			for ( ; x<xe; x++ ) {
				bool v = Interpolate( d[i], data[x], data[x+1] );
				valid.Set(i++,v);
				d[i] = data[x+1];
				valid.Set(i);
				computed2.Set(i++,computed.Get(x+1));
			}
			if ( endX > 1 ) {
				bool v = Interpolate( d[i], data[x], data[x+1], fx);
				valid.Set(i++,v);
			}
			if ( endX > 0 ) {
				d[i] = data[x+1];
				valid.Set(i);
				computed2.Set(i++,computed.Get(x+1));
			}
		}

		if ( endY > 1 ) {
			int x = ii;
			int xe = ii + ws1;

			int x2 = ii+w1+1;
			bool v1 = Interpolate( d[i], data[x], data[x2], fy );
			valid.Set(i++,v1);
			for ( ; x<xe; x++, x2++, i+=2 ) {
				bool v2 = Interpolate( d[i+1], data[x+1], data[x2+1], fy );
				valid.Set(i+1,v2);
				if ( v1 && v2 ) {
					bool v = Interpolate( d[i], d[i-1], d[i+1] );
					valid.Set(i,v);
				}
				v1 = v2;
			}
			if ( endX > 1 ) {
				bool v2 = Interpolate( d[i+1], data[x+1], data[x2+1], fy );
				valid.Set(i+1,v2);
				if ( v1 && v2 ) {
					bool v = Interpolate( d[i], d[i-1], d[i+1], fx );
					valid.Set(i,v);
				}
				i += 2;
				x++;
			} else if ( endX > 0 ) {
				bool v = Interpolate( d[i], data[x+1], data[x2+1], fy);
				valid.Set(i++,v);
				x++;
			}
		}
		if ( endY > 0 ) {
			int x = ii + w1 + 1;
			int xe = x + ws1;
			d[i] = data[x];
			valid.Set(i++);
			for ( ; x<xe; x++ ) {
				bool v = Interpolate( d[i], data[x], data[x+1] );
				valid.Set(i++,v);
				d[i] = data[x+1];
				valid.Set(i);
				computed2.Set(i++,computed.Get(x+1));
			}
			if ( endX > 1 ) {
				bool v = Interpolate( d[i], data[x], data[x+1], fx);
				valid.Set(i++,v);
			}
			if ( endX > 0 ) {
				d[i] = data[x+1];
				valid.Set(i);
				computed2.Set(i++,computed.Get(x+1));
			}
		}
		
		delete [] data;
		data = d;
		subdiv = s;

		computed.SwapData(computed2);
		computed2.SetSize(0);

		if ( conservative ) {
			for ( int i=0, y=0; y<h2; y++ ) {
				int x = i;
				int xe = i+w2;
				if ( (y&1)==0 ) {
					x++;
					for ( int ix=xe+2; x<xe; x+=2, ix+=2 ) {
						if ( ! valid.Get(ix) ) valid.Clear(x);
					}
				} else {
					for ( ; x<xe; x++ ) {
						if ( ! valid.Get(x+1) ) valid.Clear(x);
					}
				}
				i = xe+1;
			}
			for ( int i=n-1, y=h2; y>0; y-- ) {
				int x = i;
				int xe = i-w2;
				if ( (y&1)==0 || y==h2 ) {
					x--;
					for ( int ix=xe-2; x>xe; x-=2, ix-=2 ) {
						if ( ! valid.Get(ix) ) valid.Clear(x);
					}
				} else {
					for ( ; x>xe; x-- ) {
						if ( ! valid.Get(x-1) ) valid.Clear(x);
					}
				}
				i = xe-1;
			}
		}

		Validity valid1;
		valid1.CopyDataFrom(valid,n);
		for ( int i=0, y=0; y<h2; y++ ) {
			for ( int x=0; x<w2; x++, i++ ) {
				if ( valid1.Get(i) && !computed.Get(i) ) {
					if ((y>0    && !valid1.Get(i-w2)) ||
						(x>0    && !valid1.Get(i-1) ) ||
						(y<h2-1 && !valid1.Get(i+w2)) ||
						(x<w2-1 && !valid1.Get(i+1) ) ||
						(y>0    && x>0    && !valid1.Get(i-w2-1)) ||
						(y>0    && x<w2-1 && !valid1.Get(i-w2+1)) ||
						(y<h2-1 && x>0    && !valid1.Get(i+w2-1)) ||
						(y<h2-1 && x<w2-1 && !valid1.Get(i+w2+1)) )
					{
						valid.Clear(i);
					}
				}
			}
		}
		valid1.SetSize(0);
	}

	/// Returns the number of computation points.
	int GetDataCount() const { return GetDataCount(subdiv); }

	/// Sets the value of a point and marks it as valid.
	void Set(int i, const T& v) { data[i]=v; valid.Set(i); computed.Set(i); }

	/// Returns the current subdivision level
	int GetSubdivLevel() const { return subdiv; }

	/// Returns if the point with the given index is valid.
	/// If it is invalid, it must be computed and Set.
	bool IsValid(int i) const { return valid.Get(i); }

	/// Returns the value of a point.
	const T& Get(int i) const { return data[i]; }

	/// Returns the image position of the given computation point.
	void GetPosition(int i, float &x, float &y) const
	{
		int w, h;
		GetDataCount(subdiv, w, h);
		int ix = i % (w+1);
		int iy = i / (w+1);
		float skip = (subdiv<=0) ? (1<<-subdiv) : (1.0f/(1<<subdiv));
		x = skip * ix;
		if ( x > width ) x = (float) width;
		y = skip * iy;
		if ( y > height ) y = (float) height;
	}

	/// Evaluates the value at a given image position by interpolating
	/// the values of the computation points.
	/// Use this method after the computation is done.
	/// The given val should include information that is necessary
	/// to determine is the interpolation in the Filter call is good enough.
	bool Eval(T &val, float x, float y) const
	{
		if ( x<0 || y<0 || x>width || y>height ) return false;
		float iskip = (subdiv<0) ? (1.0f/(1<<-subdiv)) : (1<<subdiv);
		float xx = x*iskip;
		float yy = y*iskip;
		int ix = (int)xx;
		int iy = (int)yy;
		float fx = xx - (float)ix;
		float fy = yy - (float)iy;
		int ix2 = ix+1;
		int iy2 = iy+1;
		int w, h;
		GetDataCount(subdiv, w, h);
		if ( ix >= w+1 ) ix=ix2=w+1;
		else if ( ix == w ) {
			float skip = 1.0f / iskip;
			float d = width - skip*w;
			if ( d < skip ) fx *= d*iskip;
		}
		if ( iy >= h+1 ) iy=iy2=h+1;
		else if ( iy == h ) {
			float skip = 1.0f / iskip;
			float d = height - skip*h;
			if ( d < skip ) fy *= d*iskip;
		}
		T vx1=val, vx2=val;
		int i0 = iy *(w+1)+ix;
		int i1 = iy *(w+1)+ix2;
		int i2 = iy2*(w+1)+ix;
		int i3 = iy2*(w+1)+ix2;
		Filter(vx1, data[i0], data[i1], fx );
		Filter(vx2, data[i2], data[i3], fx );
		Filter(val, vx1, vx2, fy );
		return true;
	}

protected:

	/// Computes the average of the given two inputs and writes the interpolated result to outVal.
	/// If the interpolation is not good enough, returns false; otherwise, returns true.
	bool Interpolate( T& outVal, const T& input1, const T& input2 ) const { return Interpolate(outVal,input1,input2,0.5f); }

	/// Interpolates the given two inputs using the given weight and writes the interpolated result to outVal.
	/// If the interpolation is not good enough, returns false; otherwise, returns true.
	virtual bool Interpolate( T& outVal, const T& input1, const T& input2, float weight2 ) const=0;

	/// Interpolates the given two inputs using the given weight and writes the interpolated result to outVal.
	/// If the interpolation is not good enough, picks one of the inputs instead of interpolating them.
	virtual void Filter( T& outVal, const T& input1, const T& input2, float weight2 ) const 
	{
		if ( ! Interpolate(outVal,input1,input2,weight2) ) outVal = weight2 < 0.5f ? input1 : input2;
	}

private:
	class Validity
	{
	public:
		Validity() : validity(NULL) {}
		~Validity() { if ( validity ) delete [] validity; }
		void SetSize( int n )
		{
			if ( validity ) delete [] validity;
			validity = NULL;
			if ( n > 0 ) {
				int count = DataSize(n);
				validity = new char[count];
				for ( int i=0; i<count; i++ ) validity[i] = 0;
			}
		}
		void SwapData(Validity &v) { char *vd=validity; validity=v.validity; v.validity=vd; }
		void CopyDataFrom(const Validity &v, int n) {
			if ( validity ) delete [] validity;
			validity = NULL;
			if ( n > 0 ) {
				int count = DataSize(n);
				validity = new char[count];
				for ( int i=0; i<count; i++ ) validity[i] = v.validity[i];
			}
		}
		bool Get  (int i) const { return ((validity[i>>3]>>(i&7))&1) > 0; }
		void Clear(int i) { validity[i>>3] &= ~(char(1 << (i&7))); }
		void Set  (int i) { validity[i>>3] |=   char(1 << (i&7));  }
		void Set  (int i, bool set) { if (set) Set(i); else Clear(i); }
	private:
		char *validity;
		int DataSize(int n) {
			int count = (n >> 3);
			if ( (n&7) > 0 ) count++;
			return count;
		}
	};

	int width, height;
	int subdiv;
	Validity valid;
	Validity computed;
	T *data;

	int GetDataCount(int sub, int &w, int &h) const
	{
		if ( sub < 0 ) {
			w = (width >> -sub);
			if ( width % (1<<-sub) > 0 ) w++;
			h = (height >> -sub);
			if ( height % (1<<-sub) > 0 ) h++;
		} else {
			w = (width << sub);
			h = (height << sub);
		}
		return (w+1)*(h+1);
	}
	int GetDataCount(int sub) const
	{
		int w, h;
		return GetDataCount(sub, w, h);
	}
};

//-------------------------------------------------------------------------------

/// Irradiance map for a single floating point value per computation.
/// Uses a threshold value to determine if the interpolation is good enough.
class cyIrradianceMapFloat : public cyIrradianceMap<float>
{
public:
	cyIrradianceMapFloat() : threshold(CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_COLOR) {}
	void SetThreshold(float t) { threshold=t; }
protected:
	virtual bool Interpolate( float& outVal, const float& input1, const float& input2, float weight2 )
	{
		float d = input2 - input1;
		outVal = input1 + d*weight2;
		return fabs(d) < threshold;
	}
private:
	float threshold;
};

//-------------------------------------------------------------------------------

/// Irradiance map for a single color value per computation.
/// Uses a threshold value to determine if the interpolation is good enough.
class cyIrradianceMapColor : public cyIrradianceMap<cyColor>
{
public:
	cyIrradianceMapColor() : threshold(CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_COLOR) {}
	cyIrradianceMapColor(float _threshold) { SetThreshold(_threshold); }
	cyIrradianceMapColor(cyColor _threshold) : threshold(_threshold) {}
	void SetThreshold(float t) { threshold.Set(t,t,t); }
	void SetThreshold(const cyColor &t) { threshold=t; }
protected:
	virtual bool Interpolate( cyColor& outVal, const cyColor& input1, const cyColor& input2, float weight2 ) const
	{
		cyColor d = input2 - input1;
		outVal = input1 + d*weight2;
		return fabs(d.r)<=threshold.r && fabs(d.g)<=threshold.g && fabs(d.b)<threshold.b;
	}
private:
	cyColor threshold;
};

//-------------------------------------------------------------------------------

/// Irradiance map for with a color and a z-depth value per computation.
/// Uses a color and a z-depth threshold value to determine if the interpolation is good enough.
class cyIrradianceMapColorZ : public cyIrradianceMap<cyColorA>
{
public:
	cyIrradianceMapColorZ(	float _thresholdColor=CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_COLOR, 
							float _thresholdZ=CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_Z ) 
							: thresholdZ(_thresholdZ) { SetColorThreshold(_thresholdColor); }
	cyIrradianceMapColorZ(	cyColor _thresholdColor, 
							float _thresholdZ=CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_Z )
							: thresholdColor(_thresholdColor), thresholdZ(_thresholdZ) {}
	void SetColorThreshold(float t) { thresholdColor.Set(t,t,t); }
	void SetColorThreshold(const cyColor &t) { thresholdColor=t; }
	void SetZThreshold(float t) { thresholdZ=t; }
protected:
	virtual bool Interpolate( cyColorA& outVal, const cyColorA& input1, const cyColorA& input2, float weight2 ) const
	{
		cyColorA d = input2 - input1;
		outVal = input1 + d*weight2;
		return	fabs(d.r)<=thresholdColor.r && 
				fabs(d.g)<=thresholdColor.g && 
				fabs(d.b)<=thresholdColor.b && 
				(fabs(d.a)/fabs(input1.a+input2.a))<=thresholdZ;
	}
	/// The z-depth value is used for determining whether the interpolation is good enough.
	/// The outVal should have the z-depth value (in the alpha channel of the color) at the evaluation point.
	virtual void Filter( cyColorA& outVal, const cyColorA& input1, const cyColorA& input2, float weight2 ) const
	{
		cyColorA d = input2 - input1;
		if ( (fabs(d.a)/fabs(input1.a+input2.a))>thresholdZ ) {
			outVal = (fabs(input1.a-outVal.a)<=fabs(input2.a-outVal.a)) ? input1 : input2;
		} else {
			outVal = input1 + d*weight2;
		}
	}
private:
	cyColor thresholdColor;
	float thresholdZ;
};

//-------------------------------------------------------------------------------

/// Structure that keep a color, a z-depth, and a normal value.
/// Used in cyIrradianceMapColorZNormal.
struct cyColorZNormal
{
	cyColor   c;
	float     z;
	cyPoint3f N;
};

/// Irradiance map for with a color, a z-depth, and a normal value per computation.
/// Uses a color, a z-depth, and a normal threshold value to determine if the interpolation is good enough.
class cyIrradianceMapColorZNormal : public cyIrradianceMap<cyColorZNormal>
{
public:
	cyIrradianceMapColorZNormal(float _thresholdColor=CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_COLOR, 
								float _thresholdZ=CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_Z,
								float _thresholdN=CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_NORMAL)
								: thresholdZ(_thresholdZ), thresholdN(_thresholdN) { SetColorThreshold(_thresholdColor); }
	cyIrradianceMapColorZNormal(cyColor _thresholdColor,
								float _thresholdZ=CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_Z,
								float _thresholdN=CY_IRRADIANCE_MAP_DEFAULT_THRESHOLD_NORMAL)
								: thresholdColor(_thresholdColor), thresholdZ(_thresholdZ), thresholdN(_thresholdN) {}
	void SetColorThreshold(float t) { thresholdColor.Set(t,t,t); }
	void SetColorThreshold(const cyColor &t) { thresholdColor=t; }
	void SetZThreshold(float t) { thresholdZ=t; }
protected:
	virtual bool Interpolate( cyColorZNormal& outVal, const cyColorZNormal& input1, const cyColorZNormal& input2, float weight2 ) const
	{
		cyColor dc = input2.c - input1.c;
		outVal.c = input1.c + dc*weight2;
		outVal.N = (input1.N + (input2.N-input1.N)*weight2).GetNormalized();
		float dz = input2.z - input1.z;
		outVal.z = input1.z + dz*weight2;
		return	fabs(dc.r)<=thresholdColor.r && 
				fabs(dc.g)<=thresholdColor.g && 
				fabs(dc.b)<=thresholdColor.b && 
				(fabs(dz)/fabs(input1.z+input2.z))<=thresholdZ &&
				(input1.N%input2.N) >= thresholdN;
	}
	/// The z-depth and the normal values are used for determining whether the interpolation is good enough.
	/// The outVal should have the z-depth and the normal values at the evaluation point.
	virtual void Filter( cyColorZNormal& outVal, const cyColorZNormal& input1, const cyColorZNormal& input2, float weight2 ) const
	{
		float dz = input2.z - input1.z;
		if ( (fabs(dz)/fabs(input1.z+input2.z)) > thresholdZ ) {
			outVal = (fabs(input1.z-outVal.z)<=fabs(input2.z-outVal.z)) ? input1 : input2;
		} else if ( input1.N%input2.N < thresholdN ) {
			outVal = (input1.N%outVal.N)>=(input2.N%outVal.N) ? input1 : input2;
		} else {
			cyColor dc = input2.c - input1.c;
			outVal.c = input1.c + dc*weight2;
			outVal.z = input1.z + dz*weight2;
			outVal.N = (input1.N + (input2.N-input1.N)*weight2).GetNormalized();
		}
	}
private:
	cyColor thresholdColor;
	float thresholdZ;
	float thresholdN;
};

//-------------------------------------------------------------------------------

#endif
