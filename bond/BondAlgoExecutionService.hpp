#ifndef BOND_ALGO_EXECUTION_SERVICE_HPP
#define BOND_ALGO_EXECUTION_SERVICE_HPP

#include "../base/positionservice.hpp"
#include "../base/products.hpp"
#include "../base/soa.hpp"
#include "../base/executionservice.hpp"
#include "IOFileConnector.hpp"
#include "BondAlgoStreamingService.hpp"

#include <vector>
#include <string>
#include <iostream>

// ------------- Declaration: AlgoExecution<T> -------------

template <typename T>
class AlgoExecution {
public:
  explicit AlgoExecution(const ExecutionOrder<T> &executionOrder);

  const ExecutionOrder<T> &getExecutionOrder() const;

private:
  const ExecutionOrder<T> &executionOrder;
};

// ------------- Declaration: BondAlgoExecutionService -------------

class BondAlgoExecutionService : public Service<std::string, AlgoExecution<Bond>> {
public:
  BondAlgoExecutionService();

  void Execute(OrderBook<Bond> &orderBook);
  void OnMessage(AlgoExecution<Bond> &data) override;

private:
  std::vector<PricingSide> sideState;
  int cur_ptr;
  long int orderNumber;
};

// ------------- Declaration: BondMarketDataServiceListener -------------

class BondMarketDataServiceListener : public ServiceListener<OrderBook<Bond>> {
public:
  explicit BondMarketDataServiceListener(BondAlgoExecutionService *listeningService);

  void ProcessAdd(OrderBook<Bond> &data) override;
  void ProcessRemove(OrderBook<Bond> &data) override;
  void ProcessUpdate(OrderBook<Bond> &data) override;

private:
  BondAlgoExecutionService *listeningService;
};

// ------------- Declaration: BondExecutionOrderServiceListener -------------

class BondExecutionOrderServiceListener : public ServiceListener<ExecutionOrder<Bond>> {
public:
  explicit BondExecutionOrderServiceListener(HistoricalDataService<ExecutionOrder<Bond>> *listeningService);

  void ProcessAdd(ExecutionOrder<Bond> &data) override;
  void ProcessRemove(ExecutionOrder<Bond> &data) override;
  void ProcessUpdate(ExecutionOrder<Bond> &data) override;

private:
  HistoricalDataService<ExecutionOrder<Bond>> *listeningService;
};

// ------------- Declaration: BondExecutionOrderConnector -------------

class BondExecutionOrderConnector : public OutputFileConnector<ExecutionOrder<Bond>> {
public:
  explicit BondExecutionOrderConnector(const std::string &filePath);

private:
  std::string toString(ExecutionOrder<Bond> &data) override;
};

// ------------- Declaration: BondExecutionHistoricalDataService -------------

class BondExecutionHistoricalDataService : public HistoricalDataService<ExecutionOrder<Bond>> {
public:
  BondExecutionHistoricalDataService();
  void PersistData(std::string persistKey, const ExecutionOrder<Bond> &data) override;

private:
  void OnMessage(ExecutionOrder<Bond> &data) override;
  BondExecutionOrderConnector *connector;
};

// ------------- Definition: AlgoExecution<T> -------------

template <typename T>
AlgoExecution<T>::AlgoExecution(const ExecutionOrder<T> &executionOrder)
    : executionOrder(executionOrder) {}

template <typename T>
const ExecutionOrder<T> &AlgoExecution<T>::getExecutionOrder() const {
  return executionOrder;
}

// ------------- Definition: BondAlgoExecutionService -------------

BondAlgoExecutionService::BondAlgoExecutionService()
    : sideState({PricingSide::BID, PricingSide::OFFER}), cur_ptr(0), orderNumber(1) {}

void BondAlgoExecutionService::Execute(OrderBook<Bond> &orderBook) {
  auto topBid = orderBook.GetBidStack()[0];
  auto topOffer = orderBook.GetOfferStack()[0];
  double spread = topOffer.GetPrice() - topBid.GetPrice();

  if (spread <= 1.0 / 128) {
    long volume = sideState[cur_ptr] == BID ? topBid.GetQuantity() : topOffer.GetQuantity();
    double price = sideState[cur_ptr] == BID ? topBid.GetPrice() : topOffer.GetPrice();

    ExecutionOrder<Bond> executionOrder(
        orderBook.GetProduct(), sideState[cur_ptr], "Order_" + std::to_string(orderNumber), MARKET, price, volume, 0, "", false);
    AlgoExecution<Bond> algoExecution(executionOrder);

    for (auto listener : GetListeners())
      listener->ProcessAdd(algoExecution);

    cur_ptr = (cur_ptr + 1) % sideState.size();
    orderNumber++;
  }
}

void BondAlgoExecutionService::OnMessage(AlgoExecution<Bond> &data) {}

// ------------- Definition: BondMarketDataServiceListener -------------

BondMarketDataServiceListener::BondMarketDataServiceListener(BondAlgoExecutionService *listeningService)
    : listeningService(listeningService) {}

void BondMarketDataServiceListener::ProcessAdd(OrderBook<Bond> &data) {
  listeningService->Execute(data);
}

void BondMarketDataServiceListener::ProcessRemove(OrderBook<Bond> &data) {}

void BondMarketDataServiceListener::ProcessUpdate(OrderBook<Bond> &data) {
  listeningService->Execute(data);
}

// ------------- Definition: BondExecutionOrderServiceListener -------------

BondExecutionOrderServiceListener::BondExecutionOrderServiceListener(
    HistoricalDataService<ExecutionOrder<Bond>> *listeningService)
    : listeningService(listeningService) {}

void BondExecutionOrderServiceListener::ProcessAdd(ExecutionOrder<Bond> &data) {
  listeningService->PersistData(data.GetProduct().GetProductId(), data);
}

void BondExecutionOrderServiceListener::ProcessRemove(ExecutionOrder<Bond> &data) {}

void BondExecutionOrderServiceListener::ProcessUpdate(ExecutionOrder<Bond> &data) {}

// ------------- Definition: BondExecutionOrderConnector -------------

BondExecutionOrderConnector::BondExecutionOrderConnector(const std::string &filePath)
    : OutputFileConnector(filePath) {}

std::string BondExecutionOrderConnector::toString(ExecutionOrder<Bond> &data) {
  std::ostringstream oss;
  oss << boost::posix_time::microsec_clock::universal_time() << ","
      << data.GetProduct().GetProductId() << "," << data.GetSide() << "," << data.GetOrderId() << ","
      << data.GetOrderType() << "," << data.GetPrice() << "," << data.GetVisibleQuantity() << ","
      << data.GetHiddenQuantity() << "," << data.GetParentOrderId() << ","
      << data.IsChildOrder();
  return oss.str();
}

// ------------- Definition: BondExecutionHistoricalDataService -------------

BondExecutionHistoricalDataService::BondExecutionHistoricalDataService() {
  connector = new BondExecutionOrderConnector("output/execution.txt");
}

void BondExecutionHistoricalDataService::PersistData(std::string persistKey,
                                                     const ExecutionOrder<Bond> &data) {
  connector->Publish(const_cast<ExecutionOrder<Bond> &>(data));
}

void BondExecutionHistoricalDataService::OnMessage(ExecutionOrder<Bond> &data) {}
#endif 
