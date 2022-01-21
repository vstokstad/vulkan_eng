#pragma once


struct Matrix
{
	float m11, m12, m13;
	float m21, m22, m23;
	float m31, m32, m33;

	Matrix operator*(Matrix rhs);
};

inline Matrix Matrix::operator*(Matrix rhs)
{
	Matrix result;

	// m11 = this.col(1) * rhs.row(1)
	result.m11 = m11 * rhs.m11 + m21 * rhs.m12 + m31 * rhs.m13;
	// m12 = this.col(2) * rhs.row(1)
	result.m12 = m12 * rhs.m11 + m22 * rhs.m12 + m32 * rhs.m13;
	// m13 = this.col(3) * rhs.row(1)
	result.m13 = m13 * rhs.m11 + m23 * rhs.m12 + m33 * rhs.m13;

	// m21 = this.col(1) * rhs.row(2)
	result.m21 = m11 * rhs.m21 + m21 * rhs.m22 + m31 * rhs.m23;
	// m22 = this.col(2) * rhs.row(2)
	result.m22 = m12 * rhs.m21 + m22 * rhs.m22 + m32 * rhs.m23;
	// m23 = this.col(3) * rhs.row(2)
	result.m23 = m13 * rhs.m21 + m23 * rhs.m22 + m33 * rhs.m23;

	// m31 = this.col(1) * rhs.row(3)
	result.m31 = m11 * rhs.m31 + m21 * rhs.m32 + m31 * rhs.m33;
	// m32 = this.col(2) * rhs.row(3)
	result.m32 = m12 * rhs.m31 + m22 * rhs.m32 + m32 * rhs.m33;
	// m33 = this.col(3) * rhs.row(3)
	result.m33 = m13 * rhs.m31 + m23 * rhs.m32 + m33 * rhs.m33;

	return result;
}
