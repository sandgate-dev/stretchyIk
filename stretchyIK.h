//
//  stretchyIK.h
//  stretchyIK
//
//  Details: needs discription
//  Example: (MEL) createNode stretchyIK
//
//  Created by Stephan Osterburg on 12/23/17
//

#ifndef stretchyIK_h
#define stretchyIK_h

#include <maya/MTypeId.h>
#include <maya/MPxNode.h>

class stretchyIK : public MPxNode
{
public:
    static void*        creator();
    stretchyIK(); // constructor
    static MStatus      initialize();
    virtual MStatus     compute(const MPlug& plug, MDataBlock& data);
    
public:
    // Node ID
    static MTypeId      typeId;
    
    // start reference matrix (world space) - ik root
    static MObject      startMatrix;
    
    // end reference matrix (world space) - ik end effector
    static MObject      endMatrix;
    
    // first bone initial length (eg uparm)
    static MObject      upInitLength;
    
    // second bone initial length (eg loarm)
    static MObject      loInitLength;
    
    // stretch value
    static MObject      stretch;
    
    // slide value (knee)
    static MObject      slide;
    
    // global scale input
    static MObject      globalScale;
    
    // pole vector matrix
    static MObject      poleVectorMatrix;
    
    // pole vector lock value
    static MObject      poleVectorLock;
    
    // first bone output scale
    static MObject      outputUpScale;
    
    // second bone output scale
    static MObject      outputLoScale;
};


#endif /* stretchyIK_h */
