from queue import Queue
import zmq
import struct
import time
import numpy as np

false=False
true=True

AcceptedCommandList = {
    "run"       : "Default command for starting up and has no real meaning",
    "alive"     : "UE>PY/PY>PV check if alive for connection",
    "files"     : "request file list or send file list to pv",
    "speci"     : "request special file or order special file",
    "iso"       : "send isosurface to unreal or to paraview",
    "root"      : "root system request or command",
    "isoomni"   : "indicate that isosurface will be sent through omniverse",
    "rootomni"  : "indicate that root system will arrive through omniverse",
    "isodat"    : "indicate that isosurface will be sent through zeromq",
    "rootdat"   : "indicate that isosurface will be sent through zeromq",
    "chiso"     : "a receivable request indicating a change in isosurface",
    "isorange"  : "request (or response to) for iso surface range",
    "save"      : "warning from unreal to be able to save work, with size",
    "saveok"    : "From us that we have allocated",
    "savedat"   : "Getting the data from unreal",
    "savethx"   : "from us that we have finished",
    "random"    : "indicates the wish to render a random root with n segments",
    "topology"  : "request for a topology change from the unreal client",
    "undo"      : "request for last root system to be loaded",
  }

NetworkCommandKeys = {
    "run"       : 0b_10_00_00_00, # -> check
    "alive"     : 0b_00_00_00_01, # -> check
    "files"     : 0b_00_00_00_10, # -> check
    "speci"     : 0b_00_00_00_11, # -> today
    "iso"       : 0b_00_00_01_00, # -> check
    "root"      : 0b_00_00_01_01, # -> check
    "isoomni"   : 0b_00_00_01_10, # <- check ok only
    "rootomni"  : 0b_00_00_01_10, # <- check ok only
    "isodat"    : 0b_00_00_01_00, # <- check dat
    "rootdat"   : 0b_00_00_01_01, # <- today
    "chiso"     : 0b_00_00_10_00, # <- today (half check for isodat above)
    "isorange"  : 0b_00_10_00_00, # <- today
    "save"      : 0b_00_01_00_00, # -> today
    "saveok"    : 0b_00_01_00_01, # <- today
    "savedat"   : 0b_00_01_00_10, # -> today
    "savethx"   : 0b_00_01_01_00, # <- today
    "err"       : 0b_11_11_11_11, # <-> lolwhat
    "random"    : 0b_00_10_10_00, # today
    "topology"  : 0b_00_11_10_00, # today
    "undo"      : 0b_00_11_10_01,
  }

class Command :
  def __init__(self, command = "", data = []) :
    self.command_ = command
    self.data_ = data
  #init

  def GetCommand() :
    return self.command
  #GetCommand

  def GetData() :
    return self.data, len(self.data)
  #GetData

  def ToString() :
    retstr = "Command " + self.command + "\n"
    for i in min(10,len(dat)) :
      retstr += "Datum " + str(i) + " is " + str(data_[i])
    return retstr
  #tostring

#command

class Broker(object):
  """
    Broker
    @description Broker for zeromq communication with paraview and such
    @param inq input queue that we use for commands
    @param outq output queue that we use for dispatch
  """
  def __init__(self) :
    self.context = zmq.Context()
    self.paraviewavailable = false
  #__init__

  def SetupServer(self, port : int) :
    print("Setting up Server " + "tcp://*:"+str(port))
    self.serversocket = self.context.socket(zmq.REP)
    self.serversocket.bind("tcp://*:"+str(port))
  #

  def SendArrays(self, points, triangles, normals) :
    print("Preparing sending of points")
    warning = struct.pack('IIII',NetworkCommandKeys['isodat'], \
      np.array(points.shape).prod(),\
      np.array(triangles.shape).prod(),\
      np.array(normals.shape).prod())
    self.serversocket.send(warning)
    print("Sent the warning")
    sentpoints = False
    senttris = False
    sentnorm = False
    okayreq = self.serversocket.recv()
    replycmd = struct.unpack('II',okayreq)[0]
    if replycmd == NetworkCommandKeys['alive'] :
      print("Alive, testing for point sending")
      self.serversocket.send(points.astype('f').tostring())
      okayreq = self.serversocket.recv()
      replycmd = struct.unpack('II',okayreq)[0]
      if replycmd == NetworkCommandKeys['alive'] :
        sentpoints = True
    if replycmd == NetworkCommandKeys['alive'] :
      print("Alive, testing for triangle sending")
      self.serversocket.send(triangles.astype('I').tostring())
      okayreq = self.serversocket.recv()
      replycmd = struct.unpack('II',okayreq)[0]
      if replycmd == NetworkCommandKeys['alive'] :
        senttris = True
    if replycmd == NetworkCommandKeys['alive'] :
      print("Alive, testing for normal sending")
      self.serversocket.send(normals.astype('f').tostring())
      #okayreq = self.serversocket.recv()
      #replycmd = struct.unpack('II',okayreq)[0]
      #if replycmd == NetworkCommandKeys['alive'] :
      #  sentnorm = True
    sentnorm = True
    #self.serversocket.send(struct.pack('II',NetworkCommandKeys['alive'],0))
    #endif
    return sentpoints, senttris, sentnorm
  #enddef

  def WaitForMessage(self) :
    self.message = self.serversocket.recv()
    print(self.message)
  #waitformessage

  def ParseMessage(self) :
    cmdcheck = self.message[0:5]
    print(cmdcheck[0])
    print(cmdcheck)
    self.cmd_int_vars = struct.unpack('II',self.message)
    print(self.cmd_int_vars)
    self.cmd_flt_vars = struct.unpack('<f',self.message[4:8])
    print(self.cmd_flt_vars)
    return cmdcheck[0]
  #

  def Flush(self) :
    self.message = ""
  #flush

  def SendFileList(self,filelist) :
    liststring = "/".join(filelist).encode('ascii','replace')
    nextlength = len(liststring)
    warning = struct.pack('II',NetworkCommandKeys['files'],nextlength)
    print(liststring)
    self.serversocket.send(warning)
    okayreq = self.serversocket.recv()
    replycmd = struct.unpack('II',okayreq)[0]
    if replycmd == NetworkCommandKeys['alive'] :
      self.serversocket.send(liststring)
      return True
    else :
      return False
  #sendfilelist

  def SendRootSystem(self, roots) :
    print("Sending root system of " + str(len(roots)) + " individual roots")
    pointlistlengths = np.array([len(root.Nodes) for root in roots])
    bufferwarning = pointlistlengths.max()*4+4
    warning = struct.pack('III',NetworkCommandKeys["rootdat"],len(roots),bufferwarning)
    print("Warning: " + "rootdat" + " " + str(len(roots)) + " " + str(bufferwarning))
    print(warning)
    self.serversocket.send(warning)
    message = self.serversocket.recv()
    reply = struct.unpack('II',message)
    print("Sending root system")
    if reply[0] == NetworkCommandKeys['alive'] :
      for i,root in enumerate(roots):
        #print("sending root: " + str(i+1) + "/" + str(len(roots)) + " with " + str(len(root.Nodes)))
        meta = struct.pack('IiIi',root.RootNumber,int(root.Predecessor),len(root.Nodes),root.ParentNode)
        print(meta)
        meta += (np.array(root.Nodes)).astype(np.float32).tostring()
        #for point in root.Nodes :
        #  meta += struct.pack('fff',point[0],point[1],point[2])
        #print((np.array(root.Functions['diameter'])).min())
        meta += (np.array(root.Functions['diameter']).astype(np.float32)).tostring()
        #for dia in root.Functions['diameter'] :
        #  meta += struct.pack('f',dia)
        self.serversocket.send(meta)
        if i >= len(roots)-1:
          print("I sent the last root and will close the transmission")
          # if were at the last root we should stop otherwise we 
          # arrive at an error state because zmq requires the server
          # to respond to every request which we don't want
          break
        m = self.serversocket.recv()
        reply = struct.unpack('II',m)
        if reply[0] != NetworkCommandKeys['alive'] :
          print("Did not respond with alive in between root sends")
          break
    return

  def AnswerAlive(self) :
    outm = struct.pack('BI',NetworkCommandKeys['alive'],1)
    #print(outm)
    self.serversocket.send(outm)
    self.message = ""
  #enddef

  def ReceiveRoot(self) :
    self.serversocket.send(struct.pack('II',NetworkCommandKeys['alive'],0))
    mroot = self.serversocket.recv()
    print(len(mroot))
    # i am mroot
    if len(mroot) < 16 :
      return -1,-1,-1,[],[]
    rootnumber =  struct.unpack('I',mroot[0:4])[0]
    predecessor = struct.unpack('i',mroot[4:8])[0]
    numpoints = struct.unpack('I',mroot[8:12])[0]
    parentnode = struct.unpack('i',mroot[12:16])[0]
    print(rootnumber, " ", predecessor, " ", numpoints, " ", parentnode)
    print(16+(4*4*numpoints))
    points = []
    for i in range(numpoints) :
      bwstart = 16 + (4*3*i)
      #print(struct.unpack('f',mroot[bwstart:bwstart+4]))
      #print(struct.unpack('f',mroot[bwstart+4:bwstart+8]))
      #print(struct.unpack('f',mroot[bwstart+8:bwstart+12]))
      #point = np.array(struct.unpack('f',mroot[bwstart:bwstart+4]),
      #  struct.unpack('f',mroot[bwstart+4:bwstart+8]),
      #  struct.unpack('f',mroot[bwstart+8:bwstart+12]))
      #point = np.array(struct.unpack('fff',mroot[bwstart:bwstart+12]))
      point = np.frombuffer(mroot[bwstart:bwstart+12],count=3,dtype='f')
      points.append(point)
    diameters = np.frombuffer(mroot[16+(4*3*numpoints):],dtype='f',count=numpoints)
    return rootnumber,parentnode,predecessor,points,diameters
  
  def SendScalars(self,cmd,type,*data) :
    datastring = np.array(list(data)).astype(type).tostring()
    outmessage = struct.pack('I',NetworkCommandKeys[cmd])
    self.serversocket.send(outmessage + datastring)
  
  def SendScalars(self,cmd,type,data : list) :
    datastring = np.array(data).astype(type).tostring()
    outmessage = struct.pack('I',NetworkCommandKeys[cmd])
    self.serversocket.send(outmessage + datastring)

  def CheckForParaview(self, port : int) :
    # todo discover paraview instead of trying to do this thing
    self.parasocket = self.context.socket(zmq.REQ)
    self.parasocket.connect("tcp://localhost:"+str(port))
  def AnswerCommand(self, cmd) :
    self.serversocket.send(struct.pack('II',cmd,0))
  def __del__(self) :
    print("shutting down")
  #del
#Broker
