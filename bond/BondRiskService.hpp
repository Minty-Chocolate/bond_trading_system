#ifndef BOND_RISK_SERVICE_HPP
#define BOND_RISK_SERVICE_HPP

#include "../base/soa.hpp"
#include "../base/products.hpp"
#include "../base/streamingservice.hpp"
#include "../base/riskservice.hpp"

// ------------- Declaration: BondRiskService -------------

class BondRiskService : public RiskService<Bond> {
public:
  BondRiskService();

  void OnMessage(PV01<Bond> &data) override;
  void AddPosition(Position<Bond> &position) override;
  const PV01<BucketedSector<Bond>> &GetBucketedRisk(const BucketedSector<Bond> &sector) const override;
};

// ------------- Declaration: BondPositionRiskServiceListener -------------

class BondPositionRiskServiceListener : public ServiceListener<Position<Bond>> {
public:
  explicit BondPositionRiskServiceListener(BondRiskService *listeningService);

  void ProcessAdd(Position<Bond> &data) override;
  void ProcessRemove(Position<Bond> &data) override;
  void ProcessUpdate(Position<Bond> &data) override;

private:
  BondRiskService *listeningService;
};

// ------------- Declaration: BondRiskServiceListener -------------

class BondRiskServiceListener : public ServiceListener<PV01<Bond>> {
public:
  explicit BondRiskServiceListener(HistoricalDataService<PV01<Bond>> *listeningService);

  void ProcessAdd(PV01<Bond> &data) override;
  void ProcessRemove(PV01<Bond> &data) override;
  void ProcessUpdate(PV01<Bond> &data) override;

private:
  HistoricalDataService<PV01<Bond>> *listeningService;
};

// ------------- Declaration: BondRiskConnector -------------

class BondRiskConnector : public OutputFileConnector<PV01<Bond>> {
public:
  explicit BondRiskConnector(const std::string &filePath);

private:
  std::string toString(PV01<Bond> &data) override;
};

// ------------- Declaration: BondRiskHistoricalDataService -------------

class BondRiskHistoricalDataService : public HistoricalDataService<PV01<Bond>> {
public:
  BondRiskHistoricalDataService();

  void PersistData(std::string persistKey, const PV01<Bond> &data) override;

private:
  void OnMessage(PV01<Bond> &data) override;
  BondRiskConnector *connector;
};

// ------------- Definition: BondRiskService -------------

BondRiskService::BondRiskService() {}

void BondRiskService::OnMessage(PV01<Bond> &data) {
  // No-op: Streaming service does not have a connector.
}

void BondRiskService::AddPosition(Position<Bond> &position) {
  auto product = position.GetProduct();

  // Calculate risk for the position
  PV01<Bond> risk(product, position.GetAggregatePosition() * product.GetPV01(), position.GetAggregatePosition());

  if (dataStore.find(position.GetProduct().GetProductId()) == dataStore.end()) {
    // Insert new risk data and notify listeners
    dataStore.insert(std::make_pair(position.GetProduct().GetProductId(), risk));
    for (auto listener : this->GetListeners()) {
      listener->ProcessAdd(risk);
    }
  } else {
    // Update existing risk data and notify listeners
    dataStore.insert(std::make_pair(position.GetProduct().GetProductId(), risk));
    for (auto listener : this->GetListeners()) {
      listener->ProcessUpdate(risk);
    }
  }
}

const PV01<BucketedSector<Bond>> &BondRiskService::GetBucketedRisk(const BucketedSector<Bond> &sector) const {
  double totalPV01 = 0.0;
  long totalPosition = 0;

  for (const auto &product : sector.GetProducts()) {
    if (dataStore.find(product.GetProductId()) != dataStore.end()) {
      totalPV01 += dataStore.at(product.GetProductId()).GetPV01();
      totalPosition += dataStore.at(product.GetProductId()).GetQuantity();
    }
  }

  auto pv01 = new PV01<BucketedSector<Bond>>(sector, totalPV01, totalPosition);

  // Debugging Output
  std::cout << "BucketedRisk: Sector = " << sector.GetName() << ", TotalPV01 = " << totalPV01
            << ", TotalQuantity = " << totalPosition << std::endl;

  return *pv01;
}

// ------------- Definition: BondPositionRiskServiceListener -------------

BondPositionRiskServiceListener::BondPositionRiskServiceListener(BondRiskService *listeningService)
    : listeningService(listeningService) {}

void BondPositionRiskServiceListener::ProcessAdd(Position<Bond> &data) {
  listeningService->AddPosition(data);
}

void BondPositionRiskServiceListener::ProcessRemove(Position<Bond> &data) {
  // NO-OP: Positions are never removed in this project.
}

void BondPositionRiskServiceListener::ProcessUpdate(Position<Bond> &data) {
  // AddPosition updates the risk of existing positions.
  listeningService->AddPosition(data);
}


// ------------- Definition: BondRiskServiceListener -------------

BondRiskServiceListener::BondRiskServiceListener(
    HistoricalDataService<PV01<Bond>> *listeningService)
    : listeningService(listeningService) {}

void BondRiskServiceListener::ProcessAdd(PV01<Bond> &data) {
  listeningService->PersistData(data.GetProduct().GetProductId(), data);
}

void BondRiskServiceListener::ProcessRemove(PV01<Bond> &data) {
  // NO-OP
}

void BondRiskServiceListener::ProcessUpdate(PV01<Bond> &data) {
  listeningService->PersistData(data.GetProduct().GetProductId(), data);
}

// ------------- Definition: BondRiskConnector -------------

BondRiskConnector::BondRiskConnector(const std::string &filePath)
    : OutputFileConnector(filePath) {}

std::string BondRiskConnector::toString(PV01<Bond> &data) {
  std::ostringstream oss;
  oss << boost::posix_time::microsec_clock::universal_time() << ","
      << data.GetProduct().GetProductId() << ","
      << data.GetQuantity() << ","
      << data.GetPV01();
  return oss.str();
}

// ------------- Definition: BondRiskHistoricalDataService -------------

BondRiskHistoricalDataService::BondRiskHistoricalDataService() {
  connector = new BondRiskConnector("output/risk.txt");
}

void BondRiskHistoricalDataService::PersistData(std::string persistKey, const PV01<Bond> &data) {
  connector->Publish(const_cast<PV01<Bond> &>(data));
}

void BondRiskHistoricalDataService::OnMessage(PV01<Bond> &data) {
  // NO-OP
}
#endif 