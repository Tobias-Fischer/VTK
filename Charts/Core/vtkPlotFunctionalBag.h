/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPlotFunctionalBag.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/**
 * @class   vtkPlotFunctionalBag
 * @brief   Class for drawing an XY line plot or bag
 * given two columns from a vtkTable.
 *
 *
 * Depending on the number of components, this class will draw either
 * a line plot (for 1 component column) or, for two components columns,
 * a filled polygonal band (the bag) going from the first to the second
 * component on the Y-axis along the X-axis. The filter
 * vtkExtractFunctionalBagPlot is intended to create such "bag" columns.
 *
 * @sa
 * vtkExtractFunctionalBagPlot
 */

#ifndef vtkPlotFunctionalBag_h
#define vtkPlotFunctionalBag_h

#include "vtkChartsCoreModule.h" // For export macro
#include "vtkNew.h"              // Needed to hold SP ivars
#include "vtkPlot.h"

VTK_ABI_NAMESPACE_BEGIN
class vtkDataArray;
class vtkPlotFuntionalBagInternal;
class vtkPlotLine;
class vtkPoints2D;
class vtkScalarsToColors;

class VTKCHARTSCORE_EXPORT vtkPlotFunctionalBag : public vtkPlot
{
public:
  vtkTypeMacro(vtkPlotFunctionalBag, vtkPlot);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Creates a functional bag plot object.
   */
  static vtkPlotFunctionalBag* New();

  /**
   * Returns true if the plot is a functional bag, false if it is a simple
   * line.
   */
  virtual bool IsBag();

  /**
   * Reimplemented to enforce visibility when selected.
   */
  bool GetVisible() override;

  /**
   * Paint event for the plot, called whenever the chart needs to be drawn.
   */
  bool Paint(vtkContext2D* painter) override;

  /**
   * Paint legend event for the plot, called whenever the legend needs the
   * plot items symbol/mark/line drawn. A rect is supplied with the lower left
   * corner of the rect (elements 0 and 1) and with width x height (elements 2
   * and 3). The plot can choose how to fill the space supplied.
   */
  bool PaintLegend(vtkContext2D* painter, const vtkRectf& rect, int legendIndex) override;

  /**
   * Get the bounds for this plot as (Xmin, Xmax, Ymin, Ymax).
   */
  void GetBounds(double bounds[4]) override;

  /**
   * Get the non-log-scaled bounds on chart inputs for this plot as
   * (Xmin, Xmax, Ymin, Ymax).
   */
  void GetUnscaledInputBounds(double bounds[4]) override;

  ///@{
  /**
   * Specify a lookup table for the mapper to use.
   */
  void SetLookupTable(vtkScalarsToColors* lut);
  vtkScalarsToColors* GetLookupTable();
  ///@}

  /**
   * Create default lookup table. Generally used to create one when none
   * is available with the scalar data.
   */
  virtual void CreateDefaultLookupTable();

  /**
   * Function to query a plot for the nearest point to the specified coordinate.
   * Returns the index of the data series with which the point is associated or
   * -1.
   */
  vtkIdType GetNearestPoint(const vtkVector2f& point, const vtkVector2f& tolerance,
    vtkVector2f* location, vtkIdType* segmentId) override;
  using vtkPlot::GetNearestPoint;

  /**
   * Select all points in the specified rectangle.
   */
  bool SelectPoints(const vtkVector2f& min, const vtkVector2f& max) override;

  /**
   * Select all points in the specified polygon.
   */
  bool SelectPointsInPolygon(const vtkContextPolygon& polygon) override;

  /**
   * Update the internal cache. Returns true if cache was successfully updated. Default does
   * nothing.
   * This method is called by Update() when either the plot's data has changed or
   * CacheRequiresUpdate() returns true. It is not necessary to call this method explicitly.
   */
  bool UpdateCache() override;

protected:
  vtkPlotFunctionalBag();
  ~vtkPlotFunctionalBag() override;

  /**
   * Populate the data arrays ready to operate on input data.
   */
  bool GetDataArrays(vtkTable* table, vtkDataArray* array[2]);

  /**
   * Test if the internal cache requires an update.
   */
  virtual bool CacheRequiresUpdate() override;

  /**
   * Lookup Table for coloring points by scalar value
   */
  vtkScalarsToColors* LookupTable;

  /**
   * The plot line delegate for line series
   */
  vtkNew<vtkPlotLine> Line;

  /**
   * The bag points ordered in quadstrip fashion
   */
  vtkNew<vtkPoints2D> BagPoints;

  bool LogX, LogY;

private:
  vtkPlotFunctionalBag(const vtkPlotFunctionalBag&) = delete;
  void operator=(const vtkPlotFunctionalBag&) = delete;
};

VTK_ABI_NAMESPACE_END
#endif // vtkPlotFunctionalBag_h
