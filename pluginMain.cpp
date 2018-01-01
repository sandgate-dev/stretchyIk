//
//  pluginMain.cpp
//  stretchyIK
//
//  Created by Stephan Osterburg on 12/23/17
//
//

#include <stretchyIk.h>

#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{
    MStatus status;
    
    MFnPlugin fnPlugin(obj, "Stephan Osterburg", "0.1", "Any");
    
    status = fnPlugin.registerNode("stretchyIK",
                                 stretchyIK::typeId,
                                 stretchyIK::creator,
                                 stretchyIK::initialize,
                                 MPxNode::kDependNode);
    
    if (status != MS::kSuccess) {
        status.perror("Could not register the stretchyIK node");
    }
    
    return status;
}

MStatus uninitializePlugin(MObject object)
{
    MFnPlugin pluginFn;
    pluginFn.deregisterNode(MTypeId(0x80015));
    
    return MS::kSuccess;
}
