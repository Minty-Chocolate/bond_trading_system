cmake_minimum_required(VERSION 3.8)
project(bond_trading_system)

set(CMAKE_CXX_STANDARD 17)

include_directories("your boost dir")

set(BASE_HEADERS 
  base/executionservice.hpp
  base/historicaldataservice.hpp
  base/inquiryservice.hpp
  base/marketdataservice.hpp
  base/positionservice.hpp
  base/pricingservice.hpp
  base/products.hpp
  base/riskservice.hpp
  base/soa.hpp
  base/streamingservice.hpp
  base/tradebookingservice.hpp
)

set(BOND_HEADERS 
  bond/IOFileConnector.hpp
  bond/BondProductService.hpp
  bond/BondAlgoStreamingService.hpp
  bond/GUIService.hpp
  bond/BondPricingService.hpp
  bond/BondStreamingService.hpp
  bond/BondInquiryService.hpp
  bond/BondTradeBookingService.hpp
  bond/BondPositionService.hpp
  bond/BondRiskService.hpp
  bond/BondMarketDataService.hpp
  bond/BondAlgoExecutionService.hpp
  bond/BondExecutionService.hpp
)

set(SOURCE_FILES main.cpp ${BASE_HEADERS} ${BOND_HEADERS})

add_executable(bond_trading_system ${SOURCE_FILES})