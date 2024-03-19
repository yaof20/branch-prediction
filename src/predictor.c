//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Feng Yao";
const char *studentID   = "A59025438";
const char *email       = "fengyao@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
uint32_t* counter_table;
uint32_t global_register;



//------------------------------------//
//        Predictor Functions         //
//------------------------------------//
uint32_t get_lower_bits(uint32_t input_num){
  uint32_t mask = (1 << ghistoryBits) - 1; // mask for getting the lower bits, a binary number consists of 1s
  return input_num & mask; // 
}

int get_index(uint32_t pc){
  uint32_t pc_lower_bits = get_lower_bits(pc);
  switch (bpType) {
    case GSHARE:
      return (pc_lower_bits ^ global_register);
    default:
      break;
  }
  return 0;
};

uint8_t predict(uint32_t index, uint32_t* counter_table){
  return (counter_table[index] > 1) ? TAKEN : NOTTAKEN;
}


// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType) {
    case GSHARE:
      // init GR
      global_register = 0;

      // init counter table
      int table_length = 1 << ghistoryBits;
      counter_table = (uint32_t*)malloc(table_length * sizeof(uint32_t));
      for(int i = 0; i < table_length; i++){
        counter_table[i] = WN;
      }

    default:
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  uint32_t index=get_index(pc);

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return predict(index, counter_table);
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  uint32_t index, updated_gr;
  switch (bpType) {
    case GSHARE:
      // update counter table
      index = get_index(pc);
      if(outcome == TAKEN && counter_table[index] < 3)
        counter_table[index]++;
      if(outcome == NOTTAKEN && counter_table[index] > 0)
        counter_table[index]--;

      // update global register
      updated_gr = (global_register << 1) | outcome; // the last bit is current outcome
      global_register = get_lower_bits(updated_gr);
      break;

    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }
}
