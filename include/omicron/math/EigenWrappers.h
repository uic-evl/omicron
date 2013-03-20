/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/
#ifndef __EIGEN_WRAPPERS_HPP__
#define __EIGEN_WRAPPERS_HPP__

#define EIGEN_DEFAULT_IO_FORMAT Eigen::IOFormat(Eigen::StreamPrecision, 0, ", ", " ", "", "")

// Disable alignment (and vectorization). This is because boost.python code used in omegalib does not like
// aligned ormal parameters in visual studio builds. This may be solved in future versions.
#define EIGEN_DONT_ALIGN 

#include "Eigen/Core"
#include "Eigen/Geometry"

// Define a set of lightweight wrappers around Eigen datatypes used inside omegalib
// wrappers use low-case names to avoid clashes with their Eigen base classes.
namespace omicron {  
	// Typedef for real numbers. swithing this we can change precision of most math types and functions.
	typedef double real;

	//! A 3x3 float matrix using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<real, 3, 3>  Matrix3f; 
	//! A 4x4 float matrix using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<real, 4, 4>  Matrix4f;
	//! A three-component set of float vectors using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<real, 3, Eigen::Dynamic>  Vectors3f; 
	//! A two-component float vector using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<real, 2, 1> Vector2f;
	//! A three-component float vector of varying precision using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<real, 3, 1> Vector3f;
	//! A three-component vector of single precision using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<float, 3, 1> Vector3s;
	//! A four-component float vector using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<real, 4, 1> Vector4f; 
	//! A two-component int vector using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<int, 2, 1> Vector2i;
	//! A three-component float vector using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<int, 3, 1> Vector3i;
	//! A four-component float vector using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/TutorialMatrixClass.html
	typedef Eigen::Matrix<int, 4, 1> Vector4i; 
	//! A floating point quaternion using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/classEigen_1_1Quaternion.html
	typedef Eigen::Quaternion<real> Quaternion; 
	//! Stores three integer indices to define a triangular face.
	typedef Eigen::Matrix<unsigned int, 3, 1>     Triangle;
	//! An angle-axis rotation representation using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/classEigen_1_1AngleAxis.html
	typedef Eigen::AngleAxis<real> AngleAxis;
	//! A general transform using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/classEigen_1_1Transform.html
	typedef Eigen::Transform<real, 3, Eigen::Projective, 0> Transform3;
	//! An affine transform using the Eigen math library. More information here: http://eigen.tuxfamily.org/dox/classEigen_1_1Transform.html
	typedef Eigen::Transform<real, 3, Eigen::Affine, 0> AffineTransform3;

}; //namespace omicron

#endif
