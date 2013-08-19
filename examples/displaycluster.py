#!/usr/bin/python

# example launch script for DisplayCluster, executed by startdisplaycluster
# this should work for most cases, but can be modified for a particular
# installation if necessary

import os
import xml.etree.ElementTree as ET
import subprocess
import shlex

# get environment variable for the base DisplayCluster directory, set by
# startdisplaycluster
dcPath = None

if 'DISPLAYCLUSTER_DIR' in os.environ:
    dcPath = os.environ['DISPLAYCLUSTER_DIR']
else:
    print 'could not get DISPLAYCLUSTER_DIR!'
    exit(-3)

# get rank from appropriate MPI API environment variable
myRank = None

if 'OMPI_COMM_WORLD_RANK' in os.environ:
    myRank = int(os.environ['OMPI_COMM_WORLD_RANK'])
elif 'OMPI_MCA_ns_nds_vpid' in os.environ:
    myRank = int(os.environ['OMPI_MCA_ns_nds_vpid'])
elif 'MPIRUN_RANK' in os.environ:
    myRank = int(os.environ['MPIRUN_RANK'])
elif 'PMI_ID' in os.environ:
    myRank = int(os.environ['PMI_ID'])
elif 'PMI_RANK' in os.environ:
    myRank = int(os.environ['PMI_RANK'])
else:
    print 'could not determine MPI rank!'
    exit(-4)

# add our own lib folder
if 'LD_LIBRARY_PATH' not in os.environ:
    os.environ['LD_LIBRARY_PATH'] = dcPath + '/lib'
else:
    os.environ['LD_LIBRARY_PATH'] += os.pathsep + dcPath + '/lib'

if myRank == 0:
    # don't manipulate DISPLAY, just launch
    startCommand = dcPath + '/bin/displaycluster'
    subprocess.call(shlex.split(startCommand))
else:
    # configuration.xml gives the display
    display = None

    try:
        doc = ET.parse(dcPath + '/configuration.xml')

        elems = doc.findall('.//process')

        if len(elems) < myRank:
            print 'could not find process element for rank ' + str(myRank)
            exit(-5)

        elem = elems[myRank - 1]

        display = elem.get('display')

        if display != None:
            os.environ['DISPLAY'] = display
        else:
            os.environ['DISPLAY'] = ':0'
    except:
        print 'Error processing configuration.xml. Make sure you have created a configuration.xml and put it in ' + dcPath + '/. An example is provided in the examples/ directory.'
        exit(-6)

    startCommand = dcPath + '/bin/displaycluster'

    subprocess.call(shlex.split(startCommand))
