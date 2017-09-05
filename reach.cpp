
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/IR/ValueMap.h"

#include "llvm/ADT/DenseMap.h"
#include "reach.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"

#include "dataFlowAnalysis.h"
#include "DataFlowAnnotator.h"

#include <ostream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <vector>
using namespace llvm;
using namespace example;
using namespace reachAnal;

/* set forward false in the constructor for liveness */
Reach::Reach() : DataFlowAnalysis<BitVector>(true), FunctionPass(ID) {
	setOfValues = new std::vector<Value*>();
	indexOfValue = new ValueMap<Value*, int>();
	inInstructionSet = new ValueMap<const Instruction*, BitVector*>();
}

void Reach::boundaryCondition(BitVector *boundaryValues) {
	*boundaryValues = BitVector(Reach::sized, false);
	for(int i=0; i<totalArg; i++) (*boundaryValues)[i] = true; //set true for arguments
}


void Reach::meetFunc(BitVector* input, const BitVector* newOutput){
	*input |= *newOutput;
}

BitVector* Reach::initDomain(BasicBlock& b){
	return new BitVector(Reach::sized, false);
}


BitVector* Reach::transferFunction(BasicBlock& bb) {


	LoopInfo&   loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo() ;

	BitVector* in1 = new BitVector(*((*inSet)[&bb]));

	BitVector* out1 = in1; // for empty blocks

	BitVector *phiSetReach;	
	Loop * l = loopInfo.getLoopFor(&bb);
	for (BasicBlock::iterator i1 = bb.begin(), i2 = bb.end(); i1 != i2; i1++) {	


		out1 = (*inInstructionSet)[&*i1];
		*out1 = *in1;


		if (define(&*i1)){
			(*out1)[(*indexOfValue)[&*i1]] = true;
		}

		if(isa<PHINode>(*i1)){
			PHINode* phiNode = cast<PHINode>(&*i1);
			if(l !=NULL)
			{
				for (int predId = 0; predId < phiNode->getNumIncomingValues(); predId++) {
					Value* val = phiNode->getIncomingValue(predId);
					if (isa<Instruction>(val) || isa<Argument>(val)) {
						BasicBlock* inBlock = phiNode->getIncomingBlock(predId);
						if(inBlock == l->getLoopLatch())
						{
							(*out1)[(*indexOfValue)[&*val]] = false;

						}
						else
						{
							(*out1)[(*indexOfValue)[&*val]] = true;

						}
					}
				}
			}
			else
			{

				for (int predId = 0; predId < phiNode->getNumIncomingValues(); predId++) {
					Value* val = phiNode->getIncomingValue(predId);
					if (isa<Instruction>(val) || isa<Argument>(val)) {


						BasicBlock* inBlock = phiNode->getIncomingBlock(predId);
						if(predId==0)
						{
							(phiSetReach) =new BitVector(*(*outSet)[inBlock]);                                                                           
						}
						else
							*(phiSetReach) &=*(*outSet)[inBlock];

						(*out1)[(*indexOfValue)[&*val]] = false;
					}
				}
				(*out1) |= *(phiSetReach) ;

			}

		}

		in1 = out1;


	}

	return out1;

}
std::vector<Value*>Reach::getInState(const BasicBlock *bb) {
	std::vector<Value*> inSetForBlocks;

	if (!isa<PHINode>(*(bb))) {
		const BitVector *bv = (*inSet)[&*bb];
		for(int i = bv->size()-1; i>=0 ;i--){
			if ( (*bv)[i] ) {
				inSetForBlocks.push_back(&*((*setOfValues )[i]));
			}
		}
	}
	return inSetForBlocks;
}
std::vector<Value*> Reach::getOutState(const BasicBlock *bb) {
	std::vector<Value*> outSetForBlocks;

	if (!isa<PHINode>(*(bb))) {
		const BitVector *bv = (*outSet)[&*bb];
		for(int i =  bv->size()-1; i>=0 ;i--)
		{
			if ( (*bv)[i] ) {
				outSetForBlocks.push_back(&*((*setOfValues )[i]));
			}
		}
	}
	return outSetForBlocks;
}


bool Reach::define(Instruction *ii) {
	return !(isa<TerminatorInst>(ii)) ;
}






bool Reach::runOnFunction(Function &F) {
	domain.clear();
	setOfValues = new std::vector<Value*>();
	indexOfValue = new ValueMap<Value*, int>();
	inInstructionSet= new ValueMap<const Instruction*, BitVector*>();
	totalArg=0;
	int index = 0;
	for (Function::arg_iterator argument = F.arg_begin(); argument != F.arg_end(); ++argument){
		domain.push_back(&*argument);
		setOfValues->push_back(&*argument);
		(*indexOfValue )[&*argument] = index;
		index++;
		totalArg++;
	}

	for (inst_iterator instruction = inst_begin(F), e = inst_end(F); instruction != e; ++instruction) {
		domain.push_back(&*instruction);
		setOfValues->push_back(&*instruction);
		(* indexOfValue)[&*instruction] = index;
		index++;
	}

	Reach::sized = domain.size();



	for (inst_iterator instruction = inst_begin(F), e = inst_end(F); instruction != e; ++instruction) {
		(*inInstructionSet)[&*instruction] = new BitVector(Reach::sized, false);
	}

	DataFlowAnalysis<BitVector>::runOnFunction(F); //call the analysis method in dataflow
	DataFlowAnnotator<Reach>  annotator(*this,errs());
	annotator.print(F);
	return false;


}

void Reach::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.addRequired<LoopInfoWrapperPass>();
	AU.setPreservesAll();

}






char Reach::ID = 0;
static RegisterPass<Reach> X("reach", "Reaching Definition Analysis Pass.");
