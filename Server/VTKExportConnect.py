import vtk
import re
import numpy as np
import os
from math import isclose
from vtk.util import numpy_support as VN

class VTKExportConnect :
  def __init__(self) :
    print("VTKExportConnect")
  #
  #
  def Init(self, filename) :
    self.filename = filename
    extension = filename.rsplit('.',1)
    if "raw" in extension :
      pattern = re.search("\d+x\d+x\d+",filename)
      #spacingpatternend=filename.find("res")
      #spacing = [float(s) for s in filename[filename.rfind("/")+1:spacingpatternend].split("_")]
      resolution = [int(s)-1 for s in pattern.group(0).split('x')]
      self.reader = vtk.vtkImageReader()
      self.reader.SetFileName(filename)
      fsize = os.stat(filename).st_size
      esize = int(np.prod(np.array(resolution)+1))
      #print("\n\tFile: ",fsize," --- and E:",esize)
      if esize == fsize :
        self.reader.SetDataScalarType(vtk.VTK_UNSIGNED_CHAR)
      elif 2*esize == fsize :
        self.reader.SetDataScalarType(vtk.VTK_UNSIGNED_SHORT)
      self.reader.SetFileDimensionality(3)
      self.reader.SetDataExtent(0,resolution[0],0, resolution[1],0,resolution[2])
      # check if self.spacing is set
      if not hasattr(self, 'spacing') :
        self.spacing = [0.02734,0.02734,0.1] # MRI IBG3
      # put spacing in reader
      self.reader.SetDataSpacing(self.spacing[0],self.spacing[1],self.spacing[2])
      #print("Read spacing: ", spacing)
      #self.reader.SetDataSpacing(spacing[0],spacing[1],spacing[2])
      self.reader.SetNumberOfScalarComponents(1)
      self.reader.SetDataByteOrderToLittleEndian()
    elif "vtk" in extension :
      self.reader = vtk.vtkStructuredPointsReader()
      self.reader.SetFileName(filename)
    self.reader.Update()
    self.isosurface = vtk.vtkContourFilter()
    self.isosurface.SetInputConnection(self.reader.GetOutputPort())
    self.isosurface.ComputeNormalsOn()
    self.isosurface.Update()
    #self.tri = vtk.vtkTriangleFilter()
    #self.tri.SetInputConnection(self.isosurface.GetOutputPort())
    #self.tri.Update()
    #self.reduce = vtk.vtkQuadricDecimation()
    #self.reduce.SetInputConnection(self.isosurface.GetOutputPort())
    #self.reduce.SetTargetReduction(0.9)
    #self.reduce.Update()
    self.reduce = vtk.vtkDecimatePro()
    self.reduce.SetInputConnection(self.isosurface.GetOutputPort())
    self.reduce.SetTargetReduction(0.9)
    self.reduce.SetPreserveTopology(1)
    self.reduce.Update()
    self.normals = vtk.vtkPolyDataNormals()
    self.normals.SetInputConnection(self.reduce.GetOutputPort())
    self.normals.Update()
    self.geometry = vtk.vtkGeometryFilter()
    self.geometry.SetInputConnection(self.normals.GetOutputPort())
    self.geometry.Update()
    self.output = self.geometry
    #transform = vtk.vtkTransform()
    #self.output = vtk.vtkTransformPolyDataFilter()
    #self.output.SetInputConnection(self.geometry.GetOutputPort())
    #transform.RotateWXYZ(90,0,1,0)
    #transform.Translate(0,0,resolution[2])
    #self.output.SetTransform(transform)
    #self.output.Update()
    
    #
    #
    #pts = VN.vtk_to_numpy(v.GetPoints())
    #pts = VN.vtk_to_numpy(v.GetPoints().GetData())
    #npc = [[v.GetCell(i).GetPointIds().GetId(0),v.GetCell(i).GetPointIds().GetId(1), v.GetCell(i).GetPointIds().GetId(2)] for i in range(v.GetNumberOfCells())]
    #n = VN.vtk_to_numpy(p.GetNormals())
    #p=v.GetPointData()
  #
  def SetSpacing(self, spacing) :
    self.spacing = spacing
  #
  def GetPoints(self) :
    self.output.Update()
    o = self.output.GetOutput()
    p = o.GetPoints()
    d = p.GetData()
    return VN.vtk_to_numpy(d)
  #
  def GetIsoRange(self) :
    self.output.Update()
    return self.reader.GetOutput().GetPointData().GetScalars().GetRange()
  def GetNormals(self) :
    self.output.Update()
    pdat = self.output.GetOutput().GetPointData()
    return VN.vtk_to_numpy(pdat.GetNormals())
  def GetTriangles(self) :
    self.output.Update()
    v = self.output.GetOutput()
    return np.array([
              [v.GetCell(i).GetPointIds().GetId(2),
                v.GetCell(i).GetPointIds().GetId(1),
                v.GetCell(i).GetPointIds().GetId(0)]
              for i in range(v.GetNumberOfCells())
            ])
  #
  def SetIsoValue(self, val : float) :
    if val < 1.0 :
      val = 1.0
    print("Setting isovalue to: ", val)
    self.isosurface.SetValue(0,val)
    self.isosurface.Update()
    self.reduce.Update()
    self.normals.Update()
    self.geometry.Update()
    self.output.Update()
    print("Isovalue set to: ", val)
  #
  def ComputeIsoRangeByHistogram(self, bins : int) :
    # Retrieve data directly from reader
    self.reader.Update()
    data = VN.vtk_to_numpy(self.reader.GetOutput().GetPointData().GetScalars())
    # Compute histogram
    hist, bin_edges = np.histogram(data, bins=bins)
    # then take the one that is one higher
    self.SetIsoValue(bin_edges[1])
    # Return the range
    return (bin_edges[1], bin_edges[-1])
  #
#

if __name__ == "__main__" :
 print("Please start main.py")
#endif

