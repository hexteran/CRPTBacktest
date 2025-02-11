//#pragma once
#include "circular_buffer.hpp"
#include "order_execution_manager.hpp"
#include "market_data_simulation_manager.hpp"
#include "simulation.hpp"

int main(int argc, char* argv[])
{
         ::testing::InitGoogleTest(&argc, argv); 
  return RUN_ALL_TESTS(); 
}