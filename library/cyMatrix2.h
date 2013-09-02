// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
///
/// \file		cyMatrix2.h 
/// \author		Cem Yuksel
/// \version	1.3
/// \date		August 23, 2012
///
/// \brief 2x2 matrix class
///
//-------------------------------------------------------------------------------

#ifndef _CY_MATRIX2_H_INCLUDED_
#define _CY_MATRIX2_H_INCLUDED_

//-------------------------------------------------------------------------------

#include "cyPoint.h"

//-------------------------------------------------------------------------------

/// 2x2 matrix class.
/// Its data stores 4-value array of column-major matrix elements.
/// You can use cyMatrix2f with cyPoint2f to transform 2D points.
/// Both post-multiplication and pre-multiplication are supported.

class cyMatrix2f
{
	
	friend cyMatrix2f operator+( const float, const cyMatrix2f & );			///< add a value to a matrix
	friend cyMatrix2f operator-( const float, const cyMatrix2f & );			///< subtract the matrix from a value
	friend cyMatrix2f operator*( const float, const cyMatrix2f & );			///< multiple matrix by a value
	friend cyMatrix2f Inverse( cyMatrix2f &m ) { return m.GetInverse(); }	///< return the inverse of the matrix
	friend cyPoint2f  operator*( const cyPoint2f &, const cyMatrix2f & );	///< pre-multiply with a 3D point

public:

	/// elements of the matrix are column-major
	float data[4];

	//////////////////////////////////////////////////////////////////////////
	///@name Constructors
	cyMatrix2f() {}	///< Default constructor
	cyMatrix2f( const cyMatrix2f &matrix ) { for ( int i=0; i<4; i++ ) data[i]=matrix.data[i]; } ///< Copy constructor


	//////////////////////////////////////////////////////////////////////////
	///@name Set & Get Methods

	/// Set all the values as zero
	void Zero() { for ( int i=0; i<4; i++ ) data[ i ] = 0; }
	/// Set Matrix using an array of 4 values
	void Set( const float *array ) { for ( int i=0; i<4; i++ ) data[ i ] = array[ i ]; } 
	/// Set Matrix using x,y,z vectors and coordinate center
	void Set( const cyPoint2f &x, const cyPoint2f &y );
	/// Converts the matrix to an identity matrix
	void SetIdentity() { data[0]=1; data[1]=0; data[2]=0; data[3]=1; }
	/// Set a rotation matrix by angle theta
	void SetRotation( float theta );
	/// Set a rotation matrix by cos and sin of angle theta
	void SetRotation( float cosTheta, float sinTheta );

	// Get Row and Column
	cyPoint2f GetRow( int row ) const { return cyPoint2f( data[row], data[row+2] ); }
	void	  GetRow( int row, cyPoint2f &p ) const { p.Set( data[row], data[row+1] ); }
	void	  GetRow( int row, float *array ) const { array[0]=data[row]; array[1]=data[row+2]; }
	cyPoint2f GetColumn( int col ) const { return cyPoint2f( &data[col*2] ); }
	void	  GetColumn( int col, cyPoint2f &p ) const { p.Set( &data[col*2] ); }
	void	  GetColumn( int col, float *array ) const { array[0]=data[col*2]; array[1]=data[col*2+1]; }


	//////////////////////////////////////////////////////////////////////////
	///@name Overloaded Operators

	const cyMatrix2f &operator=( const cyMatrix2f & );	///< assign matrix

	// Overloaded comparison operators 
	bool operator==( const cyMatrix2f & ) const;		///< compare equal
	bool operator!=( const cyMatrix2f &right ) const { return ! ( *this == right ); } ///< compare not equal

	// Overloaded subscript operators
	float& operator()( int row, int column );					///< subscript operator
	float& operator[](int i) { return data[i]; }				///< subscript operator
	const float& operator()( int row, int column ) const;		///< constant subscript operator
	const float& operator[](int i) const { return data[i]; }	///< constant subscript operator
	
	// Unary operators
	cyMatrix2f operator - () const;	///< negative matrix

	// Binary operators
	cyMatrix2f operator + ( const cyMatrix2f & ) const;	///< add two Matrices
	cyMatrix2f operator - ( const cyMatrix2f & ) const;	///< subtract one cyMatrix2f from an other
	cyMatrix2f operator * ( const cyMatrix2f & ) const;	///< multiply a matrix with an other
	cyMatrix2f operator + ( const float ) const;		///< add a value to a matrix
	cyMatrix2f operator - ( const float ) const;		///< subtract a value from a matrix
	cyMatrix2f operator * ( const float ) const;		///< multiple matrix by a value
	cyMatrix2f operator / ( const float ) const;		///< divide matrix by a value;
	cyPoint2f operator * ( const cyPoint2f& p) const;

	// Assignment operators
	void	operator +=	( const cyMatrix2f & );	///< add two Matrices modify this
	void	operator -=	( const cyMatrix2f & );	///< subtract one cyMatrix2f from an other modify this matrix
	void	operator *=	( const cyMatrix2f & );	///< multiply a matrix with an other modify this matrix
	void	operator +=	( const float );		///< add a value to a matrix modify this
	void	operator -=	( const float );		///< subtract a value from a matrix modify this matrix
	void	operator *=	( const float );		///< multiply a matrix with a value modify this matrix
	void	operator /=	( const float );		///< divide the matrix by a value modify the this matrix


	//////////////////////////////////////////////////////////////////////////
	///@name Other Public Methods
	void SetTranspose();			///< Transpose this matrix
	cyMatrix2f Transpose() const;	///< return Transpose of this matrix
	void Invert();					///< Invert this matrix
	void GetInverse( cyMatrix2f &inverse ) const { inverse=*this; inverse.Invert(); }	///< Get the inverse of this matrix
	cyMatrix2f GetInverse() const { cyMatrix2f inv(*this); inv.Invert(); return inv; }	///< Get the inverse of this matrix

	//////////////////////////////////////////////////////////////////////////

};

//-------------------------------------------------------------------------------

namespace cy {
	typedef cyMatrix2f Matrix2f;
}

//-------------------------------------------------------------------------------

/// Set Matrix using x,y,z vectors and coordinate center
inline void cyMatrix2f::Set( const cyPoint2f &x, const cyPoint2f &y )
{
	x.GetValue( &data[0] );
	y.GetValue( &data[2] );
}

//-------------------------------------------------------------------------------

/// Set a rotation matrix about the given axis by angle theta
inline void cyMatrix2f::SetRotation( float theta )
{
	float c = (float) cos(theta);
	if ( c == 1 ) {
		SetIdentity();
		return;
	}
	float s = (float) sin(theta);
	SetRotation(c,s);
}

//-------------------------------------------------------------------------------

/// Set a rotation matrix about the given axis by cos and sin angle theta
inline void cyMatrix2f::SetRotation( float c, float s )
{
	data[0] = c;
	data[1] = -s;
	data[2] = s;
	data[3] = c;
}


//-------------------------------------------------------------------------------
// Overloaded Operators
//-------------------------------------------------------------------------------

inline cyPoint2f cyMatrix2f::operator * ( const cyPoint2f& p) const
{
	return cyPoint2f(	p.x * data[0] + p.y * data[2],
						p.x * data[1] + p.y * data[3] );
}

//-------------------------------------------------------------------------------

/// Overloaded assignment operator
/// const return avoids ( a1 = a2 ) = a3
inline const cyMatrix2f& cyMatrix2f::operator =( const cyMatrix2f &right )
{
	for ( int i=0; i<4; i++ ) data[i] = right.data[ i ];		// copy array into object
	return *this;	// enables x = y = z;
}

//-------------------------------------------------------------------------------

/// Determine if two arrays are equal and
/// return true, otherwise return false.
inline bool cyMatrix2f::operator ==( const cyMatrix2f &right ) const
{
	for ( int i=0; i<4; i++ ) {
		if ( data[ i ] != right.data[ i ] ) {
			return false;		// arrays are not equal
		}
	}
	return true;				// arrays are equal
}

//-------------------------------------------------------------------------------

/// Overloaded unary minus operator
/// negative of cyMatrix2f
inline cyMatrix2f cyMatrix2f::operator -() const
{
	cyMatrix2f buffer; // create a temp cyMatrix2f object not to change this
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = - data[ i ];
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix
inline cyMatrix2f cyMatrix2f::operator +( const float value ) const
{
	cyMatrix2f buffer; // create a temp cyMatrix2f object not to change this
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = data[ i ] + value;	// add value to all member of the matrix
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add two matrices
inline cyMatrix2f cyMatrix2f::operator +( const cyMatrix2f &right ) const
{
	cyMatrix2f buffer;	// create a temp cyMatrix2f object not to change this
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = data[ i ] + right.data[ i ];
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix modify matrix
inline void cyMatrix2f::operator +=( const float value )
{
	for ( int i=0; i<4; i++ )
		data[ i ] = data[ i ] + value;	// add value to all member of the matrix
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add two matrices modify this matrix
inline void cyMatrix2f::operator +=( const cyMatrix2f &right )
{
	for ( int i=0; i<4; i++ )
		data[ i ] = data[ i ] + right.data[ i ];
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a fixed value from a cyMatrix2f
inline cyMatrix2f cyMatrix2f::operator -( const float value ) const
{
	cyMatrix2f buffer; // create a temp cyMatrix2f object not to change this
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = data[ i ] - value;	// subtract a value from all member of the matrix
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a matrix right from this
inline cyMatrix2f cyMatrix2f::operator -( const cyMatrix2f &right ) const
{
	cyMatrix2f buffer;	// create a temp cyMatrix2f object not to change this
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = data[ i ] - right.data[ i ];
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a fixed value from a cyMatrix2f modify this matrix
inline void cyMatrix2f::operator -=( const float value )
{
	for ( int i=0; i<4; i++ )
		data[ i ] = data[ i ] - value;	// subtract a value from all member of the matrix
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a matrix right from this modify this matrix
inline void cyMatrix2f::operator -=( const cyMatrix2f &right )
{
	for ( int i=0; i<4; i++ )
		data[ i ] = data[ i ] - right.data[ i ];
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply a matrix with a value
inline cyMatrix2f cyMatrix2f::operator *( const float value ) const
{
	cyMatrix2f buffer;	// create a temp cyMatrix2f object not to change this
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = data[ i ] * value;
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply two matrices 
inline cyMatrix2f cyMatrix2f::operator *( const cyMatrix2f &right ) const
{
	cyMatrix2f buffer;  // a matrix of (m x k)

	buffer[0] = data[0] * right.data[0] + data[2] * right.data[1];
	buffer[1] = data[0] * right.data[2] + data[2] * right.data[3];
	buffer[2] = data[1] * right.data[0] + data[3] * right.data[1];
	buffer[3] = data[1] * right.data[2] + data[3] * right.data[3];
	
	return buffer;
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply a matrix with a value modify this matrix
inline void cyMatrix2f::operator *=( const float value )
{
	for ( int i=0; i<4; i++ )
		data[ i ] = data[ i ] * value;
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply two matrices modify this matrix
inline void cyMatrix2f::operator *=( const cyMatrix2f &right )
{
	cyMatrix2f buffer;  // a matrix of (m x k)
	
	buffer[0] = data[0] * right.data[0] + data[2] * right.data[1];
	buffer[1] = data[0] * right.data[2] + data[2] * right.data[3];
	buffer[2] = data[1] * right.data[0] + data[3] * right.data[1];
	buffer[3] = data[1] * right.data[2] + data[3] * right.data[3];
	
	*this = buffer;	// using a buffer to calculate the result
	//then copy buffer to this
}

//-------------------------------------------------------------------------------

/// Overloaded division operator
/// Divide the matrix by value
inline cyMatrix2f cyMatrix2f::operator /( const float value ) const
{
	if ( value == 0 ) return *this;
	return operator * ( (float) 1.0 / value );
}

//-------------------------------------------------------------------------------

/// Overloaded division operator
/// Divide the matrix by value
inline void cyMatrix2f::operator /=( const float value )
{
	if ( value == 0 ) return;
	
	for ( int i=0; i<4; i++ )
		data[ i ] = data[ i ] / value;
	
}

//-------------------------------------------------------------------------------

/// Overloaded subscript operator for non-const cyMatrix2f
/// reference return creates an lvalue
inline float& cyMatrix2f::operator ()( int row, int column )
{
	return data[ column * 2 + row ];	// reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subscript operator for const cyMatrix2f
/// const reference return creates an rvalue
inline const float& cyMatrix2f::operator ()( int row, int column ) const
{
	return data[ column * 2 + row ];	// const reference return
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

/// Invert this matrix
inline void cyMatrix2f::Invert()
{
	float a = 1.0f / ( data[0]*data[3] - data[1]*data[2] );

	float tmp = data[0];
	data[0] =  data[3] * a;
	data[1] = -data[1] * a;
	data[2] = -data[2] * a;
	data[3] = tmp * a;
}

//-------------------------------------------------------------------------------

/// Transpose of this matrix
inline void cyMatrix2f::SetTranspose()
{
	float tmp = data[0];
	data[0] =  data[3];
	data[3] = tmp;
}

//-------------------------------------------------------------------------------

inline cyMatrix2f cyMatrix2f::Transpose() const
{
	cyMatrix2f m;
	m.data[0] = data[0];
	m.data[2] = data[1];
	m.data[1] = data[2];
	m.data[3] = data[3];
	return m;
}


//-------------------------------------------------------------------------------
// friend function definitions
//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix
inline cyMatrix2f operator+( const float value, const cyMatrix2f &right )
{
	cyMatrix2f buffer; // create a temp cyMatrix2f object not to change right
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = right.data[ i ] + value;	// add value to all members of the matrix
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract the matrix from a fixed value
inline cyMatrix2f operator-( const float value, const cyMatrix2f &right )
{
	cyMatrix2f buffer; // create a temp cyMatrix2f object not to change right
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = value - right.data[ i ];	// subtract matrix from the value;
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// multiply a fixed value with the matrix
inline cyMatrix2f operator*( const float value, const cyMatrix2f &right )
{
	cyMatrix2f buffer; // create a temp cyMatrix2f object not to change right
	
	for ( int i=0; i<4; i++ )
		buffer.data[ i ] = right.data[ i ] * value;	// multiply value to all members of the matrix
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// left multiplication of a vector with the matrix
inline cyPoint2f operator*( const cyPoint2f &p, const cyMatrix2f &m ) 
{
	return cyPoint2f(	p.x * m.data[0] + p.y * m.data[1],
						p.x * m.data[2] + p.y * m.data[3] );
}

//-------------------------------------------------------------------------------

#endif
