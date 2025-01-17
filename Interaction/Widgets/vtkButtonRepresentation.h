/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkButtonRepresentation.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkButtonRepresentation
 * @brief   abstract class defines the representation for a vtkButtonWidget
 *
 * This abstract class is used to specify how the vtkButtonWidget should
 * interact with representations of the vtkButtonWidget. This class may be
 * subclassed so that alternative representations can be created. The class
 * defines an API, and a default implementation, that the vtkButtonWidget
 * interacts with to render itself in the scene.
 *
 * The vtkButtonWidget assumes an n-state button so that traversal methods
 * are available for changing, querying and manipulating state. Derived
 * classed determine the actual appearance. The state is represented by an
 * integral value 0<=state<numStates.
 *
 * To use this representation, always begin by specifying the number of states.
 * Then follow with the necessary information to represent each state (done through
 * a subclass API).
 *
 * @sa
 * vtkButtonWidget
 */

#ifndef vtkButtonRepresentation_h
#define vtkButtonRepresentation_h

#include "vtkDeprecation.h"              // For VTK_DEPRECATED_IN_9_2_0
#include "vtkInteractionWidgetsModule.h" // For export macro
#include "vtkWidgetRepresentation.h"

VTK_ABI_NAMESPACE_BEGIN
class VTKINTERACTIONWIDGETS_EXPORT vtkButtonRepresentation : public vtkWidgetRepresentation
{
public:
  ///@{
  /**
   * Standard methods for the class.
   */
  vtkTypeMacro(vtkButtonRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  ///@{
  /**
   * Retrieve the current button state.
   */
  vtkSetClampMacro(NumberOfStates, int, 1, VTK_INT_MAX);
  ///@}

  ///@{
  /**
   * Retrieve the current button state.
   */
  vtkGetMacro(State, int);
  ///@}

  ///@{
  /**
   * Manipulate the state. Note that the NextState() and PreviousState() methods
   * use modulo traversal. The "state" integral value will be clamped within
   * the possible state values (0<=state<NumberOfStates). Note that subclasses
   * will override these methods in many cases.
   */
  virtual void SetState(int state);
  virtual void NextState();
  virtual void PreviousState();
  ///@}

  enum InteractionStateType
  {
    Outside = 0,
    Inside
  };
#if !defined(VTK_LEGACY_REMOVE)
  VTK_DEPRECATED_IN_9_2_0("because leading underscore is reserved")
  typedef InteractionStateType _InteractionState;
#endif

  ///@{
  /**
   * These methods control the appearance of the button as it is being
   * interacted with. Subclasses will behave differently depending on their
   * particulars.  HighlightHovering is used when the mouse pointer moves
   * over the button. HighlightSelecting is set when the button is selected.
   * Otherwise, the HighlightNormal is used. The Highlight() method will throw
   * a vtkCommand::HighlightEvent.
   */
  enum HighlightStateType
  {
    HighlightNormal,
    HighlightHovering,
    HighlightSelecting
  };
#if !defined(VTK_LEGACY_REMOVE)
  VTK_DEPRECATED_IN_9_2_0("because leading underscore is reserved")
  typedef HighlightStateType _HighlightState;
#endif
  void Highlight(int) override;
  vtkGetMacro(HighlightState, int);
  ///@}

  /**
   * Satisfy some of vtkProp's API.
   */
  void ShallowCopy(vtkProp* prop) override;

protected:
  vtkButtonRepresentation();
  ~vtkButtonRepresentation() override;

  // Values
  int NumberOfStates;
  int State;
  int HighlightState;

private:
  vtkButtonRepresentation(const vtkButtonRepresentation&) = delete;
  void operator=(const vtkButtonRepresentation&) = delete;
};

VTK_ABI_NAMESPACE_END
#endif
