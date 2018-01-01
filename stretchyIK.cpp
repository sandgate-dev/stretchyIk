//
//  stretchyIK.cpp
//  stretchyIK
//
//  Created by Stephan Osterburg on 12/23/17
//
//
#include <stretchyIK.h>

#include <maya/MGlobal.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFloatVector.h>
#include <maya/MDataHandle.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <math.h>

MTypeId stretchyIK::typeId( 0x80015 );

// Declare all attributes
MObject stretchyIK::startMatrix;
MObject stretchyIK::endMatrix;
MObject stretchyIK::upInitLength;
MObject stretchyIK::loInitLength;
MObject stretchyIK::stretch;
MObject stretchyIK::slide;
MObject stretchyIK::globalScale;
MObject stretchyIK::poleVectorMatrix;
MObject stretchyIK::poleVectorLock;
MObject stretchyIK::outputUpScale;
MObject stretchyIK::outputLoScale;

// Class constructor (empty)
stretchyIK::stretchyIK()
{
    
}

// Class (override - create an instance)
void* stretchyIK::creator()
{
    return new stretchyIK();
}

// initial node creation
MStatus stretchyIK::initialize()
{
    // Declare all the needed attributes
    MFnMatrixAttribute      matrixFn;
    MFnNumericAttribute     numFn;
    
    // Input Matrixes
    startMatrix = matrixFn.create("startMatrix", "smat");
    matrixFn.setKeyable(true);
    matrixFn.setWritable(true);
    matrixFn.setStorable(true);
    addAttribute(startMatrix);
    
    endMatrix = matrixFn.create("endMatrix", "emat");
    matrixFn.setKeyable(true);
    matrixFn.setWritable(true);
    matrixFn.setStorable(true);
    addAttribute(endMatrix);
    
    poleVectorMatrix = matrixFn.create("poleVectorMatrix", "pvm");
    matrixFn.setKeyable(true);
    matrixFn.setWritable(true);
    matrixFn.setStorable(true);
    addAttribute(poleVectorMatrix);
    
    // Input
    upInitLength = numFn.create("upInitLength", "uil", MFnNumericData::kDouble, 0);
    numFn.setKeyable(true);
    numFn.setStorable(true);
    numFn.setMin(0.001);
    addAttribute(upInitLength);
    
    loInitLength = numFn.create("loInitLength", "lil", MFnNumericData::kDouble, 0);
    numFn.setKeyable(true);
    numFn.setStorable(true);
    numFn.setMin(0.001);
    addAttribute(loInitLength);

    globalScale = numFn.create("globalScale", "gsc", MFnNumericData::kDouble, 0);
    numFn.setKeyable(true);
    numFn.setStorable(true);
    numFn.setMin(0.001);
    addAttribute(globalScale);
    
    poleVectorLock = numFn.create("poleVectorLock", "pvlk", MFnNumericData::kDouble, 0);
    numFn.setKeyable(true);
    numFn.setStorable(true);
    numFn.setMin(0.0);
    numFn.setMax(1.0);
    addAttribute(poleVectorLock);
    
    stretch = numFn.create("stretch", "stch", MFnNumericData::kDouble, 0);
    numFn.setKeyable(true);
    numFn.setStorable(true);
    numFn.setMin(0.0);
    numFn.setMax(1.0);
    addAttribute(stretch);
    
    slide = numFn.create("slide", "sld", MFnNumericData::kDouble, 0);
    numFn.setKeyable(true);
    numFn.setStorable(true);
    numFn.setMin(-1.0);
    numFn.setMax(1.0);
    addAttribute(slide);
    
    // Output
    outputUpScale = numFn.create("outputUpScale", "ouscl", MFnNumericData::kDouble, 1);
    numFn.setKeyable(false);
    numFn.setStorable(false);
    numFn.setWritable(false);
    addAttribute(outputUpScale);
    
    outputLoScale = numFn.create("outputLoScale", "olscl", MFnNumericData::kDouble, 1);
    numFn.setKeyable(false);
    numFn.setStorable(false);
    numFn.setWritable(false);
    addAttribute(outputLoScale);
    
    // Link attrs
    attributeAffects(stretch, outputUpScale);
    attributeAffects(slide, outputUpScale);
    attributeAffects(startMatrix, outputUpScale);
    attributeAffects(endMatrix, outputUpScale);
    attributeAffects(globalScale, outputUpScale);
    attributeAffects(poleVectorMatrix, outputUpScale);
    attributeAffects(poleVectorLock, outputUpScale);
    attributeAffects(upInitLength, outputUpScale);
    
    attributeAffects(stretch, outputLoScale);
    attributeAffects(slide, outputLoScale);
    attributeAffects(startMatrix, outputLoScale);
    attributeAffects(endMatrix, outputLoScale);
    attributeAffects(globalScale, outputLoScale);
    attributeAffects(poleVectorMatrix, outputLoScale);
    attributeAffects(poleVectorLock, outputLoScale);
    attributeAffects(loInitLength, outputLoScale);
    
    // Return
    return MS::kSuccess;
}

MStatus stretchyIK::compute(const MPlug& plug, MDataBlock& dataBlock)
{
    // Make sure to trigger only when needed
    if ((plug == outputUpScale) || (plug == outputLoScale))
    {
        // Get input matrix's
        MMatrix startMatVal   = dataBlock.inputValue(startMatrix).asMatrix();
        MMatrix endMatVal     = dataBlock.inputValue(endMatrix).asMatrix();
        MMatrix poleVecMatVal = dataBlock.inputValue(poleVectorMatrix).asMatrix();
        
        // Get initial input length
        double upInitLengthVal = dataBlock.inputValue(upInitLength).asDouble();
        double loInitLengthVal = dataBlock.inputValue(loInitLength).asDouble();
        
        // Get input parameters
        double stretchVal = dataBlock.inputValue(stretch).asDouble();
        double slideVal   = dataBlock.inputValue(slide).asDouble();
        double lockVal    = dataBlock.inputValue(poleVectorLock).asDouble();
        double gScaleVal  = dataBlock.inputValue(globalScale).asDouble();
        
        // compute initial total chain length
        double chainLength = upInitLengthVal + loInitLengthVal;
        //chainLength = chainLength * gScaleVal;
        
        // compute the bone vector by extracting the translate values
        MVector startVec(startMatVal[3][0], startMatVal[3][1], startMatVal[3][2]);
        MVector endVec(endMatVal[3][0], endMatVal[3][1], endMatVal[3][2]);
        
        // compute length
        MVector curVecLength = endVec - startVec;
        double curLength = curVecLength.length() / gScaleVal;
        
        // initialize output var
        double upScaleOut = upInitLengthVal;
        double loScaleOut = loInitLengthVal;
        
        double delta, ratio;
        
        if (stretchVal > 0.001)
        {
            // org ratio
            delta = curLength / chainLength;
            if (delta > 1)
            {
                // if org length = 1
                // new length = 1.1
                // 1.1 - 1 = 0.1 * 0.5 (stretchVal) = 0.05 + 1 = 1.05
                delta = ((delta - 1) * stretchVal) + 1;
            } else {
                delta = 1;
            }
            
            upScaleOut = upScaleOut * delta;
            loScaleOut = loScaleOut * delta;
        }
        
        // compute slide
        if (slideVal >= 0) {
            ratio = chainLength / (upInitLengthVal * gScaleVal);
            delta = (ratio - 1) * slideVal;
            upScaleOut = upScaleOut * (delta + 1);
            loScaleOut = loScaleOut * (1 - slideVal);
        } else {
            ratio = chainLength / (loInitLengthVal * gScaleVal);
            delta = (ratio - 1) * -slideVal;
            upScaleOut = upScaleOut * (delta + 1);
            loScaleOut = loScaleOut * (1 + slideVal);
        }
        
        // compute elbow/knee lock
        if (lockVal > 0.001) {
            MVector polePos(poleVecMatVal[3][0], poleVecMatVal[3][1], poleVecMatVal[3][2]);
            
            // compute the length of the vector
            MVector startPole = polePos - startVec;
            MVector endPole   = polePos - endVec;
            double startPoleLength = startPole.length() * gScaleVal;
            double endPoleLength   = endPole.length() * gScaleVal;
            
            // linear interpolation
            upScaleOut = (upScaleOut * (1 - lockVal)) + (startPoleLength * lockVal);
            loScaleOut = (loScaleOut * (1 - lockVal)) + (endPoleLength * lockVal);
        }
        
        // set output values
        dataBlock.outputValue(outputLoScale).set(loScaleOut);
        dataBlock.outputValue(outputLoScale).setClean();
        
        dataBlock.outputValue(outputUpScale).set(upScaleOut);
        dataBlock.outputValue(outputUpScale).setClean();
    }
    
    return MS::kSuccess;
}













