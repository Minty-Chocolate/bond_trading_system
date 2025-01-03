/**
 * historicaldataservice.hpp
 * Defines the data types and Service for historical data.
 * 
 * @author Breman Thuraisingham
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP

#include <string>
#include "soa.hpp"
/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : Service<string, T> {

 public:

  virtual // Persist data to a store
  void PersistData(string persistKey, const T &data) = 0;

};

#endif
