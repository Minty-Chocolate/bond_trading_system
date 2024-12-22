#ifndef BOND_ALGO_STREAMING_SERVICE_HPP
#define BOND_ALGO_STREAMING_SERVICE_HPP

#include "../base/pricingservice.hpp"
#include "../base/products.hpp"
#include "../base/soa.hpp"
#include "../base/streamingservice.hpp"


// ------------- Declaration: AlgoStream<T> -------------

template <typename T>
class AlgoStream {
public:
  explicit AlgoStream(const PriceStream<T> &priceStream);
  const PriceStream<T> &getPriceStream() const;

private:
  const PriceStream<T> &priceStream;
};

// ------------- Declaration: BondAlgoStreamingService -------------

class BondAlgoStreamingService : public Service<std::string, AlgoStream<Bond>> {
public:
  BondAlgoStreamingService();

  void PublishPrice(Price<Bond> &newPrice);
  void OnMessage(AlgoStream<Bond> &data) override;

private:
  std::vector<int> vis_volumes{1000000, 2000000};
  int cur_ptr = 0;
};

// ------------- Declaration: BondPricesServiceListener -------------

class BondPricesServiceListener : public ServiceListener<Price<Bond>> {
public:
  explicit BondPricesServiceListener(BondAlgoStreamingService *listeningService);

  void ProcessAdd(Price<Bond> &data) override;
  void ProcessRemove(Price<Bond> &data) override;
  void ProcessUpdate(Price<Bond> &data) override;

private:
  BondAlgoStreamingService *listeningService;
};

// ------------- Definition: AlgoStream<T> -------------

template <typename T>
AlgoStream<T>::AlgoStream(const PriceStream<T> &priceStream)
    : priceStream(priceStream) {}

template <typename T>
const PriceStream<T> &AlgoStream<T>::getPriceStream() const {
  return priceStream;
}

// ------------- Definition: BondAlgoStreamingService -------------

BondAlgoStreamingService::BondAlgoStreamingService() {}

void BondAlgoStreamingService::PublishPrice(Price<Bond> &newPrice) {
  auto bond = BondProductService::GetInstance()->GetData(newPrice.GetProduct().GetProductId());

  PriceStreamOrder bidOrder(newPrice.GetMid() - newPrice.GetBidOfferSpread() / 2,
                            vis_volumes[cur_ptr],
                            2 * vis_volumes[cur_ptr],
                            PricingSide::BID);
  PriceStreamOrder offerOrder(newPrice.GetMid() + newPrice.GetBidOfferSpread() / 2,
                              vis_volumes[cur_ptr],
                              2 * vis_volumes[cur_ptr],
                              PricingSide::OFFER);
  PriceStream<Bond> priceStream(bond, bidOrder, offerOrder);
  AlgoStream<Bond> algoStream(priceStream);

  cur_ptr = (cur_ptr + 1) % vis_volumes.size();
  dataStore.insert(std::make_pair(bond.GetProductId(), algoStream));
  if (dataStore.find(bond.GetProductId()) == dataStore.end()) {
    for (auto listener : GetListeners()) {
      listener->ProcessAdd(algoStream);
    }
  } else {
    for (auto listener : GetListeners()) {
      listener->ProcessUpdate(algoStream);
    }
  }
}

void BondAlgoStreamingService::OnMessage(AlgoStream<Bond> &data) {}

// ------------- Definition: BondPricesServiceListener -------------

BondPricesServiceListener::BondPricesServiceListener(BondAlgoStreamingService *listeningService)
    : listeningService(listeningService) {}

void BondPricesServiceListener::ProcessAdd(Price<Bond> &data) {
  listeningService->PublishPrice(data);
}

void BondPricesServiceListener::ProcessRemove(Price<Bond> &data) {}

void BondPricesServiceListener::ProcessUpdate(Price<Bond> &data) {
  listeningService->PublishPrice(data);
}
#endif