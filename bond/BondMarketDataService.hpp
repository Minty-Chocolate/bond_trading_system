#ifndef BOND_MARKET_DATA_SERVICE_HPP
#define BOND_MARKET_DATA_SERVICE_HPP

#include <sstream>
#include "../base/products.hpp"
#include "../base/marketdataservice.hpp"
#include "IOFileConnector.hpp"

#include <iostream>
#include <sstream>

// ------------- Declaration: BondMarketDataConnector -------------

class BondMarketDataConnector : public InputFileConnector<std::string, OrderBook<Bond>> {
public:
  BondMarketDataConnector(const std::string &filePath, Service<std::string, OrderBook<Bond>> *connectedService);

private:
  void parse(std::string line) override;
};

// ------------- Declaration: BondMarketDataService -------------

class BondMarketDataService : public MarketDataService<Bond> {
public:
  BondMarketDataService();

  const BidOffer &GetBestBidOffer(const std::string &productId) override;
  const OrderBook<Bond> &AggregateDepth(const std::string &productId) override;

  void Subscribe(BondMarketDataConnector *connector);
  void OnMessage(OrderBook<Bond> &data) override;
};

// ------------- Definition: BondMarketDataConnector -------------

BondMarketDataConnector::BondMarketDataConnector(const std::string &filePath,
                                                 Service<std::string, OrderBook<Bond>> *connectedService)
    : InputFileConnector(filePath, connectedService) {}

void BondMarketDataConnector::parse(std::string line) {
  auto split = splitString(line, ',');
  std::string id = split[0];
  auto bond = BondProductService::GetInstance()->GetData(id);
  std::vector<Order> bidStack;
  std::vector<Order> offerStack;

  for (int i = 1; i <= 5; ++i) {
    Order bid(fractionalToDouble(split[2 * i - 1]), stol(split[2 * i]), PricingSide::BID);
    Order offer(fractionalToDouble(split[9 + 2 * i]), stol(split[10 + 2 * i]), PricingSide::OFFER);
    bidStack.push_back(bid);
    offerStack.push_back(offer);
  }

  auto book = OrderBook<Bond>(bond, bidStack, offerStack);

  // Print parsed data for debugging
  std::cout << "Parsed OrderBook: ProductId = " << bond.GetProductId() << std::endl;
  std::cout << "Top Bid: Price = " << bidStack[0].GetPrice() << ", Quantity = " << bidStack[0].GetQuantity() << std::endl;
  std::cout << "Top Offer: Price = " << offerStack[0].GetPrice() << ", Quantity = " << offerStack[0].GetQuantity() << std::endl;

  connectedService->OnMessage(book);
}

// ------------- Definition: BondMarketDataService -------------

BondMarketDataService::BondMarketDataService() {}

void BondMarketDataService::OnMessage(OrderBook<Bond> &data) {
  std::cout << "OnMessage: ProductId = " << data.GetProduct().GetProductId() << std::endl;

  if (dataStore.find(data.GetProduct().GetProductId()) == dataStore.end()) {
    dataStore.insert(std::make_pair(data.GetProduct().GetProductId(), data));
    for (auto listener : GetListeners()) {
      listener->ProcessAdd(data);
    }
    std::cout << "Processed Add for ProductId = " << data.GetProduct().GetProductId() << std::endl;
  } else {
    dataStore.insert(std::make_pair(data.GetProduct().GetProductId(), data));
    for (auto listener : GetListeners()) {
      listener->ProcessUpdate(data);
    }
    std::cout << "Processed Update for ProductId = " << data.GetProduct().GetProductId() << std::endl;
  }
}

void BondMarketDataService::Subscribe(BondMarketDataConnector *connector) {
  std::cout << "Subscribing BondMarketDataConnector..." << std::endl;
  connector->read();
}

const BidOffer &BondMarketDataService::GetBestBidOffer(const std::string &productId) {
  if (dataStore.find(productId) != dataStore.end()) {
    OrderBook<Bond> orderBook = dataStore.at(productId);
    auto *bidOffer = new BidOffer(
        Order(orderBook.GetBidStack()[0].GetPrice(), orderBook.GetBidStack()[0].GetQuantity(), PricingSide::BID),
        Order(orderBook.GetOfferStack()[0].GetPrice(), orderBook.GetOfferStack()[0].GetQuantity(), PricingSide::OFFER));

    // Debugging print
    std::cout << "Best BidOffer for ProductId = " << productId << ": Bid = " << bidOffer->GetBidOrder().GetPrice()
              << ", Offer = " << bidOffer->GetOfferOrder().GetPrice() << std::endl;

    return *bidOffer;
  }
  throw std::runtime_error("Product not found");
}

const OrderBook<Bond> &BondMarketDataService::AggregateDepth(const std::string &productId) {
  if (dataStore.find(productId) != dataStore.end()) {
    OrderBook<Bond> orderBook = dataStore.at(productId);
    double totalBidCost = 0.0;
    long totalBidVolume = 0;
    double totalOfferCost = 0.0;
    long totalOfferVolume = 0;

    for (int i = 0; i < 5; ++i) {
      totalBidVolume += orderBook.GetBidStack()[i].GetQuantity();
      totalBidCost += orderBook.GetBidStack()[i].GetQuantity() * orderBook.GetBidStack()[i].GetPrice();
      totalOfferVolume += orderBook.GetOfferStack()[i].GetQuantity();
      totalOfferCost += orderBook.GetOfferStack()[i].GetQuantity() * orderBook.GetOfferStack()[i].GetPrice();
    }

    double averageBidPrice = totalBidCost / totalBidVolume;
    double averageOfferPrice = totalOfferCost / totalOfferVolume;

    std::vector<Order> aggregatedBidStack({Order(averageBidPrice, totalBidVolume, PricingSide::BID)});
    std::vector<Order> aggregatedOfferStack({Order(averageOfferPrice, totalOfferVolume, PricingSide::OFFER)});
    auto *aggregateOrderBook =
        new OrderBook<Bond>(orderBook.GetProduct(), aggregatedBidStack, aggregatedOfferStack);

    // Debugging print
    std::cout << "AggregateDepth for ProductId = " << productId << ": AvgBid = " << averageBidPrice
              << ", AvgOffer = " << averageOfferPrice << std::endl;

    return *aggregateOrderBook;
  }
  throw std::runtime_error("Product not found");
}
#endif 