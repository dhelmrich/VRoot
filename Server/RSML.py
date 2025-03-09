import numpy as np
from numpy.linalg import norm
import vtk
from xml.dom import minidom
import xml.etree.ElementTree as ET
from vtk.util import numpy_support as VN

class Root :
  def __init__(self,Nodes = [], Diameters = [], Functions = {},RootNumber = 0, Subroots = [], Pred = -1, PaNo = -1) :
    self = self
    self.Nodes = Nodes
    self.Diameters = Diameters
    self.Functions = Functions
    self.RootNumber = RootNumber
    self.Predecessor = Pred
    self.ParentNode = PaNo
    self.Subroots = Subroots
    self.orig = RootNumber
  #end def __init__

  def PushNode(self,pt, params) :
    self.Nodes.append(pt)
    for name,param in params.items() :
      if self.Functions.get(name) != None :
        self.Functions.get(name).append(param)
      else :
        self.Functions[name] = [param]
  #enddef

  def Addroot(self,root : int) :
    self.Subroots.append(root)
    #endif
  #enddef Addroot

  def PushPointOnly(self,pt) :
    self.Nodes.appen(pt)
    for name,param in params :
      param.append(0.0) # todo i need default value

  def ConvertToPolyline(self) :
    lines = vtk.vtkPolyLine()
  #end def Convert To Polyline
  def __str__(self) :
    return "Root Number "+str(self.RootNumber)+"! Points: " + str(len(self.Nodes)) \
      + "\n\tFunctions: " + str(len(self.Functions)) \
      + "\n\tParent: " + str(self.Predecessor)

  def __len__(self) :
    return len(self.Nodes)
  #endif
  def RenderAsCylinders(self) :
    for i in range(len(self.Nodes)-1) :
      start = self.Nodes[i]
      end = self.Nodes[i+1]
      center = start + end/2
      width = self.Diameters[i]
      Meta = [f[i] for f in self.Functions]
      cylindermesh = vtk.vtkCylinderSource()
      cylindermesh.SetResolution(5)
      transmesh = vtk.vtkTransformPolyDataFilter()
      nx = [0,0,0]
      ny = [0,0,0]
      nz = [0,0,0]
      vtk.vtkMath.Subtract(end, start, nx)
      height = vtk.vtkMath.Norm(nx)
      vtk.vtkMath.Normalize(nx)
      nz = [-nx[2], 0, nx[0]]
      vtk.vtkMath.Normalize(nz)
      vtk.vtkMath.Cross(nx,nz,ny)
      vtk.vtkMath.Normalize(ny)
      mattrans = vtk.vtkMatrix4x4()
      mattrans.Identity()
      mattrans.SetElement(0,0,nx[0])
      mattrans.SetElement(1,1,ny[1])
      mattrans.SetElement(2,2,nz[2])
      trans = vtk.vtkTransform()
      trans.Translate(start)
      trans.Concatenate(mattrans)
      trans.RotateZ(-90.0)
      trans.Scale(width,height,width)
      trans.Translate(0,0.5,0)
      transmesh.SetTransform(trans)
      transmesh.SetInputConnection(cyl.GetOutputPort())
      transmesh.Update()
    #endfor
  #enddef

  def copy(self) :
    #def __init__(self,Nodes = [], Diameters = [], Functions = {},RootNumber = 0, Subroots = [], Pred = -1, PaNo = -1) :
    return Root(self.Nodes.copy(),[].copy(),{"diameter":self.Functions["diameter"].copy()}.copy(),self.RootNumber,self.Subroots.copy(),self.Predecessor,self.ParentNode)
  #enddef
#end class


def MakeXMLFromRootlist(roots : list, filename : str, metadata : dict,  invertxaxis = False) :
  print("Printing xml")
  print([root.Predecessor for root in roots])
  rsml = ET.Element("rsml")
  metadataelement = ET.SubElement(rsml,"metadata")
  scene = ET.SubElement(rsml,"scene")
  plant = ET.SubElement(scene,"plant")
  for key in metadata :
    el = ET.SubElement(metadataelement,key)
    el.text = str(metadata[key])
  rootelements = []
  for i,root in enumerate(roots) :
    if not "diameter" in root.Functions :
      print("There are some entries missing here with ",root.RootNumber)
    if root.Predecessor < 0 :
      rootelements.append(ET.SubElement(plant,"root"))
    else :
      rootelements.append(ET.SubElement(next(r for r in rootelements if root.Predecessor == int(r.get("ID"))),"root"))
    rootelements[i].set("ID",str(root.RootNumber))
    properties = ET.SubElement(rootelements[i],"properties")
    rootpid = ET.SubElement(rootelements[i],"root-id")
    rootpid.set("value",str(root.RootNumber))
    parentnode = ET.SubElement(properties,"parent-node")
    parentnode.set("value",str(root.ParentNode))
    geometry = ET.SubElement(rootelements[i],"geometry")
    polyline = ET.SubElement(geometry,"polyline")
    for position in roots[i].Nodes :
      point = ET.SubElement(polyline,"Point")
      if invertxaxis :
        point.set("x",'{p:.18f}'.format(p=-position[0]))
      else :
        point.set("x",'{p:.18f}'.format(p=position[0]))
      point.set("y",'{p:.18f}'.format(p=position[1]))
      point.set("z",'{p:.18f}'.format(p=position[2]))
      #point.set("X",str(position[0]))
      #point.set("Y",str(position[1]))
      #point.set("Z",str(position[2]))
    functions = ET.SubElement(rootelements[i],"functions")
    diameter = ET.SubElement(functions,"functions")
    emergence_time = ET.SubElement(functions,"functions")
    emergence_time.set("name","emergence_time")
    emergence_time.set("domain","polyline")
    diameter.set("domain","polyline")
    diameter.set("name","diameter")
    for value in roots[i].Functions["diameter"] :
      sample = ET.SubElement(diameter,"sample")
      sample.set("value",str(value))
      timesample = ET.SubElement(emergence_time,"sample")
      timesample.set("value","0.0")
  tree = ET.ElementTree(rsml)
  xmlstr = minidom.parseString(ET.tostring(rsml)).toprettyxml(indent="  ")
  with open(filename, "w") as f:
      f.write(xmlstr)
      f.write("\n\n\n")

def TryParseFloat(splitline : list) :
  try :
    f = float(splitline[0])
    return True
  except :
    return False

def TryParseInt(splitline : list) :
  try :
    f = int(splitline[0])
    return True
  except :
    return False

class ParseDFG :
  def __init__(self) :
    self.fname = ""
  def MakeFileFromRootList(self,rootlist) :
    points = []
    parameters = []
  def ParseFile(self) :
    roots = []
    points = []
    parameters = []
    simplex = []
    print("Parsing")
    f = open(self.fname,"r")
    lines = [line for line in f]
    f.close()
    i = 0
    maxroot = -1
    while i < len(lines) :
      line = lines[i]
      if line.startswith("DGF") | line.startswith("#") :
        i += 1
      elif line.startswith("Vertex") :
        i += 1
        line = lines[i]
        if line.startswith("parameters") :
          i += 1
        wss = line.split()
        while TryParseFloat(lines[i].split()) :
          points.append([100.0*float(s) for s in lines[i].split()])
          i += 1
      elif line.startswith("Simplex") :
        params = [{"diameter" : -1.0}.copy() for i in range(len(points))]
        i += 1
        line = lines[i]
        paranames = [x.strip() for x in line.split(':')[1].split(',')]
        pt0i = paranames.index("node1ID")
        pt1i = paranames.index("node2ID")
        diai = paranames.index("radius [cm]")
        rni = paranames.index("brnID")
        i += 1
        line = lines[i]
        wss = line.split()
        previousid = -1
        id = 0
        # change loop to directly parse roots from the list
        # if the left id is lower again than the last maximum
        # we have a new root which will directly receive the point to the right
        # or rather have A POINT IN THE MIDDLE TO HAVE SOME MEANS OF ATTACHMENT
        # this point in the middle will then be the actual thing to build the 
        # root system in unreal later
        rpoints = []
        rpa = {"diameter":[]}
        rootids = {}
        rootnumber = 1
        rootsuntil = []
        attachid = -1
        rnum = 1
        while i < len(lines) and TryParseInt(lines[i].split()) :
          data = lines[i].split()
          rootnumber = rnum
          rnum = int(data[rni])
          if id == 0 :
            rootids[rnum] = []
            t = []
            rootnumber = rnum
            rootids[rnum].append(int(data[pt0i]))
            # take both into this root either because we
            # started or because we have to do the lateral
            rpoints.append(points[int(data[pt0i])])
            rpa["diameter"].append(float(data[diai])*2.0)
          elif rootnumber != rnum :
            attachroot = -1
            trunketattach = -1
            if len(rpoints) > 0 :
              for s in rootids :
                for a,p in enumerate(rootids[s]) :
                  if p == attachid :
                    attachroot = s
                    trunketattach = a
                    break
                if trunketattach != -1 :
                  break
              #attachroot = next(s for s,rst in rootids if attachid in rst) if attachid != -1 else -1
              rootsuntil.append(previousid)
              previousid += 1
              rpa["diameter"] = np.array(rpa["diameter"])
              roots.append(Root(np.array(rpoints.copy()),[].copy(),rpa.copy(),rootnumber,[].copy,attachroot,trunketattach))
              rpoints.clear()
              rpa.clear()
              rootids[rnum] = []
              rpa["diameter"]=[]
              attachid = int(data[pt0i])
              rootnumber+=1
            rpa["diameter"].append(float(data[diai])*2.0)
            middle = (np.array(points[int(data[pt0i])]) \
                  + np.array(points[int(data[pt1i])]))/2.0
            rpoints.append(list(middle))
          rpa["diameter"].append(float(data[diai]))
          rootids[rnum].append(int(data[pt1i]))
          rpoints.append(points[int(data[pt1i])]) # the other one def
          previousid = int(data[pt1i])
          id += 1
          i = i + 1
        #endif
        print("catchup")
        attachroot = -1
        trunketattach = -1
        if len(rpoints) > 0 :
          rpa["diameter"] = np.array(rpa["diameter"])
          for s in rootids :
            for a,p in enumerate(rootids[s]) :
              if p == attachid :
                attachroot = s
                trunketattach = a
                break
            if trunketattach != -1 :
              break
          roots.append(Root(np.array(rpoints.copy()),[].copy(),rpa.copy(),rootnumber,[].copy,attachroot,trunketattach))
        break
      else :
        i += 1
    with open("rootnumberthingies.txt","w") as foo :
      for key in rootids :
        foo.write(str(key)+":")
        foo.write("\t"+str(rootids[key]))
        foo.write("\n")
    return roots
  #enddef


  def SetFilename(self, name : str) :
    self.fname = name
  #enddef

class ParseVTP :
  def __init__(self, nodef, diaf):
    self.nodef = nodef
    self.diaf = diaf
    self.fname=""
  #enddef
  def ParseFile(self) :
    roots = []
    global_points = []
    parameters = []
    simplex = []
    print("Parsing")
    reader = vtk.vtkXMLPolyDataReader()
    reader.SetFileName(self.fname)
    reader.Update()
    pd = reader.GetOutput()
    c2p = vtk.vtkCellDataToPointData()  # set cell and point data
    c2p.SetPassCellData(True)
    c2p.SetInputData(pd)
    c2p.Update()
    poly = c2p.GetPolyDataOutput()
    resolution = 0.1 #mm --> cm
    #get coordinates
    Np = poly.GetNumberOfPoints()
    nodes = np.array([poly.GetPoint(i) for i in range(poly.GetNumberOfPoints())])
    pdata = poly.GetPointData()
    nodes = np.array([self.nodef(x) for x in nodes])
    #nodes = self.nodef(nodes)
    # Always do uniform operations on the node array
    #nodes = nodes*resolution
    #nodes[:,2] = nodes[:,2]*-1+333*0.045 # negative in z direction
    #nodes[:,1] = nodes[:,1]*-1+175*0.045 #  axis reversed
    #nodes[nodes[:,2]>0,2] = 0
    #get segments
    Nc = poly.GetNumberOfCells()
    cdata = poly.GetCellData()
    global_segments = np.zeros((Nc,2))
    for r in range(0, Nc):
      cpi = vtk.vtkIdList()
      poly.GetCellPoints(r, cpi)
      for j in range(0, cpi.GetNumberOfIds()):  
        global_segments[r,j] = cpi.GetId(j)
    global_segments = global_segments.astype(int)
    #todo make cell array here
    arraynames = [cdata.GetArray(i).GetName() for i in range(cdata.GetNumberOfArrays())]
    arrays = [VN.vtk_to_numpy(cdata.GetArray(i)) for i in range(cdata.GetNumberOfArrays())]
    # todo make point array here
    parraynames = [pdata.GetArray(i).GetName() for i in range(pdata.GetNumberOfArrays())]
    parrays = [VN.vtk_to_numpy(pdata.GetArray(i)) for i in range(pdata.GetNumberOfArrays())]
    branchid = arrays[arraynames.index("branchID")].astype("int")
    diameter = arrays[arraynames.index("diameter")]
    # also do the same operations on the diameter!
    diameter = self.diaf(diameter)
    branchid = branchid + 1 if np.min(branchid) == 0 else branchid
    numRoots = np.max(branchid)
    roots = [Root() for i in range(numRoots)]
    for r in range(1,numRoots+1) :
      r_i = r-1
      global_segment_indices = np.nonzero(branchid == r)[0]
      local_segments = global_segments[global_segment_indices]
      local_segment_ordering = [0]
      while len(local_segment_ordering) < len(local_segments) :
        bottom = local_segment_ordering[-1]
        down = next((i for i in range(len(local_segments)) if local_segments[i][0] == local_segments[bottom][1]), -1)
        if down == -1 :
          top = local_segment_ordering[0]
          up = next((i for i in range(len(local_segments)) if local_segments[i][1] == local_segments[top][0]), -1)
          if up == -1 :
            print("Found a remaining segment that is unconnected!")
            print("Discrepancy between connected/unconnected is ", len(local_segment_ordering), " out of ", len(local_segments))
          else :
            local_segment_ordering.insert(0,up)
        else :
          local_segment_ordering.append(down)
      local_continuous_segments = local_segments[local_segment_ordering]
      parent = -1
      attachment_point = -1
      first_root_point = local_continuous_segments[0][0]
      attachment_cell = next((i for i in range(len(global_segments)) if global_segments[i][1] == local_continuous_segments[0][0]), -1)
      if attachment_cell == -1 and branchid[global_segment_indices[0]] > 1 :
        attachment_cell = next((i for i in range(len(global_segments)) if global_segments[i][0] == local_continuous_segments[0][0]), -1)
        if attachment_cell >= 0 :
          parent = branchid[attachment_cell]
          attachment_point = global_segments[attachment_cell][0]
      elif attachment_cell > -1 :
        parent = branchid[attachment_cell]
        attachment_point = global_segments[attachment_cell][1]
      global_points = [segment[1] for segment in local_continuous_segments]
      if parent == -1 :
        global_points.insert(0,local_continuous_segments[0][0])
        roots[r_i].Nodes = nodes[global_points]
      else :
        # the step should be the connection between the attachment point (which is a point id)
        # and the first point of the lateral (which is the first point id of the first cell)
        step = nodes[local_continuous_segments[0][1]] - nodes[local_continuous_segments[0][0]]
        step *= 0.1 * diameter[local_segment_ordering[0]]
        root_nodes = nodes[global_points].tolist()
        root_nodes.insert(0,nodes[attachment_point] + step)
        roots[r_i].Nodes = np.array(root_nodes)
        global_points.insert(0,local_continuous_segments[0][0])
        #np.insert(roots[r_i].Nodes,0,nodes[attachment_point] + step)
        # we duplicate the entry point, but leave the array values alone
      roots[r_i].Functions = dict(zip(parraynames,[masked[global_points] for masked in parrays]))
      roots[r_i].ParentNode = attachment_point
      roots[r_i].Predecessor = parent
      roots[r_i].RootNumber = r
      assert(len(roots[r_i].Functions["diameter"]) == len(roots[r_i].Nodes))
    return roots
    
    
  #enddef
  def SetFilename(self,filename) :
    self.fname = filename
  #enddef

#class Root :
#  def __init__(self,Nodes = [], Diameters = [], Functions = {},RootNumber = 0, Subroots = [], Pred = -1, PaNo = -1) :
#    self.Nodes = Nodes
#    self.Diameters = Diameters
#    self.Functions = Functions
#    self.RootNumber = RootNumber
#    self.Predecessor = Pred
#    self.ParentNode = PaNo
#    self.Subroots = Subroots
def RedrawMainRoot(roots:list,bottomroot:int) :
  print("Trying to change root topology")
  Results = []
  AttachPath = []
  roots.sort(key=lambda r : r.RootNumber)
  print("Trying to sort out whether all roots exist")
  mainroot = roots[0]
  lastroot = next(root for root in roots if root.RootNumber == bottomroot)
  if lastroot == None :
    print("I could not find the root for the root number that was given")
    print("Root number: ",bottomroot," << ",bottomseg)
    print("Root Numbers I have: ",[r.RootNumber for r in roots])
    return roots
  #endif
  bottomseg = -1
  #if len([r for r in roots if r.Predecessor == lastroot.RootNumber and r.ParentNode >= bottomseg]) == 0 :
  # !!! bottom seg is always len(lastroot) because we explicitly set it to be that way
  bottomseg = len(lastroot)
  #endif
  mainrootpoints = []
  mainrootdiameters = []
  mainrootids = []
  newsideroots = []
  modified = [1]
  # !!! bottomseg is = if the parentnode(attach) was used but +1 for len(target)
  print("Going UP the root from the bottom")
  while lastroot != None : #wait main root is important !!!
    # build up attachment skeleton
    print("Going through root number ",lastroot.RootNumber, " before segment ",bottomseg)
    modified.append(lastroot.RootNumber)
    if bottomseg < len(lastroot) :
      print("Current bottom segment is not the end of the root, thus I am making the stump its own root")
      newnodes = list(lastroot.Nodes[bottomseg:])
      newdias = list(lastroot.Functions["diameter"][bottomseg:])
      #if len(newnodes) == 1 :
      #  print("Length of cutoff root would be 1 so I have to add segments")
      #  newseg = np.array(lastroot.Nodes[bottomseg]) - np.array(lastroot.Nodes[bottomseg-1])
      #  newseg = (newseg / np.linalg.norm(newseg)) * lastroot.Functions["diameter"][bottomseg-1]
      #  newnodes.insert(0,newseg.tolist())
      if lastroot.RootNumber == 1 :
        print("Stump gets root number that we removed")
      newnumber = lastroot.RootNumber if lastroot.RootNumber != 1 else bottomroot
      newpredecessor = 1 # because this is directly attached to the main root now
      print("With root number being ",newnumber," -> making sure attachments are valid")
      modified.append(newnumber)
      attachroots = [r for r in roots if r.Predecessor == lastroot.RootNumber and r.ParentNode >= bottomseg and not r.RootNumber in modified]
      for a in attachroots :
        b = a.copy()
        b.ParentNode -= bottomseg
        b.Predecessor = newnumber
        print("Updated ",a.RootNumber," to start at ",b.ParentNode," on root ",b.Predecessor)
        modified.append(b.RootNumber)
        Results.append(b)
        # !!! check if lastroot == main root then make new ids
      print("Putting the root stump to the new side roots")

      newsideroots.append(Root(np.array(newnodes),[].copy(),{"diameter":np.array(newdias)}.copy(),newnumber,[],lastroot.RootNumber,bottomseg))
    else :
      print("From this root I am taking everything")
    print("Searching for fork attachments")
    pushup = [r.copy() for r in roots if (r.Predecessor == lastroot.RootNumber and r.ParentNode == bottomseg-1 and not r.RootNumber in modified)]
    modified.extend([r.RootNumber for r in pushup])
    newsideroots.extend(pushup)
    print("Found ",len(pushup)," that have been attached at the forking")
    print("Logging points for later use when going down")
    mainrootids.insert(0,[lastroot.RootNumber,bottomseg])
    mainrootpoints = list(lastroot.Nodes[0:bottomseg]) + mainrootpoints
    mainrootdiameters = list(lastroot.Functions["diameter"][0:bottomseg]) + mainrootdiameters
    print("Logging attached roots that are within the new main root path")
    mainpathroots = [r for r in roots if r.Predecessor == lastroot.RootNumber and r.ParentNode < bottomseg and not r.RootNumber in modified]
    newsideroots.extend(mainpathroots)
    print("Found ",len(mainpathroots),"!")
    bottomseg = lastroot.ParentNode # warning! I changed this so that the curve is earlier
    if lastroot.Predecessor >= 0 :
      lastroot = next(root for root in roots if root.RootNumber == lastroot.Predecessor)
    else :
      lastroot = None
  #endwhile
  print("Going DOWN the root from the top")
  currentnodepoint = 0
  for i in range(len(mainrootids)) :
    mainrootids[i].append(currentnodepoint)
    currentnodepoint += mainrootids[i][1]
  #endfor
  for k in range(len(newsideroots)) :
    modified.append(newsideroots[k].RootNumber)
    newr = newsideroots[k].copy()
    pred = newr.Predecessor
    indices = next(x for x in mainrootids if x[0] == pred)
    newr.ParentNode += indices[2]
    newr.Predecessor = 1
    Results.append(newr)
  #endfor
  Results.append(Root(mainrootpoints,[],{"diameter":mainrootdiameters},1,[],-1,-1))
  print(modified)
  Results.extend([r.copy() for r in roots if not r.RootNumber in modified])
  return Results
#enddef

def ConnectRootFromRootList(roots:list,toproot:int,topseg:int,bottomroot:int,bottomseg:int) :
  Results = []
  return Results
#enddef

def MakeRootComparison(roots:list,roots2:list) :
  print("RN1 \t\t # \t\t P \t\t N \t\t | \t\t RN2 \t\t # \t\t P \t\t N")
  sum1 = 0
  sum2 = 0
  for i in range(max(len(roots),len(roots2))) :
    if i < len(roots) :
      r = roots[i]
      print(r.RootNumber,"\t\t",len(r),"\t\t",r.Predecessor,"\t\t",r.ParentNode,"\t\t",end="")
      sum1 += len(r.Functions["diameter"])
    else :
      print("\t\t\t\t\t\t\t\t",end="")
    print(" | \t\t",end="")
    if i < len(roots2) :
      r = roots2[i]
      print(r.RootNumber,"(",r.orig,")","\t\t",len(r),"\t\t",r.Predecessor,"\t\t",r.ParentNode,"\t\t",end="")
      sum2 += len(r.Functions["diameter"])
    else :
      print("\t\t\t\t\t\t\t\t",end="")
    print("\n",end="")
  print("Root sums are ", sum1, " and ", sum2)

"""
Dr. Landl:
Die dgfs brauchen genau diese Struktur, damit sie bei
uns im Modell laufen. Ich denke, bis zum Radius ist
wahrscheinlich alles klar, oder? kz und kr sind die axiale
und radiale Wurzelleitfaehigkeit, da wäre es gut, wenn einfach
immer ein default wert reingeschrieben würde, und zwar zb.
kz = 10^6 und kr = 0.005. die default werte für subtype = 1 und
organtype = 2. Bzgl. der Frage, wie wir das dann verwenden,
waere eventuell noch mal ein Gespraech mit Andrea gut?
"""
def MakeDGFFromRootList(fname:str,roots:list, usecompatibility = False):
  if len(roots) < 1 :
    return
  print("Writing DGF File: ",fname)
  if usecompatibility :
    print("Using compatibility setting (v in m)")
  roots=sorted(roots,key=lambda r : r.RootNumber)
  centerpoint = roots[0].Nodes[0] if not usecompatibility else roots[0].Nodes[0] / 100.0
  pointstarts = {roots[0].RootNumber:0}
  currentpointnum = 0
  pointlist = []
  diameters = []
  preds = {}
  orders = []
  #determination of root order here
  rnummax = -1
  for i in range(len(roots)) :
    root = roots[i]
    preds[root.RootNumber] = root.Predecessor
    rnummax = max(rnummax, root.RootNumber)
    if i + 1 < len(roots) :
      pointstarts[roots[i+1].RootNumber] = pointstarts[roots[i].RootNumber] + len(root)
  orders = {i:-1 for i in range(1,rnummax+1)}
  while any([orders[n]==-1 for n in orders]) :
    for rn in range(1,rnummax+1) :
      if orders[rn] == -1 :
        if preds[rn] == -1 :
          orders[rn] = 1
        elif orders[preds[rn]] >= 0 :
          orders[rn] = orders[preds[rn]]+1
        else :
          continue
  # building of arrays for writing
  for i in range(len(roots)) :
    if usecompatibility :
      pointlist.extend((np.array(roots[i].Nodes) / 100.0).copy())
    else :
      pointlist.extend(roots[i].Nodes.copy())
    pred = None if roots[i].Predecessor == -1 \
                else next(r for r in roots if r.RootNumber == roots[i].Predecessor)
    if pred != None :
      orda = orders[roots[i].RootNumber]
      print("Predecessor is not none so we check for the ids")
      node1ID = roots[i].ParentNode + pointstarts[roots[i].Predecessor]
      print("This id will be ",node1ID)
      node2ID = pointstarts[roots[i].RootNumber]
      print("This id will be ",node2ID)
      length = norm(np.array(pointlist[node2ID])-np.array(pointlist[node1ID]))
      surf = length * np.pi * (0.5 * (pred.Functions["diameter"][roots[i].ParentNode]
                               + roots[i].Functions["diameter"][0]))
      diameters.append([node1ID,node2ID,orda,
                        roots[i].RootNumber, surf, length, roots[i].Functions["diameter"][0] / 2.0,
                        10**6, 0.0005, 0, 1, 0 ])
    for p in range(1,len(roots[i])) :
      p1 = np.array(roots[i].Nodes[p-1])
      p2 = np.array(roots[i].Nodes[p  ])
      l = norm(p2-p1)
      id0 = p - 1 + pointstarts[roots[i].RootNumber]
      orda = orders[roots[i].RootNumber]
      id1 = p     + pointstarts[roots[i].RootNumber]
      r = 0.5 * roots[i].Functions["diameter"][p]
      s = 3.14159265359 * r*r * l
      diameters.append([id0,id1,orda,roots[i].RootNumber,s,l,r,10**6,0.0004,0,1,0])
  if usecompatibility :
    pointlist = np.array(pointlist)
    pointlist[:,0] = -pointlist [:,0]
  with open(fname,"w") as f :
    f.write("DGF\nVertex\nparameters 0\n")
    for p in pointlist :
      f.write(str(p[0]) + " " + str(p[1]) + " " + str(p[2]) + "\n")
    f.write("#\nSimplex\nparameters 10: node1ID, node2ID, order, brnID, surf [cm2], length [cm], radius [cm], kz [cm4 hPa-1 d-1], kr [cm hPa-1 d-1], emergence time [d], subtype, organtype \n")
    for d in diameters :
      f.write(" ".join([str(e) for e in d])+"\n")
    f.write("BOUNDARYSEGMENTS\n")
    f.write("2\t0\n")
    f.write("3\t"+str(len(pointlist)-1)+"\n")
    f.write("BOUNDARYDOMAIN\ndefault\t1\n")
    f.write("#\n#\n#\n")
    f.close()
  with open(fname+"_center.dgf","w") as f :
    f.write("DGF\nVertex\nparameters 0\n")
    for p in pointlist - centerpoint :
      f.write(str(p[0]) + " " + str(p[1]) + " " + str(p[2]) + "\n")
    f.write("#\nSimplex\nparameters 10: node1ID, node2ID, order, brnID, surf [cm2], length [cm], radius [cm], kz [cm4 hPa-1 d-1], kr [cm hPa-1 d-1], emergence time [d], subtype, organtype \n")
    for d in diameters :
      f.write(" ".join([str(e) for e in d])+"\n")
    f.write("BOUNDARYSEGMENTS\n")
    f.write("2\t0\n")
    f.write("3\t"+str(len(pointlist)-1)+"\n")
    f.write("BOUNDARYDOMAIN\ndefault\t1\n")
    f.write("#\n#\n#\n")
    f.close()
#enddef

class ParseRSML :
  def __init__(self, nodef, diaf):
    self.nodef = nodef
    self.diaf = diaf
    self.fname=""
    self.simplify = False
  
  def SetSimplifyTrue(self) :
    self.simplify = True
  def SetSimplifyFalse(self) :
    self.simplify = False
    
  def ParseFile(self) :
    rsmldoc = ET.parse(self.fname)
    rsml = rsmldoc.getroot()
    if rsml.find("metadata") != None and rsml.find("metadata").find("software") != None :
      self.addconnector = True
    else :
      self.addconnector = False
    scene = rsml.find("scene").find("plant")
    toproots = scene.findall(".//root")
    return self.IncurseRoots(scene.find('.//root'))
    #endfor
  #enddef

  def IncurseRoots(self, rootelement) :
    if rootelement == None :
      return []
    rootlist = []
    elementlist = [[rootelement,-1]]
    while len(elementlist) > 0 :
      parentelementpair = elementlist[0]
      element = parentelementpair[0]
      rn = parentelementpair[1]
      elementlist = elementlist[1:]
      parentnode_find = element.find("properties")
      parentnode_find = parentnode_find.find("parent-node")
      if parentnode_find == None :
        parentnode = -1
      else :
        parentnode = int(parentnode_find.get("value"))
      geomnode = element.find("geometry")
      pl = geomnode.find("polyline")
      functionnames = [f.get("name") \
        for f in (element.find("functions").findall("functions"))]
      functions = [[float(v.get("value")) for v in f.findall("sample")] \
        for f in (element.find("functions").findall("functions"))]
      propertynames = [ e.tag for e in element.find("properties").iterfind(".//") ]
      points = []
      params = {}
      for i,name in enumerate(functionnames) :
        params[name] = functions[i]
      pointstring = ".//Point"
      if pl.find(pointstring) == None :
        pointstring = ".//point"
      for i,ptel in enumerate(pl.findall(pointstring)) :
        point = []
        if ptel.get("X") == None :
          point = [float(ptel.get("x")),
          float(ptel.get("y")),
          float(ptel.get("z"))]
        else :
          point = [float(ptel.get("X")),
          float(ptel.get("Y")),
          float(ptel.get("Z"))]
        points.append(point)
      #endfor
      if "diameter" in propertynames :
        d = float(element.find("properties").find("diameter").get("value"))
        params["diameter"] = np.array([d for p in points])
      if self.addconnector and parentnode >= 0 :
        # add an additional node to connect the laterals with their predecessors
        parentroot = next(r for r in rootlist if r.RootNumber == int(parentelementpair[1]))
        if parentroot == None :
          print("Did not find parent root, gonna stop trying to add junction")
        else :
          try :
            parentsocket = parentroot.Nodes[parentnode]
            parentdiamet = parentroot.Functions["diameter"][parentnode]
            childsocket = points[0]
            points.insert(0,parentsocket + (childsocket-parentsocket)*parentdiamet)
            #params["diameter"].insert(0,parentdiamet)
            params["diameter"] = np.insert(params["diameter"],0,parentdiamet,axis=0)
            print("Successfully added junction")
          except :
            print("Something else went wrong when trying to add the junction")
      sr = []
      for e in element.findall('root') :
        if e.get('ID') != element.get('ID') :
          elementlist.append([e,element.get('ID')])
          sr.append(int(e.get('ID')))
      params["diameter"] = list(self.diaf(np.array(params["diameter"])))
      #points = [self.nodef(np.array(x)) for x in points]
      
      rootlist.append(Root(self.nodef(np.array(points)),[].copy(),params.copy(),int(element.get('ID')), sr.copy(),int(rn), parentnode))
    #endwhile
    if self.simplify :
      mandatorynodes = [(r.Predecessor,r.ParentNode,r.RootNumber) for r in rootlist if not r.Predecessor == -1]
      for root in rootlist :
        checkpos = np.array([-1,-1,-1])
        direction = np.array([-1,-1,-1])
        for i in range(len(root.Nodes)) :
          p = root.Nodes[i]
          d = root.Functions['diameters'][i]
          if i == 0 :
            checkpos = p
            continue
          if i == 1 :
            direction = p - checkpos
            continue
          # now actually check the shit
          # this will be pog
      print(mandatorynodes)
    return rootlist
  #enddef

  def SetFilename(self,filename) :
    self.fname = filename
  #enddef

if __name__ == "__main__" :
  print("Please start main.py")
#endif