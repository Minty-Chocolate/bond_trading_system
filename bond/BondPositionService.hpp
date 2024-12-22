#ifndef BOND_POSITION_SERVICE_HPP
#define BOND_POSITION_SERVICE_HPP

#include "../base/positionservice.hpp"
#include "../base/products.hpp"
#include "../base/soa.hpp"

#include <string>
#include <map>
#include <vector>
#include <iostream>

// ------------- Declaration: BondPositionService -------------

class BondPositionService : public PositionService<Bond> {
public:
  BondPositionService();

  void AddTrade(const Trade<Bond> &trade) override;
  void OnMessage(Position<Bond> &data) override;
};

// ------------- Declaration: BondTradesServiceListener -------------

class BondTradesServiceListener : public ServiceListener<Trade<Bond>> {
public:
  explicit BondTradesServiceListener(BondPositionService *listeningService);

  void ProcessAdd(Trade<Bond> &data) override;
  void ProcessRemove(Trade<Bond> &data) override;
  void ProcessUpdate(Trade<Bond> &data) override;

private:
  BondPositionService *listeningService;
};


// ------------- Declaration: BondPositionServiceListener -------------

class BondPositionServiceListener : public ServiceListener<Position<Bond>> {
public:
  explicit BondPositionServiceListener(HistoricalDataService<Position<Bond>> *listeningService);

  void ProcessAdd(Position<Bond> &data) override;
  void ProcessRemove(Position<Bond> &data) override;
  void ProcessUpdate(Position<Bond> &data) override;

private:
  HistoricalDataService<Position<Bond>> *listeningService;
};

// ------------- Declaration: BondPositionConnector -------------

class BondPositionConnector : public OutputFileConnector<Position<Bond>> {
public:
  explicit BondPositionConnector(const std::string &filePath);

private:
  std::string toString(Position<Bond> &data) override;
};

// ------------- Declaration: BondPositionHistoricalDataService -------------

class BondPositionHistoricalDataService : public HistoricalDataService<Position<Bond>> {
public:
  BondPositionHistoricalDataService();

  void PersistData(std::string persistKey, const Position<Bond> &data) override;

private:
  void OnMessage(Position<Bond> &data) override;
  BondPositionConnector *connector;
};

// ------------- Definition: BondPositionService -------------

BondPositionService::BondPositionService() {}

void BondPositionService::AddTrade(const Trade<Bond> &trade) {
  std::string productId = trade.GetProduct().GetProductId();

  if (dataStore.find(productId) == dataStore.end()) {
    // Create and add a new position
    Position<Bond> newPosition(trade.GetProduct());
    newPosition.UpdatePosition(trade);  // Initialize position with the trade
    dataStore.insert(std::make_pair(productId, newPosition));

    // Notify listeners of the new position
    for (auto listener : GetListeners()) {
      listener->ProcessAdd(newPosition);
    }
  } else {
    // Update an existing position
    auto &position = dataStore.at(productId);
    position.UpdatePosition(trade);

    // Notify listeners of the updated position
    for (auto listener : GetListeners()) {
      listener->ProcessUpdate(position);
    }
  }
}

void BondPositionService::OnMessage(Position<Bond> &data) {
  // No-op
}

// ------------- Definition: BondTradesServiceListener -------------

BondTradesServiceListener::BondTradesServiceListener(BondPositionService *listeningService)
    : listeningService(listeningService) {}

void BondTradesServiceListener::ProcessAdd(Trade<Bond> &data) {
  listeningService->AddTrade(data);
}

void BondTradesServiceListener::ProcessRemove(Trade<Bond> &data) {
  // NO-OP: Trades are never removed in this project.
}

void BondTradesServiceListener::ProcessUpdate(Trade<Bond> &data) {
  // NO-OP: Trades are never updated in this project.
}


// ------------- Definition: BondPositionServiceListener -------------

BondPositionServiceListener::BondPositionServiceListener(
    HistoricalDataService<Position<Bond>> *listeningService)
    : listeningService(listeningService) {}

void BondPositionServiceListener::ProcessAdd(Position<Bond> &data) {
  listeningService->PersistData(data.GetProduct().GetProductId(), data);
}

void BondPositionServiceListener::ProcessRemove(Position<Bond> &data) {}

void BondPositionServiceListener::ProcessUpdate(Position<Bond> &data) {
  listeningService->PersistData(data.GetProduct().GetProductId(), data);
}

// ------------- Definition: BondPositionConnector -------------

BondPositionConnector::BondPositionConnector(const std::string &filePath)
    : OutputFileConnector(filePath) {}

std::string BondPositionConnector::toString(Position<Bond> &data) {
  std::ostringstream oss;
  oss << boost::posix_time::microsec_clock::universal_time() << ","
      << data.GetProduct().GetProductId() << ","
      << data.GetAggregatePosition();
  return oss.str();
}

// ------------- Definition: BondPositionHistoricalDataService -------------

BondPositionHistoricalDataService::BondPositionHistoricalDataService() {
  connector = new BondPositionConnector("output/positions.txt");
}

void BondPositionHistoricalDataService::PersistData(std::string persistKey, const Position<Bond> &data) {
  connector->Publish(const_cast<Position<Bond> &>(data));
}

void BondPositionHistoricalDataService::OnMessage(Position<Bond> &data) {}


#endif