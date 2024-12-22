#ifndef BOND_PRICING_SERVICE_HPP
#define BOND_PRICING_SERVICE_HPP

#include "BondProductService.hpp"
#include "../base/products.hpp"
#include "../base/pricingservice.hpp"
#include "IOFileConnector.hpp"

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

// ------------- Declaration: BondPricesConnector -------------

class BondPricesConnector : public InputFileConnector<std::string, Price<Bond>> {
public:
  BondPricesConnector(const std::string &filePath, Service<std::string, Price<Bond>> *connectedService);

private:
  void parse(std::string line) override;
};

// ------------- Declaration: BondPricingService -------------

class BondPricingService : public PricingService<Bond> {
public:
  BondPricingService();

  void Subscribe(BondPricesConnector *connector);
  void OnMessage(Price<Bond> &data) override;
};

// ------------- Definition: BondPricesConnector -------------

BondPricesConnector::BondPricesConnector(const std::string &filePath, Service<std::string, Price<Bond>> *connectedService)
    : InputFileConnector(filePath, connectedService) {}

void BondPricesConnector::parse(std::string line) {
  auto split = splitString(line, ',');
  std::string id = split[0];
  double mid = fractionalToDouble(split[1]);
  double bidOfferSpread = fractionalToDouble(split[2]);

  auto bond = BondProductService::GetInstance()->GetData(id);
  auto price = Price<Bond>(bond, mid, bidOfferSpread);

  // Debugging Output
  std::cout << "Parsed Price: ProductId = " << bond.GetProductId()
            << ", Mid = " << mid << ", Spread = " << bidOfferSpread << std::endl;

  connectedService->OnMessage(price);
}

// ------------- Definition: BondPricingService -------------

BondPricingService::BondPricingService() {}

void BondPricingService::OnMessage(Price<Bond> &data) {
  std::string productId = data.GetProduct().GetProductId();

  if (dataStore.find(productId) == dataStore.end()) {
    // Add new price to data store
    dataStore.insert(std::make_pair(productId, data));

    // Notify listeners about the new price
    for (auto listener : this->GetListeners()) {
      listener->ProcessAdd(data);
    }

    // Debugging Output
    std::cout << "Added Price: ProductId = " << productId
              << ", Mid = " << data.GetMid() << ", Spread = " << data.GetBidOfferSpread() << std::endl;
  } else {
    // Update existing price in data store
    dataStore.insert(std::make_pair(productId, data));

    // Notify listeners about the updated price
    for (auto listener : this->GetListeners()) {
      listener->ProcessUpdate(data);
    }

    // Debugging Output
    std::cout << "Updated Price: ProductId = " << productId
              << ", Mid = " << data.GetMid() << ", Spread = " << data.GetBidOfferSpread() << std::endl;
  }
}

void BondPricingService::Subscribe(BondPricesConnector *connector) {
  std::cout << "Subscribing BondPricesConnector..." << std::endl;
  connector->read();
}
#endif