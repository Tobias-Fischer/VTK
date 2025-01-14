/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkAMRFlashParticlesReader.cxx

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
#include "vtkAMRFlashParticlesReader.h"
#include "vtkCellArray.h"
#include "vtkDataArraySelection.h"
#include "vtkDoubleArray.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"

#include "vtkAMRFlashReaderInternal.h"

#define H5_USE_16_API
#include "vtk_hdf5.h" // for the HDF data loading engine

#include <cassert>
#include <vector>

//------------------------------------------------------------------------------
// Description:
// Helper function that reads the particle coordinates
// NOTE: it is assumed that H5DOpen has been called on the
// internal file index this->FileIndex.
VTK_ABI_NAMESPACE_BEGIN
static void GetParticleCoordinates(hid_t& dataIdx, std::vector<double>& xcoords,
  std::vector<double>& ycoords, std::vector<double>& zcoords, vtkFlashReaderInternal* iReader,
  int NumParticles)
{

  assert("pre: internal reader should not be nullptr" && (iReader != nullptr));

  hid_t theTypes[3];
  theTypes[0] = theTypes[1] = theTypes[2] = H5I_UNINIT;
  xcoords.resize(NumParticles);
  ycoords.resize(NumParticles);
  zcoords.resize(NumParticles);

  if (iReader->FileFormatVersion < FLASH_READER_FLASH3_FFV8)
  {
    theTypes[0] = H5Tcreate(H5T_COMPOUND, sizeof(double));
    theTypes[1] = H5Tcreate(H5T_COMPOUND, sizeof(double));
    theTypes[2] = H5Tcreate(H5T_COMPOUND, sizeof(double));
    H5Tinsert(theTypes[0], "particle_x", 0, H5T_NATIVE_DOUBLE);
    H5Tinsert(theTypes[1], "particle_y", 0, H5T_NATIVE_DOUBLE);
    H5Tinsert(theTypes[2], "particle_z", 0, H5T_NATIVE_DOUBLE);
  }

  // Read the coordinates from the file
  switch (iReader->NumberOfDimensions)
  {
    case 1:
      if (iReader->FileFormatVersion < FLASH_READER_FLASH3_FFV8)
      {
        H5Dread(dataIdx, theTypes[0], H5S_ALL, H5S_ALL, H5P_DEFAULT, xcoords.data());
      }
      else
      {
        iReader->ReadParticlesComponent(dataIdx, "Particles/posx", xcoords.data());
      }
      break;
    case 2:
      if (iReader->FileFormatVersion < FLASH_READER_FLASH3_FFV8)
      {
        H5Dread(dataIdx, theTypes[0], H5S_ALL, H5S_ALL, H5P_DEFAULT, xcoords.data());
        H5Dread(dataIdx, theTypes[1], H5S_ALL, H5S_ALL, H5P_DEFAULT, ycoords.data());
      }
      else
      {
        iReader->ReadParticlesComponent(dataIdx, "Particles/posx", xcoords.data());
        iReader->ReadParticlesComponent(dataIdx, "Particles/posy", ycoords.data());
      }
      break;
    case 3:
      if (iReader->FileFormatVersion < FLASH_READER_FLASH3_FFV8)
      {
        H5Dread(dataIdx, theTypes[0], H5S_ALL, H5S_ALL, H5P_DEFAULT, xcoords.data());
        H5Dread(dataIdx, theTypes[1], H5S_ALL, H5S_ALL, H5P_DEFAULT, ycoords.data());
        H5Dread(dataIdx, theTypes[2], H5S_ALL, H5S_ALL, H5P_DEFAULT, zcoords.data());
      }
      else
      {
        iReader->ReadParticlesComponent(dataIdx, "Particles/posx", xcoords.data());
        iReader->ReadParticlesComponent(dataIdx, "Particles/posy", ycoords.data());
        iReader->ReadParticlesComponent(dataIdx, "Particles/posz", zcoords.data());
      }
      break;
    default:
      std::cerr << "ERROR: Undefined dimension!\n" << std::endl;
      std::cerr.flush();
      return;
  }
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

vtkStandardNewMacro(vtkAMRFlashParticlesReader);

vtkAMRFlashParticlesReader::vtkAMRFlashParticlesReader()
{
  this->Internal = new vtkFlashReaderInternal();
  this->Initialized = false;
  this->Initialize();
}

//------------------------------------------------------------------------------
vtkAMRFlashParticlesReader::~vtkAMRFlashParticlesReader()
{
  delete this->Internal;
}

//------------------------------------------------------------------------------
void vtkAMRFlashParticlesReader::PrintSelf(std::ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkAMRFlashParticlesReader::ReadMetaData()
{
  if (this->Initialized)
  {
    return;
  }

  this->Internal->SetFileName(this->FileName);
  this->Internal->ReadMetaData();

  // In some cases, the FLASH file format may have no blocks but store
  // just particles in a single block. However, the AMRBaseParticles reader
  // would expect that in this case, the number of blocks is set to 1. The
  // following lines of code provide a simple workaround for that.
  this->NumberOfBlocks = this->Internal->NumberOfBlocks;
  if (this->NumberOfBlocks == 0 && this->Internal->NumberOfParticles > 0)
  {
    this->NumberOfBlocks = 1;
  }
  this->Initialized = true;
  this->SetupParticleDataSelections();
}

//------------------------------------------------------------------------------
int vtkAMRFlashParticlesReader::GetTotalNumberOfParticles()
{
  assert("Internal reader is null" && (this->Internal != nullptr));
  return (this->Internal->NumberOfParticles);
}

//------------------------------------------------------------------------------
vtkPolyData* vtkAMRFlashParticlesReader::GetParticles(
  const char* file, const int vtkNotUsed(blkidx))
{
  hid_t dataIdx = H5Dopen(this->Internal->FileIndex, file);
  if (dataIdx < 0)
  {
    vtkErrorMacro("Could not open particles file!");
    return nullptr;
  }

  vtkPolyData* particles = vtkPolyData::New();
  vtkPoints* positions = vtkPoints::New();
  positions->SetDataTypeToDouble();
  positions->SetNumberOfPoints(this->Internal->NumberOfParticles);

  vtkPointData* pdata = particles->GetPointData();
  assert("pre: PointData is nullptr" && (pdata != nullptr));

  // Load the particle position arrays by name
  std::vector<double> xcoords;
  std::vector<double> ycoords;
  std::vector<double> zcoords;
  GetParticleCoordinates(
    dataIdx, xcoords, ycoords, zcoords, this->Internal, this->Internal->NumberOfParticles);

  // Sub-sample particles
  int TotalNumberOfParticles = static_cast<int>(xcoords.size());
  vtkIdList* ids = vtkIdList::New();
  ids->SetNumberOfIds(TotalNumberOfParticles);

  vtkIdType NumberOfParticlesLoaded = 0;
  for (int i = 0; i < TotalNumberOfParticles; ++i)
  {
    if (i % this->Frequency == 0)
    {
      if (this->CheckLocation(xcoords[i], ycoords[i], zcoords[i]))
      {
        int pidx = NumberOfParticlesLoaded;
        ids->InsertId(pidx, i);
        positions->SetPoint(pidx, xcoords[i], ycoords[i], zcoords[i]);
        ++NumberOfParticlesLoaded;
      } // END if within requested region
    }   // END if within requested interval
  }     // END for all particles

  xcoords.clear();
  ycoords.clear();
  zcoords.clear();

  ids->SetNumberOfIds(NumberOfParticlesLoaded);
  ids->Squeeze();

  positions->SetNumberOfPoints(NumberOfParticlesLoaded);
  positions->Squeeze();

  particles->SetPoints(positions);
  positions->Squeeze();

  // Create CellArray consisting of a single polyvertex
  vtkCellArray* polyVertex = vtkCellArray::New();
  polyVertex->InsertNextCell(NumberOfParticlesLoaded);
  for (vtkIdType idx = 0; idx < NumberOfParticlesLoaded; ++idx)
  {
    polyVertex->InsertCellPoint(idx);
  }
  particles->SetVerts(polyVertex);
  polyVertex->Delete();

  // Load particle data arrays
  int numArrays = this->ParticleDataArraySelection->GetNumberOfArrays();
  for (int i = 0; i < numArrays; ++i)
  {
    const char* name = this->ParticleDataArraySelection->GetArrayName(i);
    if (this->ParticleDataArraySelection->ArrayIsEnabled(name))
    {
      int attrIdx = this->Internal->ParticleAttributeNamesToIds[name];
      hid_t attrType = this->Internal->ParticleAttributeTypes[attrIdx];

      if (attrType == H5T_NATIVE_DOUBLE)
      {
        std::vector<double> data(this->Internal->NumberOfParticles);

        if (this->Internal->FileFormatVersion < FLASH_READER_FLASH3_FFV8)
        {
          hid_t dataType = H5Tcreate(H5T_COMPOUND, sizeof(double));
          H5Tinsert(dataType, name, 0, H5T_NATIVE_DOUBLE);
          H5Dread(dataIdx, dataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());
          H5Tclose(dataType);
        }
        else
        {
          this->Internal->ReadParticlesComponent(dataIdx, name, data.data());
        }

        vtkDataArray* array = vtkDoubleArray::New();
        array->SetName(name);
        array->SetNumberOfTuples(ids->GetNumberOfIds());
        array->SetNumberOfComponents(1);

        vtkIdType numIds = ids->GetNumberOfIds();
        for (vtkIdType pidx = 0; pidx < numIds; ++pidx)
        {
          vtkIdType particleIdx = ids->GetId(pidx);
          array->SetComponent(pidx, 0, data[particleIdx]);
        } // END for all ids of loaded particles
        pdata->AddArray(array);
      }
      else if (attrType == H5T_NATIVE_INT)
      {
        hid_t dataType = H5Tcreate(H5T_COMPOUND, sizeof(int));
        H5Tinsert(dataType, name, 0, H5T_NATIVE_INT);

        std::vector<int> data(this->Internal->NumberOfParticles);
        H5Dread(dataIdx, dataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());

        vtkDataArray* array = vtkIntArray::New();
        array->SetName(name);
        array->SetNumberOfTuples(ids->GetNumberOfIds());
        array->SetNumberOfComponents(1);

        vtkIdType numIds = ids->GetNumberOfIds();
        for (vtkIdType pidx = 0; pidx < numIds; ++pidx)
        {
          vtkIdType particleIdx = ids->GetId(pidx);
          array->SetComponent(pidx, 0, data[particleIdx]);
        } // END for all ids of loaded particles
        pdata->AddArray(array);
      }
      else
      {
        vtkErrorMacro("Unsupported array type in HDF5 file!");
        return nullptr;
      }
    } // END if the array is supposed to be loaded
  }   // END for all arrays

  H5Dclose(dataIdx);

  return (particles);
}

//------------------------------------------------------------------------------
vtkPolyData* vtkAMRFlashParticlesReader::ReadParticles(const int blkidx)
{
  assert("pre: Internal reader is nullptr" && (this->Internal != nullptr));
  assert("pre: Not initialized " && (this->Initialized));

  int NumberOfParticles = this->Internal->NumberOfParticles;
  if (NumberOfParticles <= 0)
  {
    vtkPolyData* emptyParticles = vtkPolyData::New();
    assert("Cannot create particle dataset" && (emptyParticles != nullptr));
    return (emptyParticles);
  }

  vtkPolyData* particles = this->GetParticles(this->Internal->ParticleName.c_str(), blkidx);
  assert("partciles should not be nullptr " && (particles != nullptr));

  return (particles);
}

//------------------------------------------------------------------------------
void vtkAMRFlashParticlesReader::SetupParticleDataSelections()
{
  assert("pre: Internal reader is nullptr" && (this->Internal != nullptr));

  unsigned int N = static_cast<unsigned int>(this->Internal->ParticleAttributeNames.size());
  for (unsigned int i = 0; i < N; ++i)
  {
    this->ParticleDataArraySelection->AddArray(this->Internal->ParticleAttributeNames[i].c_str());
  } // END for all particles attributes

  this->InitializeParticleDataSelections();
}
VTK_ABI_NAMESPACE_END
