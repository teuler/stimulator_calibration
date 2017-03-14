# -*- coding: utf-8 -*-
# ---------------------------------------------------------------------
# Created on Tue Mar 15 23:26:30 2016
# @author: Thomas
#
# ---------------------------------------------------------------------
__author__ = "code@eulerlab.de"

import serial
import time
import numpy
import sys

# ---------------------------------------------------------------------
#
# ---------------------------------------------------------------------
class SimpleAIRecorder:
  def __init__(self):
    # Initializing
    #
    self.port = ""
    self.baud = 115200
    self.isOpen = False
    self.nTest = 5
    self.rate_ms = -1

  # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  def openLink(self, portName, baud=115200, msgChs="#,;"):
    """
    Opens a link to the Arduino at the given port.
      portname    := name of COM port
      baud        := speed of COM port in baud
      msgChs      := message separators: 
                     [0]=start, [1]=separator, [2]=end 
      
    Returns an error code (see below)
    """
    if self.isOpen:
      print("Link is already open, close first")
      return -1
      
    self.port = portName
    self.baud = baud
    self.isOpen = False
    self.startCh = msgChs[0]
    self.sepCh = msgChs[1]
    self.endCh = msgChs[2]
  
    try:
      # Open serial port
      #
      self.SerClient = serial.Serial(self.port, self.baud,
                                     parity=serial.PARITY_NONE,
                                     stopbits=serial.STOPBITS_ONE,
                                     bytesize=serial.EIGHTBITS,
                                     timeout=1.0,
                                     writeTimeout=None)
      time.sleep(1.0)
      self.SerClient.flushInput()
      self.SerClient.flushOutput()
      print("Opened serial port {0} at {1} baud".format(self.port, self.baud))
      self.isOpen = True
      
      # Determine sampling rate
      #
      nLines = 0
      self.rate_ms = 0
      for i in range(self.nTest):
        (errC, parts) = self.__readLine()
        if errC == 0:
          nLines += 1
          self.rate_ms += float(parts[1]) /1000.0
      if nLines == self.nTest   :
        self.rate_ms /= nLines
        print("Sampling rate is {0:.3f} ms".format(self.rate_ms))        
      else:
        self.rate_ms = -1
        print("ERROR: Sampling rate could not be determined")        

    except serial.SerialException as e:
      print("ERROR: Could not open {0} ({1})".format(self.port, e.strerror))
      return -2            

    
  # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  def __readLine(self):
    # INTERNAL: Read a line from the opened COM port
    #
    if not(self.isOpen):  
      print("ERROR: Link not open")
      return (-1, [])
      
    else:  
      try:
        sData = self.SerClient.readline().decode()

      except OSError:
        print("ERROR: Error reading data from link")
        return (-2, [])

      #  Check line for correct format
      #
      if (sData[0] != self.startCh) or (sData[-3] != self.endCh):
        print("ERROR: Format of message unknown (`{0}`)".format(sData))
        return (-3, [])

      else:
        # Decode data
        #
        sData = sData[1:-3]
        parts = sData.rsplit(self.sepCh)
        
        return (0, parts)


  # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  def readData(self, duration_s, toV=True, maxAI=1024, maxV=5.0,
               updateFunc=None, nPntsUpdate=1, trigger=[]):
    """
    Reads data points from the open link.
      duration_s  := duration of recordings in seconds
      toV         := if True, then the analog channel data will be 
                     converted to [V] 
      maxAI       := bit depth of AI converter, usually 10 bit
      maxV        := maximal voltage, usually the voltage of the Arduino
      updateFunc  := if given, then it will be called frequently, passing
                     the current values for time, AI0 and AI1
      nPntsUpdate := rate for _updateFunc calls
      trigger     := list with parameters for trigger detection:
                     [0]=trigger level in [V], 
                     [1]=trigger channel (0 or 1),
                     [2]=number of triggers to record
               
    Returns a tuple of four elements: an error code (see below) and 
    three numpy arrays, containing the time (in [s]) and the two analog-in
    channels (in [au] or [V], see above) 
    """
    isDone = False
    nLines = 0
    empty = numpy.array([])
    
    if self.rate_ms >= 0:
      nPnts = round(duration_s *1000.0 /self.rate_ms)
      print("{0:.3f} s duration = {1} samples".format(duration_s, nPnts))
    else:  
      nPnts = 100
      print("Sample rate invalid, {0} samples will be recorded".format(nPnts))

    if not(self.isOpen):  
      print("ERROR: Link not open")
      return (-1, empty, empty, empty)
      
    # Create empty arrays for the data
    #
    np_data_0 = numpy.zeros([nPnts], dtype=float)
    np_data_1 = numpy.zeros([nPnts], dtype=float)
    np_data_t = numpy.zeros([nPnts], dtype=float)
    np_dt_ms  = numpy.zeros([nPnts], dtype=float)
     
    # Attempt to read data
    #
    while not(isDone):
      # Read a line
      #
      (errC, parts) = self.__readLine()
      if not(errC == 0):
        return (errC, empty, empty, empty)
        
      else:  
        np_data_0[nLines] = float(parts[2])
        np_data_1[nLines] = float(parts[3])
        np_data_t[nLines] = float(parts[0])/1000.0
        np_dt_ms[nLines] = float(parts[1]) /1000.0
        """
        print("{0:.3f} {1} {2}".format(int(parts[0])/1000.0, 
                                       int(parts[2]), int(parts[3])))
        """
        # Update plot of data, if requested
        #
        if updateFunc and ((nLines % nPntsUpdate) == 0):
          updateFunc(nLines, np_data_t[nLines], np_data_0[nLines], 
                      np_data_1[nLines])
          
        if nLines < (nPnts-1):
          sys.stdout.write("\r{0:.0f}% {1:.3f} s: {2:.3f} {3:.3f} ..."
                           .format(nLines/float(nPnts) *100, 
                                   np_data_t[nLines]/1000.0,
                                   np_data_0[nLines], np_data_1[nLines]))
        else:    
          sys.stdout.write("\r100% done" +" "*40 +"\n")
        
        nLines += 1
        isDone = (nLines == nPnts)
        
    print("SUCCESS")    
    print("{0} data points recorded".format(nLines))
    print("Rate = {0:.3f} +/- {1:.3f} ms".format(numpy.mean(np_dt_ms), 
                                                 numpy.std(np_dt_ms)))

    if toV:    
      np_data_0 = np_data_0 /float(maxAI -1) *maxV
      np_data_1 = np_data_1 /float(maxAI -1) *maxV
    
    return (0, np_data_t, np_data_0, np_data_1)


  # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  def closeLink(self):
    """
    Close link to COM port
    """
    if self.isOpen:
      self.SerClient.close()
      print("Closed serial port {0}".format(self.port))

# ---------------------------------------------------------------------
# Supporting functions
# ---------------------------------------------------------------------
def findTriggers(np_data_t, np_data, triggerLev):
  """
  Finds the triggers in the data.
    np_data_t   := time in [s]
    np_data     := trigger level
  
  Returns a tuple, with the first element being a list of trigger start
  times in [s] and the second element a list of indices to data points in
  the arrays.
  """
  inTrigger = False    
  triggers_s = []
  triggers_i = []
   
  for i in range(len(np_data_t)):
    if np_data[i] > triggerLev:
      if not(inTrigger):
          inTrigger = True
          triggers_s.append(np_data_t[i])
          triggers_i.append(i)
    else:
      inTrigger = False
      
  return (triggers_s, triggers_i)

# ---------------------------------------------------------------------
def averageTrialsByTriggers(trigger_indices, np_data):
  """
  Averages the trials indicated by the triggers.
    trigger_indices := trigger starting points as indices in
                       np_data
    np_data         := data containing the trials (in sequence)
  
  Returns list of averages
  """
  trialLen = trigger_indices[1] -trigger_indices[0] -1
  data_avg = []   
  data_std = []   

  for i in trigger_indices:
    data_avg.append(numpy.average(np_data[i+1:i+trialLen-1]))           
    data_std.append(numpy.std(np_data[i+1:i+trialLen-1]))           
    
  return (data_avg, data_std)

# ---------------------------------------------------------------------
