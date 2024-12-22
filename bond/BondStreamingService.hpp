#ifndef BOND_STREAMING_SERVICE_HPP
#define BOND_STREAMING_SERVICE_HPP

#include "BondAlgoStreamingService.hpp"
#include "../base/soa.hpp"
#include "../base/products.hpp"
#include "../base/streamingservice.hpp"
#include "../base/historicaldataservice.hpp"
#include "IOFileConnector.hpp"

// ------------- Declaration: BondStreamingService -------------

class BondStreamingService : public StreamingService<Bond> {
public:
  BondStreamingService();

  void OnMessage(PriceStream<Bond> &data) override;
  void PublishPrice(const PriceStream<Bond> &priceStream) override;
};

// ------------- Declaration: BondAlgoStreamServiceListener -------------

class BondAlgoStreamServiceListener : public ServiceListener<AlgoStream<Bond>> {
public:
  explicit BondAlgoStreamServiceListener(BondStreamingService *listeningService);

  void ProcessAdd(AlgoStream<Bond> &data) override;
  void ProcessRemove(AlgoStream<Bond> &data) override;
  void ProcessUpdate(AlgoStream<Bond> &data) override;

private:
  BondStreamingService *listeningService;
};

// ------------- Declaration: BondPriceStreamsConnector -------------

class BondPriceStreamsConnector : public OutputFileConnector<PriceStream<Bond>> {
public:
  explicit BondPriceStreamsConnector(const std::string &filePath);

private:
  std::string toString(PriceStream<Bond> &data) override;
};

// ------------- Declaration: BondPriceStreamsServiceListener -------------

class BondPriceStreamsServiceListener : public ServiceListener<PriceStream<Bond>> {
public:
  explicit BondPriceStreamsServiceListener(HistoricalDataService<PriceStream<Bond>> *listeningService);

  void ProcessAdd(PriceStream<Bond> &data) override;
  void ProcessRemove(PriceStream<Bond> &data) override;
  void ProcessUpdate(PriceStream<Bond> &data) override;

private:
  HistoricalDataService<PriceStream<Bond>> *listeningService;
};

// ------------- Declaration: BondPriceStreamsHistoricalDataService -------------

class BondPriceStreamsHistoricalDataService : public HistoricalDataService<PriceStream<Bond>> {
public:
  BondPriceStreamsHistoricalDataService();

  void PersistData(std::string persistKey, const PriceStream<Bond> &data) override;

private:
  void OnMessage(PriceStream<Bond> &data) override;
  BondPriceStreamsConnector *connector;
};

// ------------- Definition: BondStreamingService -------------

BondStreamingService::BondStreamingService() {}

void BondStreamingService::OnMessage(PriceStream<Bond> &data) {
  // No-op
}

void BondStreamingService::PublishPrice(const PriceStream<Bond> &priceStream) {
  std::string productId = priceStream.GetProduct().GetProductId();

  if (dataStore.find(productId) == dataStore.end()) {
    // Add new PriceStream to the data store
    dataStore.insert(std::make_pair(productId, priceStream));

    // Notify listeners about the new PriceStream
    for (auto listener : this->GetListeners()) {
      listener->ProcessAdd(const_cast<PriceStream<Bond> &>(priceStream));
    }

    // Debugging Output
    std::cout << "Added PriceStream: ProductId = " << productId
              << ", Bid Price = " << priceStream.GetBidOrder().GetPrice()
              << ", Offer Price = " << priceStream.GetOfferOrder().GetPrice() << std::endl;
  } else {
    // Update existing PriceStream in the data store
    dataStore.insert(std::make_pair(productId, priceStream));

    // Notify listeners about the updated PriceStream
    for (auto listener : this->GetListeners()) {
      listener->ProcessUpdate(const_cast<PriceStream<Bond> &>(priceStream));
    }

    // Debugging Output
    std::cout << "Updated PriceStream: ProductId = " << productId
              << ", Bid Price = " << priceStream.GetBidOrder().GetPrice()
              << ", Offer Price = " << priceStream.GetOfferOrder().GetPrice() << std::endl;
  }
}

// ------------- Definition: BondAlgoStreamServiceListener -------------

BondAlgoStreamServiceListener::BondAlgoStreamServiceListener(BondStreamingService *listeningService)
    : listeningService(listeningService) {}

void BondAlgoStreamServiceListener::ProcessAdd(AlgoStream<Bond> &data) {
  listeningService->PublishPrice(data.getPriceStream());
}

void BondAlgoStreamServiceListener::ProcessRemove(AlgoStream<Bond> &data) {
  // No-op
}

void BondAlgoStreamServiceListener::ProcessUpdate(AlgoStream<Bond> &data) {
  listeningService->PublishPrice(data.getPriceStream());
}

// ------------- Definition: BondPriceStreamsConnector -------------

BondPriceStreamsConnector::BondPriceStreamsConnector(const std::string &filePath)
    : OutputFileConnector(filePath) {}

std::string BondPriceStreamsConnector::toString(PriceStream<Bond> &data) {
  std::ostringstream oss;
  oss << boost::posix_time::microsec_clock::universal_time() << ","
      << data.GetProduct().GetProductId() << "," << data.GetBidOrder().GetPrice() << ","
      << data.GetBidOrder().GetVisibleQuantity() << "," << data.GetBidOrder().GetHiddenQuantity() << ","
      << data.GetOfferOrder().GetPrice() << "," << data.GetOfferOrder().GetVisibleQuantity() << ","
      << data.GetOfferOrder().GetHiddenQuantity();
  return oss.str();
}

// ------------- Definition: BondPriceStreamsServiceListener -------------

BondPriceStreamsServiceListener::BondPriceStreamsServiceListener(
    HistoricalDataService<PriceStream<Bond>> *listeningService)
    : listeningService(listeningService) {}

void BondPriceStreamsServiceListener::ProcessAdd(PriceStream<Bond> &data) {
  listeningService->PersistData(data.GetProduct().GetProductId(), data);
}

void BondPriceStreamsServiceListener::ProcessRemove(PriceStream<Bond> &data) {}

void BondPriceStreamsServiceListener::ProcessUpdate(PriceStream<Bond> &data) {
  listeningService->PersistData(data.GetProduct().GetProductId(), data);
}

// ------------- Definition: BondPriceStreamsHistoricalDataService -------------

BondPriceStreamsHistoricalDataService::BondPriceStreamsHistoricalDataService() {
  connector = new BondPriceStreamsConnector("output/streaming.txt");
}

void BondPriceStreamsHistoricalDataService::PersistData(std::string persistKey,
                                                        const PriceStream<Bond> &data) {
  connector->Publish(const_cast<PriceStream<Bond> &>(data));
}

void BondPriceStreamsHistoricalDataService::OnMessage(PriceStream<Bond> &data) {}

#endif