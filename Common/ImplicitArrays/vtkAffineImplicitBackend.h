/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAffineImplicitBackend.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// Funded by CEA, DAM, DIF, F-91297 Arpajon, France
#ifndef vtkAffineImplicitBackend_h
#define vtkAffineImplicitBackend_h

/**
 * \struct vtkAffineImplicitBackend
 * \brief A utility structure serving as a backend for affine (as a function of the index) implicit
 * arrays
 *
 * This structure can be classified as a closure and can be called using syntax similar to a
 * function call.
 *
 * At construction it takes two parameters: the slope of the map and the intercept. It returns a
 * value calculated as:
 *
 *   value = slope * index + intercept
 *
 * An example of potential usage in a vtkImplicitArray
 * ```
 * double slope = some_number;
 * double intercept = some_other_number;
 * vtkNew<vtkImplicitArray<vtkAffineImplicitBackend<double>>> affineArray;
 * affineArray->SetBackend(std::make_shared<vtkAffineImplicitBackend<double>>(slope, intercept));
 * affineArray->SetNumberOfTuples(however_many_you_want);
 * affineArray->SetNumberOfComponents(whatever_youd_like);
 * double value = affineArray->GetTypedComponent(index_in_tuple_range, index_in_component_range);
 * ```
 */
template <typename ValueType>
struct vtkAffineImplicitBackend final
{
  /**
   * A non-trivially constructible constructor
   *
   * \param slope the slope of the affine function
   * \param intercept the intercept value at the origin (i.e. the value at 0)
   */
  vtkAffineImplicitBackend(ValueType slope, ValueType intercept)
    : Slope(slope)
    , Intercept(intercept)
  {
  }

  /**
   * The main call method for the backend
   *
   * \param index the index at which one wished to evaluate the backend
   * \return the affinely computed value
   */
  ValueType operator()(int index) const { return this->Slope * index + this->Intercept; }

  /**
   * The slope of the affine function on the indeces
   */
  ValueType Slope;
  /**
   * The value of the affine function at index 0
   */
  ValueType Intercept;
};

#endif // vtkAffineImplicitBackend_h
