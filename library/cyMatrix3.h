// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
///
/// \file		cyMatrix3.h 
/// \author		Cem Yuksel
/// \version	1.3
/// \date		August 23, 2012
///
/// \brief 3x3 matrix class
///
//-------------------------------------------------------------------------------

#ifndef _CY_MATRIX3_H_INCLUDED_
#define _CY_MATRIX3_H_INCLUDED_

//-------------------------------------------------------------------------------

#include "cyPoint.h"

//-------------------------------------------------------------------------------

/// 3x3 matrix class.
/// Its data stores 9-value array of column-major matrix elements.
/// You can use cyMatrix3f with cyPoint3f to transform 3D points.
/// Both post-multiplication and pre-multiplication are supported.

class cyMatrix3f
{
	
	friend cyMatrix3f operator+( const float, const cyMatrix3f & );			///< add a value to a matrix
	friend cyMatrix3f operator-( const float, const cyMatrix3f & );			///< subtract the matrix from a value
	friend cyMatrix3f operator*( const float, const cyMatrix3f & );			///< multiple matrix by a value
	friend cyMatrix3f Inverse( cyMatrix3f &m ) { return m.GetInverse(); }	///< return the inverse of the matrix
	friend cyPoint3f  operator*( const cyPoint3f &, const cyMatrix3f & );	///< pre-multiply with a 3D point

public:

	/// elements of the matrix are column-major
	float data[9];

	//////////////////////////////////////////////////////////////////////////
	///@name Constructors
	cyMatrix3f() {}	///< Default constructor
	cyMatrix3f( const cyMatrix3f &matrix ) { for ( int i=0; i<9; i++ ) data[i]=matrix.data[i]; } ///< Copy constructor
	cyMatrix3f( const cyPoint3f &p ) { Set(p); }	///< Matrix formulation of the cross product


	//////////////////////////////////////////////////////////////////////////
	///@name Set & Get Methods

	/// Set all the values as zero
	void Zero() { for ( int i=0; i<9; i++ ) data[ i ] = 0; }
	/// Set Matrix using an array of 9 values
	void Set( const float *array ) { for ( int i=0; i<9; i++ ) data[ i ] = array[ i ]; } 
	/// Matrix formulation of the cross product
	void Set( const cyPoint3f &p ) { data[0]=0; data[1]=p.z; data[2]=-p.y; data[3]=-p.z; data[4]=0; data[5]=p.x; data[6]=p.y; data[7]=-p.x; data[8]=0; }
	/// Set Matrix using x,y,z vectors and coordinate center
	void Set( const cyPoint3f &x, const cyPoint3f &y, const cyPoint3f &z );
	/// Converts the matrix to an identity matrix
	void SetIdentity() { for(int i=0; i<9; i++ ) data[i]=(i%4==0) ? 1.0f : 0.0f ; }
	/// Set View Matrix using position, target and approximate up vector
	void SetView( const cyPoint3f &target, cyPoint3f &up );
	/// Set Matrix using normal, and approximate x direction
	void SetNormal(const cyPoint3f &normal, cyPoint3f &dir );
	/// Set as rotation matrix around x axis
	void SetRotationX( float theta ) { SetRotation( cyPoint3f(1,0,0), theta ); }
	/// Set as rotation matrix around y axis
	void SetRotationY( float theta ) { SetRotation( cyPoint3f(0,1,0), theta ); }
	/// Set as rotation matrix around z axis
	void SetRotationZ( float theta ) { SetRotation( cyPoint3f(0,0,1), theta ); }
	/// Set a rotation matrix about the given axis by angle theta
	void SetRotation( cyPoint3f axis, float theta );
	/// Set a rotation matrix about the given axis by cos and sin of angle theta
	void SetRotation( cyPoint3f axis, float cosTheta, float sinTheta );
	/// Set a rotation matrix that sets <from> unit vector to <to> unit vector
	void SetRotation( cyPoint3f from, cyPoint3f to );

	// Get Row and Column
	cyPoint3f GetRow( int row ) const { return cyPoint3f( data[row], data[row+3], data[row+6] ); }
	void	  GetRow( int row, cyPoint3f &p ) const { p.Set( data[row], data[row+3], data[row+6] ); }
	void	  GetRow( int row, float *array ) const { array[0]=data[row]; array[1]=data[row+3]; array[2]=data[row+6]; }
	cyPoint3f GetColumn( int col ) const { return cyPoint3f( &data[col*3] ); }
	void	  GetColumn( int col, cyPoint3f &p ) const { p.Set( &data[col*3] ); }
	void	  GetColumn( int col, float *array ) const { array[0]=data[col*3]; array[1]=data[col*3+1]; array[2]=data[col*3+2]; }


	//////////////////////////////////////////////////////////////////////////
	///@name Overloaded Operators

	const cyMatrix3f &operator=( const cyMatrix3f & );	///< assign matrix

	// Overloaded comparison operators 
	bool operator==( const cyMatrix3f & ) const;		///< compare equal
	bool operator!=( const cyMatrix3f &right ) const { return ! ( *this == right ); } ///< compare not equal

	// Overloaded subscript operators
	float& operator()( int row, int column );					///< subscript operator
	float& operator[](int i) { return data[i]; }				///< subscript operator
	const float& operator()( int row, int column ) const;		///< constant subscript operator
	const float& operator[](int i) const { return data[i]; }	///< constant subscript operator
	
	// Unary operators
	cyMatrix3f operator - () const;	///< negative matrix

	// Binary operators
	cyMatrix3f operator + ( const cyMatrix3f & ) const;	///< add two Matrices
	cyMatrix3f operator - ( const cyMatrix3f & ) const;	///< subtract one cyMatrix3f from an other
	cyMatrix3f operator * ( const cyMatrix3f & ) const;	///< multiply a matrix with an other
	cyMatrix3f operator + ( const float ) const;		///< add a value to a matrix
	cyMatrix3f operator - ( const float ) const;		///< subtract a value from a matrix
	cyMatrix3f operator * ( const float ) const;		///< multiple matrix by a value
	cyMatrix3f operator / ( const float ) const;		///< divide matrix by a value;
	cyPoint3f operator * ( const cyPoint3f& p) const;

	// Assignment operators
	void	operator +=	( const cyMatrix3f & );	///< add two Matrices modify this
	void	operator -=	( const cyMatrix3f & );	///< subtract one cyMatrix3f from an other modify this matrix
	void	operator *=	( const cyMatrix3f & );	///< multiply a matrix with an other modify this matrix
	void	operator +=	( const float );		///< add a value to a matrix modify this
	void	operator -=	( const float );		///< subtract a value from a matrix modify this matrix
	void	operator *=	( const float );		///< multiply a matrix with a value modify this matrix
	void	operator /=	( const float );		///< divide the matrix by a value modify the this matrix


	//////////////////////////////////////////////////////////////////////////
	///@name Other Public Methods
	void SetTranspose();			///< Transpose this matrix
	cyMatrix3f Transpose() const;	///< return Transpose of this matrix
	void Invert();					///< Invert this matrix
	void GetInverse( cyMatrix3f &inverse ) const { inverse=*this; inverse.Invert(); }	///< Get the inverse of this matrix
	cyMatrix3f GetInverse() const { cyMatrix3f inv(*this); inv.Invert(); return inv; }	///< Get the inverse of this matrix

	//////////////////////////////////////////////////////////////////////////


private:

	/// \internal
	void LUBKS( int *outvect, float *output );		// Used by the Invert function to calculate the LUD of the matrix
	int LUD( int *outvect, int output );			// calculate the LUD of the matrix
};

//-------------------------------------------------------------------------------

namespace cy {
	typedef cyMatrix3f Matrix3f;
}

//-------------------------------------------------------------------------------

/// Set Matrix using x,y,z vectors and coordinate center
inline void cyMatrix3f::Set( const cyPoint3f &x, const cyPoint3f &y, const cyPoint3f &z )
{
	x.GetValue( &data[0] );
	y.GetValue( &data[3] );
	z.GetValue( &data[6] );
}

//-------------------------------------------------------------------------------

/// Set Matrix using position, normal, and approximate x direction
inline void cyMatrix3f::SetNormal( const cyPoint3f &normal, cyPoint3f &dir )
{
	cyPoint3f y = normal.Cross(dir);
	y.Normalize();
	dir = y.Cross(normal);
	Set( dir, y, normal );
	
}

//-------------------------------------------------------------------------------

/// Set View Matrix using position, target and approximate up vector
inline void cyMatrix3f::SetView( const cyPoint3f &target, cyPoint3f &up )
{
	cyPoint3f f = target;
	f.Normalize();
	cyPoint3f s = f.Cross(up);
	s.Normalize();
	cyPoint3f u = s.Cross(f);

	data[0]=s.x;	data[1]=u.x;	data[2]=-f.x;
	data[3]=s.y;	data[4]=u.y;	data[5]=-f.y;
	data[6]=s.z;	data[7]=u.z;	data[8]=-f.z;
}

//-------------------------------------------------------------------------------

/// Set a rotation matrix about the given axis by angle theta
inline void cyMatrix3f::SetRotation( cyPoint3f axis, float theta )
{
	float c = (float) cos(theta);
	if ( c == 1 ) {
		SetIdentity();
		return;
	}
	float s = (float) sin(theta);
	SetRotation(axis,c,s);
}

//-------------------------------------------------------------------------------

/// Set a rotation matrix that sets <from> unit vector to <to> unit vector
inline void cyMatrix3f::SetRotation( cyPoint3f from, cyPoint3f to )
{
	float c = from.Dot(to);
	if ( c > 0.999999 ) {
		SetIdentity();
		return;
	}
	float s = (float) sqrt( 1 - c*c );
	cyPoint3f axis = from.Cross(to);
	SetRotation(axis,c,s);
}

//-------------------------------------------------------------------------------

/// Set a rotation matrix about the given axis by cos and sin angle theta
inline void cyMatrix3f::SetRotation( cyPoint3f axis, float c, float s )
{
	if ( c == 1 ) {
		SetIdentity();
		return;
	}

	float t = 1 - c;
	float tx = t * axis.x;
	float ty = t * axis.y;
	float tz = t * axis.z;
	float txy = tx * axis.y;
	float txz = tx * axis.z;
	float tyz = ty * axis.z;
	float sx = s * axis.x;
	float sy = s * axis.y;
	float sz = s * axis.z;
	
	data[0] = tx * axis.x + c;
	data[1] = txy + sz;
	data[2] = txz - sy;

	data[3] = txy - sz;
	data[4] = ty * axis.y + c;
	data[5] = tyz + sx;

	data[6] = txz + sy;
	data[7] = tyz - sx;
	data[8] = tz * axis.z + c;
}


//-------------------------------------------------------------------------------
// Overloaded Operators
//-------------------------------------------------------------------------------

inline cyPoint3f cyMatrix3f::operator * ( const cyPoint3f& p) const
{
	return cyPoint3f(	p.x * data[0] + p.y * data[3] + p.z * data[6],
						p.x * data[1] + p.y * data[4] + p.z * data[7],
						p.x * data[2] + p.y * data[5] + p.z * data[8] );
}

//-------------------------------------------------------------------------------

/// Overloaded assignment operator
/// const return avoids ( a1 = a2 ) = a3
inline const cyMatrix3f& cyMatrix3f::operator =( const cyMatrix3f &right )
{
	for ( int i=0; i<9; i++ ) data[i] = right.data[ i ];		// copy array into object
	return *this;	// enables x = y = z;
}

//-------------------------------------------------------------------------------

/// Determine if two arrays are equal and
/// return true, otherwise return false.
inline bool cyMatrix3f::operator ==( const cyMatrix3f &right ) const
{
	for ( int i=0; i<9; i++ ) {
		if ( data[ i ] != right.data[ i ] ) {
			return false;		// arrays are not equal
		}
	}
	return true;				// arrays are equal
}

//-------------------------------------------------------------------------------

/// Overloaded unary minus operator
/// negative of cyMatrix3f
inline cyMatrix3f cyMatrix3f::operator -() const
{
	cyMatrix3f buffer; // create a temp cyMatrix3f object not to change this
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = - data[ i ];
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix
inline cyMatrix3f cyMatrix3f::operator +( const float value ) const
{
	cyMatrix3f buffer; // create a temp cyMatrix3f object not to change this
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = data[ i ] + value;	// add value to all member of the matrix
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add two matrices
inline cyMatrix3f cyMatrix3f::operator +( const cyMatrix3f &right ) const
{
	cyMatrix3f buffer;	// create a temp cyMatrix3f object not to change this
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = data[ i ] + right.data[ i ];
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix modify matrix
inline void cyMatrix3f::operator +=( const float value )
{
	for ( int i=0; i<9; i++ )
		data[ i ] = data[ i ] + value;	// add value to all member of the matrix
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add two matrices modify this matrix
inline void cyMatrix3f::operator +=( const cyMatrix3f &right )
{
	for ( int i=0; i<9; i++ )
		data[ i ] = data[ i ] + right.data[ i ];
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a fixed value from a cyMatrix3f
inline cyMatrix3f cyMatrix3f::operator -( const float value ) const
{
	cyMatrix3f buffer; // create a temp cyMatrix3f object not to change this
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = data[ i ] - value;	// subtract a value from all member of the matrix
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a matrix right from this
inline cyMatrix3f cyMatrix3f::operator -( const cyMatrix3f &right ) const
{
	cyMatrix3f buffer;	// create a temp cyMatrix3f object not to change this
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = data[ i ] - right.data[ i ];
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a fixed value from a cyMatrix3f modify this matrix
inline void cyMatrix3f::operator -=( const float value )
{
	for ( int i=0; i<9; i++ )
		data[ i ] = data[ i ] - value;	// subtract a value from all member of the matrix
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a matrix right from this modify this matrix
inline void cyMatrix3f::operator -=( const cyMatrix3f &right )
{
	for ( int i=0; i<9; i++ )
		data[ i ] = data[ i ] - right.data[ i ];
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply a matrix with a value
inline cyMatrix3f cyMatrix3f::operator *( const float value ) const
{
	cyMatrix3f buffer;	// create a temp cyMatrix3f object not to change this
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = data[ i ] * value;
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply two matrices 
inline cyMatrix3f cyMatrix3f::operator *( const cyMatrix3f &right ) const
{
	cyMatrix3f buffer;  // a matrix of (m x k)
	
	for ( int i = 0; i < 3; i++ ) {
		for ( int k = 0; k < 3; k++ ) {
			buffer.data[ i + 3 * k ] = 0;
			for ( int j = 0; j < 3; j++ ) {
				buffer.data[ i + 3 * k ] += data[ i + 3 * j ] * right.data[ j + 3 * k ];
			}
		}
	}
	
	return buffer;
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply a matrix with a value modify this matrix
inline void cyMatrix3f::operator *=( const float value )
{
	for ( int i=0; i<9; i++ )
		data[ i ] = data[ i ] * value;
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply two matrices modify this matrix
inline void cyMatrix3f::operator *=( const cyMatrix3f &right )
{
	cyMatrix3f buffer;  // a matrix of (m x k)
	
	for ( int i = 0; i < 3; i++ ) {
		for ( int k = 0; k < 3; k++ ) {
			buffer.data[ i + 3 * k ] = 0;
			for ( int j = 0; j < 3; j++ ) {
				buffer.data[ i + 3 * k ] += data[ i + 3 * j ] * right.data[ j + 3 * k ];
			}
		}
	}
	
	*this = buffer;	// using a buffer to calculate the result
	//then copy buffer to this
}

//-------------------------------------------------------------------------------

/// Overloaded division operator
/// Divide the matrix by value
inline cyMatrix3f cyMatrix3f::operator /( const float value ) const
{
	if ( value == 0 ) return *this;
	return operator * ( (float) 1.0 / value );
}

//-------------------------------------------------------------------------------

/// Overloaded division operator
/// Divide the matrix by value
inline void cyMatrix3f::operator /=( const float value )
{
	if ( value == 0 ) return;
	
	for ( int i=0; i<9; i++ )
		data[ i ] = data[ i ] / value;
	
}

//-------------------------------------------------------------------------------

/// Overloaded subscript operator for non-const cyMatrix3f
/// reference return creates an lvalue
inline float& cyMatrix3f::operator ()( int row, int column )
{
	return data[ column * 3 + row ];	// reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subscript operator for const cyMatrix3f
/// const reference return creates an rvalue
inline const float& cyMatrix3f::operator ()( int row, int column ) const
{
	return data[ column * 3 + row ];	// const reference return
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

/// Invert this matrix
inline void cyMatrix3f::Invert()
{
	float temp[3];
    int    IND[3], D = 0, i, j;

	// create the buffer matrix
	cyMatrix3f buffer;

	buffer = *this;

	if ( ! buffer.LUD( IND, D ) ) return;

    for ( j = 0; j < 3; j++ )
	{
		for ( i = 0; i < 3; i++ ) temp[i] = 0.0;
		temp[j] = 1.0;

		buffer.LUBKS( IND, temp);

		for ( i = 0; i < 3; i++ )
			data[ j * 3 + i ] = temp[ i ];
    }
}

//-------------------------------------------------------------------------------

/// Transpose of this matrix
inline void cyMatrix3f::SetTranspose()
{
	float temp;

    for ( int i = 1; i < 3; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			temp = data[ i * 3 + j ];
			data[ i * 3 + j ] = data[ j * 3 + i ];
			data[ j * 3 + i ] = temp;
		}
    }
}

//-------------------------------------------------------------------------------

inline cyMatrix3f cyMatrix3f::Transpose() const
{
	cyMatrix3f m;
	m.data[0] = data[0];
	m.data[3] = data[1];
	m.data[6] = data[2];
	m.data[1] = data[3];
	m.data[4] = data[4];
	m.data[7] = data[5];
	m.data[2] = data[6];
	m.data[5] = data[7];
	m.data[8] = data[8];
	return m;
}


//-------------------------------------------------------------------------------
// private methods
//-------------------------------------------------------------------------------

/// calculate the LUD of the matrix
inline int cyMatrix3f::LUD( int *outvect, int output )
{
	int	    i, j, k, imax; 
    float  VV[3], aamax, sum, dnum; 
	float  SMALL = (float) 1.0e-10;

    output = 1; 	     /* No row interchanges yet*/
    
    for ( i = 0; i < 3; i++ ) {
		aamax = 0.0;
		for ( j = 0; j < 3; j++ ) {
			if( fabs( data[ j * 3 + i ] ) > aamax )
				aamax = (float) fabs( data[ j * 3 + i ] );
		}
		if ( aamax == 0. ) {
			return false; /* LUD_ERR flag Singular cyMatrix3f */
		}
		VV[ i ] = (float) 1.0 / (float) aamax;
    }
    
    for ( j = 0; j < 3; j++ ) {
		if ( j > 0 ) {
			for ( i = 0; i < j; i++ ) {
				sum = data[ j * 3 + i ];
				if ( i > 0 ) {
					for ( k = 0; k < i; k++ )
						sum = sum - ( data[ k * 3 + i ] * data[ j * 3 + k ] );
					data[ j * 3 + i ] = sum;
				}
			}
		}

		aamax = 0.;

		for ( i = j; i < 3; i++ ) {
			sum = data[ j * 3 + i ];
			if ( j > 0 ) {
				for ( k = 0; k < j; k++ )
					sum = sum - ( data[ k * 3 + i ] * data[ j * 3 + k ] );
				data[ j * 3 + i ] = sum;
			}
			dnum = VV[ i ] * (float) fabs( sum );
			if( dnum >= aamax ) {
				imax = i;
				aamax = dnum;
			}
		}
		if ( j != imax ) {
			for ( k = 0; k < 3; k++ ) {
				dnum = data[ k * 3 + imax ];
				data[ k * 3 + imax ] = data[ k * 3 + j ];
				data[ k * 3 + j ] = dnum;
			}
			output = -output;
			VV[ imax ] = VV[ j ];
		}
		outvect[ j ] = imax;
		if ( j != ( 3 - 1 ) ) {
			if ( data[ j * 3 + j ] == 0. )
				data[ j * 3 + j ] = SMALL;
			dnum = (float) 1.0 / data[ j * 3 + j ];
			for ( i = ( j + 1 ); i < 3; i++ )
				data[ j * 3 + i ] = data[ j * 3 + i ] * dnum;
		}
    }
    if ( data[ 8 ] == 0. ) data[ 8 ] = SMALL;

    return true; /* normal return */
}

//-------------------------------------------------------------------------------

/// calculate the LUD of the matrix
inline void cyMatrix3f::LUBKS( int *outvect, float *output )
{
	int	i, j, ii, ll;
    float	sum;
	
    ii = -1;  /*when ii is set to a value >= 0 it is an index to
	the first non vanishing element of the output*/

    for ( i = 0; i < 3; i++ ) {
		ll = outvect[ i ];
		sum = output[ ll ];
		output[ ll ] = output[ i ];
		if ( ii != -1 ) {
			for ( j = ii; j < i; j++ )
				sum = sum - ( data[ j * 3 + i ] * output[ j ] );
		}
		else if ( sum != 0. ) {
			ii = i ; 
		}
		output[ i ] = sum;
    }
    for ( i = ( 3 - 1 ); i > -1; i-- ) {
		sum = output[ i ];
		if ( i < ( 3 - 1 ) ) {
			for ( j = ( i + 1 ); j < 3; j++ )
				sum = sum - ( data[ j * 3 + i ] * output[ j ] );
		}
		output[ i ] = sum / data[ i * 3 + i ];
    }
	
}


//-------------------------------------------------------------------------------
// friend function definitions
//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix
inline cyMatrix3f operator+( const float value, const cyMatrix3f &right )
{
	cyMatrix3f buffer; // create a temp cyMatrix3f object not to change right
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = right.data[ i ] + value;	// add value to all members of the matrix
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract the matrix from a fixed value
inline cyMatrix3f operator-( const float value, const cyMatrix3f &right )
{
	cyMatrix3f buffer; // create a temp cyMatrix3f object not to change right
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = value - right.data[ i ];	// subtract matrix from the value;
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// multiply a fixed value with the matrix
inline cyMatrix3f operator*( const float value, const cyMatrix3f &right )
{
	cyMatrix3f buffer; // create a temp cyMatrix3f object not to change right
	
	for ( int i=0; i<9; i++ )
		buffer.data[ i ] = right.data[ i ] * value;	// multiply value to all members of the matrix
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// left multiplication of a vector with the matrix
inline cyPoint3f operator*( const cyPoint3f &p, const cyMatrix3f &m ) 
{
	return cyPoint3f(	p.x * m.data[0] + p.y * m.data[1] + p.z * m.data[2],
						p.x * m.data[3] + p.y * m.data[4] + p.z * m.data[5],
						p.x * m.data[6] + p.y * m.data[7] + p.z * m.data[8] );
}

//-------------------------------------------------------------------------------

#endif
