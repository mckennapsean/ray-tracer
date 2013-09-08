// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
///
/// \file		cyColor.h 
/// \author		Cem Yuksel
/// \version	1.3
/// \date		September 2, 2013
///
/// \brief Color classes.
///
///
/// @copydoc cyColor
///
/// cyColor is color class that holds floating point color values
///
//-------------------------------------------------------------------------------

#ifndef _CY_COLOR_H_INCLUDED_
#define _CY_COLOR_H_INCLUDED_

//-------------------------------------------------------------------------------

/// Color class

class cyColor
{
	friend cyColor operator+( const float v, const cyColor &c ) { return c+v; }		///< Addition with a constant
	friend cyColor operator-( const float v, const cyColor &c ) { return -(c-v); }	///< Subtraction from a constant
	friend cyColor operator*( const float v, const cyColor &c ) { return c*v; }		///< Multiplication with a constant

public:

	float r, g, b;

	///@name Constructors
	cyColor() { }
	cyColor( float rgb ) : r(rgb), g(rgb), b(rgb) {}
	cyColor( float _r, float _g, float _b ) : r(_r), g(_g), b(_b) {}
	cyColor( const float *c ) : r(c[0]), g(c[1]), b(c[2]) {}
	cyColor( const cyColor &c ) : r(c.r), g(c.g), b(c.b) {}

	///@name Set & Get value functions
	cyColor& Black() { r=0; g=0; b=0; return *this; }									///< Sets r, g and b components as zero
	cyColor& White() { r=1; g=1; b=1; return *this; }									///< Sets r, g and b components as one
	cyColor& Set( float _r, float _g, float _b ) { r=_r; g=_g; b=_b; return *this; }	///< Sets r, g and b components as given
	cyColor& Set( const float *v ) { r=v[0]; g=v[1]; b=v[2]; return *this; }			///< Sets r, g and b components using the values in the given array
	void GetValue( float *v ) const { v[0]=r; v[1]=g; v[2]=b; }							///< Puts r, g and b values into the array

	///@name Gray-scale functions
	float	Grey() const { return (r+g+b)/3.0f; }
	float	Luma1() const { return 0.299f *r + 0.587f *g + 0.114f *b; }
	float	Luma2() const { return 0.2126f*r + 0.7152f*g + 0.0722f*b; }

	///@name Limit functions
	void ClampMinMax( float min=0, float max=1 ) { ClampMin(min); ClampMax(max); }
	void ClampMin( float n=0 ) { if(r<n)r=n; if(g<n)g=n; if(b<n)b=n; }
	void ClampMax( float n=1 ) { if(r>n)r=n; if(g>n)g=n; if(b>n)b=n; }

	///@name Unary operators
	cyColor operator-() const { return cyColor(-r,-g,-b); } 
	cyColor operator+() const { return *this; }

	///@name Binary operators
	cyColor operator+( const cyColor &c ) const { return cyColor(r+c.r, g+c.g, b+c.b); }
	cyColor operator-( const cyColor &c ) const { return cyColor(r-c.r, g-c.g, b-c.b); }
	cyColor operator*( const cyColor &c ) const { return cyColor(r*c.r, g*c.g, b*c.b); }
	cyColor operator/( const cyColor &c ) const { return cyColor(r/c.r, g/c.g, b/c.b); }
	cyColor operator+(float n) const { return cyColor(r+n, g+n, b+n); }
	cyColor operator-(float n) const { return cyColor(r-n, g-n, b-n); }
	cyColor operator*(float n) const { return cyColor(r*n, g*n, b*n); }
	cyColor operator/(float n) const { return cyColor(r/n, g/n, b/n); }

	///@name Assignment operators
	cyColor& operator+=( const cyColor &c ) { r+=c.r; g+=c.g; b+=c.b; return *this; }
	cyColor& operator-=( const cyColor &c ) { r-=c.r; g-=c.g; b-=c.b; return *this; }
	cyColor& operator*=( const cyColor &c ) { r*=c.r; g*=c.g; b*=c.b; return *this; }
	cyColor& operator/=( const cyColor &c ) { r/=c.r; g/=c.g; b/=c.b; return *this; }
	cyColor& operator+=(float n) { r+=n; g+=n; b+=n; return *this; }
	cyColor& operator-=(float n) { r-=n; g-=n; b-=n; return *this; }
	cyColor& operator*=(float n) { r*=n; g*=n; b*=n; return *this; }
	cyColor& operator/=(float n) { r/=n; g/=n; b/=n; return *this; }

	///@name Test operators
	int operator==( const cyColor& c ) const { return ( (c.r==r) && (c.g==g) && (c.b==b) ); }
	int operator!=( const cyColor& c ) const { return ( (c.r!=r) || (c.g!=g) || (c.b!=b) ); }

	///@name Access operators
	float& operator[]( int i ) { return (&r)[i]; }
	float  operator[]( int i ) const { return (&r)[i]; }
};

//-------------------------------------------------------------------------------


/// Color class with alpha
class cyColorA
{
	friend cyColorA operator+( const float v, const cyColorA &c ) { return c+v; }		///< Addition with a constant
	friend cyColorA operator-( const float v, const cyColorA &c ) { return -(c-v); }	///< Subtraction from a constant
	friend cyColorA operator*( const float v, const cyColorA &c ) { return c*v; }		///< Multiplication with a constant

public:

	float r, g, b, a;

	///@name Constructors
	cyColorA() { }
	cyColorA( float rgb, float _a=1 ) : r(rgb), g(rgb), b(rgb), a(_a) {}
	cyColorA( float _r, float _g, float _b, float _a=1 ) : r(_r), g(_g), b(_b), a(_a) {}
	cyColorA( const float *c ) : r(c[0]), g(c[1]), b(c[2]), a(c[3]) {}
	cyColorA( const cyColorA &c ) : r(c.r), g(c.g), b(c.b), a(c.a) {}
	cyColorA( const cyColor &c, float _a=1 ) : r(c.r), g(c.g), b(c.b), a(_a) {}

	///@name Set & Get value functions
	cyColorA& Black(float alpha=1) { r=0; g=0; b=0; a=alpha; return *this; }							///< Sets r, g, and b components as zero and a component as given
	cyColorA& White(float alpha=1) { r=0; g=0; b=0; a=alpha; return *this; }							///< Sets r, g, and b components as one and a component as given
	cyColorA& Set( float _r, float _g, float _b, float _a ) { r=_r; g=_g; b=_b; a=_a; return *this; }	///< Sets r, g, b and a components as given
	cyColorA& Set( const float *v ) { r=v[0]; g=v[1]; b=v[2]; a=v[3]; return *this; }					///< Sets r, g, b and a components using the values in the given array
	void GetValue( float *v ) const { v[0]=r; v[1]=g; v[2]=b; v[3]=a; }									///< Puts r, g, b and a values into the array

	///@name Gray-scale functions
	float	Grey() const { return (r+g+b)/3.0f; }
	float	Luma1() const { return 0.299f *r + 0.587f *g + 0.114f *b; }
	float	Luma2() const { return 0.2126f*r + 0.7152f*g + 0.0722f*b; }

	///@name Limit functions
	void ClampMinMax( float min, float max ) { ClampMin(min); ClampMax(max); }
	void ClampMin( float n ) { if(r<n)r=n; if(g<n)g=n; if(b<n)b=n; if(a<n)a=n; }
	void ClampMax( float n ) { if(r>n)r=n; if(g>n)g=n; if(b>n)b=n; if(a>n)a=n; }

	///@name Unary operators
	cyColorA operator-() const { return cyColorA(-r,-g,-b,-a); } 
	cyColorA operator+() const { return *this; }

	///@name Binary operators
	cyColorA operator+( const cyColorA &c ) const { return cyColorA(r+c.r, g+c.g, b+c.b, a+c.a); }
	cyColorA operator-( const cyColorA &c ) const { return cyColorA(r-c.r, g-c.g, b-c.b, a-c.a); }
	cyColorA operator*( const cyColorA &c ) const { return cyColorA(r*c.r, g*c.g, b*c.b, a*c.a); }
	cyColorA operator/( const cyColorA &c ) const { return cyColorA(r/c.r, g/c.g, b/c.b, a/c.a); }
	cyColorA operator+(float n) const { return cyColorA(r+n, g+n, b+n, a); }
	cyColorA operator-(float n) const { return cyColorA(r-n, g-n, b-n, a); }
	cyColorA operator*(float n) const { return cyColorA(r*n, g*n, b*n, a); }
	cyColorA operator/(float n) const { return cyColorA(r/n, g/n, b/n, a); }

	///@name Assignment operators
	cyColorA& operator+=( const cyColorA &c ) { r+=c.r; g+=c.g; b+=c.b; a+=c.a; return *this; }
	cyColorA& operator-=( const cyColorA &c ) { r-=c.r; g-=c.g; b-=c.b; a-=c.a; return *this; }
	cyColorA& operator*=( const cyColorA &c ) { r*=c.r; g*=c.g; b*=c.b; a*=c.a; return *this; }
	cyColorA& operator/=( const cyColorA &c ) { r/=c.r; g/=c.g; b/=c.b; a/=c.a; return *this; }
	cyColorA& operator+=(float n) { r+=n; g+=n; b+=n; a+=n; return *this; }
	cyColorA& operator-=(float n) { r-=n; g-=n; b-=n; a-=n; return *this; }
	cyColorA& operator*=(float n) { r*=n; g*=n; b*=n; a*=n; return *this; }
	cyColorA& operator/=(float n) { r/=n; g/=n; b/=n; a/=n; return *this; }

	///@name Test operators
	int operator==( const cyColorA& c ) const { return ( (c.r==r) && (c.g==g) && (c.b==b) && (c.a==a) ); }
	int operator!=( const cyColorA& c ) const { return ( (c.r!=r) || (c.g!=g) || (c.b!=b) || (c.a!=a) ); }

	///@name Access operators
	float& operator[]( int i ) { return (&r)[i]; }
	float  operator[]( int i ) const { return (&r)[i]; }

	///@name Conversion Methods
	cyColor	RGB() const { return cyColor(r,g,b); }
};

//-------------------------------------------------------------------------------

/// 24-bit color class
class cyColor24
{
public:
	unsigned char r, g, b;

	///@name Constructors
	cyColor24() {}
	cyColor24(const cyColor &c) { r=FloatToByte(c.r); g=FloatToByte(c.g); b=FloatToByte(c.b); }

	///@name Conversion Methods
	cyColor ToColor() const { return cyColor(r/255.0f,g/255.0f,b/255.0f); }

	///@name Set & Get value functions
	cyColor24& Black() { r=0; g=0; b=0; return *this; }															///< Sets r, g, and b components as zero
	cyColor24& White() { r=255; g=255; b=255; return *this; }													///< Sets r, g, and b components as 255
	cyColor24& Set( unsigned char _r, unsigned char _g, unsigned char _b ) { r=_r; g=_g; b=_b; return *this; }	///< Sets r, g, and b components as given
	cyColor24& Set( const unsigned char *v ) { r=v[0]; g=v[1]; b=v[2]; return *this; }							///< Sets r, g, and b components using the values in the given array
	void GetValue( unsigned char *v ) const { v[0]=r; v[1]=g; v[2]=b; }											///< Puts r, g, and b values into the array

private:
	static unsigned char FloatToByte(float r) { return Clamp(int(r*255)); }
	static unsigned char Clamp(int v) { return v<0 ? 0 : (v>255 ? 255 : v); }
};

//-------------------------------------------------------------------------------

/// 32-bit color class
class cyColor32
{
public:
	unsigned char r, g, b, a;

	///@name Constructors
	cyColor32() {}
	cyColor32(const cyColor  &c) { r=FloatToByte(c.r); g=FloatToByte(c.g); b=FloatToByte(c.b); a=255; }
	cyColor32(const cyColorA &c) { r=FloatToByte(c.r); g=FloatToByte(c.g); b=FloatToByte(c.b); a=FloatToByte(c.a); }

	///@name Conversion Methods
	cyColor  ToColor()  const { return cyColor(r/255.0f,g/255.0f,b/255.0f); }
	cyColorA ToColorA() const { return cyColorA(r/255.0f,g/255.0f,b/255.0f,a/255.0f); }

	///@name Set & Get value functions
	cyColor32& Black(unsigned char _a=255) { r=0; g=0; b=0; a=_a; return *this; }														///< Sets r, g, and b components as zero and a component as given
	cyColor32& White(unsigned char _a=255) { r=255; g=255; b=255; a=_a; return *this; }													///< Sets r, g, and b components as one and a component as given
	cyColor32& Set( unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a ) { r=_r; g=_g; b=_b; a=_a; return *this; }	///< Sets r, g, b and a components as given
	cyColor32& Set( const unsigned char *v ) { r=v[0]; g=v[1]; b=v[2]; a=v[3]; return *this; }											///< Sets r, g, b and a components using the values in the given array
	void GetValue( unsigned char *v ) const { v[0]=r; v[1]=g; v[2]=b; v[3]=a; }															///< Puts r, g, b and a values into the array

private:
	static unsigned char FloatToByte(float r) { return Clamp(int(r*255)); }
	static unsigned char Clamp(int v) { return v<0 ? 0 : (v>255 ? 255 : v); }
};

//-------------------------------------------------------------------------------

#endif

