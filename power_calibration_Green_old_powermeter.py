# -*- coding: utf-8 -*-
"""
Stimuilus to calibrate the output power of the lightcrafter.
-full field ellipse that increments intensity in 5 unit steps (0 to 255) every
2 seconds. Runs green by default

Created on Fri Jul  8 14:25:30 2016

@author: andre maia chagas
"""


import QDS 

QDS.Initialize("power_calibration", "'G/B'")

# Define global stimulus parameters
#
minVal = 0
maxVal = 256
step = 5

nsteps = ((maxVal-minVal)/step)*2 # increase and decrease

p = {"nTrials"         : 1, 
     "dxStim_um"       : 500,   # Stimulus size
     "durFr_s"         : 1/60.0, # Frame duration
     "nFrPerMarker"    : 3,
     "RGB_green"       : (0,255,0),
     "RGB_blue"        : (255,0,0)}
QDS.LogUserParameters(p)

# Do some calculations
#
durMarker_s    = p["durFr_s"] *p["nFrPerMarker"]

# Define stimulus objects
#
QDS.DefObj_Ellipse(1, p["dxStim_um"], p["dxStim_um"])

# Start of stimulus run
#
QDS.StartScript()

for iT in range(p["nTrials"]):
    for i in range (minVal,maxVal,step):   
        p["RGB_green"] = (0,i,0)
        QDS.SetObjColor (1, [1], [p["RGB_green"]])
        QDS.Scene_Render(durMarker_s, 1, [1], [(0,0)], 1)
        QDS.Scene_Render(2.0 -durMarker_s, 1, [1], [(0,0)], 0)
        #print("green: " + str(i))
        #QDS.Scene_Clear(0.0, 0)
	   
    QDS.Scene_Clear(3.0, 0)

# Finalize stimulus
#
QDS.EndScript()

# --------------------------------------------------------------------------


