
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
#include "live.h"
#include "dataFlowAnalysis.h"
#include "DataFlowAnnotator.h"

#include <ostream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <vector>
using namespace llvm;
using namespace example;
using namespace liveAnal;

/* set forward false in the constructor for liveness */
Live::Live() : DataFlowAnalysis<BitVector>(false), FunctionPass(ID) {
	setOfValues = new std::vector<Value*>();
	indexOfValue = new ValueMap<Value*, int>();
	inInstructionSet = new ValueMap<const Instruction*, BitVector*>();
}

void Live::boundaryCondition(BitVector *boundaryValues) {
	*boundaryValues = BitVector(Live::sized, false);
}


void Live::meetFunc(BitVector* output, const BitVector* newInput){
	*output |= *newInput;
}

BitVector* Live::initDomain(BasicBlock& b){
	return new BitVector(Live::sized, false);
}


BitVector* Live::transferFunction(BasicBlock& bb) {
	BitVector* out1 = new BitVector(*((*outSet)[&bb]));

	BitVector* in1 = out1; // for empty blocks
	Instruction* inst1;

	BasicBlock::iterator ii = --(bb.end()), ib = bb.begin();
	while (true) {


		inst1 = &*ii;
		in1 = (*inInstructionSet)[inst1];
		*in1 = *out1;


		if (define(inst1)){
			(*in1)[(*indexOfValue)[inst1]] = false;
		}

		if (!isa<PHINode>(*ii)) {
			User::op_iterator OI, OE;
			for (OI = inst1->op_begin(), OE=inst1->op_end(); OI != OE; ++OI) {
				if (isa<Instruction>(*OI) || isa<Argument>(*OI)) {

					(*in1)[(*indexOfValue)[*OI]] = true;
				}
			}
		}else if(isa<PHINode>(*ii)){
			PHINode* phiNode = cast<PHINode>(&*ii);
			for (int predId = 0; predId < phiNode->getNumIncomingValues(); predId++) {
				Value* val = phiNode->getIncomingValue(predId);
				if (isa<Instruction>(val) || isa<Argument>(val)) {
					int val1 = (*indexOfValue)[val];
					BasicBlock* inBlock = phiNode->getIncomingBlock(predId);
					if ((*phiSet).find(inBlock) == (*phiSet).end())
						(*phiSet)[inBlock] = new BitVector(sized);
					(*(*phiSet)[inBlock]).set(val1);
				}
			}


		}

		out1 = in1;


		if (ii == ib) break;

		--ii;
	}

	return in1;

}
std::vector<Value*> Live::getInState(const BasicBlock *bb) {
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
	std::vector<Value*> Live::getOutState(const BasicBlock *bb) {
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


		bool Live::define(Instruction *ii) {
			return !(isa<TerminatorInst>(ii)) ;
		}






		bool Live::runOnFunction(Function &F) {
			domain.clear();
			setOfValues = new std::vector<Value*>();
			indexOfValue = new ValueMap<Value*, int>();
			inInstructionSet= new ValueMap<const Instruction*, BitVector*>();

			int index = 0;
			for (Function::arg_iterator argument = F.arg_begin(); argument != F.arg_end(); ++argument){
				domain.push_back(&*argument);
				setOfValues->push_back(&*argument);
				(*indexOfValue )[&*argument] = index;
				index++;
			}

			for (inst_iterator instruction = inst_begin(F), e = inst_end(F); instruction != e; ++instruction) {
				domain.push_back(&*instruction);
				setOfValues->push_back(&*instruction);
				(* indexOfValue)[&*instruction] = index;
				index++;
			}

			Live::sized = domain.size();



			for (inst_iterator instruction = inst_begin(F), e = inst_end(F); instruction != e; ++instruction) {
				(*inInstructionSet)[&*instruction] = new BitVector(Live::sized, false);
			}

			DataFlowAnalysis<BitVector>::runOnFunction(F); //call the analysis method in dataflow

			DataFlowAnnotator<Live>  annotator(*this,errs());
			annotator.print(F);
			return false;


		}

		void Live::getAnalysisUsage(AnalysisUsage &AU) const {
			AU.setPreservesAll();
		}






		char Live::ID = 0;
		static RegisterPass<Live> X("live", "Liveness Analysis Pass.");
