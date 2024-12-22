#ifndef BOND_TRADE_BOOKING_SERVICE_HPP
#define BOND_TRADE_BOOKING_SERVICE_HPP

#include "../base/products.hpp"
#include "../base/tradebookingservice.hpp"
#include "../base/executionservice.hpp"
#include "IOFileConnector.hpp"


// ------------- Declaration: BondTradesConnector -------------

class BondTradesConnector : public InputFileConnector<std::string, Trade<Bond>> {
public:
  BondTradesConnector(const std::string &filePath, Service<std::string, Trade<Bond>> *connectedService);

private:
  void parse(std::string line) override;
};

// ------------- Declaration: BondTradeBookingService -------------

class BondTradeBookingService : public TradeBookingService<Bond> {
public:
  BondTradeBookingService();
  void Subscribe(BondTradesConnector *connector);
  void OnMessage(Trade<Bond> &data) override;
  void BookTrade(const Trade<Bond> &trade) override;
};

// ------------- Declaration: BondExecutionServiceListener -------------

class BondExecutionServiceListener : public ServiceListener<ExecutionOrder<Bond>> {
private:
  BondTradeBookingService *listeningService;
  vector<std::string> TradeBooks = {"TRSY1", "TRSY2", "TRSY3"};
  int cur_ptr = 0;

  void cycleState();

public:
  explicit BondExecutionServiceListener(BondTradeBookingService *listeningService);
  void ProcessAdd(ExecutionOrder<Bond> &data) override;
  void ProcessRemove(ExecutionOrder<Bond> &data) override;
  void ProcessUpdate(ExecutionOrder<Bond> &data) override;
};

// ------------- Definition: BondTradesConnector -------------

BondTradesConnector::BondTradesConnector(const std::string &filePath,
                                         Service<std::string, Trade<Bond>> *connectedService)
    : InputFileConnector(filePath, connectedService) {}

void BondTradesConnector::parse(std::string line) {
  auto split = splitString(line, ',');
  std::string productId = split[0], tradeId = split[1], bookId = split[3];
  double price = std::stod(split[2]);
  long quantity = std::stol(split[4]);
  Side side = split[5].compare("0") == 0 ? Side::BUY : Side::SELL;

  auto bond = BondProductService::GetInstance()->GetData(productId);
  auto trade = Trade<Bond>(bond, tradeId, price, bookId, quantity, side);
  connectedService->OnMessage(trade);
}

// ------------- Definition: BondTradeBookingService -------------

BondTradeBookingService::BondTradeBookingService() {}

void BondTradeBookingService::Subscribe(BondTradesConnector *connector) {
  connector->read();
}

void BondTradeBookingService::OnMessage(Trade<Bond> &data) {
  dataStore.insert(std::make_pair(data.GetTradeId(), data));
  BookTrade(data);
}

void BondTradeBookingService::BookTrade(const Trade<Bond> &trade) {
  for (auto listener : this->GetListeners()) {
    listener->ProcessAdd(const_cast<Trade<Bond> &>(trade));
  }
}

// ------------- Definition: BondExecutionServiceListener -------------

BondExecutionServiceListener::BondExecutionServiceListener(BondTradeBookingService *listeningService)
    : listeningService(listeningService) {}

void BondExecutionServiceListener::cycleState() {
  cur_ptr = (cur_ptr + 1) % TradeBooks.size();
}

void BondExecutionServiceListener::ProcessAdd(ExecutionOrder<Bond> &data) {
  Trade<Bond> trade(data.GetProduct(),
                    "tradeid",  // TODO: Replace with generated trade ID
                    data.GetPrice(),
                    TradeBooks[cur_ptr],
                    data.GetVisibleQuantity() + data.GetHiddenQuantity(),
                    data.GetSide() == OFFER ? BUY : SELL);

  listeningService->BookTrade(trade);
  cycleState();
}

void BondExecutionServiceListener::ProcessRemove(ExecutionOrder<Bond> &data) {
  // NO-OP : ExecutionOrders are never removed in this project.
}

void BondExecutionServiceListener::ProcessUpdate(ExecutionOrder<Bond> &data) {
  // NO-OP : ExecutionOrders are never updated in this project.
}

#endif