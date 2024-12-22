#ifndef BOND_INQUIRY_SERVICE_HPP
#define BOND_INQUIRY_SERVICE_HPP

#include "../base/products.hpp"
#include "../base/inquiryservice.hpp"
#include "IOFileConnector.hpp"

// ------------- Declaration: BondInquirySubscriber -------------

class BondInquirySubscriber : public InputFileConnector<std::string, Inquiry<Bond>> {
public:
  BondInquirySubscriber(const std::string &filePath, Service<std::string, Inquiry<Bond>> *connectedService);

private:
  void parse(std::string line) override;
};

// ------------- Declaration: BondInquiryPublisher -------------

class BondInquiryPublisher : public OutputFileConnector<Inquiry<Bond>> {
public:
  explicit BondInquiryPublisher(const std::string &filePath);

  std::string toString(Inquiry<Bond> &data) override;
  void Publish(Inquiry<Bond> &data) override;
};

// ------------- Declaration: BondInquiryService -------------

class BondInquiryService : public InquiryService<Bond> {
public:
  BondInquiryService();

  void OnMessage(Inquiry<Bond> &data) override;
  void SendQuote(const std::string &inquiryId, double price) override;
  void Subscribe(BondInquirySubscriber *subscriber);

private:
  void NotifyListeners(Inquiry<Bond> &data);
  std::unique_ptr<BondInquiryPublisher> publishConnector;
};

// ------------- Declaration: BondInquiryServiceListener -------------

class BondInquiryServiceListener : public ServiceListener<Inquiry<Bond>> {
public:
  explicit BondInquiryServiceListener(BondInquiryService *listeningService);

  void ProcessAdd(Inquiry<Bond> &data) override;
  void ProcessRemove(Inquiry<Bond> &data) override;
  void ProcessUpdate(Inquiry<Bond> &data) override;

private:
  BondInquiryService *listeningService;
};

// ------------- Definition: BondInquirySubscriber -------------

BondInquirySubscriber::BondInquirySubscriber(const std::string &filePath,
                                             Service<std::string, Inquiry<Bond>> *connectedService)
    : InputFileConnector(filePath, connectedService) {}

void BondInquirySubscriber::parse(std::string line) {
  auto split = splitString(line, ',');
  auto bond = BondProductService::GetInstance()->GetData(split[0]);
  Inquiry<Bond> inquiry(split[1], bond, split[2] == "0" ? BUY : SELL, std::stol(split[3]), 0.0, InquiryState::RECEIVED);

  std::cout << "Parsed Inquiry: " << inquiry.GetInquiryId() << std::endl;
  connectedService->OnMessage(inquiry);
}

// ------------- Definition: BondInquiryPublisher -------------

BondInquiryPublisher::BondInquiryPublisher(const std::string &filePath) : OutputFileConnector(filePath) {}

std::string BondInquiryPublisher::toString(Inquiry<Bond> &data) {
  std::ostringstream oss;
  oss << boost::posix_time::microsec_clock::universal_time() << ","
      << data.GetInquiryId() << "," << data.GetProduct().GetProductId() << ","
      << data.GetSide() << "," << data.GetQuantity() << "," << data.GetPrice() << "," << data.GetState();
  return oss.str();
}

void BondInquiryPublisher::Publish(Inquiry<Bond> &data) {
  std::cout << "Publishing Inquiry to file: " << data.GetInquiryId() << std::endl;
  OutputFileConnector::Publish(data);
}

// ------------- Definition: BondInquiryService -------------

BondInquiryService::BondInquiryService() {
  publishConnector = std::make_unique<BondInquiryPublisher>("output/allinquires.txt");
}

void BondInquiryService::OnMessage(Inquiry<Bond> &data) {
  std::cout << "OnMessage: InquiryId=" << data.GetInquiryId() << ", State=" << data.GetState() << std::endl;

  dataStore.emplace(data.GetInquiryId(), data);

  if (data.GetState() == InquiryState::RECEIVED) {
    SendQuote(data.GetInquiryId(), 100.0);
  } else if (data.GetState() == InquiryState::QUOTED) {
    data.SetState(InquiryState::DONE);
    publishConnector->Publish(data);
    NotifyListeners(data);
  }
}

void BondInquiryService::SendQuote(const std::string &inquiryId, double price) {
  auto &data = dataStore.at(inquiryId);
  data.SetPrice(price);
  data.SetState(InquiryState::QUOTED);

  std::cout << "Sending Quote for InquiryId: " << inquiryId << ", Price: " << price << std::endl;

  for (auto listener : GetListeners()) {
    listener->ProcessAdd(data);
  }
}

void BondInquiryService::Subscribe(BondInquirySubscriber *subscriber) {
  subscriber->read();
}

void BondInquiryService::NotifyListeners(Inquiry<Bond> &data) {
  for (auto listener : GetListeners()) {
    listener->ProcessUpdate(data);
  }
}

// ------------- Definition: BondInquiryServiceListener -------------

BondInquiryServiceListener::BondInquiryServiceListener(BondInquiryService *listeningService)
    : listeningService(listeningService) {}

void BondInquiryServiceListener::ProcessAdd(Inquiry<Bond> &data) {
  data.SetState(InquiryState::QUOTED);
  listeningService->OnMessage(data);
}

void BondInquiryServiceListener::ProcessRemove(Inquiry<Bond> &data) {}

void BondInquiryServiceListener::ProcessUpdate(Inquiry<Bond> &data) {}
#endif