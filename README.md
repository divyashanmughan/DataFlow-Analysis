# DataFlow-Analysis
An LLVM pass which designes a general parameterized iterative data-flow analysis framework, which can be used for Liveness Analysis and Reaching Definition Analysis.

The "dataFlowAnalysis.h" file consists of the base template class, from which live analysis and reaching
definition analysis passes  derive.This file consists of DataFlowAnalysis class which has a data member
"direction" of type bool, which is used to identify whether it is forward or backward analysis.It is a templated
class, and contains a map called inSet , outSet , phiSet, of basicblock and domainValue.Here the
domainValue(template)will be BitVector.It consists of forwardAnalysis method for forward flow
analysis.This will call virtual functions like meetFunc , for calculating meet operation and virtual function
called transferFunction.It also contains backwardAnalysis function for doing backward flow analysis , which
also calls virtual functions like meetFunc and transferFunction.

 The “Liveness analysis pass” uses a Live class which derives from DataFlowAnalysis class and
FunctionPass.It contains a boundaryCondition method which will set the boundary values to false.The
meetFunc methos will do the union of in[Block] to get out[block].The transferFunction method is used for
calculating the transfer function.

 The “Reaching  definition pass” uses a Reach class which derives from DataFlowAnalysis class and
FunctionPass.It contains a boundaryCondition method which will set the boundary values arguments as true
and rest of them to false.The meetFunc method will do the union of out[Block] to get in[block].The
transferFunction method is used for calculating the transfer function.

The Liveness Analysis class is registered,using command line argument “live”, and a name “Liveness
Analysis Pass.”. The Reaching Definition class is registered,using command line argument “reach”, and a
name “Reaching Definition Analysis Pass.”.

To run the script , use    ./run.sh filename
Eq.)
small.c file can be run by
./run.sh small
