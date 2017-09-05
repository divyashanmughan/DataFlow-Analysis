#ifndef __DATAFLOW_H__
#define __DATAFLOW_H__

#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/DenseMap.h"

#include <ostream>
#include <list>


using namespace llvm;


namespace {
typedef std::list<BasicBlock*> listBlocks;

    template <typename domainValue>
        class DataFlowAnalysis{
public:
            bool direction;
            

            DataFlowAnalysis(bool direction) {
                this->direction = direction;  
            }

  typedef llvm::ValueMap<const llvm::BasicBlock*, domainValue*> bitValue;
                                bitValue *inSet;
                                bitValue *outSet;
                                bitValue *phiSet;
           //                     bitValue *phiSetReach;
                                llvm::ValueMap<llvm::BasicBlock*, bool> *done;


void forwardAnalysis(listBlocks  &l){
        bool changed = false;
        BasicBlock *block1 = *l.begin();
        l.pop_front();
        (*done)[block1] = true;
        int pred = 0;
        for (pred_iterator PI = pred_begin(block1), E = pred_end(block1); PI != E; ++PI){
                ++pred;
                if(PI == pred_begin(block1)){
                        *(*inSet)[block1] = *(*outSet)[*PI];
                }else{
                        meetFunc((*inSet)[block1], (*outSet)[*PI]);
                }
        }


        if(pred == 0)  boundaryCondition((*inSet)[block1]);


        domainValue *out1 = transferFunction(*block1);
        changed = (*out1 != *(*outSet)[block1]);

        if(changed)
                *(*outSet)[block1] = *out1;

        for (succ_iterator SI = succ_begin(block1), E = succ_end(block1); SI != E; ++SI) {

                if(changed || !(*done)[*SI]){
                        l.push_back(*SI);
                }
        }
}








void backwardAnalysis(listBlocks  &l){
        bool changed = false;

        BasicBlock *block1 = *l.begin();
        l.pop_front();
        (*done)[block1] = true;
           int succ = 0;
        for (succ_iterator SI = succ_begin(block1), E = succ_end(block1); SI != E; ++SI) {
                succ++;
                if(SI == succ_begin(block1)){
                        *(*outSet)[block1] = *(*inSet)[*SI];
                }else{
                        meetFunc((*outSet)[block1], (*inSet)[*SI]);
                }
        }


        if((*phiSet).find(block1) != (*phiSet).end() ){


                meetFunc((*outSet)[block1], (*phiSet)[block1]);
        }

        if(succ == 0) boundaryCondition((*outSet)[block1]);

        domainValue *in1 = transferFunction(*block1);

        changed = ((*in1) != (*(*inSet)[block1]));

        if(changed)
                *(*inSet)[block1] = *in1;

        for (pred_iterator PI = pred_begin(block1), E = pred_end(block1); PI != E; ++PI) {


                if(changed || !(*done)[*PI]){
                        l.push_back(*PI);
                }
        }
}





void getInForPhiNodes(Function &func){
        for (Function::iterator i = func.begin(), e = func.end(); i != e; ++i){
                int succ = 0;
                BasicBlock *block1 = &*i;
                if((*phiSet).find(block1) != (*phiSet).end()){
                        for (succ_iterator SI = succ_begin(&*i), SE = succ_end(&*i); SI != SE; SI++){
                                meetFunc((*inSet)[*SI], (*phiSet)[block1]);
                        }
                }
        }
}


void initList(Function &func, std::list<BasicBlock*>  &listBlocks){
        if(direction){
                BasicBlock& entry = func.getEntryBlock();
                listBlocks.push_back(&entry);
                return;
        }

        for (Function::iterator i = func.begin(), e = func.end(); i != e; ++i)
        {
                int succ = 0;

                for (succ_iterator SI = succ_begin(&*i), SE = succ_end(&*i); SI != SE; SI++)
                        succ++;

                if(succ==0) listBlocks.push_back(&*i);
        }
}



bool runOnFunction(Function &F){
        inSet  = new bitValue();
        outSet = new bitValue();
        phiSet = new bitValue();
        //phiSetReach = new bitValue();
        done = new ValueMap<BasicBlock*, bool>();
        for (Function::iterator i1 = F.begin(), e1 = F.end(); i1 != e1; i1++) {
                BasicBlock * bb = &*(i1);


                (*inSet)[bb] =  initDomain(*i1);
                (*outSet)[bb] =  initDomain(*i1);
        }

        typedef std::list<BasicBlock*> listBlock;

        listBlock *list2 = new listBlock();
        initList(F,*list2);
        while(!list2->empty()){
                if(direction)
                        forwardAnalysis(*list2);
                else
                        backwardAnalysis(*list2);
        }
        getInForPhiNodes(F);
return false;
}
 protected:
                                virtual domainValue* initDomain(llvm::BasicBlock& b) = 0;
                                virtual domainValue* transferFunction(llvm::BasicBlock& b) = 0;

                                virtual void boundaryCondition(domainValue*) = 0;
                                virtual void meetFunc(domainValue* output, const domainValue* newInput) = 0;


    };
}

#endif
