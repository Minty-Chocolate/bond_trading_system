#include "bond/BondPricingService.hpp"
#include "bond/BondAlgoStreamingService.hpp"
#include "bond/BondStreamingService.hpp"
#include "bond/BondInquiryService.hpp"
#include "bond/BondTradeBookingService.hpp"
#include "bond/BondPositionService.hpp"
#include "bond/BondRiskService.hpp"
#include "bond/BondMarketDataService.hpp"
#include "bond/BondAlgoExecutionService.hpp"
#include "bond/BondExecutionService.hpp"
#include "bond/GUIService.hpp"

int main()
{
  auto productService = BondProductService::GetInstance();

  Bond T2("91282CME8", CUSIP, "T", 4., date(2026, Nov, 30), 0.019063);
  Bond T3("91282CMB4", CUSIP, "T", 4., date(2027, Dec, 15), 0.028002);
  Bond T5("91282CMD0", CUSIP, "T", 4., date(2029, Dec, 31), 0.044902);
  Bond T7("91282CMC2", CUSIP, "T", 4., date(2031, Dec, 31), 0.060510);
  Bond T10("91282CLW9", CUSIP, "T", 4., date(2034, Nov, 15), 0.081718);
  Bond T20("912810UF3", CUSIP, "T", 4., date(2044, Nov, 15), 0.136657);
  Bond T30("912810UE6", CUSIP, "T", 4., date(2054, Nov, 15), 0.173594);

  productService->Add(T2);
  productService->Add(T3);
  productService->Add(T5);
  productService->Add(T7);
  productService->Add(T10);
  productService->Add(T20);
  productService->Add(T30);

// ------------- Inquiry -------------

  BondInquiryService inquiryService;
  BondInquiryServiceListener inquiryServiceListener(&inquiryService);
  inquiryService.AddListener(&inquiryServiceListener);

  std::cout << "Processing inquiries.txt" << std::endl;
  BondInquirySubscriber inquirySubscriber("input/inquiries.txt", &inquiryService);
  inquiryService.Subscribe(&inquirySubscriber);
  std::cout << "Processing inquiries.txt done\n" << std::endl;

// ------------- Price -------------

  BondPricingService pricingService;
  GUIService guiService(300);
  BondAlgoStreamingService algoStreamingService;
  BondStreamingService streamingService;
  BondPriceStreamsHistoricalDataService historicalDataService;

  BondPriceServiceListener guiServiceListener(&guiService);
  BondPricesServiceListener algoStreamingServiceListener(&algoStreamingService);
  BondAlgoStreamServiceListener streamingServiceListener(&streamingService);
  BondPriceStreamsServiceListener historicalDataServiceListener(&historicalDataService);

  pricingService.AddListener(&guiServiceListener);
  pricingService.AddListener(&algoStreamingServiceListener);
  algoStreamingService.AddListener(&streamingServiceListener);
  streamingService.AddListener(&historicalDataServiceListener);

  std::cout << "Processing prices.txt" << std::endl;
  BondPricesConnector pricesConnector("input/prices.txt", &pricingService);
  pricingService.Subscribe(&pricesConnector);
  std::cout << "Processing prices.txt done\n" << std::endl;

// -------------- Trade -------------

  BondTradeBookingService tradeBookingService;
  BondPositionService positionService;
  BondRiskService riskService;
  BondPositionHistoricalDataService positionHistoricalDataService;
  BondRiskHistoricalDataService riskHistoricalDataService;

  BondTradesServiceListener tradeListener(&positionService);
  BondPositionServiceListener positionListener(&positionHistoricalDataService);
  BondPositionRiskServiceListener positionListenerFromRisk(&riskService);
  BondRiskServiceListener riskListener(&riskHistoricalDataService);

  tradeBookingService.AddListener(&tradeListener);
  positionService.AddListener(&positionListener);
  positionService.AddListener(&positionListenerFromRisk);
  riskService.AddListener(&riskListener);

  std::cout << "Processing trades.txt" << std::endl;
  BondTradesConnector tradesSubscriber("input/trades.txt", &tradeBookingService);
  tradeBookingService.Subscribe(&tradesSubscriber);
  std::cout << "Processing trades.txt done\n" << std::endl;

// -------------- MarketData -------------

  BondMarketDataService marketDataService;
  BondAlgoExecutionService algoExecutionService;
  BondExecutionService executionService;
  BondExecutionHistoricalDataService executionHistoricalDataService;

  BondMarketDataServiceListener marketDataListener(&algoExecutionService);
  BondAlgoExecutionServiceListener algoExecutionListener(&executionService);
  BondExecutionOrderServiceListener executionListener(&executionHistoricalDataService);
  BondExecutionServiceListener executionListenerFromTrade(&tradeBookingService); // Assuming tradeBookingService exists

  marketDataService.AddListener(&marketDataListener);
  algoExecutionService.AddListener(&algoExecutionListener);
  executionService.AddListener(&executionListener);
  executionService.AddListener(&executionListenerFromTrade);

  std::cout << "Processing marketdata.txt" << std::endl;
  BondMarketDataConnector marketdataSubscriber("input/marketdata.txt", &marketDataService);
  marketDataService.Subscribe(&marketdataSubscriber);
  std::cout << "Processing marketdata.txt done\n" << std::endl;
}
