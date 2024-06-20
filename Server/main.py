import vtk
#from paraview.simple import *
#from rsmi_polyline_parse import Root
import configparser
import time
from VTKExportConnect import VTKExportConnect
from RSML import *
import numpy as np
import Broker
import os
import easygui
import math
import random
from sys import platform
from datetime import datetime
import traceback

Port = 12575
RootFolder = ""

KnownFileExtensions = ['raw','vtk']
KnownRootExtensions = ['rsml','vtp']

resolution = 0.045
spacing = [resolution,resolution,resolution]

NodeTransformer = lambda x : x #+ np.array([111.0*0.05,111.0*0.05,100.0*0.1])
DiameterTransform = lambda r : r


#temporary fix for reading the correct root system
#KnownRootExtensions = ['rsml']

def PrepareIsosurface(vtkfile) :
  reader = vtk.vtkImageReader()

def MakeTestShape() :
  points = np.array([[0,0,0],[1,0,0],[0,1,0],
            [1,1,0]])
  triangles = np.array([[0,1,2],[3,2,1]])
  normals = np.array([[0,0,1],[0,0,1]])
  return points, triangles, normals

class RootFileHandle :
  def __init__(self,folder) :
    self.folder = (folder  + "/") if folder[-1] != "/" else folder
    folderlist = os.listdir(folder)
    datasetlist = sorted([f for f in folderlist if any(ex in f for ex in KnownFileExtensions)])
    print(sorted([s.lower() for s in datasetlist]))
    datasetlist = [datasetlist[i] for i,s in (sorted(enumerate([s.lower() for s in datasetlist]),key=lambda x:x[1]))]
    print(datasetlist)
    self.datalist = []
    self.marked = -1
    i=0
    for f in datasetlist :
      print(f)
      stem,ex = f.rsplit('.',1)
      self.datalist.append({"file":f,"ext":ex})
      if not os.path.isdir(folder + "/" + stem) :
        os.mkdir(folder + "/" + stem)
      subf = [folder+"/"+stem+"/"+f for f in os.listdir(folder+"/"+stem) if any([ex in f for ex in KnownRootExtensions])]
      self.datalist[i]["roots"] = subf.copy()
      self.datalist[i]["rootfolder"] = folder + "/" + stem + "/"
      self.datalist[i]["version"] = 0
      self.datalist[i]["subf"] = folder+"/"+stem+"/"
      i += 1
  def CheckVersion(self,root) :
    return os.stat(root).st_mtime
  def MarkFile(self,id) :
    self.version = len(self.datalist[id]["roots"])
    self.marked = id
    if len(self.datalist[id]["roots"]) <= 0 :
      self.root = ""
    else :
      self.root = sorted(self.datalist[id]["roots"],key = lambda root: os.stat(root).st_mtime, reverse=True)[0]
    self.file = self.folder + "/" + self.datalist[id]["file"]
  def HasMarked(self) :
    return True
  def NextVersion(self) :
    if self.marked < 0 :
      time_string = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
      return self.folder + time_string + "_vr_roots"
    if self.version < 0 :
      print("check for making folder")
    self.version += 1
    res = self.datalist[self.marked]["subf"] + "roots_vr" + "_" + str(self.version)
    return res
  def PreviousVersion(self) :
    if self.marked < 0 :
      return ""
    res = self.datalist[self.marked]["subf"] + "roots_vr" + "_" + str(max(0,self.version-2)) + ".rsml"
    self.version -= 1
    return res
  def GetFile(self) :
    return self.file
  def IsDGF(self) :
    return "dgf" in self.root
  def IsVTP(self) :
    return "vtp" in self.root
  def GetRoot(self) :
    return self.root
  def UpdateFolder(self,id) :
    if id < 0 :
      return
    fo = self.datalist[id]["rootfolder"]
    subf = [fo+f for f in os.listdir(fo) if any([ex in f for ex in KnownRootExtensions])]
    self.datalist[id]["roots"] = subf.copy()
  def __len__(self) :
    return len(self.datalist)
  def __getitem__(self,key) :
    return self.folder + self.datalist[key]["file"] if key < len(self.datalist) else ""
  def list(self) :
    return sorted([f["file"] for f in self.datalist])



def ReadFromIniFile(filename) :
  print("Starting in background from ini")
  config = configparser.ConfigParser()
  config.read(filename)
  #folder = easygui.diropenbox()
  if all(x in config for x in ['FILES','NETWORK']) :
    print("Conifg file successfully loaded")
    filescfg = config['FILES']
    networkcfg = config['NETWORK']
    return filescfg, networkcfg
  else :
    print("wtf am i gonna do")
    return {"Error" : "ini file not complete"},{}

def MakeLambdas() :
  a = np.random.normal(0,0.1,9)
  u = lambda x: a[0]*x*x + 10*a[1]*x
  v = lambda y: a[3]*y*y + 10*a[4]*y
  w = lambda z: a[6]*z*z + 10*a[7]*z
  return [u,v,w]

def Main() :
  global spacing
  fhandle = VTKExportConnect()
  fhandle.SetSpacing(spacing)
  rhandle = ParseRSML(NodeTransformer, DiameterTransform)
  dhandle = ParseDFG()
  vhandle = ParseVTP(NodeTransformer, DiameterTransform)
  print("Main function")
  broker = Broker.Broker()
  broker.SetupServer(Port)
  if len(RootFolder) == 0 :
    dirn = easygui.diropenbox("Open a folder containing the volume data","Volume Date","~")
  else :
    dirn = RootFolder
  #dirn = 
  folder = RootFileHandle(dirn)
  a = np.random.normal(0,0.1,9)
  uvw = [MakeLambdas()]
  attaches = [-1]
  preds = [-1]
  while True :
    time.sleep(1)
    print("Checking if you wanted sth")
    print("Waiting now")
    broker.WaitForMessage()
    cmd = broker.ParseMessage()
    # WE WERE SEND A SERVER ALIVE REQUEST
    if cmd == Broker.NetworkCommandKeys['alive'] :
      print("yes we are alive")
      broker.AnswerAlive()
    # RECEIVED COMMAND FOR FILE LIST
    elif cmd == Broker.NetworkCommandKeys['files'] :
      folder = RootFileHandle(dirn)
      for f in folder.list():
        print(f)
      if not broker.SendFileList(folder.list()) :
        print("Client cannot allocate resources for the file list")
        break
      broker.Flush()
    # RECEIVED SPECIFIC FILE COMMAND
    elif cmd == Broker.NetworkCommandKeys['speci'] :
      fileid = broker.cmd_int_vars[1]
      if fileid >= len(folder) :
        zmqerror = struct.pack('II',Broker.NetworkCommandKeys['err'],len(folder)-1)
        break
      folder.MarkFile(fileid)
      print("Remote end marked fileid: ",folder.GetFile())
      print("Remote end marked fileid: ",folder.marked)
      print(folder.list())
      filename = folder.GetFile()
      fhandle.Init(filename)
      fhandle.SetIsoValue(fhandle.GetIsoRange()[0] + 0.5*(fhandle.GetIsoRange()[1]-fhandle.GetIsoRange()[0]))
      #fhandle.SetIsoValue(150)
      if not broker.paraviewavailable :
        p = fhandle.GetPoints()
        if len(p) == 0 :
          print("Points are empty")
        t = fhandle.GetTriangles()
        n = fhandle.GetNormals()
        if not all(broker.SendArrays(p,t,n)) :
          print("Could not send everything")
        else :
          print("Success")
      broker.Flush()
    # RECEIVED ROOT SYSTEM COMMAND
    # usually we receive this after the isosurface thing
    elif cmd == Broker.NetworkCommandKeys['root'] :
      print("Client askes for root system")
      rootfile = folder.GetRoot()
      if len(rootfile) == 0 :
        print("no root")
        broker.AnswerCommand(Broker.NetworkCommandKeys['rootomni'])
      else :
        rootdata = []
        try :
          if folder.IsVTP() :
            print("File is a VTP file")
            vhandle.SetFilename(rootfile)
            rootdata = vhandle.ParseFile()
          elif not folder.IsDGF() :
            print("File is not a DGF file")
            rhandle.SetFilename(rootfile)
            rootdata = rhandle.ParseFile()
          else :
            print("File IS indeed a DGF file")
            dhandle.SetFilename(rootfile)
            rootdata = dhandle.ParseFile()
        except Exception as e:
          print(traceback.format_exc(e))
          broker.AnswerCommand(Broker.NetworkCommandKeys['rootomni'])
        else :
          broker.SendRootSystem(rootdata)
      #broker.SendScalars("rootomni",'I',[0])
      #print("No no root right now sowwy")
    elif cmd == Broker.NetworkCommandKeys['random'] :
      nums = [broker.cmd_int_vars[1]]
      print("Making ",nums[0]," segments as random roots")
      testercount = 0
      maximumPointsPerRoot = 100
      while nums[testercount] > maximumPointsPerRoot :
        nums.append(nums[testercount] - maximumPointsPerRoot)
        nums[testercount] = maximumPointsPerRoot
        testercount += 1
        if testercount > len(attaches)-1 :
          attaches.append(random.randint(0,len(attaches)-1))
        if testercount > len(preds)-1 :
          preds.append(random.randint(0,maximumPointsPerRoot-1))
        if testercount > len(uvw)-1 :
          uvw.append(MakeLambdas())
      Roots = []
      for n,num in enumerate(nums) :
        joint = preds[n]
        predecessor = attaches[n]
        Points = []
        scale = 10.0
        if n == 0 :
          Points = [[uvw[n][0](i/scale),uvw[n][1](i/scale),uvw[n][2](i/scale)] for i in range(num)]
        else :
          orpoint = Roots[predecessor].Nodes[joint]
          Points = [[uvw[n][0](i/scale)+orpoint[0],uvw[n][1](i/scale)+orpoint[1],uvw[n][2](i/scale)+orpoint[2]] for i in range(num)]
        Diameters = [random.gauss(0.2,0.02) for i in range(num)]
        Roots.append(Root(np.array(Points),[].copy(),{"diameter":np.array(Diameters)}.copy(),RootNumber=n,Pred=predecessor,PaNo=joint))
      broker.SendRootSystem(Roots)
    elif cmd == Broker.NetworkCommandKeys['chiso'] :
      print("received iso request")
      isovalue = broker.cmd_flt_vars[0]
      fhandle.SetIsoValue(isovalue)
      if not broker.paraviewavailable :
        p = fhandle.GetPoints()
        t = fhandle.GetTriangles()
        n = fhandle.GetNormals()
        if not all(broker.SendArrays(p,t,n)) :
          print("Could not send everything")
        else :
          print("Success")
      broker.Flush()
    elif cmd == Broker.NetworkCommandKeys['isorange'] :
      print("Received a request for the range")
      isorange = fhandle.ComputeIsoRangeByHistogram(32)
      isorange = (isorange[0], isorange[0] + 0.7*(isorange[1]-isorange[0]))
      print("Histogram isorange is now: ",isorange)
      broker.SendScalars('isorange','f',isorange)
      print("Got")
    elif cmd == Broker.NetworkCommandKeys['save'] :
      num = broker.cmd_int_vars[1]
      roots = [Root() for i in range(num)]
      roots = []
      for i in range(num) :
        r,p,a,n,d = broker.ReceiveRoot()
        if len(n) > 0 :
          roots.append(Root(n.copy(),[].copy(),{"diameter":d.copy()}.copy(),r,[].copy(),a,p))
        #roots[i].RootNumber = r
        #roots[i].ParentNode = p
        #roots[i].Predecessor = a
        #roots[i].Nodes = n.copy()
        #roots[i].Functions = {"diameter":d.copy()}.copy()
      print([root.RootNumber for root in roots])
      broker.AnswerAlive()
      vers = folder.NextVersion()
      try:
        MakeXMLFromRootlist(roots,vers+".rsml",{}, False)
      except Exception as e:
        print(traceback.format_exc(e))
      time.sleep(1)
      try:
        MakeDGFFromRootList(vers+".dgf",roots,True)
      except Exception as e:
        print(traceback.format_exc(e))
      folder.UpdateFolder(folder.marked)
      print("Got")
    elif cmd == Broker.NetworkCommandKeys['topology'] :
      print("Received command to get topology")
      num = broker.cmd_int_vars[1]
      #roots = [Root() for i in range(num)]
      roots = []
      for i in range(num) :
        r,p,a,n,d = broker.ReceiveRoot()
        if len(n) > 0 :
          roots.append(Root(n.copy(),[].copy(),{"diameter":d.copy()}.copy(),r,[].copy(),a,p))
      broker.AnswerAlive()
      print("Got root, waiting for command which root I should make taproot")
      broker.WaitForMessage()
      cmd = broker.ParseMessage()
      taproot = broker.cmd_int_vars[1]
      print("Have to make number ",taproot, " to be the main root")
      roots2 = sorted(RedrawMainRoot(roots,taproot),key=lambda r:r.RootNumber)
      MakeRootComparison(roots,roots2)
      broker.SendRootSystem(roots2)
    elif cmd == Broker.NetworkCommandKeys['undo'] :
      print("Client asked for the undo operation!")
      rootfile = folder.PreviousVersion()
      rootdata = []
      try :
        if not folder.IsDGF() :
          print("File is not a DGF file")
          rhandle.SetFilename(rootfile)
          rootdata = rhandle.ParseFile()
        else :
          print("File IS indeed a DGF file")
          dhandle.SetFilename(rootfile)
          rootdata = dhandle.ParseFile()
      except Exception as e:
        print(e)
        broker.AnswerCommand(Broker.NetworkCommandKeys['rootomni'])
      else :
        broker.SendRootSystem(rootdata)
    elif cmd == Broker.NetworkCommandKeys['err'] :
      print("Got")
    elif cmd == Broker.NetworkCommandKeys['undo'] :
      print("Received the request to undo the last action")
    else :
      print("Unknown Command")
      print(cmd)
        #surface = PrepareIsosurface(folderlist[fileid])
        # we expect a triangulated mesh here, that has normals and tangents
        # what we generate ourselves is only the
    broker.Flush()

if __name__ == "__main__" :
  """
  filecfg,networkcfg = ReadFromIniFile("rootmodellingconfig.ini")
  print(filecfg)
  print(networkcfg)
  if "Error" in filecfg :
    print("USAGE -----------------------------------------------------------")
    print("Necessary parameters:                                            ")
    print("                     Filename                                    ")
    print("This ini file must include a FILES and a NETWORK section         ")
    print("Otherwise this program will fail                                 ")
    print("                                                                 ")
    print("     *--------------------------*                                ")
    print("     |                          |                                ")
    print("     |           BYE            |                                ")
    print("     |                          |                                ")
    print("     *--------------------------*                                ")
    print("                                                                 ")
    print("                                                                 ")
  else :
  """
  print("Loading folder ")
  #MainLoop(filecfg,networkcfg)
  Main()

    
