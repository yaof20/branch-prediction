//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <string.h>

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

// gshare
uint32_t* counter_table;
uint32_t global_history;

// tournament
uint32_t local_history;
uint32_t* local_history_table;
uint32_t* local_counter_table;

uint32_t* global_counter_table;
uint32_t* predict_select_table;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//
uint32_t get_lower_bits(uint32_t input_num, uint32_t bit_num){
  uint32_t mask = (1 << bit_num) - 1; // mask for getting the lower bits, a binary number consists of 1s
  return input_num & mask; // 
}

int get_index(uint32_t pc, uint32_t predictor_id){
  uint32_t pc_lower_bits, pc_index;
  switch (bpType) {
    case GSHARE:
      pc_lower_bits = get_lower_bits(pc, ghistoryBits);
      return (pc_lower_bits ^ global_history);

    case TOURNAMENT:
      // index to local history table
      if(predictor_id == 0){ 
        pc_index = get_lower_bits(pc, pcIndexBits);
        return pc_index;
      }
      // index to local counter table
      if(predictor_id == 1){
        pc_index = get_lower_bits(pc, pcIndexBits);
        local_history = local_history_table[pc_index];
        return get_lower_bits(local_history, lhistoryBits);
      }
      // index to global counter table
      if(predictor_id == 2){
        return get_lower_bits(global_history, ghistoryBits);
      }
    default:
      break;
  }
  return 0;
};

uint8_t predict(uint32_t pc){
  if(bpType == GSHARE){
    uint32_t index = get_index(pc, 0);
    return (counter_table[index] > 1) ? TAKEN : NOTTAKEN;
  }

  if(bpType==TOURNAMENT){
    uint32_t local_index = get_index(pc, 1);
    uint32_t global_index = get_index(pc, 2);

    if(predict_select_table[global_index] > 1)
      return (local_counter_table[local_index] > 1) ? TAKEN : NOTTAKEN;
    else
      return (global_counter_table[global_index] > 1) ? TAKEN : NOTTAKEN;
  }
  return -1;
}

void init_table(uint32_t* table, int length, uint32_t value){
  for(int i = 0; i < length; i++){
        table[i] = value;
      }
}

void update_table(uint32_t* table, uint32_t index, uint8_t increase, uint8_t decrease){
  if(increase && table[index] < 3)
    table[index]++;
  if(decrease && table[index] > 0)
    table[index]--;
}

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  int table_length;
  switch (bpType) {
    case GSHARE:
      // init GR
      global_history = 0;

      // init counter table
      table_length = 1 << ghistoryBits;
      counter_table = (uint32_t*)malloc(table_length * sizeof(uint32_t));
      init_table(counter_table, table_length, WN);
    case TOURNAMENT:
      global_history = 0;

      table_length = 1 << pcIndexBits;
      local_history_table = (uint32_t*)malloc(table_length * sizeof(uint32_t));
      init_table(local_history_table, table_length, 0);

      table_length = 1 << lhistoryBits;
      local_counter_table = (uint32_t*)malloc(table_length * sizeof(uint32_t));
      init_table(local_counter_table, table_length, 1);

      table_length = 1 << ghistoryBits;
      global_counter_table = (uint32_t*)malloc(table_length * sizeof(uint32_t));
      init_table(global_counter_table, table_length, 1);

      predict_select_table = (uint32_t*)malloc(table_length * sizeof(uint32_t));
      init_table(predict_select_table, table_length, 2);
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

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return predict(pc);
    case TOURNAMENT:
      return predict(pc);
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
  uint32_t index, local_index, global_index;
  uint32_t local_counter, global_counter;
  uint32_t updated_history, updated_counter;
  uint8_t increase, decrease;
  switch (bpType) {
    case GSHARE:
      // update counter table
      index = get_index(pc, 0);
      if(outcome == TAKEN && counter_table[index] < 3)
        counter_table[index]++;
      if(outcome == NOTTAKEN && counter_table[index] > 0)
        counter_table[index]--;

      // update global register
      updated_history = (global_history << 1) | outcome; // the last bit is current outcome
      global_history = get_lower_bits(updated_history, ghistoryBits);
      break;

    case TOURNAMENT:
      // update predict select table
      local_index = get_index(pc, 1);
      global_index = get_index(pc, 2);
      increase = (local_counter_table[local_index] == outcome && global_counter_table[global_index] != outcome);
      decrease = (local_counter_table[local_index] != outcome && global_counter_table[global_index] == outcome);
      update_table(predict_select_table, global_index, increase, decrease);

      // update local counter table & global counter table
      increase = (outcome == TAKEN);
      decrease = (outcome == NOTTAKEN);
      update_table(local_counter_table, local_index, increase, decrease);
      update_table(global_counter_table, global_index, increase, decrease);
      
      // update local history table
      index = get_index(pc, 0);
      local_history = local_history_table[index];
      updated_history = (local_history << 1) | outcome; // the last bit is current outcome
      local_history_table[index] = get_lower_bits(updated_history, lhistoryBits);

      // update global history
      updated_history = (global_history << 1) | outcome; // the last bit is current outcome
      global_history = get_lower_bits(updated_history, ghistoryBits);
      
      break;

    case CUSTOM:
    default:
      break;
  }
}
