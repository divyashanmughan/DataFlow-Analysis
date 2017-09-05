#ifndef __DATAFLOW2_H__
#define __DATAFLOW2_H__

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
namespace  reachAnal{

	class Reach : public FunctionPass, public DataFlowAnalysis<BitVector>, public AssemblyAnnotationWriter{

		public:
			static char ID;

			/* set forward false in the constructor for liveness */
			Reach() ;//: DataFlowAnalysis<BitVector>(false), FunctionPass(ID) ;

			std::vector<llvm::Value*> domain;
			std::vector<llvm::Value*> *setOfValues ; 
			ValueMap<llvm::Value*, int> *indexOfValue;
			ValueMap<const llvm::Instruction*, llvm::BitVector*> *inInstructionSet;  

			int sized;
			int totalArg;
			bool define(Instruction *ii) ;

			std::vector<Value*> getInState(const BasicBlock *bb);

			std::vector<Value*> getOutState(const BasicBlock *bb);

			void boundaryCondition(BitVector *boundaryValues) ;


			void meetFunc(BitVector* input, const BitVector* newOutput);


			BitVector* initDomain(BasicBlock& b);



			BitVector* transferFunction(BasicBlock& bb) ;

			bool runOnFunction(Function &F) ;

			void getAnalysisUsage(AnalysisUsage &AU) const ;




	};


}
#endif
