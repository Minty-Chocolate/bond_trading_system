#ifndef BOND_PRODUCT_SERVICE_HPP
#define BOND_PRODUCT_SERVICE_HPP


#include <map>
#include "../base/products.hpp"
#include "../base/soa.hpp"

// ------------- Declaration: BondProductService -------------

class BondProductService : public Service<string, Bond> {

 public:
  static BondProductService *GetInstance();

  Bond &GetData(string productId) override;
  void Add(Bond& bond);
  void OnMessage(Bond &data) override;

 private:
  unordered_map<string, Bond> bondMapper; 
  static BondProductService *instance;  

  BondProductService();
};

// ------------- Definition: BondProductService -------------

BondProductService *BondProductService::instance = nullptr;

BondProductService::BondProductService() {
  bondMapper = unordered_map<string, Bond>();
}

void BondProductService::OnMessage(Bond &data) {}

Bond &BondProductService::GetData(string productId) {
  return bondMapper[productId];
}

void BondProductService::Add(Bond& bond) {
  bondMapper.insert(make_pair(bond.GetProductId(), bond));
}

BondProductService *BondProductService::GetInstance() {
  if (instance == nullptr) {
    instance = new BondProductService();
  }
  return instance;
}



#endif 