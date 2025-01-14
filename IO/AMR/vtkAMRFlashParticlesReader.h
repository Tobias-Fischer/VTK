/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkAMRFlashParticlesReader.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
/**
 * @class   vtkAMRFlashParticlesReader
 * @brief   A concrete instance of vtkAMRBaseParticlesReader that implements
 *  functionality for reading flash particle datasets.
 */

#ifndef vtkAMRFlashParticlesReader_h
#define vtkAMRFlashParticlesReader_h

#include "vtkAMRBaseParticlesReader.h"
#include "vtkIOAMRModule.h" // For export macro

VTK_ABI_NAMESPACE_BEGIN
class vtkIndent;
class vtkPolyData;
class vtkPointData;
class vtkIdList;
class vtkFlashReaderInternal;

class VTKIOAMR_EXPORT vtkAMRFlashParticlesReader : public vtkAMRBaseParticlesReader
{
public:
  static vtkAMRFlashParticlesReader* New();
  vtkTypeMacro(vtkAMRFlashParticlesReader, vtkAMRBaseParticlesReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * See vtkAMRBaseParticlesReader::GetTotalNumberOfParticles.
   */
  int GetTotalNumberOfParticles() override;

protected:
  vtkAMRFlashParticlesReader();
  ~vtkAMRFlashParticlesReader() override;

  /**
   * See vtkAMRBaseParticlesReader::ReadMetaData
   */
  void ReadMetaData() override;

  /**
   * See vtkAMRBaseParticlesReader::SetupParticlesDataSelections
   */
  void SetupParticleDataSelections() override;

  /**
   * See vtkAMRBaseParticlesReader::ReadParticles
   */
  vtkPolyData* ReadParticles(const int blkidx) override;

  /**
   * Reads the particlles of the given block from the given file.
   */
  vtkPolyData* GetParticles(const char* file, const int blkidx);

  vtkFlashReaderInternal* Internal;

private:
  vtkAMRFlashParticlesReader(const vtkAMRFlashParticlesReader&) = delete;
  void operator=(const vtkAMRFlashParticlesReader&) = delete;
};

VTK_ABI_NAMESPACE_END
#endif /* vtkAMRFlashParticlesReader_h */
