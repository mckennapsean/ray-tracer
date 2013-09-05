// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
///
/// \file		cyMatrix4.h 
/// \author		Cem Yuksel
/// \version	1.3
/// \date		August 23, 2012
///
/// \brief 4x4 matrix class
///
//-------------------------------------------------------------------------------

#ifndef _CY_MATRIX4_H_INCLUDED_
#define _CY_MATRIX4_H_INCLUDED_

//-------------------------------------------------------------------------------

#include "cyPoint.h"

//-------------------------------------------------------------------------------

/// 4x4 matrix class.
/// Its data stores 16-value array of column-major matrix elements.
/// I chose column-major format to be compatible with OpenGL
/// You can use cyMatrix4f with cyPoint3f and cyPoint4f
/// to transform 3D and 4D points.
/// Both post-multiplication and pre-multiplication are supported.

class cyMatrix4f
{
	
	friend cyMatrix4f operator+( const float, const cyMatrix4f & );			///< add a value to a matrix
	friend cyMatrix4f operator-( const float, const cyMatrix4f & );			///< subtract the matrix from a value
	friend cyMatrix4f operator*( const float, const cyMatrix4f & );			///< multiple matrix by a value
	friend cyMatrix4f Inverse( cyMatrix4f &m ) { return m.GetInverse(); }	///< return the inverse of the matrix
	friend cyPoint3f  operator*( const cyPoint3f &, const cyMatrix4f & );	///< pre-multiply with a 3D point
	friend cyPoint4f  operator*( const cyPoint4f &, const cyMatrix4f & );	///< pre-multiply with a 4D point

public:

	/// elements of the matrix are column-major as in OpenGL
	float data[16];


	//////////////////////////////////////////////////////////////////////////
	///@name Constructors
	cyMatrix4f() {}	///< Default constructor
	cyMatrix4f( const cyMatrix4f &matrix ) { for ( int i=0; i<16; i++ ) data[i]=matrix.data[i]; } ///< Copy constructor


	//////////////////////////////////////////////////////////////////////////
	///@name Set & Get Methods

	/// Set all the values as zero
	void Zero() { for ( int i=0; i<16; i++ ) data[ i ] = 0; }
	/// Set Matrix using an array of 16 values
	void Set( const float *array ) { for ( int i=0; i<16; i++ ) data[ i ] = array[ i ]; } 
	/// Set Matrix using x,y,z vectors and coordinate center
	void Set( const cyPoint3f &x, const cyPoint3f &y, const cyPoint3f &z, const cyPoint3f &pos );
	/// Set Matrix using x,y,z,w vectors
	void Set( const cyPoint4f &x, const cyPoint4f &y, const cyPoint4f &z, const cyPoint4f &w );
	/// Set Matrix using position, normal, and approximate x direction
	void Set( const cyPoint3f &pos, const cyPoint3f &normal, cyPoint3f &dir );
	/// Converts the matrix to an identity matrix
	void SetIdentity() { for(int i=0; i<16; i++) data[i]=(i%5==0) ? 1.0f : 0.0f ; }
	/// Set View Matrix using position, target and approximate up vector
	void SetView( const cyPoint3f &pos, const cyPoint3f &target, cyPoint3f &up );
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
	/// Set translation matrix
	void SetTrans( cyPoint3f move );

	// Get Row and Column
	cyPoint4f GetRow( int row ) const { return cyPoint4f( data[row], data[row+4], data[row+8], data[row+12] ); }
	void	  GetRow( int row, cyPoint4f &p ) const { p.Set( data[row], data[row+4], data[row+8], data[row+12] ); }
	void	  GetRow( int row, float *array ) const { array[0]=data[row]; array[1]=data[row+4]; array[2]=data[row+8]; array[3]=data[row+12]; }
	cyPoint4f GetColumn( int col ) const { return cyPoint4f( &data[col*4] ); }
	void	  GetColumn( int col, cyPoint4f &p ) const { p.Set( &data[col*4] ); }
	void	  GetColumn( int col, float *array ) const { array[0]=data[col*4]; array[1]=data[col*4+1]; array[2]=data[col*4+2]; array[3]=data[col*4+3]; }


	//////////////////////////////////////////////////////////////////////////
	///@name Overloaded Operators

	const cyMatrix4f &operator=( const cyMatrix4f & );	///< assign matrix

	// Overloaded comparison operators 
	bool operator==( const cyMatrix4f & ) const;		///< compare equal
	bool operator!=( const cyMatrix4f &right ) const { return ! ( *this == right ); } ///< compare not equal

	// Overloaded subscript operators
	float& operator()( int row, int column );					///< subscript operator
	float& operator[](int i) { return data[i]; }				///< subscript operator
	const float& operator()( int row, int column ) const;		///< constant subscript operator
	const float& operator[](int i) const { return data[i]; }	///< constant subscript operator

	// Unary operators
	cyMatrix4f operator - () const;	///< negative matrix

	// Binary operators
	cyMatrix4f operator + ( const cyMatrix4f & ) const;	///< add two Matrices
	cyMatrix4f operator - ( const cyMatrix4f & ) const;	///< subtract one cyMatrix4f from an other
	cyMatrix4f operator * ( const cyMatrix4f & ) const;	///< multiply a matrix with an other
	cyMatrix4f operator + ( const float ) const;		///< add a value to a matrix
	cyMatrix4f operator - ( const float ) const;		///< subtract a value from a matrix
	cyMatrix4f operator * ( const float ) const;		///< multiple matrix by a value
	cyMatrix4f operator / ( const float ) const;		///< divide matrix by a value;
	cyPoint3f operator * ( const cyPoint3f& p) const;
	cyPoint4f operator * ( const cyPoint4f& p) const;

	// Assignment operators
	void	operator +=	( const cyMatrix4f & );	///< add two Matrices modify this
	void	operator -=	( const cyMatrix4f & );	///< subtract one cyMatrix4f from an other modify this matrix
	void	operator *=	( const cyMatrix4f & );	///< multiply a matrix with an other modify this matrix
	void	operator +=	( const float );		///< add a value to a matrix modify this
	void	operator -=	( const float );		///< subtract a value from a matrix modify this matrix
	void	operator *=	( const float );		///< multiply a matrix with a value modify this matrix
	void	operator /=	( const float );		///< divide the matrix by a value modify the this matrix


	//////////////////////////////////////////////////////////////////////////
	///@name Other Public Methods
	void SetTranspose();			///< Transpose this matrix
	cyMatrix4f Transpose() const;	///< return Transpose of this matrix
	void Invert();					///< Invert this matrix
	void GetInverse( cyMatrix4f &inverse ) const { inverse=*this; inverse.Invert(); }	///< Get the inverse of this matrix
	cyMatrix4f GetInverse() const { cyMatrix4f inv(*this); inv.Invert(); return inv; }	///< Get the inverse of this matrix

	//////////////////////////////////////////////////////////////////////////


private:

	/// \internal
	void LUBKS( int *outvect, float *output );		// Used by the Invert function to calculate the LUD of the matrix
	int LUD( int *outvect, int output );			// calculate the LUD of the matrix
};

//-------------------------------------------------------------------------------

namespace cy {
	typedef cyMatrix4f Matrix4f;
}

//-------------------------------------------------------------------------------

/// Set Matrix using x,y,z vectors and coordinate center
inline void cyMatrix4f::Set( const cyPoint3f &x, const cyPoint3f &y, const cyPoint3f &z, const cyPoint3f &pos )
{
	x.GetValue( &data[0] );		data[3]=0;
	y.GetValue( &data[4] );		data[7]=0;
	z.GetValue( &data[8] );		data[11]=0;
	pos.GetValue( &data[12] );	data[15]=1;
}

//-------------------------------------------------------------------------------

/// Set Matrix using x,y,z,w vectors
inline void cyMatrix4f::Set( const cyPoint4f &x, const cyPoint4f &y, const cyPoint4f &z, const cyPoint4f &w )
{
	x.GetValue( &data[0] );
	y.GetValue( &data[4] );
	z.GetValue( &data[8] );
	w.GetValue( &data[12] );
}

//-------------------------------------------------------------------------------

/// Set Matrix using position, normal, and approximate x direction
inline void cyMatrix4f::Set( const cyPoint3f &pos, const cyPoint3f &normal, cyPoint3f &dir )
{
	cyPoint3f y = normal.Cross(dir);
	y.Normalize();
	dir = y.Cross(normal);
	Set( dir, y, normal, pos );
	
}

//-------------------------------------------------------------------------------

/// Set View Matrix using position, target and approximate up vector
inline void cyMatrix4f::SetView( const cyPoint3f &pos, const cyPoint3f &target, cyPoint3f &up )
{
	cyPoint3f f = target - pos;
	f.Normalize();
	cyPoint3f s = f.Cross(up);
	s.Normalize();
	cyPoint3f u = s.Cross(f);

	cyMatrix4f m;
	m.SetIdentity();
	m.data[0]=s.x;	m.data[1]=u.x;	m.data[2]=-f.x;
	m.data[4]=s.y;	m.data[5]=u.y;	m.data[6]=-f.y;
	m.data[8]=s.z;	m.data[9]=u.z;	m.data[10]=-f.z;

	cyMatrix4f t;
	t.SetIdentity();
	t.data[12] = - pos.x;
	t.data[13] = - pos.y;
	t.data[14] = - pos.z;

	*this = m * t;

}

//-------------------------------------------------------------------------------

/// Set Matrix using position, normal, and approximate x direction
inline void cyMatrix4f::SetNormal( const cyPoint3f &normal, cyPoint3f &dir )
{
	cyPoint3f y = normal.Cross(dir);
	y.Normalize();
	dir = y.Cross(normal);
	Set( dir, y, normal, cyPoint3f(0,0,0) );
}

//-------------------------------------------------------------------------------

/// Set a rotation matrix about the given axis by angle theta
inline void cyMatrix4f::SetRotation( cyPoint3f axis, float theta )
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
inline void cyMatrix4f::SetRotation( cyPoint3f from, cyPoint3f to )
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
inline void cyMatrix4f::SetRotation( cyPoint3f axis, float c, float s )
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
	data[3] = 0;
	data[4] = txy - sz;
	data[5] = ty * axis.y + c;
	data[6] = tyz + sx;
	data[7] = 0;
	data[8] = txz + sy;
	data[9] = tyz - sx;
	data[10] = tz * axis.z + c;
	data[11] = 0;
	data[12] = 0;
	data[13] = 0;
	data[14] = 0;
	data[15] = 1;
}

//-------------------------------------------------------------------------------

/// Set translation matrix
inline void cyMatrix4f::SetTrans( cyPoint3f move )
{
	data[12] = move.x;
	data[13] = move.y;
	data[14] = move.z;
}


//-------------------------------------------------------------------------------
// Overloaded Operators
//-------------------------------------------------------------------------------

inline cyPoint3f cyMatrix4f::operator * ( const cyPoint3f& p) const
{
	return cyPoint3f(	p.x * data[0] + p.y * data[4] + p.z * data[8]  + data[12],
						p.x * data[1] + p.y * data[5] + p.z * data[9]  + data[13],
						p.x * data[2] + p.y * data[6] + p.z * data[10] + data[14] );
}

//-------------------------------------------------------------------------------

inline cyPoint4f cyMatrix4f::operator * ( const cyPoint4f& p) const
{
	return cyPoint4f(	p.x * data[0] + p.y * data[4] + p.z * data[8]  + p.w * data[12],
						p.x * data[1] + p.y * data[5] + p.z * data[9]  + p.w * data[13],
						p.x * data[2] + p.y * data[6] + p.z * data[10] + p.w * data[14],
						p.x * data[3] + p.y * data[7] + p.z * data[11] + p.w * data[15] );
}

//-------------------------------------------------------------------------------

/// Overloaded assignment operator
/// const return avoids ( a1 = a2 ) = a3
inline const cyMatrix4f& cyMatrix4f::operator =( const cyMatrix4f &right )
{
	for ( int i = 0; i < 16; i++ ) data[i] = right.data[ i ];		// copy array into object
	return *this;	// enables x = y = z;
}

//-------------------------------------------------------------------------------

/// Determine if two arrays are equal and
/// return true, otherwise return false.
inline bool cyMatrix4f::operator ==( const cyMatrix4f &right ) const
{
	for ( int i = 0; i < 16; i++ ) {
		if ( data[ i ] != right.data[ i ] ) {
			return false;		// arrays are not equal
		}
	}
	return true;				// arrays are equal
}

//-------------------------------------------------------------------------------

/// Overloaded unary minus operator
/// negative of cyMatrix4f
inline cyMatrix4f cyMatrix4f::operator -() const
{
	cyMatrix4f buffer; // create a temp cyMatrix4f object not to change this

	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = - data[ i ];

	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix
inline cyMatrix4f cyMatrix4f::operator +( const float value ) const
{
	cyMatrix4f buffer; // create a temp cyMatrix4f object not to change this
	
	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = data[ i ] + value;	// add value to all member of the matrix
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add two matrices
inline cyMatrix4f cyMatrix4f::operator +( const cyMatrix4f &right ) const
{
	cyMatrix4f buffer;	// create a temp cyMatrix4f object not to change this
	
	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = data[ i ] + right.data[ i ];
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix modify matrix
inline void cyMatrix4f::operator +=( const float value )
{
	for ( int i = 0; i < 16; i++ )
		data[ i ] = data[ i ] + value;	// add value to all member of the matrix
}

//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add two matrices modify this matrix
inline void cyMatrix4f::operator +=( const cyMatrix4f &right )
{
	for ( int i = 0; i < 16; i++ )
		data[ i ] = data[ i ] + right.data[ i ];
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a fixed value from a cyMatrix4f
inline cyMatrix4f cyMatrix4f::operator -( const float value ) const
{
	cyMatrix4f buffer; // create a temp cyMatrix4f object not to change this
	
	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = data[ i ] - value;	// subtract a value from all member of the matrix
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a matrix right from this
inline cyMatrix4f cyMatrix4f::operator -( const cyMatrix4f &right ) const
{
	cyMatrix4f buffer;	// create a temp cyMatrix4f object not to change this
	
	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = data[ i ] - right.data[ i ];
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a fixed value from a cyMatrix4f modify this matrix
inline void cyMatrix4f::operator -=( const float value )
{
	for ( int i = 0; i < 16; i++ )
		data[ i ] = data[ i ] - value;	// subtract a value from all member of the matrix
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract a matrix right from this modify this matrix
inline void cyMatrix4f::operator -=( const cyMatrix4f &right )
{
	for ( int i = 0; i < 16; i++ )
		data[ i ] = data[ i ] - right.data[ i ];
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply a matrix with a value
inline cyMatrix4f cyMatrix4f::operator *( const float value ) const
{
	cyMatrix4f buffer;	// create a temp cyMatrix4f object not to change this
	
	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = data[ i ] * value;
	
	// return temporary object not to change this
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply two matrices 
inline cyMatrix4f cyMatrix4f::operator *( const cyMatrix4f &right ) const
{
	cyMatrix4f buffer;  // a matrix of (m x k)
	
	for ( int i = 0; i < 4; i++ ) {
		for ( int k = 0; k < 4; k++ ) {
			buffer.data[ i + 4 * k ] = 0;
			for ( int j = 0; j < 4; j++ ) {
				buffer.data[ i + 4 * k ] += data[ i + 4 * j ] * right.data[ j + 4 * k ];
			}
		}
	}
	
	return buffer;
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply a matrix with a value modify this matrix
inline void cyMatrix4f::operator *=( const float value )
{
	for ( int i = 0; i < 16; i++ )
		data[ i ] = data[ i ] * value;
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// Multiply two matrices modify this matrix
inline void cyMatrix4f::operator *=( const cyMatrix4f &right )
{
	cyMatrix4f buffer;  // a matrix of (m x k)
	
	for ( int i = 0; i < 4; i++ ) {
		for ( int k = 0; k < 4; k++ ) {
			buffer.data[ i + 4 * k ] = 0;
			for ( int j = 0; j < 4; j++ ) {
				buffer.data[ i + 4 * k ] += data[ i + 4 * j ] * right.data[ j + 4 * k ];
			}
		}
	}
	
	*this = buffer;	// using a buffer to calculate the result
	//then copy buffer to this
}

//-------------------------------------------------------------------------------

/// Overloaded division operator
/// Divide the matrix by value
inline cyMatrix4f cyMatrix4f::operator /( const float value ) const
{
	if ( value == 0 ) return *this;
	return operator * ( (float) 1.0 / value );
}

//-------------------------------------------------------------------------------

/// Overloaded division operator
/// Divide the matrix by value
inline void cyMatrix4f::operator /=( const float value )
{
	if ( value == 0 ) return;
	
	for ( int i = 0; i < 16; i++ )
		data[ i ] = data[ i ] / value;
	
}

//-------------------------------------------------------------------------------

/// Overloaded subscript operator for non-const cyMatrix4f
/// reference return creates an lvalue
inline float& cyMatrix4f::operator ()( int row, int column )
{
	return data[ column * 4 + row ];	// reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subscript operator for const cyMatrix4f
/// const reference return creates an rvalue
inline const float& cyMatrix4f::operator ()( int row, int column ) const
{
	return data[ column * 4 + row ];	// const reference return
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

/// Invert this matrix
inline void cyMatrix4f::Invert()
{
	float temp[4];
    int    IND[4], D = 0, i, j;

	// create the buffer matrix
	cyMatrix4f buffer;

	buffer = *this;

	if ( ! buffer.LUD( IND, D ) ) return;

    for ( j = 0; j < 4; j++ )
	{
		for ( i = 0; i < 4; i++ ) temp[i] = 0.0;
		temp[j] = 1.0;

		buffer.LUBKS( IND, temp);

		for ( i = 0; i < 4; i++ )
			data[ j * 4 + i ] = temp[ i ];
    }
}

//-------------------------------------------------------------------------------

/// Transpose of this matrix
inline void cyMatrix4f::SetTranspose()
{
	float temp;

    for ( int i = 1; i < 4; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			temp = data[ i * 4 + j ];
			data[ i * 4 + j ] = data[ j * 4 + i ];
			data[ j * 4 + i ] = temp;
		}
    }
}

//-------------------------------------------------------------------------------

inline cyMatrix4f cyMatrix4f::Transpose() const
{
	cyMatrix4f m;
	m.data[0]  = data[0];
	m.data[4]  = data[1];
	m.data[8]  = data[2];
	m.data[12] = data[3];
	m.data[1]  = data[4];
	m.data[5]  = data[5];
	m.data[9]  = data[6];
	m.data[13] = data[7];
	m.data[2]  = data[8];
	m.data[6]  = data[9];
	m.data[10] = data[10];
	m.data[14] = data[11];
	m.data[3]  = data[12];
	m.data[7]  = data[13];
	m.data[11] = data[14];
	m.data[15] = data[15];
	return m;
}


//-------------------------------------------------------------------------------
// private methods
//-------------------------------------------------------------------------------

/// calculate the LUD of the matrix
inline int cyMatrix4f::LUD( int *outvect, int output )
{
	int	    i, j, k, imax; 
    float  VV[4], aamax, sum, dnum; 
	float  SMALL = (float) 1.0e-10;

    output = 1; 	     /* No row interchanges yet*/
    
    for ( i = 0; i < 4; i++ ) {
		aamax = 0.0;
		for ( j = 0; j < 4; j++ ) {
			if( fabs( data[ j * 4 + i ] ) > aamax )
				aamax = (float) fabs( data[ j * 4 + i ] );
		}
		if ( aamax == 0. ) {
			return false; /* LUD_ERR flag Singular cyMatrix4f */
		}
		VV[ i ] = (float) 1.0 / (float) aamax;
    }
    
    for ( j = 0; j < 4; j++ ) {
		if ( j > 0 ) {
			for ( i = 0; i < j; i++ ) {
				sum = data[ j * 4 + i ];
				if ( i > 0 ) {
					for ( k = 0; k < i; k++ )
						sum = sum - ( data[ k * 4 + i ] * data[ j * 4 + k ] );
					data[ j * 4 + i ] = sum;
				}
			}
		}

		aamax = 0.;

		for ( i = j; i < 4; i++ ) {
			sum = data[ j * 4 + i ];
			if ( j > 0 ) {
				for ( k = 0; k < j; k++ )
					sum = sum - ( data[ k * 4 + i ] * data[ j * 4 + k ] );
				data[ j * 4 + i ] = sum;
			}
			dnum = VV[ i ] * (float) fabs( sum );
			if( dnum >= aamax ) {
				imax = i;
				aamax = dnum;
			}
		}
		if ( j != imax ) {
			for ( k = 0; k < 4; k++ ) {
				dnum = data[ k * 4 + imax ];
				data[ k * 4 + imax ] = data[ k * 4 + j ];
				data[ k * 4 + j ] = dnum;
			}
			output = -output;
			VV[ imax ] = VV[ j ];
		}
		outvect[ j ] = imax;
		if ( j != ( 4 - 1 ) ) {
			if ( data[ j * 4 + j ] == 0. )
				data[ j * 4 + j ] = SMALL;
			dnum = (float) 1.0 / data[ j * 4 + j ];
			for ( i = ( j + 1 ); i < 4; i++ )
				data[ j * 4 + i ] = data[ j * 4 + i ] * dnum;
		}
    }
    if ( data[ 15 ] == 0. ) data[ 15 ] = SMALL;

    return true; /* normal return */
}

//-------------------------------------------------------------------------------

/// calculate the LUD of the matrix
inline void cyMatrix4f::LUBKS( int *outvect, float *output )
{
	int	i, j, ii, ll;
    float	sum;
	
    ii = -1;  /*when ii is set to a value >= 0 it is an index to
	the first non vanishing element of the output*/

    for ( i = 0; i < 4; i++ ) {
		ll = outvect[ i ];
		sum = output[ ll ];
		output[ ll ] = output[ i ];
		if ( ii != -1 ) {
			for ( j = ii; j < i; j++ )
				sum = sum - ( data[ j * 4 + i ] * output[ j ] );
		}
		else if ( sum != 0. ) {
			ii = i ; 
		}
		output[ i ] = sum;
    }
    for ( i = ( 4 - 1 ); i > -1; i-- ) {
		sum = output[ i ];
		if ( i < ( 4 - 1 ) ) {
			for ( j = ( i + 1 ); j < 4; j++ )
				sum = sum - ( data[ j * 4 + i ] * output[ j ] );
		}
		output[ i ] = sum / data[ i * 4 + i ];
    }
	
}


//-------------------------------------------------------------------------------
// friend function definitions
//-------------------------------------------------------------------------------

/// Overloaded addition operator
/// add a fixed value to the matrix
inline cyMatrix4f operator+( const float value, const cyMatrix4f &right )
{
	cyMatrix4f buffer; // create a temp cyMatrix4f object not to change right
	
	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = right.data[ i ] + value;	// add value to all members of the matrix
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded subtraction operator
/// subtract the matrix from a fixed value
inline cyMatrix4f operator-( const float value, const cyMatrix4f &right )
{
	cyMatrix4f buffer; // create a temp cyMatrix4f object not to change right
	
	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = value - right.data[ i ];	// subtract matrix from the value;
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

/// Overloaded multiplication operator
/// multiply a fixed value with the matrix
inline cyMatrix4f operator*( const float value, const cyMatrix4f &right )
{
	cyMatrix4f buffer; // create a temp cyMatrix4f object not to change right
	
	for ( int i = 0; i < 16; i++ )
		buffer.data[ i ] = right.data[ i ] * value;	// multiply value to all members of the matrix
	
	// return temporary object not to change right
	return buffer;			// value return; not a reference return
}

//-------------------------------------------------------------------------------

inline cyPoint3f operator * ( const cyPoint3f& p, const cyMatrix4f &m )
{
	return cyPoint3f(	p.x * m.data[0] + p.y * m.data[1] + p.z * m.data[2]  + m.data[3],
						p.x * m.data[4] + p.y * m.data[5] + p.z * m.data[6]  + m.data[7],
						p.x * m.data[8] + p.y * m.data[9] + p.z * m.data[10] + m.data[11] );
}

//-------------------------------------------------------------------------------

inline cyPoint4f operator * ( const cyPoint4f& p, const cyMatrix4f &m )
{
	return cyPoint4f(	p.x * m.data[0]  + p.y * m.data[1]  + p.z * m.data[2]  + p.w * m.data[3],
						p.x * m.data[4]  + p.y * m.data[5]  + p.z * m.data[6]  + p.w * m.data[7],
						p.x * m.data[8]  + p.y * m.data[9]  + p.z * m.data[10] + p.w * m.data[11],
						p.x * m.data[12] + p.y * m.data[13] + p.z * m.data[14] + p.w * m.data[15] );
}

//-------------------------------------------------------------------------------

#endif
