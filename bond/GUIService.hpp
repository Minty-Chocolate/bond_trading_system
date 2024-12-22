#ifndef BOND_GUISERVICE_HPP
#define BOND_GUISERVICE_HPP

#include "../base/products.hpp"
#include "../base/pricingservice.hpp"
#include "IOFileConnector.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

// ------------- Declaration: GUIConnector -------------

class GUIConnector : public OutputFileConnector<Price<Bond>> {
public:
  explicit GUIConnector(const std::string &filePath);
  std::string toString(Price<Bond> &data) override;
};

// ------------- Declaration: GUIService -------------

class GUIService : public Service<std::string, Price<Bond>> {
public:
  GUIService(const int throttle);
  void OnMessage(Price<Bond> &data) override;

private:
  const int throttle = 300;  // Defined in milliseconds
  GUIConnector *connector;
  boost::posix_time::ptime lastTick = boost::posix_time::microsec_clock::universal_time();
};

// ------------- Declaration: BondPriceServiceListener -------------

class BondPriceServiceListener : public ServiceListener<Price<Bond>> {
public:
  explicit BondPriceServiceListener(GUIService *listeningService);

private:
  void ProcessAdd(Price<Bond> &data) override;
  void ProcessRemove(Price<Bond> &data) override;
  void ProcessUpdate(Price<Bond> &data) override;

  GUIService *listeningService;
};

// ------------- Definition: GUIConnector -------------

GUIConnector::GUIConnector(const std::string &filePath) : OutputFileConnector(filePath) {}

std::string GUIConnector::toString(Price<Bond> &data) {
  std::ostringstream oss;
  oss << boost::posix_time::microsec_clock::universal_time() << ","
      << data.GetProduct().GetProductId() << ","
      << (data.GetMid() - data.GetBidOfferSpread() / 2) << ","
      << (data.GetMid() + data.GetBidOfferSpread() / 2);
  return oss.str();
}

// ------------- Definition: GUIService -------------

GUIService::GUIService(const int throttle) : throttle(throttle) {
  connector = new GUIConnector("output/gui.txt");
}

void GUIService::OnMessage(Price<Bond> &data) {
  auto currentTick = boost::posix_time::microsec_clock::universal_time();
  boost::posix_time::time_duration diff = currentTick - lastTick;
  if (diff.total_milliseconds() > 300) {
    connector->Publish(data);
    lastTick = boost::posix_time::microsec_clock::universal_time();
  }
}

// ------------- Definition: BondPriceServiceListener -------------

BondPriceServiceListener::BondPriceServiceListener(GUIService *listeningService) : listeningService(listeningService) {}

void BondPriceServiceListener::ProcessAdd(Price<Bond> &data) {
  listeningService->OnMessage(data);
}

void BondPriceServiceListener::ProcessRemove(Price<Bond> &data) {}

void BondPriceServiceListener::ProcessUpdate(Price<Bond> &data) {
  listeningService->OnMessage(data);
}
#endif